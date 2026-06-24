param(
  [string] $RepoRoot = ".",
  [string] $BackendRoot = "D:\VSProjects\moqi-ime",
  [switch] $Strict,
  [ValidateSet("", "RejectedUatBehavior", "VmEvidenceMissing")]
  [string] $ExpectRed = ""
)

$ErrorActionPreference = "Stop"

function Resolve-FullPath([string] $Path) {
  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Path))
}

function Assert-True([bool] $Condition, [string] $Message) {
  if (-not $Condition) {
    throw $Message
  }
}

function Assert-File([string] $Path, [string] $Message) {
  Assert-True (Test-Path -LiteralPath $Path -PathType Leaf) $Message
}

function Assert-Directory([string] $Path, [string] $Message) {
  Assert-True (Test-Path -LiteralPath $Path -PathType Container) $Message
}

function Get-JsonFile([string] $Path) {
  Assert-File $Path "Missing JSON evidence file: $Path"
  return Get-Content -Raw -Encoding UTF8 -LiteralPath $Path | ConvertFrom-Json
}

function Invoke-Guard([string] $ScriptName, [string[]] $Arguments) {
  $scriptPath = Join-Path $repo "scripts/$ScriptName"
  Assert-File $scriptPath "Missing Phase 5 guard script: $scriptPath"
  & pwsh -NoProfile -ExecutionPolicy Bypass -File $scriptPath @Arguments
  if ($LASTEXITCODE -ne 0) {
    throw "$ScriptName failed with exit code $LASTEXITCODE"
  }
}

function Add-Violation([System.Collections.Generic.List[string]] $Violations, [string] $Message) {
  $Violations.Add($Message)
}

function Test-EvidenceSlotPresent([object] $Slot) {
  if ($null -eq $Slot) {
    return $false
  }
  $status = [string] $Slot.status
  if ($status -notin @("captured", "verified", "passed", "recorded")) {
    return $false
  }
  if ($Slot.PSObject.Properties.Name -contains "path") {
    $path = [string] $Slot.path
    if ([string]::IsNullOrWhiteSpace($path)) {
      return $false
    }
    $fullPath = if ([System.IO.Path]::IsPathRooted($path)) {
      [System.IO.Path]::GetFullPath($path)
    } else {
      [System.IO.Path]::GetFullPath((Join-Path $repo $path))
    }
    if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
      return $false
    }
  }
  if ($Slot.PSObject.Properties.Name -contains "notes") {
    if ([string]::IsNullOrWhiteSpace([string] $Slot.notes)) {
      return $false
    }
  }
  return $true
}

function Assert-SlotsPresent([object] $Object, [string[]] $RequiredNames, [string] $Label) {
  $missing = @()
  foreach ($name in $RequiredNames) {
    if (-not (Test-EvidenceSlotPresent $Object.$name)) {
      $missing += $name
    }
  }
  Assert-True ($missing.Count -eq 0) "$Label evidence slots missing or incomplete: $($missing -join ', ')"
}

function Get-MissingSlots([object] $Object, [string[]] $RequiredNames) {
  $missing = @()
  foreach ($name in $RequiredNames) {
    if (-not (Test-EvidenceSlotPresent $Object.$name)) {
      $missing += $name
    }
  }
  return $missing
}

$repo = Resolve-FullPath $RepoRoot
$backend = Resolve-FullPath $BackendRoot
$manifestPath = Join-Path $repo ".planning/product/ui-fixtures/phase-05/phase05-ui-evidence.json"
$screenshotsDir = Join-Path $repo ".planning/product/ui-fixtures/phase-05/screenshots"
$manualNotesPath = Join-Path $repo ".planning/product/ui-fixtures/phase-05/manual-uat-notes.md"
$sourceMetadataPath = Join-Path $repo ".planning/product/web-alpha-fixtures/2026-06-23/source-metadata.json"
$runtimeProvenancePath = Join-Path $repo ".planning/product/ui-fixtures/phase-05/runtime-provenance.json"

Assert-Directory $repo "Repo root is missing: $repo"
Assert-Directory $backend "Backend root is missing: $backend"
Assert-File $sourceMetadataPath "Missing TypeDuck Web alpha source metadata: $sourceMetadataPath"
Assert-Directory $screenshotsDir "Missing Phase 5 screenshots directory: $screenshotsDir"

if ($Strict) {
  Invoke-Guard -ScriptName "Test-TypeDuckAppearanceTheme.ps1" -Arguments @("-RepoRoot", $repo, "-BackendRoot", $backend, "-Strict")
  Invoke-Guard -ScriptName "Test-TypeDuckCandidateData.ps1" -Arguments @("-RepoRoot", $repo, "-Strict")
  Invoke-Guard -ScriptName "Test-TypeDuckCandidateWindow.ps1" -Arguments @("-RepoRoot", $repo, "-Strict")
  Invoke-Guard -ScriptName "Test-TypeDuckSettingsPersistence.ps1" -Arguments @("-RepoRoot", $repo, "-BackendRoot", $backend, "-Strict")
  if ($ExpectRed -eq "RejectedUatBehavior") {
    Invoke-Guard -ScriptName "Test-TypeDuckSettingsAboutUi.ps1" -Arguments @("-RepoRoot", $repo, "-BackendRoot", $backend, "-Strict", "-ExpectRed", "RejectedUatBehavior")
    Invoke-Guard -ScriptName "Test-TypeDuckIconPackaging.ps1" -Arguments @("-RepoRoot", $repo, "-BackendRoot", $backend, "-Strict", "-ExpectRed", "RejectedUatBehavior")
  } else {
    Invoke-Guard -ScriptName "Test-TypeDuckSettingsAboutUi.ps1" -Arguments @("-RepoRoot", $repo, "-BackendRoot", $backend, "-Strict")
    Invoke-Guard -ScriptName "Test-TypeDuckIconPackaging.ps1" -Arguments @("-RepoRoot", $repo, "-BackendRoot", $backend, "-Strict")
  }
}

$sourceMetadata = Get-JsonFile $sourceMetadataPath
Assert-True ($sourceMetadata.captureDate -eq "2026-06-23") "Aggregate evidence must use the 2026-06-23 TypeDuck Web alpha fixture."
Assert-True ($sourceMetadata.localSource.path -eq "I:\GitHub\TypeDuck-Web") "Aggregate evidence must reference the local TypeDuck-Web source path."
Assert-True ($sourceMetadata.localSource.commit -eq "db21054") "Aggregate evidence must stay tied to the recorded TypeDuck-Web fixture commit."
Assert-True ($sourceMetadata.runtimeObservation.dictionaryHoverBehavior -match "requires mouse movement") "Fixture must record movement-triggered dictionary reveal."

if (Test-Path -LiteralPath $sourceMetadata.localSource.path -PathType Container) {
  $currentSourceCommit = (& git -C $sourceMetadata.localSource.path rev-parse --short HEAD).Trim()
  Assert-True ($currentSourceCommit -eq $sourceMetadata.localSource.commit) "Local TypeDuck-Web source commit changed from fixture commit $($sourceMetadata.localSource.commit) to $currentSourceCommit."
}

$manifest = Get-JsonFile $manifestPath
$staleEvidence = [System.Collections.Generic.List[string]]::new()
foreach ($staleName in @("settings-two-column-layout.bmp", "vm-about-dialog.bmp")) {
  $stalePath = Join-Path $screenshotsDir $staleName
  if (Test-Path -LiteralPath $stalePath -PathType Leaf) {
    Add-Violation $staleEvidence "Stale screenshot remains active: $staleName"
  }
  if ((Get-Content -Raw -Encoding UTF8 -LiteralPath $manifestPath) -match [regex]::Escape($staleName)) {
    Add-Violation $staleEvidence "Evidence manifest still references stale screenshot: $staleName"
  }
}
foreach ($staleSlotName in @("settingsTwoColumn", "aboutDialog")) {
  if ((Get-Content -Raw -Encoding UTF8 -LiteralPath $manifestPath) -match "`"$staleSlotName`"\s*:") {
    Add-Violation $staleEvidence "Evidence manifest still exposes stale slot name: $staleSlotName"
  }
}
if ($ExpectRed -eq "RejectedUatBehavior") {
  Write-Host "PASS RED: RejectedUatBehavior aggregate guard proved rejected child guards are active."
  exit 0
}

Assert-True ($staleEvidence.Count -eq 0) "Stale Phase 5 evidence is still accepted: $($staleEvidence -join '; ')"
if ((Test-Path -LiteralPath $runtimeProvenancePath -PathType Leaf) -or ($ExpectRed -ne "VmEvidenceMissing")) {
  Assert-File $runtimeProvenancePath "Missing runtime/Web provenance record: $runtimeProvenancePath"
  $runtimeProvenance = Get-JsonFile $runtimeProvenancePath
  Assert-True ($runtimeProvenance.schemaVersion -ge 1) "Runtime provenance must declare schemaVersion >= 1."
  Assert-True ($runtimeProvenance.repositories.windows.path -eq $repo) "Runtime provenance must record this Windows repo path."
  Assert-True ($runtimeProvenance.repositories.backend.path -eq $backend) "Runtime provenance must record the sibling backend path."
  Assert-True ($runtimeProvenance.repositories.typeDuckWeb.path -eq "I:\GitHub\TypeDuck-Web") "Runtime provenance must record the TypeDuck-Web path."
  Assert-True ($runtimeProvenance.webAlphaFixture.sourceMetadata -eq ".planning/product/web-alpha-fixtures/2026-06-23/source-metadata.json") "Runtime provenance must link the Web alpha fixture metadata."
  Assert-True ($runtimeProvenance.lookupFilter.commit -eq "3671814d4e4aeab8d616ceea3c7f6d88e96bba02") "Runtime provenance must include the Phase 2 lookup-filter commit."
  if (Test-Path -LiteralPath $runtimeProvenance.repositories.backend.path -PathType Container) {
    $currentBackendHead = (& git -C $runtimeProvenance.repositories.backend.path rev-parse HEAD).Trim()
    Assert-True ($currentBackendHead -eq $runtimeProvenance.repositories.backend.head) "Runtime provenance backend HEAD is stale: $($runtimeProvenance.repositories.backend.head) vs $currentBackendHead."
  }
  if (Test-Path -LiteralPath $runtimeProvenance.repositories.typeDuckWeb.path -PathType Container) {
    $currentWebHead = (& git -C $runtimeProvenance.repositories.typeDuckWeb.path rev-parse HEAD).Trim()
    Assert-True ($currentWebHead -eq $runtimeProvenance.repositories.typeDuckWeb.head) "Runtime provenance TypeDuck-Web HEAD is stale: $($runtimeProvenance.repositories.typeDuckWeb.head) vs $currentWebHead."
  }
  if ($runtimeProvenance.PSObject.Properties.Name -contains "artifacts") {
    foreach ($artifact in $runtimeProvenance.artifacts) {
      if ($artifact.PSObject.Properties.Name -contains "path" -and $artifact.PSObject.Properties.Name -contains "sha256") {
        $artifactPath = [string] $artifact.path
        if (-not [System.IO.Path]::IsPathRooted($artifactPath)) {
          $artifactPath = Join-Path $repo $artifactPath
        }
        Assert-File $artifactPath "Runtime provenance artifact is missing: $($artifact.path)"
        $actualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $artifactPath).Hash
        Assert-True ($actualHash -eq $artifact.sha256) "Runtime provenance artifact hash is stale for $($artifact.path): expected $($artifact.sha256), got $actualHash."
      }
    }
  }
}
Assert-True ($manifest.schemaVersion -ge 1) "Evidence manifest must declare schemaVersion >= 1."
Assert-True ($manifest.fixture.sourceMetadata -eq ".planning/product/web-alpha-fixtures/2026-06-23/source-metadata.json") "Evidence manifest must link the Web alpha source metadata."
Assert-True ($manifest.fixture.localTypeDuckWebSource -eq "I:\GitHub\TypeDuck-Web") "Evidence manifest must link local TypeDuck-Web source authority."
Assert-True ($manifest.interaction.dictionaryReveal.trigger -eq "pointer-movement") "Candidate dictionary reveal must be movement-triggered per D-04."
Assert-True ($manifest.interaction.dictionaryReveal.stationaryPointerNoFlicker -eq $true) "Manifest must record no dictionary flicker for stationary pointer typing."
Assert-True ($manifest.settings.deployFailureEvidence.status -in @("recorded", "verified")) "Settings deploy/reconfigure failure evidence is required per D-47."
Assert-True ($manifest.about.identity.status -in @("recorded", "verified")) "About identity/version/engine-schema evidence is required for SET-10."
Assert-True ($manifest.package.legacyMoqiAssetScan.status -in @("passed", "verified")) "Legacy Moqi image exclusion evidence is required for D-31."

$requiredVmSlots = @(
  "installerFirstRunSettings",
  "postInstallSettingsEntryPoint",
  "settingsApplyPersistence",
  "settingsPersistenceAfterRestart",
  "separateAboutExecutable",
  "notepadCandidateNei",
  "browserCandidateHousam",
  "movementReveal",
  "stationaryPointerNoFlicker",
  "highDpiPlacement",
  "multiMonitorPlacement",
  "uiLessHost",
  "imperfectCompositionRectangle",
  "inputPickerAndExecutableIcons"
)

$missingVmSlots = @(Get-MissingSlots $manifest.vmEvidence $requiredVmSlots)
if ($ExpectRed -eq "VmEvidenceMissing") {
  Assert-True ($missingVmSlots.Count -gt 0) "Expected RED VmEvidenceMissing, but all required VM evidence slots are already present."
  Write-Host "PASS RED: VmEvidenceMissing proved required VM/manual evidence is absent: $($missingVmSlots -join ', ')"
  exit 0
}

$requiredPreviewScreenshots = @(
  "candidateBaseline",
  "candidateMultilingual",
  "dictionaryDetail",
  "compoundHousam",
  "reverseLookupCangjie"
)
$requiredPackageSlots = @(
  "installer",
  "stagedSettingsExecutable",
  "aboutAssets",
  "aboutVersionSource",
  "engineSchemaAttribution",
  "themeFiles",
  "iconAssignments"
)

Assert-SlotsPresent $manifest.automated.previewScreenshots $requiredPreviewScreenshots "Automated preview screenshot"
Assert-SlotsPresent $manifest.package $requiredPackageSlots "Package"

Assert-SlotsPresent $manifest.vmEvidence $requiredVmSlots "VM/manual"
Assert-File $manualNotesPath "Missing manual UAT notes: $manualNotesPath"

Write-Host "[PASS] TypeDuck Phase 5 aggregate UI/package/VM evidence guard passed."
