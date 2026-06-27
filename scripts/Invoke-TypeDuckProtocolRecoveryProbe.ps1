#Requires -Version 5.1
<#
.SYNOPSIS
  Builds and runs the focused TypeDuck protocol recovery probe.
#>
param(
  [string] $RepoRoot = ".",
  [string] $BuildDir = "build-vs32",
  [string] $EvidencePath = ".planning\product\release-fixtures\phase-07\protocol-recovery-results.json",
  [switch] $SkipBuild,
  [switch] $Strict
)

$ErrorActionPreference = "Stop"

function Resolve-ProbePath {
  param(
    [string] $BasePath,
    [string] $Path
  )
  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path $BasePath $Path))
}

function Add-Failure {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [string] $Message
  )
  $Failures.Add($Message) | Out-Null
}

function Assert-RecoveryResults {
  param(
    [string] $Path,
    [switch] $Strict
  )

  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    throw "Protocol recovery evidence was not written: $Path"
  }

  $evidence = Get-Content -Raw -LiteralPath $Path | ConvertFrom-Json
  $failures = [System.Collections.Generic.List[string]]::new()
  $requiredCases = @(
    "backend-timeout",
    "backend-restart-crash",
    "settings-update-redeploy-failure",
    "bounded-degraded-state"
  )

  foreach ($caseId in $requiredCases) {
    $result = @($evidence.results | Where-Object { $_.case_id -eq $caseId } | Select-Object -First 1)[0]
    if (-not $result) {
      Add-Failure $failures "Missing executed result for $caseId."
      continue
    }
    if ($result.executed -ne $true) {
      Add-Failure $failures "$caseId must have executed=true."
    }
    if ([string]::IsNullOrWhiteSpace([string] $result.status)) {
      Add-Failure $failures "$caseId must record a concrete status."
    }
    if ($result.PSObject.Properties.Name -match "screenshot|previewScreenshot|screenshotManifest|rawTypedContent|typed_content|typedContent") {
      Add-Failure $failures "$caseId evidence must not contain screenshot or raw typed content fields."
    }
    if ($Strict -and $result.redacted -ne $true) {
      Add-Failure $failures "$caseId must set redacted=true in strict mode."
    }
  }

  if ($Strict -and $evidence.screenshot_automation -ne $false) {
    Add-Failure $failures "Recovery evidence must explicitly record screenshot_automation=false."
  }

  if ($failures.Count -gt 0) {
    throw ("Protocol recovery probe evidence failed validation:`n - " + ($failures -join "`n - "))
  }
}

$root = Resolve-ProbePath -BasePath (Get-Location).Path -Path $RepoRoot
$buildRoot = Resolve-ProbePath -BasePath $root -Path $BuildDir
$evidenceFull = Resolve-ProbePath -BasePath $root -Path $EvidencePath
$logFull = Resolve-ProbePath -BasePath $root -Path ".planning\product\release-fixtures\phase-07\protocol-recovery-probe.ndjson"

if (-not (Test-Path -LiteralPath $root -PathType Container)) {
  throw "RepoRoot missing: $root"
}

if (-not $SkipBuild) {
  & cmake --build $buildRoot --config Debug --target TypeduckBackendProbe -- /m:1
  if ($LASTEXITCODE -ne 0) {
    throw "TypeduckBackendProbe build failed with exit code $LASTEXITCODE."
  }
}

$probeExe = Join-Path $buildRoot "Tools\TypeduckBackendProbe\Debug\TypeduckBackendProbe.exe"
if (-not (Test-Path -LiteralPath $probeExe -PathType Leaf)) {
  throw "TypeduckBackendProbe executable missing: $probeExe"
}

New-Item -ItemType Directory -Path (Split-Path -Parent $evidenceFull) -Force | Out-Null

& $probeExe --mode protocol-recovery --output $evidenceFull --recovery-log $logFull
if ($LASTEXITCODE -ne 0) {
  throw "TypeduckBackendProbe protocol-recovery mode failed with exit code $LASTEXITCODE."
}

Assert-RecoveryResults -Path $evidenceFull -Strict:$Strict
Write-Host "TypeDuck protocol recovery probe evidence written to $EvidencePath"
