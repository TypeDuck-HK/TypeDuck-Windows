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
$settingsOrderPath = Join-Path $repo ".planning/product/web-alpha-fixtures/2026-06-23/settings-order.json"

Assert-True (Test-Path -LiteralPath $settingsCmakePath) "TypeDuckSettings/CMakeLists.txt is missing."
Assert-True (Test-Path -LiteralPath $windowPath) "TypeDuckSettingsWindow.cpp is missing."
Assert-True (Test-Path -LiteralPath $mainPath) "TypeDuckSettings main.cpp is missing."
Assert-True (Test-Path -LiteralPath $rcPath) "TypeDuckSettings.rc is missing."
Assert-True (Test-Path -LiteralPath $resourcePath) "TypeDuckSettings resource.h is missing."

$settingsCmake = Get-Content -Raw -Encoding UTF8 -LiteralPath $settingsCmakePath
$window = Get-Content -Raw -Encoding UTF8 -LiteralPath $windowPath
$preferences = Get-Content -Raw -Encoding UTF8 -LiteralPath $preferencesPath
$fixture = Get-Content -Raw -Encoding UTF8 -LiteralPath $settingsOrderPath | ConvertFrom-Json

Assert-Text $topCmake "add_subdirectory\(.+TypeDuckSettings" "Top-level CMake must include TypeDuckSettings."
Assert-Text $settingsCmake "add_executable\(TypeDuckSettings\s+WIN32" "TypeDuckSettings must be a native Win32 executable."
Assert-Text $settingsCmake "MoqLauncher/TypeDuckPreferences\.cpp" "Settings executable must reuse TypeDuckPreferences."
Assert-Text $settingsCmake "TypeDuckSettingsVersion\.h\.in" "TypeDuckSettings must generate a version header from version.txt-derived CMake values."

$combined = "$topCmake`n$settingsCmake`n$window"
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
  Assert-Ordered $about @(
    "IDB_TYPEDUCK_ABOUT_BANNER",
    "歡迎使用 TypeDuck 打得",
    "IDB_TYPEDUCK_CREDIT_LOGOS",
    "TypeDuck Windows IME version",
    "TypeDuck-HK librime fork",
    "rime-dictionary-lookup-filter",
    "TypeDuck-HK schema",
    "TypeDuck 網站 Website",
    "LearnDuck 粵拼打字入門 Introduction to Cantonese Jyutping Typing",
    "粵拼方案 Jyutping Scheme",
    "TypeDuck 原始碼 Source Code"
  ) "About dialog resource/text/attribution/link order"
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
