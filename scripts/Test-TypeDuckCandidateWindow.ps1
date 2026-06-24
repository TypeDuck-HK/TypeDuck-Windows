param(
  [string]$RepoRoot = ".",
  [switch]$Strict,
  [ValidateSet("", "CandidateRenderingMissing")]
  [string]$ExpectRed = ""
)

$ErrorActionPreference = "Stop"

function Resolve-RepoPath {
  param([string]$Path)
  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Path))
}

function Assert-File {
  param([string]$Path)
  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    throw "Required file is missing: $Path"
  }
}

function Assert-Dir {
  param([string]$Path)
  if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
    throw "Required directory is missing: $Path"
  }
}

function Assert-Contains {
  param([string]$Path, [string]$Pattern, [string]$Description)
  $text = Get-Content -Raw -LiteralPath $Path
  if ($text -notmatch $Pattern) {
    throw "$Description missing from $Path"
  }
}

function Assert-NotContains {
  param([string]$Path, [string]$Pattern, [string]$Description)
  $text = Get-Content -Raw -LiteralPath $Path
  if ($text -match $Pattern) {
    throw "$Description forbidden in $Path"
  }
}

$Root = Resolve-RepoPath $RepoRoot
$windowHeader = Join-Path $Root "MoqiTextService/MoqiCandidateWindow.h"
$windowSource = Join-Path $Root "MoqiTextService/MoqiCandidateWindow.cpp"
$textServiceHeader = Join-Path $Root "MoqiTextService/MoqiTextService.h"
$textServiceSource = Join-Path $Root "MoqiTextService/MoqiTextService.cpp"
$candidateInfoHeader = Join-Path $Root "MoqiTextService/TypeDuckCandidateInfo.h"
$candidateInfoSource = Join-Path $Root "MoqiTextService/TypeDuckCandidateInfo.cpp"
$previewSource = Join-Path $Root "Preview/main.cpp"
$previewCmake = Join-Path $Root "Preview/CMakeLists.txt"
$fixtureDir = Join-Path $Root ".planning/product/ui-fixtures/phase-05/candidate-preview"
$captureDoc = Join-Path $fixtureDir "capture-commands.md"

Assert-File $windowHeader
Assert-File $windowSource
Assert-File $textServiceHeader
Assert-File $textServiceSource
Assert-File $candidateInfoHeader
Assert-File $candidateInfoSource
Assert-File $previewSource
Assert-File $previewCmake
Assert-Dir $fixtureDir
Assert-File $captureDoc

$tsfPopupFiles = Get-ChildItem -LiteralPath (Join-Path $Root "MoqiTextService") -Include *.cpp,*.h -Recurse
foreach ($file in $tsfPopupFiles) {
  Assert-NotContains $file.FullName '#include\s+<Q|#include\s+"Q|QApplication|QWidget|QPainter|Qt5|Qt6|target_link_libraries\([^)]*Qt' "Qt/Qt toolkit usage in TSF popup path"
}

Assert-Contains $candidateInfoHeader 'struct CandidateInfo' "CandidateInfo model"
Assert-Contains $candidateInfoHeader 'struct CandidateEntry' "CandidateEntry model"
Assert-Contains $candidateInfoSource 'kMaxRawCommentLength|kMaxCsvRowLength|kMaxCsvFieldLength' "bounded candidate parser"
Assert-Contains $previewSource 'TypeDuckCandidateInfo\.h' "preview CandidateInfo model include"
Assert-Contains $previewSource 'MakeNeiSample|MakeHousamSample|MakeReverseLookupSample|MakeMultilingualIndonesianSample' "source-backed preview samples"
Assert-Contains $previewSource 'CandidateInfo' "preview CandidateInfo usage"
Assert-Contains $previewSource 'SavePreviewCaptureCommand|--capture' "documented preview screenshot capture path"
Assert-Contains $previewCmake 'TypeDuckCandidateInfo\.cpp' "preview CandidateInfo CMake wiring"
Assert-Contains $captureDoc 'nei|housam|reverse|multilingual' "candidate preview capture scenarios"

$productionAnchors = @(
  @{ Path = $windowSource; Pattern = 'kTypeDuckCandidatePanelRenderer'; Description = 'TypeDuck candidate renderer marker' },
  @{ Path = $windowSource; Pattern = 'paintInputBuffer|drawInputBuffer'; Description = 'input buffer rendering' },
  @{ Path = $windowSource; Pattern = 'paintPageNavigation|drawPageNavigation'; Description = 'page navigation rendering' },
  @{ Path = $windowSource; Pattern = 'paintCandidateRow|drawCandidateRow'; Description = 'source-backed candidate row rendering' },
  @{ Path = $windowSource; Pattern = 'paintDictionaryPanel|drawDictionaryPanel'; Description = 'dictionary side panel rendering' },
  @{ Path = $windowSource; Pattern = 'More Languages'; Description = 'dictionary More Languages rendering' },
  @{ Path = $windowSource; Pattern = 'movementRevealThreshold_|kMovementRevealThreshold'; Description = 'movement-triggered dictionary reveal threshold' },
  @{ Path = $windowSource; Pattern = 'actualPointerMovement|mouseMoveCount|dictionaryMoveCount'; Description = 'actual pointer movement counter' },
  @{ Path = $windowHeader; Pattern = 'dictionaryRevealIndex_|dictionaryPanel'; Description = 'dictionary panel state' },
  @{ Path = $windowHeader; Pattern = 'lastMouseMovePoint_|lastPointerPoint'; Description = 'stationary pointer tracking' },
  @{ Path = $windowSource; Pattern = 'panel_background|selection_background|pronunciation_text|definition_text'; Description = 'semantic theme role consumption' },
  @{ Path = $windowSource; Pattern = 'definitionLayout|displayLanguages|mainLanguage|otherLanguages'; Description = 'settings-aware display language layout' }
)

$missing = @()
foreach ($anchor in $productionAnchors) {
  $text = Get-Content -Raw -LiteralPath $anchor.Path
  if ($text -notmatch $anchor.Pattern) {
    $missing += $anchor.Description
  }
}

if ($ExpectRed -eq "CandidateRenderingMissing") {
  if ($missing.Count -eq 0) {
    throw "Expected RED CandidateRenderingMissing, but all native TypeDuck renderer anchors are already present."
  }
  Write-Host "PASS RED: CandidateRenderingMissing proved native TypeDuck renderer anchors are absent: $($missing -join ', ')"
  exit 0
}

if ($missing.Count -gt 0) {
  throw "Native TypeDuck candidate rendering anchors missing: $($missing -join ', ')"
}

if ($Strict) {
  Assert-Contains $windowSource 'WS_EX_NOACTIVATE|MA_NOACTIVATE|SWP_NOACTIVATE' "focus-safe non-activating popup behavior"
  Assert-Contains $windowSource 'TrackMouseEvent|WM_MOUSELEAVE' "mouse leave tracking"
  Assert-Contains $windowSource 'GetDpiForWindow|LOGPIXELSX|scalePx' "DPI-aware sizing"
  Assert-Contains $textServiceSource 'clampCandidateWindowToWorkArea|MonitorFromRect|GetMonitorInfo' "multi-monitor work-area placement"
  Assert-Contains $textServiceSource 'fallbackAnchorRect|GetGUIThreadInfo' "composition rectangle fallback"
  Assert-Contains $textServiceSource 'SWP_NOACTIVATE' "non-activating SetWindowPos placement"
  Assert-Contains $textServiceSource 'effectiveUiLess\(\)|shouldShowCandidateWindowUI_' "UI-less host suppression"
}

Write-Host "PASS: TypeDuck native candidate/dictionary popup guard passed."
