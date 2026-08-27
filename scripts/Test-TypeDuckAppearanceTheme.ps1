#Requires -Version 5.1
<#
.SYNOPSIS
  Validates the frontend-owned TypeDuck candidate appearance contract.

.PARAMETER RepoRoot
  Root of TypeDuck-Windows.

.PARAMETER BackendRoot
  Deprecated compatibility parameter. TypeDuckAppearance.json is no longer
  read from the backend.

.PARAMETER Strict
  Enables all currently implemented guard checks.
#>
param(
    [string] $RepoRoot = ".",
    [string] $BackendRoot = "",
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
    $actualJoined = (@($Actual) | Sort-Object) -join ","
    $expectedJoined = (@($Expected) | Sort-Object) -join ","
    if ($actualJoined -ne $expectedJoined) {
        throw "$Label mismatch. Expected [$expectedJoined], got [$actualJoined]."
    }
}

$repoRootPath = Resolve-FullPath -Path $RepoRoot
$appearancePath = Join-Path $repoRootPath "configs\TypeDuckAppearance.json"

Assert-True (Test-Path -LiteralPath $appearancePath) "Missing frontend TypeDuck appearance file: $appearancePath"

$rawJson = Get-Content -Raw -Encoding UTF8 -LiteralPath $appearancePath
$appearance = $rawJson | ConvertFrom-Json

Assert-True ($appearance.version -ge 1) "TypeDuckAppearance.json must declare a schema version."
Assert-True ($null -ne $appearance.fonts) "TypeDuckAppearance.json must define top-level fonts."
Assert-True ($null -ne $appearance.themes) "TypeDuckAppearance.json must define themes."
Assert-ArraySetEquals -Actual @($appearance.themes | ForEach-Object { $_.id }) -Expected @("light", "dark") -Label "Bundled theme IDs"

$requiredPaletteRoles = @(
    "panel_background",
    "dictionary_background",
    "input_buffer_background",
    "panel_border",
    "selection_background",
    "item_text",
    "label_text",
    "pronunciation_text",
    "definition_text",
    "metalanguage_text",
    "disabled_text",
    "active_text",
    "pos_pill_border",
    "dictionary_scroll_track",
    "dictionary_scroll_thumb"
)

foreach ($theme in $appearance.themes) {
    $themeProperties = @($theme.PSObject.Properties.Name)
    Assert-True (-not ($themeProperties -contains "fonts")) "Theme '$($theme.id)' must not contain font data; fonts belong at the top level."
    Assert-True ($null -ne $theme.palette) "Theme '$($theme.id)' must define a role-based palette."
    $paletteKeys = @($theme.palette.PSObject.Properties.Name)
    foreach ($role in $requiredPaletteRoles) {
        Assert-True ($paletteKeys -contains $role) "Theme '$($theme.id)' is missing palette role '$role'."
    }
    foreach ($removedRole in @("pos_pill_background", "pos_pill_text", "selection_text", "link_hover_text", "tooltip_background", "tooltip_text")) {
        Assert-True (-not ($paletteKeys -contains $removedRole)) "Theme '$($theme.id)' must not define removed palette role '$removedRole'."
    }
}

$displayLanguages = $appearance.fonts.display_languages
Assert-True ($null -ne $displayLanguages) "Top-level fonts must include display_languages."
Assert-ArraySetEquals -Actual @($displayLanguages.PSObject.Properties.Name) -Expected @("eng", "hin", "ind", "nep", "urd") -Label "Display-language font IDs"

foreach ($fontKey in @("default_interface", "selection_label", "chinese_sung", "chinese_hei")) {
    Assert-True ($null -ne $appearance.fonts.$fontKey) "Top-level fonts missing '$fontKey'."
}

foreach ($fontKey in @("candidate_chinese_sung", "candidate_chinese_hei", "dictionary_headword_sung", "dictionary_headword_hei", "dictionary_comment")) {
    Assert-True ($null -eq $appearance.fonts.$fontKey) "Removed font role remains: '$fontKey'."
}

Assert-True ($rawJson -notmatch '"point_size"\s*:') "TypeDuckAppearance.json must not contain point_size keys."
Assert-True ($rawJson -notmatch '"lang"\s*:') "TypeDuckAppearance.json must not contain lang keys."
Assert-True ($rawJson -notmatch '"label"\s*:') "TypeDuckAppearance.json must not contain label keys; use description."

$textServiceSource = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $repoRootPath "MoqiTextService\MoqiTextService.cpp")
$installScript = Get-Content -Raw -Encoding UTF8 -LiteralPath (Join-Path $repoRootPath "scripts\install.ps1")

Assert-True ($textServiceSource.Contains("configs\\TypeDuckAppearance.json")) "TextService must load the frontend TypeDuckAppearance.json asset."
Assert-True (-not $textServiceSource.Contains("appearance_themes.json")) "TextService must not load backend appearance_themes.json."
Assert-True (-not $textServiceSource.Contains("ProgramFiles(x86)")) "TextService must not guess the install path from Program Files."
Assert-True ($textServiceSource.Contains("programDir()")) "TextService must resolve appearance from the configured app directory."
Assert-True ($installScript.Contains("configs")) "Installer staging must create/copy the frontend configs folder."
Assert-True ($installScript.Contains("TypeDuckAppearance.json")) "Installer staging must include TypeDuckAppearance.json."

Write-Host "[PASS] TypeDuckAppearance.json is frontend-owned, role-based, and packaged under configs."
