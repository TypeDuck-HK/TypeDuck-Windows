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

    if ($SurfaceName -in @("scripts/install.ps1", "scripts/_all_in_package.ps1")) {
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
        "MyAppPublisher\s+`"香港教育大學 The Education University of Hong Kong`"",
        "AppPublisher=\{#MyAppPublisher\}",
        "DefaultDirName=\{autopf32\}\\TypeDuckIME",
        "CloseApplications=no",
        "OutputBaseFilename=typeduck-windows-ime-setup",
        "WizardImageFile=\.\.\\TypeDuckSettings\\resources\\Installer\.bmp",
        "TypeDuckLauncher\.exe",
        "TypeDuckSetupHelper\.exe",
        "TypeDuckTextService\.dll",
        "ValueName:\s*`"TypeDuckLauncher`"",
        "TypeDuckIME-ReRegisterTSF",
        "\{autoprograms\}\\TypeDuckIME\\輸入法設定 IME Settings",
        "\{autoprograms\}\\TypeDuckIME\\關於 About TypeDuck…",
        "\{autoprograms\}\\TypeDuckIME\\解除安裝 Uninstall"
    ) "Installer script must use TypeDuck AppId, CLSID, names, paths, startup, and scheduled-task identity."

    Assert-AllMatch $Failures $Iss @(
        "歡迎使用 TypeDuck 打得 —— 設有少數族裔語言提示粵拼輸入法！有字想打？一裝即用，毋須再等，即刻打得！",
        "Welcome to TypeDuck: a Cantonese input keyboard with minority language prompts! Got something you want to type\? Have your fingers ready, get, set, TYPE DUCK!",
        "如有任何查詢，歡迎電郵至 info@typeduck\.hk 或 lchaakming@eduhk\.hk。",
        "Should you have any enquiries, please email info@typeduck\.hk or lchaakming@eduhk\.hk\.",
        "本輸入法由香港教育大學語言學及現代語言系開發。特別鳴謝「語文教育及研究常務委員會」資助本計劃。",
        "This input method is developed by the Department of Linguistics and Modern Language Studies, the Education University of Hong Kong\. Special thanks to the Standing Committee on Language Education and Research for funding this project\."
    ) "Installer welcome page must source the exact Phase 5 About intro/contact/credit bilingual text."

    Assert-NotMatch $Failures $Iss "MessagesFile\s*=" "Installer must not depend on external Inno message files for visible setup/uninstall chrome."
    Assert-AllMatch $Failures $Iss @(
        "\[Messages\]",
        "ButtonNext=.*下一步.*Next",
        "WizardSelectDir=.*選擇安裝位置.*Select Destination Location",
        "ReadyLabel1=.*TypeDuck",
        "InstallingLabel=.*TypeDuck",
        "ConfirmUninstall=.*是否要移除.*Do you want to remove",
        "UninstalledAll=.*TypeDuck 已解除安裝.*TypeDuck is uninstalled"
    ) "Installer must provide TypeDuck-controlled bilingual setup and uninstall chrome in installer/MoqiTsf.iss."
    Assert-BilingualCopy $Failures $Iss "installer/MoqiTsf.iss"
    Assert-NotMatch $Failures $Iss "MoqiLauncher\.exe|(?<!TypeDuck)SetupHelper\.exe|MoqiTextService\.dll|DefaultDirName=\{autopf32\}\\MoqiIM|OutputBaseFilename=moqi-im-windows-setup" `
        "Installer-controlled payload, install directory, and artifact names must not use Moqi deployed names."
    Assert-Match $Failures $Iss "RegPurgeTypeDuckResiduals|PurgeTypeDuck" "Uninstall must purge TypeDuck COM/TSF registry residue."
    Assert-NotMatch $Failures $Iss "RegPurgeLegacyMoqiResiduals|LegacyMoqi|MoqiLauncher|\\MoqiIM|\\Moqi\\|Software\\Classes\\CLSID\\\{8F204C91|SOFTWARE\\Microsoft\\CTF\\TIP\\\{8F204C91" `
        "Installer cleanup must not delete, unregister, repair, or otherwise touch Legacy Moqi state."
    Assert-NotMatch $Failures $Iss "CloseApplications=yes|AppMutex|CloseApplicationsFilter|RestartApplications=yes" `
        "Installer must not rely on Inno running-application close/restart pages for TypeDuck process cleanup."
    Assert-AllMatch $Failures $Iss @(
        "TryKillProcessImage\('TypeDuckLauncher\.exe'\)",
        "TryKillProcessImage\('TypeDuckSettings\.exe'\)",
        "TryKillProcessImage\('TypeDuckAbout\.exe'\)",
        "TryKillProcessInAppDir\('server'\)"
    ) "StopTypeDuckProcesses must be allowlisted to TypeDuck executables and the app-dir-scoped runtime server."
    Assert-AllMatch $Failures $Iss @(
        "RunSetupHelper\(BuildUninstallSetupHelperParameters\('/u'\)",
        "RegDeleteValue\(HKEY_CURRENT_USER, StartupSubkey, 'TypeDuckLauncher'\)",
        "DeleteTypeDuckReregisterTask",
        "Type:\s*filesandordirs;\s*Name:\s*`"\{app\}`"",
        "Type:\s*filesandordirs;\s*Name:\s*`"\{localappdata\}\\TypeDuckIME`"",
        "Type:\s*filesandordirs;\s*Name:\s*`"\{userappdata\}\\TypeDuckIME`";\s*Check:\s*ShouldDeleteUserDataOnUninstall",
        "PromptDeleteUserDataOnUninstall",
        "Form := CreateCustomForm\(ScaleX\(360\),\s*ScaleY\(132\),\s*False,\s*True\)",
        "Form\.Color := clWhite",
        "ContentWidth := Form\.ClientWidth - \(ContentLeft \* 2\)",
        "ButtonTop := Form\.ClientHeight - ScaleY\(48\)",
        "ButtonsLeft := \(Form\.ClientWidth - \(\(ButtonWidth \* 2\) \+ ButtonGap\)\) div 2",
        "DeleteUserDataOnUninstall := False"
    ) "Uninstall cleanup must cover TypeDuck registration, startup, task, install files, and make roaming user-data deletion opt-in."
    Assert-NotMatch $Failures $Iss "TSetupForm\.Create" `
        "Uninstaller prompt must use Inno's CreateCustomForm helper instead of constructing TSetupForm directly."
    Assert-AllMatch $Failures $Iss @(
        "Type:\s*filesandordirs;\s*Name:\s*`"\{app\}\\x64`"",
        "Type:\s*filesandordirs;\s*Name:\s*`"\{app\}\\TypeDuckRuntime`"",
        "Type:\s*filesandordirs;\s*Name:\s*`"\{app\}\\resources`""
    ) "InstallDelete must pre-clean only the current staged TypeDuck app folders."
    Assert-NotMatch $Failures $Iss "\{app\}\\moqi-ime|Legacy transitional cleanup" `
        "Installer cleanup must not include legacy Moqi runtime folders because TypeDuck has not been distributed with that layout."
    Assert-AllMatch $Failures $Iss @(
        "CurPageChanged\(CurPageID: Integer\)",
        "WizardForm\.FinishedLabel\.Caption := InstallFinishedText",
        "HelperInstallFailed",
        "could not finish installation",
        "UninstalledAll=.*If TypeDuck still appears, restart your computer",
        "HadExistingInstall := ExistingImeInstallationPresent",
        "歡迎使用 TypeDuck",
        "Welcome to TypeDuck",
        "close and reopen the apps",
        "If you are unable to type, restart your computer",
        "If TypeDuck still appears, restart your computer"
    ) "Restart guidance must be rendered on final installer/uninstaller surfaces with reopen-apps-first wording."
    Assert-NotMatch $Failures $Iss "SuppressibleMsgBox\(" `
        "Restart guidance must not use a separate popup."

    $finalGuidanceMatches = [regex]::Matches($Iss, "(?s)function (InstallFinishedText|UninstallFinishedText): String;.*?end;")
    foreach ($match in $finalGuidanceMatches) {
        if ($match.Value -match "\b(TSF|DLL|COM|registration)\b|註冊") {
            Add-Failure $Failures "Final-page restart guidance must not expose TSF, DLL, COM, registration, or comparable technical terms."
        }
    }
}

function Assert-InstallerStagingScript {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $InstallScript
    )

    Assert-Match $Failures $InstallScript '(?s)MoqLauncher\\Release\\TypeDuckLauncher\.exe.*MoqLauncher\\Debug\\TypeDuckLauncher\.exe' `
        "Installer staging must prefer the Win32 Release launcher before any Debug launcher."
    Assert-Match $Failures $InstallScript '(?s)MoqiTextService\\Release\\TypeDuckTextService\.dll.*MoqiTextService\\Debug\\TypeDuckTextService\.dll' `
        "Installer staging must prefer the Win32 Release text service before any Debug text service."
    Assert-Match $Failures $InstallScript '(?s)SetupHelper\\Release\\TypeDuckSetupHelper\.exe.*SetupHelper\\Debug\\TypeDuckSetupHelper\.exe' `
        "Installer staging must prefer the Win32 Release setup helper before any Debug setup helper."
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
        [string] $Root,
        [hashtable] $Files
    )

    foreach ($entry in $Files.GetEnumerator()) {
        Assert-NotMatch $Failures $entry.Value "chinesesimplified|ChineseSimplified\.isl|输入法|检测到|安装程序|请在|卸载清理|墨奇" `
            "$($entry.Key) must not contain Simplified-only installer/setup chrome."
    }

    $removedTranslationPath = Join-Path $Root "installer/Inno-Setup-Chinese-Simplified-Translation"
    if (Test-Path -LiteralPath $removedTranslationPath) {
        Add-Failure $Failures "Removed Simplified Chinese Inno translation directory must not exist: installer/Inno-Setup-Chinese-Simplified-Translation"
    }

    $gitmodulesPath = Join-Path $Root ".gitmodules"
    if (Test-Path -LiteralPath $gitmodulesPath) {
        $gitmodules = [System.IO.File]::ReadAllText($gitmodulesPath, [System.Text.Encoding]::UTF8)
        Assert-NotMatch $Failures $gitmodules "installer/Inno-Setup-Chinese-Simplified-Translation|ChineseSimplified\.isl|chinesesimplified" `
            ".gitmodules must not reference the removed Simplified Chinese Inno translation dependency."
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
Assert-InstallerStagingScript $failures $files["scripts/install.ps1"]
Assert-SetupHelper $failures $files["SetupHelper/SetupHelper.cpp"] $files["SetupHelper/CMakeLists.txt"]
Assert-StagingPipeline $failures $files["installer/build-installer.ps1"] $files["scripts/install.ps1"] $files["scripts/_all_in_package.ps1"] $files["MoqLauncher/CMakeLists.txt"]

foreach ($entry in $files.GetEnumerator()) {
    Assert-NarrowLegacyMoqiAllowlist $failures $entry.Value $entry.Key
}

if ($Strict) {
    Assert-StrictNoSimplifiedInstallerChrome $failures $root $files
}

if ($failures.Count -gt 0) {
    Write-Error ("TypeDuck installer skeleton check failed:`n - " + ($failures -join "`n - "))
}

Write-Host "TypeDuck installer skeleton check passed."
exit 0
