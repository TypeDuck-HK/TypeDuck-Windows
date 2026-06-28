#Requires -Version 5.1
<#
.SYNOPSIS
  Validates the aggregate TypeDuck Phase 7 release verification manifest.
#>
param(
  [string] $RepoRoot = ".",
  [string] $EvidenceRoot = ".planning\product\release-fixtures\phase-07",
  [string] $ManifestPath = "",
  [switch] $Strict
)

$ErrorActionPreference = "Stop"

function Resolve-FullPath {
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

function Read-JsonRequired {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [string] $Path,
    [string] $Label
  )

  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    Add-Failure $Failures "$Label missing: $Path"
    return $null
  }
  try {
    return Get-Content -Raw -Encoding UTF8 -LiteralPath $Path | ConvertFrom-Json
  } catch {
    Add-Failure $Failures "$Label is not valid JSON: $($_.Exception.Message)"
    return $null
  }
}

function Assert-Property {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [object] $Object,
    [string] $Name,
    [string] $Context
  )

  if ($null -eq $Object -or $null -eq $Object.PSObject.Properties[$Name]) {
    Add-Failure $Failures "$Context missing required section/property '$Name'."
  }
}

function Test-ForbiddenScreenshotRequirement {
  param([object] $Value)

  if ($null -eq $Value) {
    return $false
  }
  if ($Value -is [System.Array]) {
    foreach ($item in $Value) {
      if (Test-ForbiddenScreenshotRequirement $item) { return $true }
    }
    return $false
  }
  if ($Value -is [psobject] -and -not ($Value -is [string])) {
    foreach ($property in $Value.PSObject.Properties) {
      if ($property.Name -match '(?i)screenshot(File|Path|Manifest|Capture|Comparison)|requiredScreenshot|screenshotRequired') {
        return $true
      }
      if (Test-ForbiddenScreenshotRequirement $property.Value) { return $true }
    }
    return $false
  }
  $text = [string] $Value
  return ($text -match '(?i)capture\s+screenshot|save\s+screenshot|screenshot\s+manifest|screenshot\s+comparison|image\s+diff')
}

function Assert-ContainsAll {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [object[]] $Actual,
    [string[]] $Expected,
    [string] $Context
  )

  $actualValues = @($Actual | ForEach-Object { [string] $_ })
  foreach ($expectedValue in $Expected) {
    if ($actualValues -notcontains $expectedValue) {
      Add-Failure $Failures "$Context missing '$expectedValue'."
    }
  }
}

$root = Resolve-FullPath -BasePath (Get-Location).Path -Path $RepoRoot
$evidenceFull = Resolve-FullPath -BasePath $root -Path $EvidenceRoot
if ([string]::IsNullOrWhiteSpace($ManifestPath)) {
  $ManifestPath = Join-Path $evidenceFull "release-verification.json"
}
$manifestFull = Resolve-FullPath -BasePath $root -Path $ManifestPath
$failures = [System.Collections.Generic.List[string]]::new()

$manifest = Read-JsonRequired $failures $manifestFull "release verification manifest"
if ($null -eq $manifest) {
  throw "TypeDuck release verification guard failed:`n - $($failures -join "`n - ")"
}

foreach ($section in @(
    "installer",
    "guard_commands",
    "automated_checks",
    "vm_evidence",
    "interactive_human_checks",
    "protocol_recovery",
    "host_apps",
    "dpi",
    "release_artifacts",
    "limitations",
    "rerun_after_repairs"
  )) {
  Assert-Property $failures $manifest $section "release verification manifest"
}

if (Test-ForbiddenScreenshotRequirement $manifest) {
  Add-Failure $failures "Release manifest must not require screenshot capture, screenshot manifests, screenshot comparisons, or image diffs."
}

$requiredInstallerCases = @(
  "clean-install",
  "reinstall-upgrade",
  "uninstall-cleanup",
  "reboot-required-registration",
  "bitness-win32-x64",
  "artifact-name-and-sha256"
)
Assert-ContainsAll $failures @($manifest.installer.case_ids) $requiredInstallerCases "installer.case_ids"

$requiredProtocolCases = @(
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
Assert-ContainsAll $failures @($manifest.protocol_recovery.case_ids) $requiredProtocolCases "protocol_recovery.case_ids"
Assert-ContainsAll $failures @($manifest.release_artifacts.case_ids) @("artifact-name-and-sha256") "release_artifacts.case_ids"

$resultsPath = [string] $manifest.protocol_recovery.results_path
if ([string]::IsNullOrWhiteSpace($resultsPath)) {
  Add-Failure $failures "protocol_recovery.results_path is required."
} else {
  $resultsFull = Resolve-FullPath -BasePath $root -Path $resultsPath
  $resultsDoc = Read-JsonRequired $failures $resultsFull "protocol recovery executed results"
  if ($null -ne $resultsDoc) {
    foreach ($caseId in @("backend-timeout", "backend-restart-crash", "settings-update-redeploy-failure", "bounded-degraded-state")) {
      $result = @($resultsDoc.results | Where-Object { $_.case_id -eq $caseId } | Select-Object -First 1)[0]
      if (-not $result) {
        Add-Failure $failures "Executed protocol result '$caseId' is missing from protocol-recovery-results.json."
        continue
      }
      if ($result.executed -ne $true) {
        Add-Failure $failures "Executed protocol result '$caseId' must have executed=true."
      }
      if ([string]::IsNullOrWhiteSpace([string] $result.status)) {
        Add-Failure $failures "Executed protocol result '$caseId' must include status."
      }
    }
  }
}

foreach ($command in @($manifest.guard_commands)) {
  foreach ($field in @("id", "command", "status", "started_at_utc", "completed_at_utc", "artifact_path")) {
    if ([string]::IsNullOrWhiteSpace([string] $command.$field)) {
      Add-Failure $failures "guard_commands entry missing required field '$field'."
    }
  }
}

if ($manifest.release_artifacts.installer_name -notmatch '^typeduck-windows-ime-setup(\-.+)?\.exe$') {
  Add-Failure $failures "release_artifacts.installer_name must be typeduck-windows-ime-setup.exe or the tag-or-sha release asset variant."
}
if ([string] $manifest.release_artifacts.sha256 -notmatch '^[0-9A-Fa-f]{64}$') {
  Add-Failure $failures "release_artifacts.sha256 must be a SHA-256 hex digest."
}
if ([int64] $manifest.release_artifacts.byte_size -le 0) {
  Add-Failure $failures "release_artifacts.byte_size must be greater than zero."
}
if ($manifest.release_artifacts.workflow_frontend_repo -ne "TypeDuck-Windows") {
  Add-Failure $failures "release_artifacts.workflow_frontend_repo must be TypeDuck-Windows."
}
if ($manifest.release_artifacts.workflow_backend_repo -ne "TypeDuck-Windows-backend") {
  Add-Failure $failures "release_artifacts.workflow_backend_repo must be TypeDuck-Windows-backend."
}
if ($manifest.release_artifacts.schema_repository -ne '${{ github.repository_owner }}/schema') {
  Add-Failure $failures "release_artifacts.schema_repository must be `${{ github.repository_owner }}/schema."
}
if ($manifest.release_artifacts.schema_branch -ne "aap2-alpha") {
  Add-Failure $failures "release_artifacts.schema_branch must be aap2-alpha."
}
if ($manifest.release_artifacts.schema_prune_list -ne "scripts\typeduck-schema-prune-list.txt") {
  Add-Failure $failures "release_artifacts.schema_prune_list must reference scripts\typeduck-schema-prune-list.txt."
}
if ($manifest.release_artifacts.installer_release_asset_pattern -ne 'typeduck-windows-ime-setup-${{ github.event.release.tag_name || github.sha }}.exe') {
  Add-Failure $failures "release_artifacts.installer_release_asset_pattern must use the release tag or commit SHA."
}
if ($manifest.release_artifacts.standalone_schema_artifact -ne $false) {
  Add-Failure $failures "release_artifacts.standalone_schema_artifact must be false."
}

Assert-ContainsAll $failures @($manifest.host_apps.targets) @(
  "Notepad",
  "Browser text field",
  "Office or Office-like app",
  "Terminal/console context",
  "Elevated app",
  "Awkward TSF host available in VM"
) "host_apps.targets"
Assert-ContainsAll $failures @($manifest.dpi.scales) @("100%", "140% if available", "175%", "200%") "dpi.scales"

if ($Strict) {
  if ($manifest.interactive_human_checks.status -notin @("pending-human", "passed", "failed", "complete")) {
    Add-Failure $failures "Strict mode requires interactive_human_checks.status to be pending-human, passed, failed, or complete."
  }
  if ([string]::IsNullOrWhiteSpace([string] $manifest.rerun_after_repairs.rule)) {
    Add-Failure $failures "Strict mode requires rerun_after_repairs.rule."
  }
  Assert-ContainsAll $failures @($manifest.requirements_covered) @("VER-03", "VER-04", "VER-05", "VER-06") "requirements_covered"
}

if ($failures.Count -gt 0) {
  Write-Error ("TypeDuck release verification guard failed:`n - " + ($failures -join "`n - "))
}

Write-Host "TypeDuck release verification guard passed."
exit 0
