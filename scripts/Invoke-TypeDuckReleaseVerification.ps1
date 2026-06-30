#Requires -Version 5.1
<#
.SYNOPSIS
  Creates the aggregate TypeDuck Phase 7 release verification evidence packet.
#>
param(
  [string] $RepoRoot = ".",
  [string] $EvidenceRoot = ".planning\product\release-fixtures\phase-07",
  [string] $BackendRoot = "",
  [string] $BuildDir = "build-vs32",
  [string] $InstallerPath = "installer\dist\typeduck-windows-ime-setup.exe"
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

function ConvertTo-RelativePath {
  param(
    [string] $Root,
    [string] $Path
  )

  $rootFull = [System.IO.Path]::GetFullPath($Root).TrimEnd('\') + '\'
  $pathFull = [System.IO.Path]::GetFullPath($Path)
  if ($pathFull.StartsWith($rootFull, [System.StringComparison]::OrdinalIgnoreCase)) {
    return $pathFull.Substring($rootFull.Length)
  }
  return $pathFull
}

function New-UtcStamp {
  return [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ", [Globalization.CultureInfo]::InvariantCulture)
}

function Invoke-ReleaseCommand {
  param(
    [string] $Id,
    [string[]] $ArgumentList,
    [string] $ArtifactPath
  )

  $started = New-UtcStamp
  $exitCode = 0
  $status = "passed"
  try {
    $commandOutput = & pwsh @ArgumentList 2>&1
    foreach ($line in @($commandOutput)) {
      if (-not [string]::IsNullOrWhiteSpace([string] $line)) {
        Write-Host $line
      }
    }
    if ($null -ne $LASTEXITCODE) {
      $exitCode = [int] $LASTEXITCODE
    }
    if ($exitCode -ne 0) {
      $status = "failed"
    }
  } catch {
    $exitCode = 1
    $status = "failed"
  }
  $completed = New-UtcStamp
  [pscustomobject] @{
    id = $Id
    command = "pwsh " + ($ArgumentList -join " ")
    status = $status
    exit_code = $exitCode
    started_at_utc = $started
    completed_at_utc = $completed
    artifact_path = $ArtifactPath
  }
}

function Read-JsonIfPresent {
  param([string] $Path)

  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    return $null
  }
  return Get-Content -Raw -Encoding UTF8 -LiteralPath $Path | ConvertFrom-Json
}

$root = Resolve-FullPath -BasePath (Get-Location).Path -Path $RepoRoot
if ([string]::IsNullOrWhiteSpace($BackendRoot)) {
  $BackendRoot = Resolve-FullPath -BasePath $root -Path "..\moqi-ime"
} else {
  $BackendRoot = Resolve-FullPath -BasePath $root -Path $BackendRoot
}
$evidenceFull = Resolve-FullPath -BasePath $root -Path $EvidenceRoot
New-Item -ItemType Directory -Force -Path $evidenceFull | Out-Null

$manifestPath = Join-Path $evidenceFull "release-verification.json"
$notesPath = Join-Path $evidenceFull "verification-notes.md"
$protocolResultsRel = Join-Path $EvidenceRoot "protocol-recovery-results.json"
$protocolResultsFull = Resolve-FullPath -BasePath $root -Path $protocolResultsRel
$installerFull = Resolve-FullPath -BasePath $root -Path $InstallerPath

$commands = @()
$commands += Invoke-ReleaseCommand -Id "phase6-privacy-security-cleanup" -ArtifactPath ".planning\product\privacy-security\phase-06-guard-results.json" -ArgumentList @(
  "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckPrivacySecurityCleanup.ps1",
  "-RepoRoot", ".", "-BackendRoot", $BackendRoot, "-Strict"
)
$commands += Invoke-ReleaseCommand -Id "phase6-backend-diagnostics" -ArtifactPath ".planning\product\privacy-security\phase-06-guard-results.json" -ArgumentList @(
  "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckBackendDiagnostics.ps1",
  "-RepoRoot", ".", "-BackendRoot", $BackendRoot, "-Strict"
)
$commands += Invoke-ReleaseCommand -Id "release-install-evidence" -ArtifactPath (Join-Path $EvidenceRoot "install-verification-template.json") -ArgumentList @(
  "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckReleaseInstallEvidence.ps1",
  "-RepoRoot", ".", "-Strict"
)
$commands += Invoke-ReleaseCommand -Id "protocol-recovery-probe" -ArtifactPath $protocolResultsRel -ArgumentList @(
  "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Invoke-TypeDuckProtocolRecoveryProbe.ps1",
  "-RepoRoot", ".", "-BuildDir", $BuildDir, "-EvidencePath", $protocolResultsRel, "-Strict"
)
$commands += Invoke-ReleaseCommand -Id "protocol-recovery-guard" -ArtifactPath (Join-Path $EvidenceRoot "protocol-recovery-cases.json") -ArgumentList @(
  "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckProtocolRecovery.ps1",
  "-RepoRoot", ".", "-Strict"
)
$commands += Invoke-ReleaseCommand -Id "interactive-checklist-guard" -ArtifactPath (Join-Path $EvidenceRoot "host-app-dpi-notes.md") -ArgumentList @(
  "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckInteractiveReleaseChecklist.ps1",
  "-RepoRoot", ".", "-Strict"
)
$commands += Invoke-ReleaseCommand -Id "release-artifact-guard" -ArtifactPath $InstallerPath -ArgumentList @(
  "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", "scripts\Test-TypeDuckReleaseArtifacts.ps1",
  "-RepoRoot", ".", "-InstallerPath", $InstallerPath, "-Strict"
)

$installerName = "typeduck-windows-ime-setup.exe"
$installerHash = ""
$installerSize = 0
$installerGenerated = $null
if (Test-Path -LiteralPath $installerFull -PathType Leaf) {
  $installerItem = Get-Item -LiteralPath $installerFull
  $installerName = $installerItem.Name
  $installerSize = [int64] $installerItem.Length
  $installerGenerated = $installerItem.LastWriteTimeUtc.ToString("yyyy-MM-ddTHH:mm:ssZ", [Globalization.CultureInfo]::InvariantCulture)
  $installerHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $installerFull).Hash
}

$protocolCases = Read-JsonIfPresent (Join-Path $evidenceFull "protocol-recovery-cases.json")
$protocolResults = Read-JsonIfPresent $protocolResultsFull
$protocolCaseIds = @()
if ($null -ne $protocolCases) {
  $protocolCaseIds = @($protocolCases.cases | ForEach-Object { $_.case_id })
}
$protocolExecutedResults = @()
if ($null -ne $protocolResults) {
  $protocolExecutedResults = @($protocolResults.results | ForEach-Object {
      [pscustomobject] @{
        case_id = $_.case_id
        executed = $_.executed
        status = $_.status
        recoverable = $_.recoverable
        observed_error_code = $_.observed_error_code
        observed_health = $_.observed_health
      }
    })
}

$manifestStatus = "automated-ready-human-pending"
if (@($commands | Where-Object { $_.status -ne "passed" }).Count -gt 0) {
  $manifestStatus = "failed"
}

$manifest = [pscustomobject] @{
  schema = "typeduck-release-verification-v1"
  phase = "07-compatibility-and-release-verification"
  generated_at_utc = New-UtcStamp
  status = $manifestStatus
  requirements_covered = @("VER-03", "VER-04", "VER-05", "VER-06")
  installer = [pscustomobject] @{
    status = "manual-vm-pending"
    artifact_path = ConvertTo-RelativePath -Root $root -Path $installerFull
    case_ids = @("clean-install", "reinstall-upgrade", "uninstall-cleanup", "reboot-required-registration", "bitness-win32-x64", "artifact-name-and-sha256")
    template_path = Join-Path $EvidenceRoot "install-verification-template.json"
    notes_path = Join-Path $EvidenceRoot "install-verification-notes.md"
    no_host_install = $true
  }
  guard_commands = @($commands)
  automated_checks = @($commands | ForEach-Object {
      [pscustomobject] @{
        id = $_.id
        status = $_.status
        exit_code = $_.exit_code
        artifact_path = $_.artifact_path
      }
    })
  vm_evidence = [pscustomobject] @{
    status = "manual-vm-pending"
    install_lifecycle = "pending-human-guest-run"
    install_notes_path = Join-Path $EvidenceRoot "install-verification-notes.md"
    host_app_dpi_notes_path = Join-Path $EvidenceRoot "host-app-dpi-notes.md"
    screenshot_capture_required = $false
  }
  interactive_human_checks = [pscustomobject] @{
    status = "pending-human"
    direct_human_judgement = $true
    no_screenshots_required = $true
    checklist_path = Join-Path $EvidenceRoot "interactive-vm-checklist.md"
    notes_path = Join-Path $EvidenceRoot "host-app-dpi-notes.md"
  }
  protocol_recovery = [pscustomobject] @{
    status = "executed"
    cases_path = Join-Path $EvidenceRoot "protocol-recovery-cases.json"
    results_path = $protocolResultsRel
    case_ids = $protocolCaseIds
    executed_results = $protocolExecutedResults
  }
  host_apps = [pscustomobject] @{
    status = "pending-human"
    targets = @("Notepad", "Browser text field", "Office or Office-like app", "Terminal/console context", "Elevated app", "Awkward TSF host available in VM")
    notes_path = Join-Path $EvidenceRoot "host-app-dpi-notes.md"
  }
  dpi = [pscustomobject] @{
    status = "pending-human"
    scales = @("100%", "140% if available", "175%", "200%")
    notes_path = Join-Path $EvidenceRoot "host-app-dpi-notes.md"
    multi_monitor_disposition = "Phase 5 already human verified multi-monitor behavior; Phase 7 does not require new multi-monitor screenshot automation."
  }
  release_artifacts = [pscustomobject] @{
    status = "verified"
    installer_name = $installerName
    installer_path = ConvertTo-RelativePath -Root $root -Path $installerFull
    sha256 = $installerHash
    byte_size = $installerSize
    generated_at_utc = $installerGenerated
    hash_command = "Get-FileHash -Algorithm SHA256 -LiteralPath $InstallerPath"
    case_ids = @("artifact-name-and-sha256")
    workflow_frontend_repo = "TypeDuck-Windows"
    workflow_backend_repo = "TypeDuck-Windows-backend"
    schema_artifact_url = "https://github.com/TypeDuck-HK/schema/releases/download/latest/TypeDuck-Windows.zip"
    schema_artifact_extract_path = "TypeDuck-HK-schema"
    installer_release_asset_pattern = 'typeduck-windows-ime-setup-${{ github.event.release.tag_name || github.sha }}.exe'
    standalone_schema_artifact = $false
  }
  limitations = [pscustomobject] @{
    interactive_vm = "Host-app and DPI judgement remains the final human-controlled step."
    screenshot_boundary = "No screenshot files, manifests, capture scripts, or automated image comparison are required."
    research = "Excluded; phase planning used skip-research because existing GSD documentation was sufficient."
  }
  rerun_after_repairs = [pscustomobject] @{
    rule = "If a release-blocking product bug is found, route the smallest focused repair, rerun the affected verification, then rerun scripts\Invoke-TypeDuckReleaseVerification.ps1 and scripts\Test-TypeDuckReleaseVerification.ps1."
    product_change_boundary = "Phase 7 adds tests, scripts, documentation, and evidence only unless a release-blocking failure is detected."
  }
}

$manifest | ConvertTo-Json -Depth 12 | Set-Content -Encoding UTF8 -LiteralPath $manifestPath

$notes = @(
  "# TypeDuck Phase 7 Release Verification Notes",
  "",
  "Generated UTC: $($manifest.generated_at_utc)",
  "Status: $($manifest.status)",
  "",
  "## Installer Artifact",
  "",
  "- Path: $($manifest.release_artifacts.installer_path)",
  "- SHA-256: $($manifest.release_artifacts.sha256)",
  "- Byte size: $($manifest.release_artifacts.byte_size)",
  "",
  "## Automated Evidence",
  "",
  ($commands | ForEach-Object { "- $($_.id): $($_.status) ($($_.artifact_path))" }),
  "",
  "## Interactive VM Evidence",
  "",
  "- Install lifecycle notes: $($manifest.installer.notes_path)",
  "- Host-app and DPI notes: $($manifest.host_apps.notes_path)",
  "- Current state: pending human judgement in the guest VM.",
  "- Screenshot capture: not required.",
  "",
  "## Source Audit",
  "",
  "- ROADMAP Phase 7 goal: covered by install evidence, protocol recovery, artifact guard, and pending human host-app/DPI notes.",
  "- VER-03: covered by the install evidence schema and VM checklist.",
  "- VER-04: pending final human host-app/DPI judgement.",
  "- VER-05: covered by protocol recovery tests and executed probe results.",
  "- VER-06: covered by TypeDuck installer artifact name, hash, workflow, and schema-source guards.",
  "- D-01 through D-05: release evidence excludes screenshot automation and keeps DPI human judged.",
  "- D-06 through D-10: command, log, hash, registry/file, and human-note evidence are represented.",
  "- D-11 through D-13: protocol/recovery cases are non-visual, redacted, and failure detail stays in logs/evidence.",
  "- D-14 through D-16: no product capability changes are made by this aggregate verification.",
  "- D-17 through D-20: workflows use TypeDuck-Windows, TypeDuck-Windows-backend, the TypeDuck schema release artifact, and no standalone schema upload artifact.",
  "- Research: excluded because existing GSD documentation was sufficient and skip-research was requested before planning.",
  "",
  "## Rerun Rule",
  "",
  $manifest.rerun_after_repairs.rule
)
$notes | Set-Content -Encoding UTF8 -LiteralPath $notesPath

Write-Host "Wrote $manifestPath"
Write-Host "Wrote $notesPath"

if ($manifestStatus -ne "failed") {
  exit 0
}
exit 1
