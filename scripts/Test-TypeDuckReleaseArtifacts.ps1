#Requires -Version 5.1
<#
.SYNOPSIS
  Validates TypeDuck Phase 7 release artifact and workflow evidence.
#>
param(
  [string] $RepoRoot = ".",
  [string] $InstallerPath = "installer\dist\typeduck-windows-ime-setup.exe",
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

function Read-RequiredText {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [string] $Path,
    [string] $Label
  )

  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    Add-Failure $Failures "Missing required file: $Label ($Path)."
    return ""
  }
  return [System.IO.File]::ReadAllText($Path, [System.Text.Encoding]::UTF8)
}

function Assert-Text {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [string] $Text,
    [string] $Pattern,
    [string] $Message
  )

  if ($Text -notmatch $Pattern) {
    Add-Failure $Failures $Message
  }
}

function Assert-NoText {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [string] $Text,
    [string] $Pattern,
    [string] $Message
  )

  if ($Text -match $Pattern) {
    Add-Failure $Failures $Message
  }
}

function Get-UploadArtifactBlocks {
  param([string] $WorkflowText)

  $matches = [regex]::Matches($WorkflowText, '(?ms)^\s*-\s+name:\s+Upload[^\r\n]*\r?\n(?:(?!^\s*-\s+name:).*\r?\n?)*')
  $blocks = @()
  foreach ($match in $matches) {
    if ($match.Value -match 'actions/upload-artifact') {
      $blocks += $match.Value
    }
  }
  return $blocks
}

$root = Resolve-FullPath -BasePath (Get-Location).Path -Path $RepoRoot
$failures = [System.Collections.Generic.List[string]]::new()

$releasePath = Join-Path $root ".github\workflows\release.yml"
$nightlyPath = Join-Path $root ".github\workflows\nightly.yml"
$packagePath = Join-Path $root "scripts\_all_in_package.ps1"
$installerBuildPath = Join-Path $root "installer\build-installer.ps1"

$release = Read-RequiredText $failures $releasePath ".github/workflows/release.yml"
$nightly = Read-RequiredText $failures $nightlyPath ".github/workflows/nightly.yml"
$package = Read-RequiredText $failures $packagePath "scripts/_all_in_package.ps1"
$installerBuild = Read-RequiredText $failures $installerBuildPath "installer/build-installer.ps1"
$workflows = $release + "`n" + $nightly
$packageText = $package + "`n" + $installerBuild

Assert-NoText $failures $workflows '(?i)rime-frost' "Release workflows must not use rime-frost."
Assert-NoText $failures $workflows '(?im)^\s*(repository|path):\s*.*\bmoqi-im-windows\b' "Workflow checkout repository/path must not use moqi-im-windows."
Assert-NoText $failures $workflows '(?im)^\s*(repository|path):\s*.*\bmoqi-ime\b' "Workflow checkout repository/path must not use moqi-ime."
Assert-NoText $failures $packageText '(?i)moqi-im-windows-setup\.exe|moqi.*setup\.exe' "Package scripts must not emit old Moqi installer asset names."

foreach ($workflow in @(
    @{ Name = "release.yml"; Text = $release; ArtifactPattern = 'typeduck-windows-ime-release' },
    @{ Name = "nightly.yml"; Text = $nightly; ArtifactPattern = 'typeduck-windows-ime-nightly' }
  )) {
  Assert-Text $failures $workflow.Text 'path:\s*TypeDuck-Windows\b' "$($workflow.Name) must checkout the frontend into TypeDuck-Windows."
  Assert-Text $failures $workflow.Text 'repository:\s*\$\{\{\s*github\.repository_owner\s*\}\}/TypeDuck-Windows-backend|repository:\s*TypeDuck-HK/TypeDuck-Windows-backend' "$($workflow.Name) must checkout TypeDuck-Windows-backend."
  Assert-Text $failures $workflow.Text 'path:\s*TypeDuck-Windows-backend\b' "$($workflow.Name) must checkout the backend into TypeDuck-Windows-backend."
  Assert-NoText $failures $workflow.Text 'repository:\s*\$\{\{\s*github\.repository_owner\s*\}\}/schema|ref:\s*aap2-alpha|typeduck-schema-prune-list\.txt|rime_deployer\.exe' "$($workflow.Name) must consume the schema release artifact instead of checking out or building schema data."
  Assert-Text $failures $workflow.Text 'https://github\.com/TypeDuck-HK/schema/releases/download/latest/TypeDuck-Windows\.zip' "$($workflow.Name) must download the TypeDuck schema release artifact."
  Assert-Text $failures $workflow.Text 'Expand-Archive\s+-Path\s+\$schemaZip\s+-DestinationPath\s+\$schemaDir\s+-Force' "$($workflow.Name) must extract the schema artifact to TypeDuck-HK-schema."
  Assert-Text $failures $workflow.Text '-RimeDataSource\s+"?\$env:GITHUB_WORKSPACE\\TypeDuck-HK-schema' "$($workflow.Name) must pass the extracted schema artifact to the backend build."
  Assert-Text $failures $workflow.Text $workflow.ArtifactPattern "$($workflow.Name) must use TypeDuck-owned workflow artifact names."
  Assert-Text $failures $workflow.Text 'typeduck-windows-ime-setup-\$\{\{\s*github\.event\.release\.tag_name\s*\|\|\s*github\.sha\s*\}\}\.exe' "$($workflow.Name) must prepare the tag-or-sha installer asset name."
  Assert-Text $failures $workflow.Text 'installer/dist/\$\{\{\s*env\.TYPEDUCK_INSTALLER_ASSET\s*\}\}|installer\\dist\\.*TYPEDUCK_INSTALLER_ASSET' "$($workflow.Name) must upload the renamed installer asset."

  foreach ($block in (Get-UploadArtifactBlocks $workflow.Text)) {
    Assert-NoText $failures $block '(?i)TypeDuck-HK-schema|schema\b|rime_deployer|input_methods[\\/]+rime[\\/]+build' "$($workflow.Name) must not upload schema inputs or build output as a standalone artifact."
  }
}

Assert-Text $failures $packageText 'installer\\dist\\typeduck-windows-ime-setup\.exe|installer/dist/typeduck-windows-ime-setup\.exe' "Package scripts must point at installer/dist/typeduck-windows-ime-setup.exe."
Assert-Text $failures $packageText 'RimeDataSource' "Package scripts must forward the TypeDuck schema source to the backend build."
Assert-NoText $failures $package '`\\"\$[A-Za-z][A-Za-z0-9_]*`\\"' "Package script native argument arrays must not embed literal quote characters around variable values."

$installerFullPath = Resolve-FullPath -BasePath $root -Path $InstallerPath
if (Test-Path -LiteralPath $installerFullPath -PathType Leaf) {
  $installerItem = Get-Item -LiteralPath $installerFullPath
  if ($installerItem.Name -ne "typeduck-windows-ime-setup.exe") {
    Add-Failure $failures "Installer artifact name must be typeduck-windows-ime-setup.exe."
  }
  if ($installerItem.Length -le 0) {
    Add-Failure $failures "Installer artifact must have non-zero byte length."
  }
  $hash = Get-FileHash -Algorithm SHA256 -LiteralPath $installerFullPath
  if ($hash.Hash -notmatch '^[0-9A-Fa-f]{64}$') {
    Add-Failure $failures "Installer SHA-256 hash must be a 64-character hex digest."
  }
} elseif ($Strict) {
  Add-Failure $failures "Strict mode requires a present installer artifact at $InstallerPath."
}

if ($Strict) {
  Assert-Text $failures $workflows 'choco install innosetup' "Strict mode requires CI installer build prerequisites to be explicit."
  Assert-Text $failures $workflows 'actions/upload-artifact@v4' "Strict mode requires workflow artifact upload."
}

if ($failures.Count -gt 0) {
  Write-Error ("TypeDuck release artifact guard failed:`n - " + ($failures -join "`n - "))
}

Write-Host "TypeDuck release artifact guard passed."
exit 0
