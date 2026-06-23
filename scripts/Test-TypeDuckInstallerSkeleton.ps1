#Requires -Version 5.1
<#
.SYNOPSIS
  Static contract guard for the TypeDuck installer, setup helper, and staging skeleton.

.PARAMETER RepoRoot
  Repository root to inspect. Defaults to the parent directory of this script.

.PARAMETER Strict
  Enables stronger checks for Simplified-only installer chrome and legacy scaffold leakage.
#>
param(
    [string] $RepoRoot = "",
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

function Read-RequiredFile {
    param(
        [string] $Root,
        [string] $RelativePath
    )

    $path = Join-Path $Root $RelativePath
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Required file not found: $RelativePath"
    }
    return [System.IO.File]::ReadAllText($path, [System.Text.Encoding]::UTF8)
}

function Add-Failure {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Message
    )

    $Failures.Add($Message) | Out-Null
}

function Assert-Match {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Content,
        [string] $Pattern,
        [string] $Message
    )

    if ($Content -notmatch $Pattern) {
        Add-Failure $Failures $Message
    }
}

function Assert-NotMatch {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Content,
        [string] $Pattern,
        [string] $Message
    )

    if ($Content -match $Pattern) {
        Add-Failure $Failures $Message
    }
}

function Assert-AllMatch {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Content,
        [string[]] $Patterns,
        [string] $Message
    )

    foreach ($pattern in $Patterns) {
        if ($Content -notmatch $pattern) {
            Add-Failure $Failures "$Message Missing pattern: $pattern"
        }
    }
}

function Assert-BilingualCopy {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Content,
        [string] $SurfaceName
    )

    if (($Content -notmatch "粵語|繁體|香港|重啟|安裝|解除安裝|輸入法") -or
        ($Content -notmatch "TypeDuck|Cantonese|Install|Uninstall|Restart|launcher|setup")) {
        Add-Failure $Failures "$SurfaceName must contain Traditional Hong Kong Chinese and English user-facing copy."
    }
}

function Assert-NarrowLegacyMoqiAllowlist {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Content,
        [string] $SurfaceName
    )

    $legacyMatches = [regex]::Matches($Content, "Moqi|墨奇|MoqiIM|MoqiLauncher|MoqiTextService|moqi-im-windows|ChineseSimplified|chinesesimplified")
    if ($legacyMatches.Count -eq 0) {
        return
    }

    if ($Content -notmatch "Legacy Moqi migration cleanup|Moqi scaffold compatibility|transition-only|legacy scaffold") {
        Add-Failure $Failures "$SurfaceName contains legacy Moqi/Simplified markers without an explicit narrow migration or scaffold-compatibility note."
    }
}

function Assert-InstallerScript {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Iss
    )

    Assert-AllMatch $Failures $Iss @(
        "9B52CF20-1C5D-4C74-9F5D-9E66377C8F37",
        "7D92985A-BC53-47B5-A5CC-6E47F86B9D18",
        "TypeDuck\s+粵語輸入法\s*/\s*TypeDuck\s+Cantonese\s+IME",
        "AppPublisher=\{#MyAppPublisher\}",
        "DefaultDirName=\{autopf32\}\\TypeDuckIME",
        "OutputBaseFilename=typeduck-windows-ime-setup",
        "TypeDuckLauncher\.exe",
        "TypeDuckSetupHelper\.exe",
        "TypeDuckTextService\.dll",
        "ValueName:\s*`"TypeDuckLauncher`"",
        "TypeDuckIME-ReRegisterTSF"
    ) "Installer script must use TypeDuck AppId, CLSID, names, paths, startup, and scheduled-task identity."

    Assert-Match $Failures $Iss "Name:\s*`"english`"" "Installer must include English wizard resources."
    Assert-Match $Failures $Iss "chinesetraditional|ChineseTraditional|Traditional Chinese" "Installer must use Traditional Chinese-compatible resources."
    Assert-BilingualCopy $Failures $Iss "installer/MoqiTsf.iss"
    Assert-NotMatch $Failures $Iss "MoqiLauncher\.exe|SetupHelper\.exe|MoqiTextService\.dll|DefaultDirName=\{autopf32\}\\MoqiIM|OutputBaseFilename=moqi-im-windows-setup" `
        "Installer-controlled payload, install directory, and artifact names must not use Moqi deployed names."
    Assert-Match $Failures $Iss "RegPurgeTypeDuckResiduals|PurgeTypeDuck" "Uninstall must purge TypeDuck COM/TSF registry residue."
    Assert-Match $Failures $Iss "RegPurgeLegacyMoqiResiduals|Legacy Moqi migration cleanup" "Any Moqi cleanup must be explicit and narrow."
}

function Assert-SetupHelper {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $SetupHelper,
        [string] $SetupHelperCMake
    )

    Assert-AllMatch $Failures $SetupHelper @(
        "TYPEDUCK_PROGRAM_DIR",
        "MOQI_PROGRAM_DIR",
        "transition-only|compatibility alias",
        "TypeDuckIME-ReRegisterTSF",
        "TypeDuckTextService\.dll",
        "SysWOW64|GetSyswow64DirectoryPath",
        "System32|GetNativeSystemDirectoryPath",
        "regsvr32\.exe",
        "/u",
        "TypeDuckSetupHelper"
    ) "SetupHelper must use TypeDuck DLL/env/task identity and keep dual-bitness registration mechanics."

    Assert-BilingualCopy $Failures $SetupHelper "SetupHelper/SetupHelper.cpp"
    Assert-Match $Failures $SetupHelperCMake "OUTPUT_NAME\s+`"?TypeDuckSetupHelper|PROPERTIES[\s\S]*OUTPUT_NAME[\s\S]*TypeDuckSetupHelper" `
        "SetupHelper CMake output must deploy as TypeDuckSetupHelper.exe."
}

function Assert-StagingPipeline {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $BuildInstaller,
        [string] $InstallScript,
        [string] $PackageScript,
        [string] $LauncherCMake
    )

    Assert-AllMatch $Failures $BuildInstaller @(
        "win32\\TypeDuckIME",
        "x64\\TypeDuckIME",
        "TypeDuckLauncher\.exe",
        "TypeDuckSetupHelper\.exe",
        "TypeDuckTextService\.dll",
        "typeduck-windows-ime-setup\.exe"
    ) "installer/build-installer.ps1 must validate the TypeDuck staged payload and output artifact."

    Assert-AllMatch $Failures $InstallScript @(
        "win32\\TypeDuckIME",
        "x64\\TypeDuckIME",
        "TypeDuckLauncher\.exe",
        "TypeDuckSetupHelper\.exe",
        "TypeDuckTextService\.dll",
        "TypeDuckIME",
        "SkipMoqiImeCopy"
    ) "scripts/install.ps1 must stage TypeDuck roots and deployed binary names."

    Assert-AllMatch $Failures $PackageScript @(
        "pwsh",
        "typeduck-windows-ime-setup\.exe",
        "TypeDuck"
    ) "scripts/_all_in_package.ps1 must align with the TypeDuck installer pipeline and use pwsh for project scripts."

    Assert-Match $Failures $LauncherCMake "OUTPUT_NAME\s+`"?TypeDuckLauncher|PROPERTIES[\s\S]*OUTPUT_NAME[\s\S]*TypeDuckLauncher" `
        "MoqLauncher CMake target must output TypeDuckLauncher.exe."
}

function Assert-StrictNoSimplifiedInstallerChrome {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [hashtable] $Files
    )

    foreach ($entry in $Files.GetEnumerator()) {
        Assert-NotMatch $Failures $entry.Value "chinesesimplified|ChineseSimplified\.isl|输入法|检测到|安装程序|请在|卸载清理|墨奇" `
            "$($entry.Key) must not contain Simplified-only installer/setup chrome."
    }
}

$root = Resolve-RepoRoot -RequestedRoot $RepoRoot
$failures = [System.Collections.Generic.List[string]]::new()

$files = @{
    "SetupHelper/SetupHelper.cpp" = Read-RequiredFile $root "SetupHelper/SetupHelper.cpp"
    "SetupHelper/CMakeLists.txt" = Read-RequiredFile $root "SetupHelper/CMakeLists.txt"
    "installer/MoqiTsf.iss" = Read-RequiredFile $root "installer/MoqiTsf.iss"
    "installer/build-installer.ps1" = Read-RequiredFile $root "installer/build-installer.ps1"
    "scripts/install.ps1" = Read-RequiredFile $root "scripts/install.ps1"
    "scripts/_all_in_package.ps1" = Read-RequiredFile $root "scripts/_all_in_package.ps1"
    "MoqLauncher/CMakeLists.txt" = Read-RequiredFile $root "MoqLauncher/CMakeLists.txt"
}

Assert-InstallerScript $failures $files["installer/MoqiTsf.iss"]
Assert-SetupHelper $failures $files["SetupHelper/SetupHelper.cpp"] $files["SetupHelper/CMakeLists.txt"]
Assert-StagingPipeline $failures $files["installer/build-installer.ps1"] $files["scripts/install.ps1"] $files["scripts/_all_in_package.ps1"] $files["MoqLauncher/CMakeLists.txt"]

foreach ($entry in $files.GetEnumerator()) {
    Assert-NarrowLegacyMoqiAllowlist $failures $entry.Value $entry.Key
}

if ($Strict) {
    Assert-StrictNoSimplifiedInstallerChrome $failures $files
}

if ($failures.Count -gt 0) {
    Write-Error ("TypeDuck installer skeleton check failed:`n - " + ($failures -join "`n - "))
}

Write-Host "TypeDuck installer skeleton check passed."
exit 0
