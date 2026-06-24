param(
  [string]$RepoRoot = ".",
  [switch]$Strict,
  [ValidateSet("", "CandidateInfoMissing")]
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

function Assert-Contains {
  param([string]$Path, [string]$Pattern, [string]$Description)
  $text = Get-Content -Raw -LiteralPath $Path
  if ($text -notmatch $Pattern) {
    throw "$Description missing from $Path"
  }
}

$Root = Resolve-RepoPath $RepoRoot
$fixturePath = Join-Path $Root ".planning/product/candidate-fixtures/phase-05/candidate-data-contract.json"
$testPath = Join-Path $Root "Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp"
$testCmakePath = Join-Path $Root "Tests/TypeDuckCandidateData/CMakeLists.txt"
$rootCmakePath = Join-Path $Root "CMakeLists.txt"
$headerPath = Join-Path $Root "MoqiTextService/TypeDuckCandidateInfo.h"
$sourcePath = Join-Path $Root "MoqiTextService/TypeDuckCandidateInfo.cpp"
$windowHeaderPath = Join-Path $Root "MoqiTextService/MoqiCandidateWindow.h"
$windowSourcePath = Join-Path $Root "MoqiTextService/MoqiCandidateWindow.cpp"

Assert-File $fixturePath
Assert-File $testPath
Assert-File $testCmakePath
Assert-File $rootCmakePath
Assert-File $windowHeaderPath
Assert-File $windowSourcePath

$fixture = Get-Content -Raw -Encoding UTF8 -LiteralPath $fixturePath | ConvertFrom-Json
$expectedHeader = @(
  "match_input_buffer", "honzi", "jyutping", "canonical_honzi", "canonical_jyutping",
  "components_honzi", "components_jyutping", "pron_label", "lit_col_reading", "pos",
  "register", "label", "written_form", "vernacular_form", "collocation",
  "eng", "hin", "urd", "nep", "ind"
)
if (@($fixture.lookupFilterHeader).Count -ne $expectedHeader.Count) {
  throw "D-10 lookup-filter header length mismatch in candidate-data-contract.json"
}
for ($i = 0; $i -lt $expectedHeader.Count; $i++) {
  if ($fixture.lookupFilterHeader[$i] -ne $expectedHeader[$i]) {
    throw "D-10 lookup-filter header mismatch at index $i"
  }
}
if ($fixture.controlSeparators.leadingReverseLookupMarker -ne "`v" -or
    $fixture.controlSeparators.noteSeparator -ne "`f" -or
    $fixture.controlSeparators.dictionaryRowSeparator -ne "`r") {
  throw "D-11 control separator contract mismatch"
}

Assert-Contains $rootCmakePath 'Tests/TypeDuckCandidateData' "Candidate data CMake subtree registration"
Assert-Contains $testCmakePath 'TypeDuckCandidateInfo_test' "CandidateInfo test target"
Assert-Contains $testPath 'MoqiTextService/TypeDuckCandidateInfo\.h' "CandidateInfo include"
Assert-Contains $testPath 'PreservesLookupFilterControlSeparatorSemantics' "D-11 separator test"
Assert-Contains $testPath 'ParsesCsvRowsWithLookupFilterHeaderOrderAndQuotes' "D-12 CSV test"
Assert-Contains $testPath 'MapsNeiToCandidateDictionaryAndMoreLanguages' "CAND-02/CAND-03 nei test"
Assert-Contains $testPath 'MapsCompoundHousamToMultipleDictionaryEntries' "D-33 compound test"
Assert-Contains $testPath 'CarriesJyutpingVisibilityAndTypefacePreferences' "CAND-04/CAND-05 preference test"

$forbiddenFiles = @(
  "MoqiTextService/MoqiClient.cpp",
  "MoqLauncher/PipeServer.cpp",
  "MoqLauncher/BackendServer.cpp",
  "proto/moqi.proto"
)
foreach ($relative in $forbiddenFiles) {
  $path = Join-Path $Root $relative
  if (Test-Path -LiteralPath $path -PathType Leaf) {
    $text = Get-Content -Raw -LiteralPath $path
    if ($text -match 'ConsumedString|lookupFilterHeader|match_input_buffer|TypeDuckCandidateInfo') {
      throw "D-39 violation: candidate dictionary parsing appears in transport file $relative"
    }
  }
}

if ($ExpectRed -eq "CandidateInfoMissing") {
  if ((Test-Path -LiteralPath $headerPath) -or (Test-Path -LiteralPath $sourcePath)) {
    throw "Expected RED CandidateInfoMissing, but native CandidateInfo production files already exist."
  }
  Write-Host "PASS: RED guard proved CandidateInfo/CandidateEntry production files are missing while tests and D-32..D-41 contracts are present."
  exit 0
}

Assert-File $headerPath
Assert-File $sourcePath
Assert-Contains $headerPath 'struct CandidateInfo|class CandidateInfo' "CandidateInfo native model"
Assert-Contains $headerPath 'struct CandidateEntry|class CandidateEntry' "CandidateEntry native model"
Assert-Contains $sourcePath 'ConsumedString' "ConsumedString-style helper"
Assert-Contains $sourcePath 'match_input_buffer' "D-10 header mapping"
Assert-Contains $sourcePath 'LANGUAGE_LABELS|languageLabel' "D-41 language labels"
Assert-Contains $sourcePath 'kMaxRawCommentLength|kMaxCsvRowLength|kMaxCsvFieldLength' "Bounded untrusted parser limits"
Assert-Contains $windowHeaderPath 'TypeDuckCandidateInfo\.h' "Candidate window CandidateInfo boundary"
Assert-Contains $windowHeaderPath 'DisplayPreferences' "Candidate window display preferences"
Assert-Contains $windowSourcePath 'CandidateInfo' "Candidate window consumes CandidateInfo"

if ($Strict) {
  $buildDir = Join-Path $Root "build-vs32"
  $testExe = Join-Path $buildDir "Tests/TypeDuckCandidateData/Debug/TypeDuckCandidateInfo_test.exe"
  if (-not (Test-Path -LiteralPath $testExe -PathType Leaf)) {
    throw "Strict mode expected built test executable at $testExe. Run the CMake build first."
  }
  & $testExe
  if ($LASTEXITCODE -ne 0) {
    throw "TypeDuckCandidateInfo_test.exe failed with exit code $LASTEXITCODE"
  }
}

Write-Host "PASS: TypeDuck candidate data parser/model guard passed."
