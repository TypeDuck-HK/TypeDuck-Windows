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
$clientSource = Join-Path $Root "MoqiTextService/MoqiClient.cpp"
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
Assert-File $clientSource
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
Assert-Contains $candidateInfoSource 'formattedPartsOfSpeech' "structured part-of-speech mapping"
Assert-Contains $previewSource 'TypeDuckCandidateInfo\.h' "preview CandidateInfo model include"
Assert-Contains $previewSource 'MakeNeiSample|MakeHousamSample|MakeReverseLookupSample|MakeMultilingualIndonesianSample' "source-backed preview samples"
Assert-Contains $previewSource 'CandidateInfo' "preview CandidateInfo usage"
Assert-Contains $previewSource 'SavePreviewCaptureCommand|--capture' "documented preview screenshot capture path"
Assert-Contains $previewCmake 'TypeDuckCandidateInfo\.cpp' "preview CandidateInfo CMake wiring"
Assert-Contains $captureDoc 'nei|housam|reverse|multilingual' "candidate preview capture scenarios"
Assert-Contains $captureDoc 'stale TypeDuck 1\.1\.2|runtime-provenance' "candidate preview VM divergence note"

$productionAnchors = @(
  @{ Path = $windowSource; Pattern = 'kTypeDuckCandidatePanelRenderer'; Description = 'TypeDuck candidate renderer marker' },
  @{ Path = $windowSource; Pattern = 'paintInputBuffer|drawInputBuffer'; Description = 'input buffer rendering' },
  @{ Path = $windowSource; Pattern = 'paintPageNavigation|drawPageNavigation'; Description = 'page navigation rendering' },
  @{ Path = $windowSource; Pattern = 'paintCandidateRow|drawCandidateRow'; Description = 'source-backed candidate row rendering' },
  @{ Path = $windowSource; Pattern = 'paintDictionaryPanel|drawDictionaryPanel'; Description = 'dictionary side panel rendering' },
  @{ Path = $windowSource; Pattern = 'paintPartOfSpeechPills|drawPartOfSpeechPills'; Description = 'part-of-speech pill rendering' },
  @{ Path = $windowSource; Pattern = 'kPosPillBorder|kPosPillText|kPosPillBackground'; Description = 'part-of-speech pill theme colors' },
  @{ Path = $windowSource; Pattern = 'More Languages'; Description = 'dictionary More Languages rendering' },
  @{ Path = $windowSource; Pattern = 'entryRowCount|matchedEntries'; Description = 'multi-row candidate detail rendering' },
  @{ Path = $windowSource; Pattern = 'movementRevealThreshold_|kMovementRevealThreshold'; Description = 'movement-triggered dictionary reveal threshold' },
  @{ Path = $windowSource; Pattern = 'actualPointerMovement|mouseMoveCount|dictionaryMoveCount'; Description = 'actual pointer movement counter' },
  @{ Path = $windowHeader; Pattern = 'dictionaryRevealIndex_|dictionaryPanel'; Description = 'dictionary panel state' },
  @{ Path = $windowHeader; Pattern = 'lastMouseMovePoint_|lastPointerPoint'; Description = 'stationary pointer tracking' },
  @{ Path = $windowSource; Pattern = 'panel_background|selection_background|input_buffer_background|input_buffer_text|pronunciation_text|definition_text'; Description = 'theme palette role consumption' },
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
  $requiredCaptures = @(
    "nei-light.bmp",
    "multilingual-indonesian-main.bmp",
    "housam-compound.bmp",
    "reverse-cangjie-onf.bmp",
    "edge-clamp.bmp",
    "high-dpi.bmp",
    "fallback-anchor.bmp"
  )
  foreach ($capture in $requiredCaptures) {
    $capturePath = Join-Path $fixtureDir $capture
    Assert-File $capturePath
    $captureItem = Get-Item -LiteralPath $capturePath
    if ($captureItem.Length -lt 100000) {
      throw "Candidate preview capture looks empty or truncated: $capturePath ($($captureItem.Length) bytes)"
    }
  }
  Assert-NotContains $windowSource 'L"\\\[" \+ part|body \+= L"\\\["|\\[[^\\]]*形容詞' "literal bracketed POS rendering in native candidate window"
  Assert-NotContains $previewSource 'L"\\\[" \+ pos|body \+= L"\\\["|\\[[^\\]]*形容詞' "literal bracketed POS rendering in preview harness"
  Assert-Contains $windowSource 'DT_VCENTER|candidateBaseline|baselineAligned' "candidate row baseline alignment guard"
  Assert-Contains $windowSource 'DT_CALCRECT|GetTextExtentPoint32W' "input buffer measured text guard"
  Assert-Contains $previewSource 'candidate-data-contract|runtime-provenance|TypeDuck-1\.1\.2' "preview provenance/divergence documentation anchors"
  Assert-Contains $clientSource 'rawLookupComment' "candidate raw lookup comment preservation"
  Assert-Contains $clientSource 'inputCode' "candidate input-code preservation"
  Assert-Contains $clientSource 'jyutping' "candidate Jyutping fallback preservation"
  Assert-Contains $windowHeader 'std::wstring inputCode' "candidate UI fallback input-code field"
  Assert-Contains $windowSource 'CandidateInfo\(\s*label,\s*item\.text,\s*rawComment,\s*item\.inputCode\)' "candidate info input-code fallback wiring"
  Assert-Contains $windowSource 'jyutpingContentWidth|definitionContentWidth|honziContentWidth' "candidate window must measure visible content columns"
  Assert-Contains $windowSource 'minWidth_\s*=\s*scalePx\(kCandidateMinWidth\)' "candidate window must keep compact content-sized minimum width"
  Assert-Contains $windowSource 'entryIndex\s*==\s*0\s*&&\s*!note\.empty\(\)\s*&&\s*noteColumnWidth_' "candidate notes must follow TypeDuck Web first-row reverse-code visibility without depending on Jyutping display"
  Assert-Contains $windowSource 'nextColumn\(jyutpingColumnWidth_\)' "candidate row painter must use measured dynamic columns"
  Assert-Contains $clientSource 'selStart|selEnd|setCandidatePreeditSelection' "candidate input buffer must consume backend active selection range"
  Assert-Contains $windowHeader 'preeditSelectionStart_|preeditSelectionEnd_' "candidate window must store split input-buffer selection"
  Assert-Contains $windowSource 'const std::wstring before|const std::wstring active|const std::wstring after' "input buffer must draw before/active/after spans separately"
  Assert-Contains $windowSource 'CreateRoundRectRgn\(activeRc\.left' "input buffer highlight must be limited to the active span"
  Assert-NotContains $clientSource 'setCandBackgroundColor\(color\)|setCandHighlightColor\(color\)|setCandTextColor\(color\)|setCandHighlightTextColor\(color\)|setCandCommentColor\(color\)|setCandCommentHighlightColor\(color\)' "backend-driven native candidate theme color override"
  Assert-NotContains $textServiceHeader 'setPreeditText\(effectiveInlinePreedit\(\) \? L"" : candidatePreedit_\)|setPreeditCursor\(effectiveInlinePreedit\(\) \? 0 : candidatePreeditCursor_\)' "inline preedit hiding the popup input buffer in header"
  Assert-NotContains $textServiceSource 'setPreeditText\(effectiveInlinePreedit\(\) \? L"" : candidatePreedit_\)|setPreeditCursor\(effectiveInlinePreedit\(\) \? 0 : candidatePreeditCursor_\)' "inline preedit hiding the popup input buffer in source"
  Assert-Contains $windowSource 'WS_EX_NOACTIVATE|MA_NOACTIVATE|SWP_NOACTIVATE' "focus-safe non-activating popup behavior"
  Assert-Contains $windowSource 'TrackMouseEvent|WM_MOUSELEAVE' "mouse leave tracking"
  Assert-Contains $windowSource 'hitTestPageNavigation' "candidate page navigation must have mouse hit testing"
  Assert-Contains $windowSource 'candidateHasPrevious|candidateHasNext' "candidate page navigation must consume backend page availability"
  Assert-Contains $clientSource 'setCandidateHasPrevious|setCandidateHasNext' "backend page availability must be applied to the text service"
  Assert-Contains $clientSource '(?s)bool\s+Client::selectCandidate\(int index\).*?TF_ES_ASYNCDONTCARE \| TF_ES_READWRITE' "Word candidate click regression: popup mouse selectCandidate must use async edit session because Word drops synchronous popup selection"
  Assert-Contains $windowSource 'GetDpiForWindow|LOGPIXELSX|scalePx' "DPI-aware sizing"
  Assert-Contains $windowSource 'dpiForOwnerWindow|createPointFontForDpi|ThreadDpiAwarenessScope' "candidate popup must use owner DPI instead of host-virtualized window DC sizing"
  Assert-Contains $windowSource 'resolveFontFace|EnumFontFamiliesExW|candFontName' "candidate popup DPI-owned fonts must preserve configured Chinese font fallback lists"
  Assert-Contains $textServiceSource 'clampCandidateWindowToWorkArea|MonitorFromRect|GetMonitorInfo' "multi-monitor work-area placement"
  Assert-Contains $textServiceSource 'fallbackAnchorRect|GetGUIThreadInfo' "composition rectangle fallback"
  Assert-Contains $textServiceSource 'SWP_NOACTIVATE' "non-activating SetWindowPos placement"
  Assert-Contains $textServiceSource 'effectiveUiLess\(\)|shouldShowCandidateWindowUI_' "UI-less host suppression"
}

Write-Host "PASS: TypeDuck native candidate/dictionary popup guard passed."
