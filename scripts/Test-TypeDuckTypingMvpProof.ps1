#Requires -Version 5.1
<#
.SYNOPSIS
  Validate Phase 4 TypeDuck typing MVP proof evidence.
#>
param(
  [string] $RepoRoot = "",
  [string] $EvidencePath = ".planning\product\protocol-fixtures\phase-04\typing-mvp-evidence.json",
  [string] $GoldenCasesPath = ".planning\product\protocol-fixtures\phase-04\golden-typing-cases.json",
  [switch] $Strict
)

$ErrorActionPreference = "Stop"

function Resolve-RepoRoot {
  param([string] $RequestedRoot)
  if ([string]::IsNullOrWhiteSpace($RequestedRoot)) {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
  }
  return [System.IO.Path]::GetFullPath($RequestedRoot)
}

function Resolve-ProofPath {
  param(
    [string] $BasePath,
    [string] $Path
  )
  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path $BasePath $Path))
}

function Read-JsonFile {
  param(
    [string] $Path,
    [string] $Label
  )
  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    throw "$Label missing: $Path"
  }
  return Get-Content -Raw -LiteralPath $Path | ConvertFrom-Json
}

function Add-Failure {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [string] $Message
  )
  $Failures.Add($Message) | Out-Null
}

function Assert-CommandSucceeded {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [object] $CommandEvidence,
    [string] $Id,
    [string] $Root
  )
  if (-not $CommandEvidence) {
    Add-Failure $Failures "Missing guard command evidence: $Id"
    return
  }
  if ([string]::IsNullOrWhiteSpace([string] $CommandEvidence.command)) {
    Add-Failure $Failures "Guard command '$Id' is missing command text."
  }
  if ([string]::IsNullOrWhiteSpace([string] $CommandEvidence.timestamp)) {
    Add-Failure $Failures "Guard command '$Id' is missing timestamp."
  }
  if ($CommandEvidence.status -ne "passed") {
    Add-Failure $Failures "Guard command '$Id' must be passed."
  }
  if (-not [string]::IsNullOrWhiteSpace([string] $CommandEvidence.artifact_path)) {
    $artifactPath = Resolve-ProofPath -BasePath $Root -Path ([string] $CommandEvidence.artifact_path)
    if (-not (Test-Path -LiteralPath $artifactPath -PathType Leaf)) {
      Add-Failure $Failures "Guard command '$Id' artifact_path does not exist: $($CommandEvidence.artifact_path)"
    }
  }
}

function Assert-EvidenceItem {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [object] $Item,
    [string] $Label,
    [string] $Root,
    [bool] $RequirePass = $true
  )
  if (-not $Item) {
    Add-Failure $Failures "Missing evidence item: $Label"
    return
  }
  foreach ($field in @("id", "status", "command", "timestamp", "artifact_path", "observed_outcome")) {
    if ([string]::IsNullOrWhiteSpace([string] $Item.$field)) {
      Add-Failure $Failures "Evidence '$Label' is missing required field '$field'."
    }
  }
  if ($RequirePass -and $Item.status -ne "passed") {
    Add-Failure $Failures "Evidence '$Label' must be passed."
  }
  if (-not [string]::IsNullOrWhiteSpace([string] $Item.artifact_path)) {
    $artifactPath = Resolve-ProofPath -BasePath $Root -Path ([string] $Item.artifact_path)
    if (-not (Test-Path -LiteralPath $artifactPath -PathType Leaf)) {
      Add-Failure $Failures "Evidence '$Label' artifact_path does not exist: $($Item.artifact_path)"
    }
  }
}

function Assert-NoDictionaryParsingEvidence {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [object] $Value
  )
  $json = $Value | ConvertTo-Json -Depth 30 -Compress
  if ($json -match "(?i)dictionary_ui|parsed_dictionary_rows|native_dictionary_view|structured_dictionary") {
    Add-Failure $Failures "Phase 4 proof must not use parsed/native dictionary UI evidence."
  }
}

function Assert-RawLookupEvidence {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [object] $Item,
    [string] $Root
  )
  if (-not $Item) {
    return
  }
  $artifactPath = Resolve-ProofPath -BasePath $Root -Path ([string] $Item.artifact_path)
  if (-not (Test-Path -LiteralPath $artifactPath -PathType Leaf)) {
    Add-Failure $Failures "Raw lookup artifact is missing: $($Item.artifact_path)"
    return
  }
  $artifact = Read-JsonFile -Path $artifactPath -Label "raw lookup artifact"
  if ($artifact.artifact_type -ne "raw-lookup-observation") {
    Add-Failure $Failures "Raw lookup artifact must have artifact_type raw-lookup-observation."
  }
  $json = $artifact | ConvertTo-Json -Depth 20 -Compress
  foreach ($needle in @("\\u000b", "\\u000c", "\\r")) {
    if ($json -notmatch [regex]::Escape($needle)) {
      Add-Failure $Failures "Raw lookup evidence must include escaped separator $needle."
    }
  }
  if ([string]::IsNullOrWhiteSpace([string] $artifact.details.phase2_source.sha256)) {
    Add-Failure $Failures "Raw lookup evidence must include SHA-256 from Phase 2 raw output."
  }
  $phase2RawPath = Resolve-ProofPath -BasePath $Root -Path ([string] $artifact.details.phase2_source.artifact)
  if (-not (Test-Path -LiteralPath $phase2RawPath -PathType Leaf)) {
    Add-Failure $Failures "Raw lookup Phase 2 source artifact is missing: $($artifact.details.phase2_source.artifact)"
  } else {
    $matchingSha = $false
    foreach ($line in Get-Content -LiteralPath $phase2RawPath) {
      if ([string]::IsNullOrWhiteSpace($line)) {
        continue
      }
      $record = $line | ConvertFrom-Json
      if ($record.candidateText -eq $artifact.details.candidate_text -and $record.sha256 -eq $artifact.details.phase2_source.sha256) {
        $matchingSha = $true
        break
      }
    }
    if (-not $matchingSha) {
      Add-Failure $Failures "Raw lookup SHA does not match Phase 2 raw output for candidate '$($artifact.details.candidate_text)'."
    }
  }
}

function Get-CaseIds {
  param([object] $Cases)
  return @($Cases | ForEach-Object { [string] $_.id })
}

function Invoke-Guard {
  param(
    [string] $Root,
    [string] $RelativeScript,
    [string[]] $Arguments
  )
  $scriptPath = Join-Path $Root $RelativeScript
  if (-not (Test-Path -LiteralPath $scriptPath -PathType Leaf)) {
    throw "Required guard script missing: $RelativeScript"
  }
  & pwsh -NoProfile -ExecutionPolicy Bypass -File $scriptPath @Arguments
  if ($LASTEXITCODE -ne 0) {
    throw "Guard failed: $RelativeScript"
  }
}

$root = Resolve-RepoRoot -RequestedRoot $RepoRoot
$evidenceFull = Resolve-ProofPath -BasePath $root -Path $EvidencePath
$goldenFull = Resolve-ProofPath -BasePath $root -Path $GoldenCasesPath
$launcherFull = Join-Path $root ".planning\product\protocol-fixtures\phase-04\launcher-recovery.json"
$typingClientFull = Join-Path $root ".planning\product\protocol-fixtures\phase-04\typing-client.json"
$checklistFull = Join-Path $root ".planning\product\protocol-fixtures\phase-04\windows-smoke-checklist.md"

Invoke-Guard -Root $root -RelativeScript "scripts\Test-TypeDuckProtocolContract.ps1" -Arguments @("-RepoRoot", $root, "-Strict")
Invoke-Guard -Root $root -RelativeScript "scripts\Test-TypeDuckLauncherProtocol.ps1" -Arguments @("-RepoRoot", $root, "-Strict")
Invoke-Guard -Root $root -RelativeScript "scripts\Test-TypeDuckTypingClient.ps1" -Arguments @("-RepoRoot", $root, "-Strict")

$failures = [System.Collections.Generic.List[string]]::new()
$golden = Read-JsonFile -Path $goldenFull -Label "golden typing cases"
$launcher = Read-JsonFile -Path $launcherFull -Label "launcher recovery fixture"
$typingClient = Read-JsonFile -Path $typingClientFull -Label "typing client fixture"
if (-not (Test-Path -LiteralPath $checklistFull -PathType Leaf)) {
  Add-Failure $failures "Windows smoke checklist missing: $checklistFull"
}

if ($golden.phase -ne "04-typeduck-protocol-and-typing-mvp" -or $golden.plan -ne "04-04") {
  Add-Failure $failures "golden-typing-cases.json must identify Phase 4 Plan 04."
}

$requiredGoldenIds = @($golden.required_case_ids | ForEach-Object { [string] $_ })
foreach ($required in @(
  "common-cantonese-input",
  "raw-lookup-payload",
  "reverse-lookup-marker",
  "cangjie-key-forwarding",
  "malformed-frame",
  "oversized-frame",
  "backend-timeout",
  "backend-restart"
)) {
  if ($requiredGoldenIds -notcontains $required) {
    Add-Failure $failures "Golden case list is missing required case '$required'."
  }
}

if (-not (Test-Path -LiteralPath $evidenceFull -PathType Leaf)) {
  Add-Failure $failures "typing MVP evidence missing: $evidenceFull"
} else {
  $evidence = Read-JsonFile -Path $evidenceFull -Label "typing MVP evidence"
  if ($evidence.phase -ne "04-typeduck-protocol-and-typing-mvp" -or $evidence.plan -ne "04-04") {
    Add-Failure $failures "typing-mvp-evidence.json must identify Phase 4 Plan 04."
  }

  $guardMap = @{}
  foreach ($guard in @($evidence.guard_commands)) {
    $guardMap[[string] $guard.id] = $guard
  }
  foreach ($guardId in @("protocol-contract", "launcher-protocol", "typing-client", "engine-proof-check", "lookup-payload")) {
    Assert-CommandSucceeded -Failures $failures -CommandEvidence $guardMap[$guardId] -Id $guardId -Root $root
  }
  if ($Strict) {
    foreach ($guardId in @("build-win32-launcher", "build-win32-textservice", "build-protoframing-test", "run-protoframing-test")) {
      Assert-CommandSucceeded -Failures $failures -CommandEvidence $guardMap[$guardId] -Id $guardId -Root $root
    }
  }

  $goldenMap = @{}
  foreach ($case in @($evidence.golden_cases)) {
    $goldenMap[[string] $case.id] = $case
  }
  foreach ($caseId in $requiredGoldenIds) {
    Assert-EvidenceItem -Failures $failures -Item $goldenMap[$caseId] -Label "golden:$caseId" -Root $root
  }
  Assert-RawLookupEvidence -Failures $failures -Item $goldenMap["raw-lookup-payload"] -Root $root
  Assert-NoDictionaryParsingEvidence -Failures $failures -Value $evidence

  $obligationMap = @{}
  foreach ($obligation in @($evidence.imported_obligations)) {
    $key = "$($obligation.source):$($obligation.id)"
    $obligationMap[$key] = $obligation
  }
  foreach ($case in @($launcher.runtime_cases)) {
    Assert-EvidenceItem -Failures $failures -Item $obligationMap["launcher-recovery:$($case.id)"] -Label "launcher-recovery:$($case.id)" -Root $root -RequirePass:$Strict
  }
  foreach ($case in @($typingClient.runtime_cases)) {
    $item = $obligationMap["typing-client:$($case.id)"]
    $vmRequired = $false
    if ($item -and $item.details) {
      $vmRequired = $item.details.vm_required_for_host_process_observation -eq $true
    }
    Assert-EvidenceItem -Failures $failures -Item $item -Label "typing-client:$($case.id)" -Root $root -RequirePass:($Strict -or -not $vmRequired)
  }

  if ($Strict) {
    if (-not $evidence.windows_smoke) {
      Add-Failure $failures "Strict proof evidence must include windows_smoke section."
    } else {
      if ($evidence.windows_smoke.status -ne "passed") {
        Add-Failure $failures "Strict proof requires windows_smoke.status to be passed."
      }
      if ([string]::IsNullOrWhiteSpace([string] $evidence.windows_smoke.artifact_path)) {
        Add-Failure $failures "windows_smoke must include artifact_path."
      } else {
        $smokePath = Resolve-ProofPath -BasePath $root -Path ([string] $evidence.windows_smoke.artifact_path)
        if (-not (Test-Path -LiteralPath $smokePath -PathType Leaf)) {
          Add-Failure $failures "windows_smoke artifact_path does not exist: $($evidence.windows_smoke.artifact_path)"
        }
      }
      foreach ($field in @("composition", "candidates", "paging", "numeric_selection", "commit", "bounded_recovery")) {
        if ($evidence.windows_smoke.observations.$field -ne $true) {
          Add-Failure $failures "Strict proof requires windows_smoke.observations.$field = true."
        }
      }
    }
  }
}

if ($failures.Count -gt 0) {
  Write-Error ("TypeDuck typing MVP proof check failed:`n - " + ($failures -join "`n - "))
}

Write-Host "TypeDuck typing MVP proof check passed."
exit 0
