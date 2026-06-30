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

function Assert-NoticeFormat {
  param(
    [System.Collections.Generic.List[string]] $Failures,
    [string] $NoticeText
  )

  if ($NoticeText.Length -eq 0) {
    Add-Failure $Failures "Third-party notice must not be empty."
    return
  }

  if ($NoticeText -notmatch '^THIRD PARTY NOTICES\r\n\r\n') {
    Add-Failure $Failures "Third-party notice must start with a title followed by one empty line."
  }

  if ($NoticeText -match "`f") {
    Add-Failure $Failures "Third-party notice must not contain form feed characters."
  }

  if ($NoticeText -match "(?<!`r)`n") {
    Add-Failure $Failures "Third-party notice must use CRLF line endings."
  }

  if ($NoticeText -match "(?m)[ `t]+$") {
    Add-Failure $Failures "Third-party notice must not contain trailing spaces."
  }

  $separator = "=" * 80
  Assert-Text $Failures $NoticeText "(?m)^$([regex]::Escape($separator))`r?`n$([regex]::Escape($separator))`r?`n$([regex]::Escape($separator))`r?`nLicense: " "Third-party notice sections must start with three separator lines followed by a License header."

  $sectionStartPattern = [regex]::Escape("$separator`r`n$separator`r`n$separator`r`nLicense: ")
  foreach ($match in [regex]::Matches($NoticeText, $sectionStartPattern)) {
    if ($match.Index -lt 4 -or $NoticeText.Substring($match.Index - 4, 4) -ne "`r`n`r`n") {
      Add-Failure $Failures "Each third-party notice group must be preceded by a blank line."
    }
  }

  $lines = $NoticeText -split '\r\n'
  foreach ($line in $lines) {
    if ($line -match '^Source: ' -and $line -notmatch '^Source: https?://') {
      Add-Failure $Failures "Third-party notice source must be a URL: $line"
    }
  }
}

$root = Resolve-FullPath -BasePath (Get-Location).Path -Path $RepoRoot
$failures = [System.Collections.Generic.List[string]]::new()

$releasePath = Join-Path $root ".github\workflows\release.yml"
$nightlyPath = Join-Path $root ".github\workflows\nightly.yml"
$packagePath = Join-Path $root "scripts\_all_in_package.ps1"
$buildPath = Join-Path $root "scripts\build.ps1"
$installerBuildPath = Join-Path $root "installer\build-installer.ps1"
$licenseNoticePath = Join-Path $root "THIRD_PARTY_NOTICES.txt"

$release = Read-RequiredText $failures $releasePath ".github/workflows/release.yml"
$nightly = Read-RequiredText $failures $nightlyPath ".github/workflows/nightly.yml"
$package = Read-RequiredText $failures $packagePath "scripts/_all_in_package.ps1"
$build = Read-RequiredText $failures $buildPath "scripts/build.ps1"
$installerBuild = Read-RequiredText $failures $installerBuildPath "installer/build-installer.ps1"
$licenseNotice = Read-RequiredText $failures $licenseNoticePath "THIRD_PARTY_NOTICES.txt"
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
Assert-Text $failures $build '-DCMAKE_POLICY_VERSION_MINIMUM:STRING=3\.5' "Build script must configure CMake with a policy-version minimum for CMake 4 compatibility."
Assert-Text $failures $licenseNotice 'TypeDuck-Windows-backend' "Third-party license notice must include the backend license."
Assert-Text $failures $licenseNotice 'TypeDuck-HK librime fork' "Third-party license notice must include librime."
Assert-Text $failures $licenseNotice 'rime-dictionary-lookup-filter' "Third-party license notice must include the dictionary lookup filter."
Assert-Text $failures $licenseNotice 'darts-clone / OpenCC deps/darts-clone-0\.32' "Third-party notice must include checked OpenCC deps."
Assert-Text $failures $licenseNotice 'Source: https://github\.com/s-yata/darts-clone' "Vendored single-file dependencies must trace to upstream sources."
Assert-Text $failures $licenseNotice 'OpenCC deps/rapidjson-1\.1\.0 msinttypes' "Third-party notice must include recursively checked nested dependency notices."
Assert-Text $failures $licenseNotice 'OpenCC deps/tclap-1\.2\.5' "Third-party notice must include checked OpenCC deps."
Assert-Text $failures $licenseNotice 'luna-pinyin schema data' "Third-party license notice must include schema dependency licenses."
Assert-Text $failures $licenseNotice 'cangjie3 schema data' "Third-party license notice must include cangjie3 schema data."
Assert-NoticeFormat $failures $licenseNotice
Assert-NoText $failures $licenseNotice '(?i)([A-Z]:\\|\\VSProjects\\|\\\.cache\\|TypeDuck-Windows:|TypeDuck-Windows-backend:)' "Third-party notice must not contain local checkout paths or repo-relative source labels."
Assert-NoText $failures $licenseNotice '(?m)^THIRD_PARTY_LICENSES\.txt$|^COMPONENTS$|^NOTICE TEXTS$|^OPENCC VENDORED DEPENDENCY INVENTORY$|This file is installed|collects license texts|OpenCC deps checked from' "Third-party notice must not use the old filename, table headings, or meta-commentary preamble."

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
}
elseif ($Strict) {
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
