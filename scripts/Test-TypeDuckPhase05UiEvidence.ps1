param(
  [string] $RepoRoot = ".",
  [string] $BackendRoot = "D:\VSProjects\moqi-ime",
  [switch] $Strict,
  [ValidateSet("", "VmEvidenceMissing")]
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

Assert-Directory $repo "Repo root is missing: $repo"
Assert-Directory $backend "Backend root is missing: $backend"
Assert-File $sourceMetadataPath "Missing TypeDuck Web alpha source metadata: $sourceMetadataPath"
Assert-Directory $screenshotsDir "Missing Phase 5 screenshots directory: $screenshotsDir"

if ($Strict) {
  Invoke-Guard -ScriptName "Test-TypeDuckAppearanceTheme.ps1" -Arguments @("-RepoRoot", $repo, "-BackendRoot", $backend, "-Strict")
  Invoke-Guard -ScriptName "Test-TypeDuckCandidateData.ps1" -Arguments @("-RepoRoot", $repo, "-Strict")
  Invoke-Guard -ScriptName "Test-TypeDuckCandidateWindow.ps1" -Arguments @("-RepoRoot", $repo, "-Strict")
  Invoke-Guard -ScriptName "Test-TypeDuckSettingsPersistence.ps1" -Arguments @("-RepoRoot", $repo, "-BackendRoot", $backend, "-Strict")
  Invoke-Guard -ScriptName "Test-TypeDuckSettingsAboutUi.ps1" -Arguments @("-RepoRoot", $repo, "-BackendRoot", $backend, "-Strict")
  Invoke-Guard -ScriptName "Test-TypeDuckIconPackaging.ps1" -Arguments @("-RepoRoot", $repo, "-BackendRoot", $backend, "-Strict")
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
  "settingsPersistenceAfterRestart",
  "aboutDialog",
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
  "reverseLookupCangjie",
  "settingsTwoColumn",
  "aboutDialog"
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
