#Requires -Version 5.1
<#
.SYNOPSIS
  Validates the TypeDuck appearance theme contract.

.PARAMETER RepoRoot
  Root of moqi-im-windows.

.PARAMETER BackendRoot
  Root of the sibling moqi-ime backend runtime.

.PARAMETER Strict
  Enables all currently implemented guard checks.
#>
param(
    [string] $RepoRoot = ".",
    [string] $BackendRoot = "D:\VSProjects\moqi-ime",
    [switch] $Strict
)

$ErrorActionPreference = "Stop"

function Resolve-FullPath {
    param([string] $Path)
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }
    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Path))
}

function Assert-True {
    param(
        [bool] $Condition,
        [string] $Message
    )
    if (-not $Condition) {
        throw $Message
    }
}

function Assert-ArraySetEquals {
    param(
        [object[]] $Actual,
        [string[]] $Expected,
        [string] $Label
    )
    $actualSorted = @($Actual | Sort-Object)
    $expectedSorted = @($Expected | Sort-Object)
    $actualJoined = $actualSorted -join ","
    $expectedJoined = $expectedSorted -join ","
    if ($actualJoined -ne $expectedJoined) {
        throw "$Label mismatch. Expected [$expectedJoined], got [$actualJoined]."
    }
}

$repoRootPath = Resolve-FullPath -Path $RepoRoot
$backendRootPath = Resolve-FullPath -Path $BackendRoot
$themePath = Join-Path $backendRootPath "input_methods\rime\appearance_themes.json"

Assert-True (Test-Path -LiteralPath $themePath) "Missing canonical TypeDuck appearance theme file: $themePath"

$rawThemeJson = Get-Content -Raw -Encoding UTF8 -LiteralPath $themePath
$themeFile = $rawThemeJson | ConvertFrom-Json

Assert-True ($themeFile.version -ge 2) "appearance_themes.json must use the TypeDuck schema version 2 or newer."
Assert-True ($null -ne $themeFile.fonts) "appearance_themes.json must define top-level fonts."
Assert-True ($null -ne $themeFile.themes) "appearance_themes.json must define themes."
Assert-ArraySetEquals -Actual @($themeFile.themes | ForEach-Object { $_.id }) -Expected @("light", "dark") -Label "Bundled theme IDs"

$requiredPaletteRoles = @(
    "panel_background",
    "dictionary_background",
    "input_buffer_background",
    "panel_border",
    "selection_background",
    "accent",
    "text_primary",
    "text_secondary",
    "pronunciation_text",
    "definition_text",
    "disabled_text",
    "link_text"
)

$bannedThemeIds = @("default", "theme2", "moqi", "purple", "wallgray", "orange", "redplum", "shacheng", "globe", "soymilk", "chrysanthemum", "qinhuangdao", "bubblegum", "pepsi")
$bannedTextPatterns = @("Moqi", "moqi", "墨奇", "默认主题", "橘白", "很有韵味", "墙灰", "橙狗", "老红梅", "沙城老窖", "地球仪", "豆浆杯", "菊花茶", "秦皇岛", "歪比巴卜", "百事可乐")
$bannedPaletteKeys = @("primary", "primary-content", "primary-content-200", "highlighted", "secondary", "secondary-content", "base-100", "base-200", "base-300", "base-400", "base-500", "base-content", "base-content-200", "base-content-300", "base-content-400")

foreach ($theme in $themeFile.themes) {
    Assert-True (-not ($bannedThemeIds -contains $theme.id)) "Banned scaffold theme ID remains: $($theme.id)"
    $themeProperties = @($theme.PSObject.Properties.Name)
    Assert-True (-not ($themeProperties -contains "fonts")) "Theme '$($theme.id)' must not contain font data; fonts belong at the top level."
    Assert-True (-not ($themeProperties -contains "appearance")) "Theme '$($theme.id)' must not use the legacy appearance object."
    Assert-True ($null -ne $theme.palette) "Theme '$($theme.id)' must define a semantic palette."

    $paletteKeys = @($theme.palette.PSObject.Properties.Name)
    foreach ($role in $requiredPaletteRoles) {
        Assert-True ($paletteKeys -contains $role) "Theme '$($theme.id)' is missing semantic palette role '$role'."
    }
    foreach ($key in $paletteKeys) {
        Assert-True (-not ($bannedPaletteKeys -contains $key)) "Theme '$($theme.id)' uses banned Tailwind/DaisyUI token key '$key'."
    }
}

foreach ($pattern in $bannedTextPatterns) {
    Assert-True (-not $rawThemeJson.Contains($pattern)) "Banned scaffold theme text remains in appearance_themes.json: $pattern"
}

$displayLanguages = $themeFile.fonts.display_languages
Assert-True ($null -ne $displayLanguages) "Top-level fonts must include display_languages."
Assert-ArraySetEquals -Actual @($displayLanguages.PSObject.Properties.Name) -Expected @("eng", "hin", "ind", "nep", "urd") -Label "Display-language font IDs"

foreach ($fontKey in @("candidate_chinese_sung", "candidate_chinese_hei", "dictionary_comment")) {
    Assert-True ($null -ne $themeFile.fonts.$fontKey) "Top-level fonts missing '$fontKey'."
}

Write-Host "[PASS] TypeDuck appearance theme schema is semantic, light/dark only, and font data is top-level."
