param(
  [string] $RepoRoot = ".",
  [string] $BackendRoot = "D:\VSProjects\moqi-ime",
  [switch] $Strict,
  [ValidateSet("", "RejectedUatBehavior")]
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

function Assert-Text([string] $Text, [string] $Pattern, [string] $Message) {
  if ($Text -notmatch $Pattern) {
    throw $Message
  }
}

function Assert-Ordered([string] $Text, [string[]] $Patterns, [string] $Label) {
  $position = -1
  foreach ($pattern in $Patterns) {
    $match = [regex]::Match($Text.Substring($position + 1), $pattern)
    if (-not $match.Success) {
      throw "$Label missing or out of order: $pattern"
    }
    $position = $position + 1 + $match.Index + $match.Length
  }
}

function Add-Violation([System.Collections.Generic.List[string]] $Violations, [string] $Message) {
  $Violations.Add($Message)
}

$repo = Resolve-FullPath $RepoRoot
$backend = Resolve-FullPath $BackendRoot

$topCmake = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $repo "CMakeLists.txt")
$settingsCmakePath = Join-Path $repo "TypeDuckSettings/CMakeLists.txt"
$windowPath = Join-Path $repo "TypeDuckSettings/TypeDuckSettingsWindow.cpp"
$mainPath = Join-Path $repo "TypeDuckSettings/main.cpp"
$aboutMainPath = Join-Path $repo "TypeDuckSettings/TypeDuckAboutMain.cpp"
$rcPath = Join-Path $repo "TypeDuckSettings/TypeDuckSettings.rc"
$aboutRcPath = Join-Path $repo "TypeDuckSettings/TypeDuckAbout.rc"
$resourcePath = Join-Path $repo "TypeDuckSettings/resource.h"
$preferencesPath = Join-Path $repo "MoqLauncher/TypeDuckPreferences.cpp"
$imeModulePath = Join-Path $repo "MoqiTextService/MoqiImeModule.cpp"
$textServicePath = Join-Path $repo "MoqiTextService/MoqiTextService.cpp"
$textServiceCmakePath = Join-Path $repo "MoqiTextService/CMakeLists.txt"
$pipeServerPath = Join-Path $repo "MoqLauncher/PipeServer.cpp"
$installScriptPath = Join-Path $repo "scripts/install.ps1"
$packageScriptPath = Join-Path $repo "scripts/_all_in_package.ps1"
$installerPath = Join-Path $repo "installer/MoqiTsf.iss"
$settingsOrderPath = Join-Path $repo ".planning/product/web-alpha-fixtures/2026-06-23/settings-order.json"

Assert-True (Test-Path -LiteralPath $settingsCmakePath) "TypeDuckSettings/CMakeLists.txt is missing."
Assert-True (Test-Path -LiteralPath $windowPath) "TypeDuckSettingsWindow.cpp is missing."
Assert-True (Test-Path -LiteralPath $mainPath) "TypeDuckSettings main.cpp is missing."
Assert-True (Test-Path -LiteralPath $aboutMainPath) "TypeDuckAbout main.cpp is missing."
Assert-True (Test-Path -LiteralPath $rcPath) "TypeDuckSettings.rc is missing."
Assert-True (Test-Path -LiteralPath $aboutRcPath) "TypeDuckAbout.rc is missing."
Assert-True (Test-Path -LiteralPath $resourcePath) "TypeDuckSettings resource.h is missing."
Assert-True (Test-Path -LiteralPath $imeModulePath) "MoqiImeModule.cpp is missing."
Assert-True (Test-Path -LiteralPath $pipeServerPath) "PipeServer.cpp is missing."
Assert-True (Test-Path -LiteralPath $installScriptPath) "scripts/install.ps1 is missing."
Assert-True (Test-Path -LiteralPath $packageScriptPath) "scripts/_all_in_package.ps1 is missing."
Assert-True (Test-Path -LiteralPath $installerPath) "installer/MoqiTsf.iss is missing."

$settingsCmake = Get-Content -Raw -Encoding UTF8 -LiteralPath $settingsCmakePath
$window = Get-Content -Raw -Encoding UTF8 -LiteralPath $windowPath
$aboutMain = Get-Content -Raw -Encoding UTF8 -LiteralPath $aboutMainPath
$preferences = Get-Content -Raw -Encoding UTF8 -LiteralPath $preferencesPath
$imeModule = Get-Content -Raw -Encoding UTF8 -LiteralPath $imeModulePath
$textService = Get-Content -Raw -Encoding UTF8 -LiteralPath $textServicePath
$textServiceCmake = Get-Content -Raw -Encoding UTF8 -LiteralPath $textServiceCmakePath
$pipeServer = Get-Content -Raw -Encoding UTF8 -LiteralPath $pipeServerPath
$installScript = Get-Content -Raw -Encoding UTF8 -LiteralPath $installScriptPath
$packageScript = Get-Content -Raw -Encoding UTF8 -LiteralPath $packageScriptPath
$installer = Get-Content -Raw -Encoding UTF8 -LiteralPath $installerPath
$fixture = Get-Content -Raw -Encoding UTF8 -LiteralPath $settingsOrderPath | ConvertFrom-Json

$rejectedBehavior = [System.Collections.Generic.List[string]]::new()
if ($window -match "TypeDuckPreferences\.json") {
  Add-Violation $rejectedBehavior "Settings window exposes implementation file name TypeDuckPreferences.json in user-facing copy."
}
if ($window -match "Unsupported controls are disabled with a reason|不支援時會停用") {
  Add-Violation $rejectedBehavior "Settings window exposes unsupported-state wording without a real user-facing disabled state."
}
foreach ($tick in 4..10) {
  if ($window -notmatch "L`"$tick`"") {
    Add-Violation $rejectedBehavior "Page-size slider is missing visible tick label $tick."
  }
}
if ((Test-Path -LiteralPath (Join-Path $repo "TypeDuckSettings/TypeDuckAboutDialog.cpp")) -and
  ($window -match "關於 About|ShowTypeDuckAboutDialog|kAbout")) {
  Add-Violation $rejectedBehavior "Settings panel still exposes an About button/modal instead of leaving About to a separate executable."
}

if ($ExpectRed -eq "RejectedUatBehavior") {
  Assert-True ($rejectedBehavior.Count -gt 0) "Expected RED RejectedUatBehavior, but the rejected settings/About behavior was not present."
  Write-Host "PASS RED: RejectedUatBehavior caught settings/About gaps: $($rejectedBehavior -join '; ')"
  exit 0
}

Assert-True ($rejectedBehavior.Count -eq 0) "Rejected UAT settings/About behavior found: $($rejectedBehavior -join '; ')"

Assert-Text $topCmake "add_subdirectory\(.+TypeDuckSettings" "Top-level CMake must include TypeDuckSettings."
Assert-Text $settingsCmake "add_executable\(TypeDuckSettings\s+WIN32" "TypeDuckSettings must be a native Win32 executable."
Assert-Text $settingsCmake "add_executable\(TypeDuckAbout\s+WIN32" "TypeDuckAbout must be a separate native Win32 executable."
Assert-Text $settingsCmake "TypeDuckAboutMain\.cpp" "TypeDuckAbout executable must have its own entry point."
Assert-Text $settingsCmake "TypeDuckAbout\.rc" "TypeDuckAbout executable must compile its own About resources."
Assert-Text $settingsCmake "TypeDuckAboutVersion\.rc\.in" "TypeDuckAbout must generate About-specific version metadata."
$settingsTargetBlock = [regex]::Match($settingsCmake, "add_executable\(TypeDuckSettings[\s\S]*?\n\)").Value
Assert-True ($settingsTargetBlock -notmatch "TypeDuckAboutDialog\.cpp") "TypeDuckSettings target must not link the About dialog implementation."
Assert-Text $aboutMain "ShowTypeDuckAboutDialog\(instance,\s*nullptr\)" "TypeDuckAbout must launch the About dialog directly."
Assert-Text $window "Local\\\\TypeDuckSettingsWindowInstance" "Settings executable must prevent duplicate interactive windows."
Assert-Text $window "bringExistingWindowToForeground" "Settings duplicate launch must foreground the existing window."
Assert-Text $settingsCmake "MoqLauncher/TypeDuckPreferences\.cpp" "Settings executable must reuse TypeDuckPreferences."
Assert-Text $settingsCmake "TypeDuckSettingsVersion\.h\.in" "TypeDuckSettings must generate a version header from version.txt-derived CMake values."
Assert-Text $installScript "Resolve-ArtifactPath\s+-Label\s+`"TypeDuckSettings\.exe`"" "Installer staging must resolve TypeDuckSettings.exe."
Assert-Text $installScript "Resolve-ArtifactPath\s+-Label\s+`"TypeDuckAbout\.exe`"" "Installer staging must resolve TypeDuckAbout.exe."
Assert-Text $installScript '\$settingsExe\s*=\s*Resolve-ArtifactPath' "Installer staging must assign the resolved TypeDuckSettings.exe artifact."
Assert-Text $installScript '\$aboutExe\s*=\s*Resolve-ArtifactPath' "Installer staging must assign the resolved TypeDuckAbout.exe artifact."
Assert-Text $installScript 'Copy-IfExists\s+-Source\s+\$settingsExe' "Installer staging must copy TypeDuckSettings.exe from the resolved artifact."
Assert-Text $installScript 'Copy-IfExists\s+-Source\s+\$aboutExe' "Installer staging must copy TypeDuckAbout.exe from the resolved artifact."
Assert-Text $installScript 'Join-Path\s+\$stageWin32Root\s+"TypeDuckSettings\.exe"' "Installer staging must place TypeDuckSettings.exe in the app payload root."
Assert-Text $installScript 'Join-Path\s+\$stageWin32Root\s+"TypeDuckAbout\.exe"' "Installer staging must place TypeDuckAbout.exe in the app payload root."
Assert-Text $installer "Filename:\s+`"\{app\}\\TypeDuckSettings\.exe`"" "Installer must launch TypeDuckSettings.exe during setup."
Assert-Text $installer "Filename:\s+`"\{app\}\\TypeDuckAbout\.exe`"" "Installer must launch TypeDuckAbout.exe during setup."
Assert-Text $installer "Description:\s+`"開啟 TypeDuck 設定 / Open TypeDuck Settings`"" "Installer settings launch description must be bilingual."
Assert-Text $installer "Description:\s+`"開啟 TypeDuck 關於 / Open TypeDuck About`"" "Installer About launch description must be bilingual."
Assert-Text $installer "function\s+ShouldLaunchSettings\(\):\s+Boolean" "Installer settings launch must be gated by a dedicated function."
Assert-Text $installer "function\s+ShouldSeedDefaultSettings\(\):\s+Boolean" "Installer must gate default settings seeding."
Assert-Text $installer "function\s+ShouldLaunchAbout\(\):\s+Boolean" "Installer About launch must be gated by a dedicated function."
Assert-Text $installer 'TypeDuck About";\s+Filename:\s+"\{app\}\\TypeDuckAbout\.exe"' "Installer must create a TypeDuck About Start Menu shortcut."
Assert-Text $installer 'TypeDuck 關於";\s+Filename:\s+"\{app\}\\TypeDuckAbout\.exe"' "Installer must create a TypeDuck 關於 Start Menu shortcut."
Assert-Ordered $installer @(
  "Filename:\s+`"\{app\}\\TypeDuckLauncher\.exe`"",
  "Parameters:\s+`"/apply-defaults`"",
  "Filename:\s+`"\{app\}\\TypeDuckSettings\.exe`"",
  "Filename:\s+`"\{app\}\\TypeDuckAbout\.exe`""
) "Installer run order"
Assert-Text $packageScript "scripts\\install\.ps1" "All-in package script must continue to route packaging through scripts/install.ps1."
Assert-Text $imeModule "TypeDuckSettings\.exe" "TSF Configure entry point must launch TypeDuckSettings.exe."
Assert-Text $imeModule "launchTypeDuckSettings" "TSF Configure entry point must route through a fixed TypeDuck settings launch helper."
Assert-Text $imeModule "ShellExecuteW" "TSF Configure entry point must use a native launch call for the fixed settings executable."
Assert-True ($imeModule -notmatch "configTool|configToolParams|configToolDir") "TSF Configure entry point must not use backend-declared config tool metadata."
Assert-Text $pipeServer "TypeDuckSettings\.exe" "Launcher post-install entry point must launch TypeDuckSettings.exe."
Assert-Text $pipeServer "openTypeDuckSettings" "Launcher must expose a fixed TypeDuck settings launch helper."
Assert-Text $pipeServer "TypeDuckAbout\.exe" "Launcher tray entry must launch TypeDuckAbout.exe."
Assert-Text $pipeServer "openTypeDuckAbout" "Launcher must expose a fixed TypeDuck About launch helper."
Assert-Text $pipeServer "輸入法設定 IME Settings" "Launcher settings menu label must be bilingual."
Assert-Text $pipeServer "關於 / About TypeDuck" "Launcher About menu label must be bilingual."
Assert-Ordered $pipeServer @(
  "ID_OPEN_TYPEDUCK_SETTINGS",
  "ID_OPEN_TYPEDUCK_ABOUT"
) "Launcher tray settings/About command ordering"
Assert-Ordered $pipeServer @(
  "輸入法設定 IME Settings",
  "關於 / About TypeDuck"
) "Launcher tray settings/About menu ordering"
Assert-True ($pipeServer -notmatch "configTool|configToolParams|configToolDir") "Launcher settings entry point must not use backend-declared config tool metadata."

$combined = "$topCmake`n$settingsCmake`n$window`n$imeModule`n$pipeServer"
Assert-True ($combined -notmatch "find_package\s*\(\s*Qt|Qt[0-9]::|QApplication|#include\s*<Q") "TypeDuckSettings must not use Qt."
Assert-True ($combined -notmatch "configTool|configToolParams|configToolDir") "Settings UI must not use backend-declared config tools."

$expectedOrder = @(
  "顯示語言 Display Languages",
  "每頁候選詞數量 No\. of Candidates Per Page",
  "中文字體 Chinese Typeface",
  "候選詞粵拼 Candidates Jyutping",
  "自動完成 Auto-completion",
  "自動校正 Auto-correction",
  "自動組詞 Auto-composition",
  "輸入記憶 Input Memory",
  "反查設定 Reverse Lookup Settings",
  "顯示完整輸入碼 Show Full Input Code",
  "倉頡版本 Cangjie Version"
)
Assert-Ordered $window $expectedOrder "Settings Web-alpha order"

foreach ($setting in $fixture.settings) {
  Assert-Text $window ([regex]::Escape($setting.label)) "Missing Web-alpha setting label: $($setting.label)"
}

foreach ($language in $fixture.displayLanguageBehavior.languages) {
  Assert-Text $window ([regex]::Escape($language.label)) "Missing display language option: $($language.label)"
}
Assert-Text $window "主要語言 Main Language" "Missing corrected in-group main-language label."
Assert-Text $window "kLeftColumnX" "Settings UI must expose a left column anchor."
Assert-Text $window "kRightColumnX" "Settings UI must expose a right column anchor."
Assert-Text $window "COLOR_WINDOW" "Settings controls must share the white dialog background."
Assert-Text $window "addSectionHeader" "Settings section headings must use a dedicated section header path."
Assert-Text $window "FW_BOLD" "Settings section headings must be bold."
Assert-Text $window "applyHeaderFont" "Settings section headings must keep the header font after child font application."
Assert-Text $window "kRadioGroupStartStyle\s*=\s*BS_AUTORADIOBUTTON\s*\|\s*WS_GROUP" "Settings radio clusters must start separate Win32 radio groups."
Assert-Text $window "index == 0 \? kRadioGroupStartStyle : kRadioStyle" "Main-language radios must form their own group without sharing later settings radios."
Assert-True ($window -notmatch "addButton\([^;]+BS_AUTORADIOBUTTON") "Settings radio controls must use grouped radio style constants, not raw shared radio styles."
Assert-Text $window "kPageSizeTrackInset" "Page-size tick labels must leave side padding instead of spanning the full fieldset width."
Assert-Text $window "kSettingsButtonWidth" "Confirm and Cancel buttons must use an explicit widened button width."
Assert-Text $window "kApplyDefaultsSwitch" "Settings executable must expose a quiet default-preference seed mode for installation."
Assert-Text $window "applyViaLauncher" "Settings confirmation must apply through the launcher so Rime redeploy runs."
Assert-Text $window "METHOD_TYPEDUCK_SETTINGS_UPDATE" "Settings confirmation must send the TypeDuck settings update IPC request."
Assert-Ordered $window @(
  "主要語言 Main Language",
  "顯示 Display",
  "kMainLanguageBase",
  "kDisplayLanguageBase"
) "Display/main-language label alignment"
Assert-Text $window "addPageSizeTickLabels" "Candidate page-size control must render fixed tick labels."
foreach ($tick in 4..10) {
  Assert-Text $window "L`"$tick`"" "Candidate page-size control must show tick label $tick."
}
Assert-Text $window "TBM_SETRANGE.+MAKELPARAM\(4,\s*10\)" "Candidate count control must be bounded 4-10."
Assert-Text $window "applyViaLauncher" "Apply must use the launcher-mediated TypeDuckPreferences path."
Assert-Text $window "fillSettingsUpdate" "Settings must serialize the shared TypeDuckPreferences state into IPC updates."
Assert-Text $window "METHOD_TYPEDUCK_SETTINGS_UPDATE" "Settings confirmation must send the TypeDuck settings update IPC request."
Assert-Text $window "kApplyDefaultsSwitch" "Settings executable must expose a quiet default-preference seed mode for installation."
Assert-Text $window "applyDefaultPreferencesIfMissing" "Installation seed mode must apply defaults when preferences are missing."
Assert-Text $textServiceCmake "MoqLauncher/TypeDuckPreferences\.cpp" "Text service must link the shared preferences loader for UI-only settings."
Assert-Text $textService "reloadTypeDuckDisplayPreferences" "Text service must reload saved UI preferences during candidate refresh."
Assert-Text $textService "displayPreferencesFromSavedPreferences" "Text service must map saved preferences to candidate display preferences."
Assert-Text $window "確定 Confirm" "Confirm button must be bilingual."
Assert-Text $window "取消 Cancel" "Cancel button must be bilingual."
Assert-True ($window -notmatch "套用後即時更新輸入法設定|Apply updates TypeDuck input settings|設定已儲存") "Settings window must not expose removed apply/status helper labels."
Assert-True ($window -notmatch "TypeDuckPreferences\.json|Unsupported controls are disabled|不支援時會停用") "Settings copy must not expose internal persistence files or unsupported-state placeholders."
Assert-Text $preferences "TypeDuckPreferences\.json" "Settings source of truth must remain TypeDuckPreferences.json."

$aboutPath = Join-Path $repo "TypeDuckSettings/TypeDuckAboutDialog.cpp"
if (Test-Path -LiteralPath $aboutPath) {
  $about = Get-Content -Raw -Encoding UTF8 -LiteralPath $aboutPath
  $rc = Get-Content -Raw -Encoding UTF8 -LiteralPath $rcPath
  $aboutRc = Get-Content -Raw -Encoding UTF8 -LiteralPath $aboutRcPath
  $banner = Join-Path $repo "TypeDuckSettings/assets/About_Banner.bmp"
  $credits = Join-Path $repo "TypeDuckSettings/assets/Credit_Logos.bmp"
  Assert-True (Test-Path -LiteralPath $banner) "About_Banner.bmp must live under TypeDuckSettings/assets."
  Assert-True (Test-Path -LiteralPath $credits) "Credit_Logos.bmp must live under TypeDuckSettings/assets."
  Assert-Text $rc "About_Banner\.bmp" "About banner bitmap must be compiled as a resource."
  Assert-Text $rc "Credit_Logos\.bmp" "Credit logos bitmap must be compiled as a resource."
  Assert-Text $aboutRc "About_Banner\.bmp" "Separate About executable must compile the About banner bitmap."
  Assert-Text $aboutRc "Credit_Logos\.bmp" "Separate About executable must compile the credit logos bitmap."
  Assert-True ($about -notmatch "Installer\.bmp" -and $rc -notmatch "Installer\.bmp" -and $aboutRc -notmatch "Installer\.bmp") "Installer.bmp must not be referenced in Phase 5 settings/About."
  Assert-Text $about "Local\\\\TypeDuckAboutWindowInstance" "About executable must prevent duplicate interactive windows."
  Assert-Text $about "bringExistingAboutToForeground" "About duplicate launch must foreground the existing window."
  Assert-Text $about "歡迎使用 TypeDuck 打得" "About dialog must include the exact D-27 bilingual text block."
  $createControlsStart = $about.IndexOf("void createControls()")
  Assert-True ($createControlsStart -ge 0) "About dialog must create controls in a dedicated createControls method."
  $aboutCreateControls = $about.Substring($createControlsStart)
  Assert-Ordered $aboutCreateControls @(
    "kIntroText",
    "kContactText",
    "kCreditText",
    "attributionText",
    "aboutLinks"
  ) "About dialog resource/text/attribution/control order"
  Assert-Ordered $about @(
    "TypeDuck Windows IME 版本 Version",
    "TypeDuck-HK librime fork",
    "TypeDuck-HK schema"
  ) "About attribution order"
  Assert-Ordered $about @(
    "TypeDuck 網站 Website",
    "LearnDuck 粵拼打字入門 Introduction to Jyutping Typing",
    "粵拼方案 Jyutping Scheme",
    "TypeDuck 原始碼 Source Code"
  ) "About link order"
  Assert-Text $about "TYPEDUCK_VERSION_TEXT" "About version must use the version.txt-derived build definition."
  Assert-Text $about "https://typeduck\.hk" "Missing TypeDuck Website URL."
  Assert-Text $about "https://learn\.typeduck\.hk" "Missing LearnDuck URL."
  Assert-Text $about "https://lshk\.org/jyutping-scheme/" "Missing Jyutping Scheme URL."
  Assert-Text $about "https://github\.com/TypeDuck-HK/TypeDuck-Windows" "Missing TypeDuck source URL."
  Assert-Text $about "ShellExecuteW" "About links must use ShellExecuteW or an approved native URL opener."
}

if ($Strict) {
  Assert-True (Test-Path -LiteralPath (Join-Path $backend "icons/About_Banner.bmp")) "Backend source About_Banner.bmp is missing."
  Assert-True (Test-Path -LiteralPath (Join-Path $backend "icons/Credit_Logos.bmp")) "Backend source Credit_Logos.bmp is missing."
}

Write-Host "[PASS] TypeDuck settings/About UI guard passed."
