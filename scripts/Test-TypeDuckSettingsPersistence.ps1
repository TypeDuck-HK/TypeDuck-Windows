param(
  [Parameter(Mandatory = $true)][string]$RepoRoot,
  [Parameter(Mandatory = $true)][string]$BackendRoot,
  [switch]$Strict,
  [ValidateSet("PreferencesMissing")][string]$ExpectRed
)

$ErrorActionPreference = "Stop"

function Resolve-FullPath([string]$Path) {
  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Path))
}

function Assert-File([string]$Path) {
  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    throw "Required file is missing: $Path"
  }
}

function Assert-Text([string]$Path, [string]$Pattern, [string]$Message) {
  $text = Get-Content -Raw -Encoding UTF8 -LiteralPath $Path
  if ($text -notmatch $Pattern) {
    throw $Message
  }
}

$repo = Resolve-FullPath $RepoRoot
$backend = Resolve-FullPath $BackendRoot

$testFile = Join-Path $repo "Tests/TypeDuckSettings/TypeDuckPreferences_test.cpp"
$cmakeFile = Join-Path $repo "Tests/TypeDuckSettings/CMakeLists.txt"
$preferencesHeader = Join-Path $repo "MoqLauncher/TypeDuckPreferences.h"
$preferencesSource = Join-Path $repo "MoqLauncher/TypeDuckPreferences.cpp"
$backendGuard = Join-Path $backend "scripts/Test-TypeDuckSettingsCustomization.ps1"
$backendAppearance = Join-Path $backend "input_methods/rime/appearance_config.go"
$backendRime = Join-Path $backend "input_methods/rime/rime.go"

Assert-File $testFile
Assert-File $cmakeFile
Assert-File $backendAppearance
Assert-File $backendRime
Assert-File $backendGuard

Assert-Text $testFile "D-17" "Preference tests must cite D-17."
Assert-Text $testFile "D-20" "Preference tests must cite D-20."
Assert-Text $testFile "D-21" "Preference tests must cite D-21."
Assert-Text $testFile "D-42" "Preference tests must cite D-42."
Assert-Text $testFile "D-43" "Preference tests must cite D-43."
Assert-Text $testFile "D-44" "Preference tests must cite D-44."
Assert-Text $testFile "D-45" "Preference tests must cite D-45."
Assert-Text $testFile "D-46" "Preference tests must cite D-46."
Assert-Text $testFile "D-47" "Preference tests must cite D-47."

Assert-Text $testFile "hooks\.ts|DEFAULT_PREFERENCES" "Tests must compare against TypeDuck Web hooks.ts DEFAULT_PREFERENCES."
Assert-Text $testFile "types\.ts" "Tests must cite TypeDuck Web types.ts split."
Assert-Text $testFile "worker\.ts|api\.cpp" "Tests must cite TypeDuck Web worker/api customization mapping."
Assert-Text $testFile "common\.yaml" "Tests must cite TypeDuck Web common.yaml patch consumption."
Assert-Text $testFile "menu/page_size" "Tests must assert pageSize writes menu/page_size."

if ($ExpectRed -eq "PreferencesMissing") {
  if ((Test-Path -LiteralPath $preferencesHeader) -or (Test-Path -LiteralPath $preferencesSource)) {
    throw "Expected RED PreferencesMissing, but TypeDuckPreferences implementation exists."
  }
  Write-Host "PASS RED: TypeDuckPreferences implementation is absent and tests/guards are present."
  exit 0
}

Assert-File $preferencesHeader
Assert-File $preferencesSource

$sourceText = Get-Content -Raw -Encoding UTF8 -LiteralPath $preferencesSource
if ($sourceText -match "common\.custom\.yaml.*persist|default\.custom\.yaml.*persist") {
  throw "Generated YAML must not be treated as persisted preferences database."
}
if ($sourceText -notmatch "TypeDuckPreferences\.json") {
  throw "Preferences source must persist to a TypeDuck-owned JSON file."
}
if ($sourceText -notmatch "menu/page_size") {
  throw "pageSize must map to the Rime menu/page_size path."
}
if ($sourceText -notmatch "displayLanguages[\s\S]*mainLanguage[\s\S]*isHeiTypeface[\s\S]*showRomanization[\s\S]*showReverseCode") {
  throw "Interface-only settings must be explicit in the native model."
}
if ($sourceText -notmatch "設定未能套用[\s\S]*Settings could not be applied") {
  throw "Apply failure state must be bounded and bilingual."
}

if ($Strict) {
  if ($sourceText -match "displayLanguages[\s\S]{0,120}common:/|mainLanguage[\s\S]{0,120}common:/|isHeiTypeface[\s\S]{0,120}menu/page_size|showRomanization[\s\S]{0,120}common:/|showReverseCode[\s\S]{0,120}common:/") {
    throw "Interface-only settings must not trigger Rime customization."
  }
}

Write-Host "PASS: TypeDuck settings persistence guard passed."
