#Requires -Version 5.1
<#
.SYNOPSIS
  Validates Phase 7 TypeDuck protocol recovery matrix and evidence.
#>
param(
  [string] $RepoRoot = ".",
  [string] $CasesPath = ".planning\product\release-fixtures\phase-07\protocol-recovery-cases.json",
  [string] $ResultsPath = ".planning\product\release-fixtures\phase-07\protocol-recovery-results.json",
  [switch] $Strict
)

$ErrorActionPreference = "Stop"

function Resolve-GuardPath {
  param(
    [string] $BasePath,
    [string] $Path
  )
  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path $BasePath $Path))
}

function Read-JsonRequired {
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

function Test-ContainsForbiddenEvidenceField {
  param([object] $Value)

  if ($null -eq $Value) {
    return $false
  }
  if ($Value -is [System.Array]) {
    foreach ($item in $Value) {
      if (Test-ContainsForbiddenEvidenceField -Value $item) {
        return $true
      }
    }
    return $false
  }
  if ($Value -is [psobject]) {
    foreach ($property in $Value.PSObject.Properties) {
      if ($property.Name -match '(?i)screenshot|previewScreenshot|screenshotManifest|rawTypedContent|typed_content|typedContent') {
        return $true
      }
      if (Test-ContainsForbiddenEvidenceField -Value $property.Value) {
        return $true
      }
    }
  }
  return $false
}

function Assert-CoverageReference {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [object] $Case,
    [string[]] $RequiredPatterns
  )
  $joined = (@($Case.coverage_refs) + @($Case.command) + @($Case.evidence)) -join "`n"
  foreach ($pattern in $RequiredPatterns) {
    if ($joined -notmatch [regex]::Escape($pattern)) {
      Add-Failure $Failures "Case '$($Case.case_id)' must cite $pattern."
    }
  }
}

$root = Resolve-GuardPath -BasePath (Get-Location).Path -Path $RepoRoot
$casesFull = Resolve-GuardPath -BasePath $root -Path $CasesPath
$resultsFull = Resolve-GuardPath -BasePath $root -Path $ResultsPath
$protocolTestPath = Join-Path $root "Tests\TypeDuckProtocol\ProtocolRecovery_test.cpp"
$protocolCMakePath = Join-Path $root "Tests\TypeDuckProtocol\CMakeLists.txt"

$casesDoc = Read-JsonRequired -Path $casesFull -Label "protocol recovery cases"
$resultsDoc = Read-JsonRequired -Path $resultsFull -Label "protocol recovery results"
$failures = [System.Collections.Generic.List[string]]::new()

$requiredCaseIds = @(
  "normal-cantonese-frames",
  "raw-lookup-payload",
  "reverse-lookup-if-supported",
  "malformed-frame",
  "oversized-frame",
  "invalid-protobuf",
  "backend-timeout",
  "backend-restart-crash",
  "stale-mismatched-sequence",
  "settings-update-redeploy-failure",
  "bounded-degraded-state"
)

$actualCases = @($casesDoc.cases)
$actualCaseIds = @($actualCases | ForEach-Object { $_.case_id })
foreach ($caseId in $requiredCaseIds) {
  if ($actualCaseIds -notcontains $caseId) {
    Add-Failure $failures "Missing required protocol recovery case '$caseId'."
  }
}

$liveCaseIds = @(
  "backend-timeout",
  "backend-restart-crash",
  "settings-update-redeploy-failure",
  "bounded-degraded-state"
)
foreach ($caseId in $liveCaseIds) {
  $result = @($resultsDoc.results | Where-Object { $_.case_id -eq $caseId } | Select-Object -First 1)[0]
  if (-not $result) {
    Add-Failure $failures "Missing live executed result for '$caseId'."
    continue
  }
  if ($result.executed -ne $true) {
    Add-Failure $failures "Live result '$caseId' must have executed=true."
  }
  if ([string]::IsNullOrWhiteSpace([string] $result.status)) {
    Add-Failure $failures "Live result '$caseId' must have a concrete status."
  }
  if ($Strict -and $result.redacted -ne $true) {
    Add-Failure $failures "Live result '$caseId' must record redacted=true."
  }
}

$unitCaseIds = @(
  "normal-cantonese-frames",
  "raw-lookup-payload",
  "malformed-frame",
  "oversized-frame",
  "invalid-protobuf",
  "stale-mismatched-sequence"
)
foreach ($caseId in $unitCaseIds) {
  $case = @($actualCases | Where-Object { $_.case_id -eq $caseId } | Select-Object -First 1)[0]
  if (-not $case) {
    continue
  }
  Assert-CoverageReference $failures $case @("ProtocolRecovery_test")
}

foreach ($caseId in @("malformed-frame", "oversized-frame")) {
  $case = @($actualCases | Where-Object { $_.case_id -eq $caseId } | Select-Object -First 1)[0]
  if ($case) {
    Assert-CoverageReference $failures $case @("Proto::FrameBuffer", "launcher degraded response")
  }
}

$invalidCase = @($actualCases | Where-Object { $_.case_id -eq "invalid-protobuf" } | Select-Object -First 1)[0]
if ($invalidCase) {
  Assert-CoverageReference $failures $invalidCase @("parsePayload")
}

$reverseCase = @($actualCases | Where-Object { $_.case_id -eq "reverse-lookup-if-supported" } | Select-Object -First 1)[0]
if ($reverseCase) {
  $state = [string] $reverseCase.conditional_state
  if ($state -notin @("supported", "unsupported", "skipped-with-reason")) {
    Add-Failure $failures "reverse-lookup-if-supported must state supported, unsupported, or skipped-with-reason."
  }
  if (($state -eq "unsupported" -or $state -eq "skipped-with-reason") -and
      [string]::IsNullOrWhiteSpace([string] $reverseCase.reason)) {
    Add-Failure $failures "reverse-lookup-if-supported must include a reason when not supported."
  }
}

if (-not (Test-Path -LiteralPath $protocolTestPath -PathType Leaf)) {
  Add-Failure $failures "ProtocolRecovery_test.cpp is missing."
} else {
  $protocolTest = Get-Content -Raw -LiteralPath $protocolTestPath
  foreach ($pattern in @(
      "ValidCantoneseFrameRoundTripsWithCandidates",
      "RawLookupSeparatorsSurviveServerResponseCandidatePage",
      "InvalidProtobufPayloadClearsMessages",
      "MalformedAndOversizedFramesClearBufferedBytes",
      "StaleAndMismatchedSequencesAreIdentifiable")) {
    if ($protocolTest -notmatch $pattern) {
      Add-Failure $failures "ProtocolRecovery_test.cpp missing test pattern $pattern."
    }
  }
}

if (-not (Test-Path -LiteralPath $protocolCMakePath -PathType Leaf) -or
    (Get-Content -Raw -LiteralPath $protocolCMakePath) -notmatch "ProtocolRecovery_test") {
  Add-Failure $failures "Tests/TypeDuckProtocol/CMakeLists.txt must wire ProtocolRecovery_test."
}

if (Test-ContainsForbiddenEvidenceField -Value $casesDoc) {
  Add-Failure $failures "Case matrix must not contain screenshot or raw typed content evidence fields."
}
if (Test-ContainsForbiddenEvidenceField -Value $resultsDoc) {
  Add-Failure $failures "Result evidence must not contain screenshot or raw typed content evidence fields."
}

if ($casesDoc.failure_policy.routine_user_facing_technical_detail -ne $false) {
  Add-Failure $failures "Failure policy must keep routine user-facing technical detail disabled."
}
if ($casesDoc.failure_policy.technical_detail_location -notmatch "logs/evidence") {
  Add-Failure $failures "Failure policy must route technical detail to logs/evidence."
}
if ($Strict -and $casesDoc.requirements_covered -notcontains "VER-05") {
  Add-Failure $failures "Strict mode requires VER-05 in requirements_covered."
}

if ($failures.Count -gt 0) {
  Write-Error ("TypeDuck protocol recovery guard failed:`n - " + ($failures -join "`n - "))
}

Write-Host "TypeDuck protocol recovery guard passed."
exit 0
