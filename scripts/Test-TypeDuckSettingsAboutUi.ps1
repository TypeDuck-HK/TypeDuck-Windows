param(
  [string] $RepoRoot = ".",
  [string] $BackendRoot = "D:\VSProjects\moqi-ime",
  [switch] $Strict
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

$repo = Resolve-FullPath $RepoRoot
$backend = Resolve-FullPath $BackendRoot

$topCmake = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $repo "CMakeLists.txt")
$settingsCmakePath = Join-Path $repo "TypeDuckSettings/CMakeLists.txt"
$windowPath = Join-Path $repo "TypeDuckSettings/TypeDuckSettingsWindow.cpp"
$mainPath = Join-Path $repo "TypeDuckSettings/main.cpp"
$rcPath = Join-Path $repo "TypeDuckSettings/TypeDuckSettings.rc"
$resourcePath = Join-Path $repo "TypeDuckSettings/resource.h"
$preferencesPath = Join-Path $repo "MoqLauncher/TypeDuckPreferences.cpp"
$imeModulePath = Join-Path $repo "MoqiTextService/MoqiImeModule.cpp"
$pipeServerPath = Join-Path $repo "MoqLauncher/PipeServer.cpp"
$installScriptPath = Join-Path $repo "scripts/install.ps1"
$packageScriptPath = Join-Path $repo "scripts/_all_in_package.ps1"
$installerPath = Join-Path $repo "installer/MoqiTsf.iss"
$settingsOrderPath = Join-Path $repo ".planning/product/web-alpha-fixtures/2026-06-23/settings-order.json"

Assert-True (Test-Path -LiteralPath $settingsCmakePath) "TypeDuckSettings/CMakeLists.txt is missing."
Assert-True (Test-Path -LiteralPath $windowPath) "TypeDuckSettingsWindow.cpp is missing."
Assert-True (Test-Path -LiteralPath $mainPath) "TypeDuckSettings main.cpp is missing."
Assert-True (Test-Path -LiteralPath $rcPath) "TypeDuckSettings.rc is missing."
Assert-True (Test-Path -LiteralPath $resourcePath) "TypeDuckSettings resource.h is missing."
Assert-True (Test-Path -LiteralPath $imeModulePath) "MoqiImeModule.cpp is missing."
Assert-True (Test-Path -LiteralPath $pipeServerPath) "PipeServer.cpp is missing."
Assert-True (Test-Path -LiteralPath $installScriptPath) "scripts/install.ps1 is missing."
Assert-True (Test-Path -LiteralPath $packageScriptPath) "scripts/_all_in_package.ps1 is missing."
Assert-True (Test-Path -LiteralPath $installerPath) "installer/MoqiTsf.iss is missing."

$settingsCmake = Get-Content -Raw -Encoding UTF8 -LiteralPath $settingsCmakePath
$window = Get-Content -Raw -Encoding UTF8 -LiteralPath $windowPath
$preferences = Get-Content -Raw -Encoding UTF8 -LiteralPath $preferencesPath
$imeModule = Get-Content -Raw -Encoding UTF8 -LiteralPath $imeModulePath
$pipeServer = Get-Content -Raw -Encoding UTF8 -LiteralPath $pipeServerPath
$installScript = Get-Content -Raw -Encoding UTF8 -LiteralPath $installScriptPath
$packageScript = Get-Content -Raw -Encoding UTF8 -LiteralPath $packageScriptPath
$installer = Get-Content -Raw -Encoding UTF8 -LiteralPath $installerPath
$fixture = Get-Content -Raw -Encoding UTF8 -LiteralPath $settingsOrderPath | ConvertFrom-Json

Assert-Text $topCmake "add_subdirectory\(.+TypeDuckSettings" "Top-level CMake must include TypeDuckSettings."
Assert-Text $settingsCmake "add_executable\(TypeDuckSettings\s+WIN32" "TypeDuckSettings must be a native Win32 executable."
Assert-Text $settingsCmake "MoqLauncher/TypeDuckPreferences\.cpp" "Settings executable must reuse TypeDuckPreferences."
Assert-Text $settingsCmake "TypeDuckSettingsVersion\.h\.in" "TypeDuckSettings must generate a version header from version.txt-derived CMake values."
Assert-Text $installScript "Resolve-ArtifactPath\s+-Label\s+`"TypeDuckSettings\.exe`"" "Installer staging must resolve TypeDuckSettings.exe."
Assert-Text $installScript '\$settingsExe\s*=\s*Resolve-ArtifactPath' "Installer staging must assign the resolved TypeDuckSettings.exe artifact."
Assert-Text $installScript 'Copy-IfExists\s+-Source\s+\$settingsExe' "Installer staging must copy TypeDuckSettings.exe from the resolved artifact."
Assert-Text $installScript 'Join-Path\s+\$stageWin32Root\s+"TypeDuckSettings\.exe"' "Installer staging must place TypeDuckSettings.exe in the app payload root."
Assert-Text $installer "Filename:\s+`"\{app\}\\TypeDuckSettings\.exe`"" "Installer must launch TypeDuckSettings.exe during setup."
Assert-Text $installer "Description:\s+`"開啟 TypeDuck 設定 / Open TypeDuck Settings`"" "Installer settings launch description must be bilingual."
Assert-Text $installer "function\s+ShouldLaunchSettings\(\):\s+Boolean" "Installer settings launch must be gated by a dedicated function."
Assert-Ordered $installer @(
  "Filename:\s+`"\{app\}\\TypeDuckSettings\.exe`"",
  "Filename:\s+`"\{app\}\\TypeDuckLauncher\.exe`""
) "Installer run order"
Assert-Text $packageScript "scripts\\install\.ps1" "All-in package script must continue to route packaging through scripts/install.ps1."
Assert-Text $imeModule "TypeDuckSettings\.exe" "TSF Configure entry point must launch TypeDuckSettings.exe."
Assert-Text $imeModule "launchTypeDuckSettings" "TSF Configure entry point must route through a fixed TypeDuck settings launch helper."
Assert-Text $imeModule "ShellExecuteW" "TSF Configure entry point must use a native launch call for the fixed settings executable."
Assert-True ($imeModule -notmatch "configTool|configToolParams|configToolDir") "TSF Configure entry point must not use backend-declared config tool metadata."
Assert-Text $pipeServer "TypeDuckSettings\.exe" "Launcher post-install entry point must launch TypeDuckSettings.exe."
Assert-Text $pipeServer "openTypeDuckSettings" "Launcher must expose a fixed TypeDuck settings launch helper."
Assert-Text $pipeServer "設定 / Settings" "Launcher settings menu label must be bilingual."
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
Assert-Text $window ([regex]::Escape($fixture.displayLanguageBehavior.mainLanguageMarker)) "Missing main-language marker."
Assert-Text $window "kLeftColumnX" "Settings UI must expose a left column anchor."
Assert-Text $window "kRightColumnX" "Settings UI must expose a right column anchor."
Assert-Text $window "TBM_SETRANGE.+MAKELPARAM\(4,\s*10\)" "Candidate count control must be bounded 4-10."
Assert-Text $window "TypeDuck::applyPreferences" "Apply must use the shared TypeDuckPreferences apply path."
Assert-Text $window "套用 Apply" "Apply button must be bilingual."
Assert-Text $window "取消 Cancel" "Cancel button must be bilingual."
Assert-Text $window "Unsupported controls are disabled with a reason" "Capability-gated explanatory text is missing."
Assert-Text $preferences "TypeDuckPreferences\.json" "Settings source of truth must remain TypeDuckPreferences.json."

$aboutPath = Join-Path $repo "TypeDuckSettings/TypeDuckAboutDialog.cpp"
if (Test-Path -LiteralPath $aboutPath) {
  $about = Get-Content -Raw -Encoding UTF8 -LiteralPath $aboutPath
  $rc = Get-Content -Raw -Encoding UTF8 -LiteralPath $rcPath
  $banner = Join-Path $repo "TypeDuckSettings/assets/About_Banner.bmp"
  $credits = Join-Path $repo "TypeDuckSettings/assets/Credit_Logos.bmp"
  Assert-True (Test-Path -LiteralPath $banner) "About_Banner.bmp must live under TypeDuckSettings/assets."
  Assert-True (Test-Path -LiteralPath $credits) "Credit_Logos.bmp must live under TypeDuckSettings/assets."
  Assert-Text $rc "About_Banner\.bmp" "About banner bitmap must be compiled as a resource."
  Assert-Text $rc "Credit_Logos\.bmp" "Credit logos bitmap must be compiled as a resource."
  Assert-True ($about -notmatch "Installer\.bmp" -and $rc -notmatch "Installer\.bmp") "Installer.bmp must not be referenced in Phase 5 settings/About."
  Assert-Text $about "歡迎使用 TypeDuck 打得" "About dialog must include the exact D-27 bilingual text block."
  $createControlsStart = $about.IndexOf("void createControls()")
  Assert-True ($createControlsStart -ge 0) "About dialog must create controls in a dedicated createControls method."
  $aboutCreateControls = $about.Substring($createControlsStart)
  Assert-Ordered $aboutCreateControls @(
    "IDB_TYPEDUCK_ABOUT_BANNER",
    "kAboutBodyText",
    "IDB_TYPEDUCK_CREDIT_LOGOS",
    "attributionText",
    "aboutLinks",
    "關閉 Close"
  ) "About dialog resource/text/attribution/control order"
  Assert-Ordered $about @(
    "TypeDuck Windows IME version",
    "TypeDuck-HK librime fork",
    "rime-dictionary-lookup-filter",
    "TypeDuck-HK schema"
  ) "About attribution order"
  Assert-Ordered $about @(
    "TypeDuck 網站 Website",
    "LearnDuck 粵拼打字入門 Introduction to Cantonese Jyutping Typing",
    "粵拼方案 Jyutping Scheme",
    "TypeDuck 原始碼 Source Code"
  ) "About link order"
  Assert-Text $about "TYPEDUCK_VERSION_TEXT" "About version must use the version.txt-derived build definition."
  Assert-Text $about "3671814d4e4aeab8d616ceea3c7f6d88e96bba02" "Lookup-filter attribution must include recorded commit evidence."
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
