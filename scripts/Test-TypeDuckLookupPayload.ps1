#Requires -Version 5.1
<#
.SYNOPSIS
  Validate the Phase 2 TypeDuck lookup-filter raw payload evidence.

.PARAMETER TypingProofPath
  Plan 02-02 typing-proof.json path.

.PARAMETER RawCommentsPath
  NDJSON evidence path containing raw candidate comment payload records.

.PARAMETER CsvRowsPath
  CSV evidence path containing raw dictionary rows extracted from comments.

.PARAMETER ValidationPath
  JSON validation report path to write.
#>
param(
  [string] $TypingProofPath = ".planning\product\engine-runtime-fixtures\phase-02\typing-proof.json",
  [string] $RawCommentsPath = ".planning\product\engine-runtime-fixtures\phase-02\lookup-filter-raw-comments.ndjson",
  [string] $CsvRowsPath = ".planning\product\engine-runtime-fixtures\phase-02\lookup-filter-csv-rows.csv",
  [string] $ValidationPath = ".planning\product\engine-runtime-fixtures\phase-02\lookup-filter-validation.json"
)

$ErrorActionPreference = "Stop"

[void][System.Reflection.Assembly]::LoadWithPartialName("Microsoft.VisualBasic")

$ExpectedColumns = @(
  "match_input_buffer",
  "honzi",
  "jyutping",
  "canonical_honzi",
  "canonical_jyutping",
  "components_honzi",
  "components_jyutping",
  "pron_label",
  "lit_col_reading",
  "pos",
  "register",
  "label",
  "written_form",
  "vernacular_form",
  "collocation",
  "eng",
  "hin",
  "urd",
  "nep",
  "ind"
)
$ExpectedHeader = $ExpectedColumns -join ","
$VerticalTab = [string][char]0x0b
$FormFeed = [string][char]0x0c
$CarriageReturn = [string][char]0x0d
$LineFeed = [string][char]0x0a

$checks = New-Object System.Collections.Generic.List[object]
$failures = New-Object System.Collections.Generic.List[string]

function Add-Check {
  param(
    [string] $Name,
    [bool] $Passed,
    [string] $Detail
  )

  $checks.Add([ordered] @{
    name = $Name
    passed = $Passed
    detail = $Detail
  }) | Out-Null
  if (-not $Passed) {
    $failures.Add($Detail) | Out-Null
  }
}

function Resolve-LookupPath {
  param([string] $Path)

  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path (Get-Location).Path $Path))
}

function Get-Sha256Text {
  param([string] $Text)

  $sha = [System.Security.Cryptography.SHA256]::Create()
  try {
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($Text)
    return (($sha.ComputeHash($bytes) | ForEach-Object { $_.ToString("x2") }) -join "")
  }
  finally {
    $sha.Dispose()
  }
}

function Get-Utf8Hex {
  param([string] $Text)

  return (([System.Text.Encoding]::UTF8.GetBytes($Text) | ForEach-Object { $_.ToString("x2") }) -join " ")
}

function Get-ControlCounts {
  param([string] $Text)

  return [ordered] @{
    verticalTab = ($Text.ToCharArray() | Where-Object { [int]$_ -eq 0x0b }).Count
    formFeed = ($Text.ToCharArray() | Where-Object { [int]$_ -eq 0x0c }).Count
    carriageReturn = ($Text.ToCharArray() | Where-Object { [int]$_ -eq 0x0d }).Count
  }
}

function ConvertTo-EscapedComment {
  param([string] $Text)

  $builder = New-Object System.Text.StringBuilder
  foreach ($ch in $Text.ToCharArray()) {
    switch ([int]$ch) {
      0x0b { [void]$builder.Append("\v"); break }
      0x0c { [void]$builder.Append("\f"); break }
      0x0d { [void]$builder.Append("\r"); break }
      0x0a { [void]$builder.Append("\n"); break }
      default {
        if ($ch -eq "\") {
          [void]$builder.Append("\\")
        } else {
          [void]$builder.Append($ch)
        }
      }
    }
  }
  return $builder.ToString()
}

function Parse-CsvLine {
  param([string] $Line)

  $reader = New-Object System.IO.StringReader($Line)
  $parser = New-Object Microsoft.VisualBasic.FileIO.TextFieldParser($reader)
  try {
    $parser.TextFieldType = [Microsoft.VisualBasic.FileIO.FieldType]::Delimited
    $parser.SetDelimiters(",")
    $parser.HasFieldsEnclosedInQuotes = $true
    $fields = $parser.ReadFields()
    if (-not $parser.EndOfData) {
      throw "CSV row contained an unexpected embedded line break."
    }
    return @($fields)
  }
  finally {
    $parser.Close()
    $reader.Close()
  }
}

function Parse-LookupComment {
  param([string] $RawComment)

  if ($null -eq $RawComment) {
    throw "Raw comment is null."
  }

  $body = $RawComment
  $hasReverseMarker = $body.StartsWith($VerticalTab, [System.StringComparison]::Ordinal)
  if ($hasReverseMarker) {
    $body = $body.Substring(1)
  }

  $firstNoteSeparator = $body.IndexOf($FormFeed, [System.StringComparison]::Ordinal)
  if ($firstNoteSeparator -lt 0) {
    throw "Raw comment is missing D-11 note separator \f."
  }

  $note = $body.Substring(0, $firstNoteSeparator)
  $tail = $body.Substring($firstNoteSeparator + 1)
  $isDictionaryPayload = $tail.StartsWith($CarriageReturn, [System.StringComparison]::Ordinal)
  if ($isDictionaryPayload) {
    $rowText = $tail.Substring(1)
    if ([string]::IsNullOrEmpty($rowText)) {
      throw "Dictionary payload starts with \r but has no CSV rows."
    }
    $rows = @($rowText.Split([char]0x0d) | Where-Object { $_ -ne "" })
    foreach ($row in $rows) {
      $fields = Parse-CsvLine -Line $row
      if ($fields.Count -ne $ExpectedColumns.Count) {
        throw "Dictionary CSV row has $($fields.Count) columns, expected $($ExpectedColumns.Count): $row"
      }
    }
    return [ordered] @{
      hasLeadingReverseLookupMarker = $hasReverseMarker
      note = $note
      mode = "dictionaryRows"
      dictionaryRows = $rows
      jyutpingOnlySegments = @()
    }
  }

  $segments = @()
  if ($tail.Length -gt 0) {
    $segments = @($tail.Split([char]0x0c))
  }
  return [ordered] @{
    hasLeadingReverseLookupMarker = $hasReverseMarker
    note = $note
    mode = "jyutpingOnly"
    dictionaryRows = @()
    jyutpingOnlySegments = $segments
  }
}

function Read-JsonFile {
  param([string] $Path)

  return Get-Content -Raw -LiteralPath $Path -Encoding UTF8 | ConvertFrom-Json
}

function Read-Ndjson {
  param([string] $Path)

  $records = New-Object System.Collections.Generic.List[object]
  $lineNumber = 0
  foreach ($line in Get-Content -LiteralPath $Path -Encoding UTF8) {
    $lineNumber++
    if ([string]::IsNullOrWhiteSpace($line)) {
      continue
    }
    try {
      $record = $line | ConvertFrom-Json
      $record | Add-Member -NotePropertyName lineNumber -NotePropertyValue $lineNumber -Force
      $records.Add($record) | Out-Null
    } catch {
      throw "Invalid NDJSON at line ${lineNumber}: $($_.Exception.Message)"
    }
  }
  return $records.ToArray()
}

function Get-TypingProofComments {
  param([object] $Proof)

  $comments = New-Object System.Collections.Generic.List[object]
  foreach ($inputItem in @($Proof.inputs)) {
    $index = 0
    foreach ($candidate in @($inputItem.candidates)) {
      $comments.Add([ordered] @{
        inputName = [string]$inputItem.name
        candidateIndex = $index
        candidateText = [string]$candidate.text
        rawComment = [string]$candidate.comment
      }) | Out-Null
      $index++
    }
  }
  return $comments.ToArray()
}

function Test-InternalParserCases {
  $quoted = '1,"quote ""inside""",nei5,,,,,,,oth,,,,,,meaning,hin,urd,nep,'
  $quotedFields = Parse-CsvLine -Line $quoted
  if ($quotedFields.Count -ne $ExpectedColumns.Count) {
    throw "Internal CSV parser rejected a quoted row with empty fields."
  }
  if ($quotedFields[1] -ne 'quote "inside"') {
    throw "Internal CSV parser failed doubled quote handling."
  }
  if ($quotedFields[19] -ne "") {
    throw "Internal CSV parser failed trailing empty field handling."
  }

  $dictionarySample = $VerticalTab + "note" + $FormFeed + $CarriageReturn + $quoted
  $dictionaryParsed = Parse-LookupComment -RawComment $dictionarySample
  if (-not $dictionaryParsed.hasLeadingReverseLookupMarker -or $dictionaryParsed.mode -ne "dictionaryRows") {
    throw "Internal separator parser failed \v + \f + \r dictionary semantics."
  }

  $jyutpingOnlySample = "note" + $FormFeed + "nei5" + $FormFeed + "ni1"
  $jyutpingParsed = Parse-LookupComment -RawComment $jyutpingOnlySample
  if ($jyutpingParsed.mode -ne "jyutpingOnly" -or $jyutpingParsed.jyutpingOnlySegments.Count -ne 2) {
    throw "Internal separator parser failed Jyutping-only \f semantics."
  }
}

function Write-ValidationReport {
  param(
    [string] $Path,
    [object] $Report
  )

  $dir = Split-Path -Parent $Path
  if ($dir) {
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
  }
  $json = $Report | ConvertTo-Json -Depth 20
  Set-Content -LiteralPath $Path -Encoding UTF8 -Value $json
}

$typingProofFull = Resolve-LookupPath -Path $TypingProofPath
$rawCommentsFull = Resolve-LookupPath -Path $RawCommentsPath
$csvRowsFull = Resolve-LookupPath -Path $CsvRowsPath
$validationFull = Resolve-LookupPath -Path $ValidationPath
$summary = [ordered] @{
  rawCommentRecords = 0
  dictionaryRecordCount = 0
  jyutpingOnlyRecordCount = 0
  csvRowCount = 0
  extractedDictionaryRows = 0
  inputs = @()
}

try {
  Test-InternalParserCases
  Add-Check -Name "parser-self-tests" -Passed $true -Detail "CSV quote/empty-field parsing and D-11 \v/\f/\r separator parsing passed."
} catch {
  Add-Check -Name "parser-self-tests" -Passed $false -Detail $_.Exception.Message
}

$proof = $null
$proofComments = @()
if (-not (Test-Path -LiteralPath $typingProofFull -PathType Leaf)) {
  Add-Check -Name "typing-proof-exists" -Passed $false -Detail "Missing typing proof: $TypingProofPath"
} else {
  try {
    $proof = Read-JsonFile -Path $typingProofFull
    $proofComments = @(Get-TypingProofComments -Proof $proof)
    Add-Check -Name "typing-proof-exists" -Passed $true -Detail "Loaded $($proofComments.Count) candidate comments from typing-proof.json."
  } catch {
    Add-Check -Name "typing-proof-exists" -Passed $false -Detail "Failed to read typing proof: $($_.Exception.Message)"
  }
}

$rawRecords = @()
if (-not (Test-Path -LiteralPath $rawCommentsFull -PathType Leaf)) {
  Add-Check -Name "raw-comments-exist" -Passed $false -Detail "Missing raw lookup evidence: $RawCommentsPath"
} else {
  try {
    $rawRecords = @(Read-Ndjson -Path $rawCommentsFull)
    $summary.rawCommentRecords = $rawRecords.Count
    Add-Check -Name "raw-comments-exist" -Passed ($rawRecords.Count -gt 0) -Detail "Loaded $($rawRecords.Count) raw comment records."
  } catch {
    Add-Check -Name "raw-comments-exist" -Passed $false -Detail $_.Exception.Message
  }
}

$csvRows = @()
if (-not (Test-Path -LiteralPath $csvRowsFull -PathType Leaf)) {
  Add-Check -Name "csv-evidence-exists" -Passed $false -Detail "Missing CSV lookup evidence: $CsvRowsPath"
} else {
  try {
    $csvLines = @(Get-Content -LiteralPath $csvRowsFull -Encoding UTF8)
    if ($csvLines.Count -eq 0) {
      throw "CSV evidence is empty."
    }
    Add-Check -Name "d-10-header-order" -Passed ($csvLines[0] -eq $ExpectedHeader) -Detail "CSV header is '$($csvLines[0])'."
    for ($i = 1; $i -lt $csvLines.Count; $i++) {
      if ($csvLines[$i] -eq "") {
        continue
      }
      $fields = Parse-CsvLine -Line $csvLines[$i]
      if ($fields.Count -ne $ExpectedColumns.Count) {
        throw "CSV row $($i + 1) has $($fields.Count) columns, expected $($ExpectedColumns.Count)."
      }
      $csvRows += $csvLines[$i]
    }
    $summary.csvRowCount = $csvRows.Count
    Add-Check -Name "d-12-csv-rows-parse" -Passed ($csvRows.Count -gt 0) -Detail "Parsed $($csvRows.Count) CSV evidence rows with quoted-field support."
  } catch {
    Add-Check -Name "d-12-csv-rows-parse" -Passed $false -Detail $_.Exception.Message
  }
}

$rawDictionaryRows = New-Object System.Collections.Generic.List[string]
if ($rawRecords.Count -gt 0) {
  $seenInputs = New-Object System.Collections.Generic.HashSet[string]
  foreach ($record in $rawRecords) {
    try {
      $rawComment = [string]$record.rawComment
      if ($null -eq $record.rawComment) {
        throw "Record line $($record.lineNumber) missing rawComment."
      }
      $source = @($proofComments | Where-Object {
        $_.inputName -eq [string]$record.inputName -and
        $_.candidateText -eq [string]$record.candidateText -and
        $_.rawComment -eq $rawComment
      } | Select-Object -First 1)
      if ($source.Count -eq 0) {
        throw "Record line $($record.lineNumber) does not exactly match typing-proof raw comment evidence."
      }
      if ($record.PSObject.Properties["escapedComment"] -and [string]$record.escapedComment -ne (ConvertTo-EscapedComment -Text $rawComment)) {
        throw "Record line $($record.lineNumber) escapedComment does not preserve exact control separators."
      }
      if ($record.PSObject.Properties["sha256"] -and [string]$record.sha256 -ne (Get-Sha256Text -Text $rawComment)) {
        throw "Record line $($record.lineNumber) sha256 does not match rawComment."
      }
      if ($record.PSObject.Properties["utf8Hex"] -and [string]$record.utf8Hex -ne (Get-Utf8Hex -Text $rawComment)) {
        throw "Record line $($record.lineNumber) utf8Hex does not match rawComment."
      }
      $parsed = Parse-LookupComment -RawComment $rawComment
      $controlCounts = Get-ControlCounts -Text $rawComment
      if ($record.PSObject.Properties["controlCounts"]) {
        if ([int]$record.controlCounts.verticalTab -ne [int]$controlCounts.verticalTab -or
            [int]$record.controlCounts.formFeed -ne [int]$controlCounts.formFeed -or
            [int]$record.controlCounts.carriageReturn -ne [int]$controlCounts.carriageReturn) {
          throw "Record line $($record.lineNumber) controlCounts do not match rawComment."
        }
      }
      if ($record.PSObject.Properties["hasLeadingReverseLookupMarker"] -and
          [bool]$record.hasLeadingReverseLookupMarker -ne [bool]$parsed.hasLeadingReverseLookupMarker) {
        throw "Record line $($record.lineNumber) reverse lookup marker metadata does not match rawComment."
      }
      if ($parsed.mode -eq "dictionaryRows") {
        $summary.dictionaryRecordCount++
        foreach ($row in @($parsed.dictionaryRows)) {
          $rawDictionaryRows.Add($row) | Out-Null
        }
      } else {
        $summary.jyutpingOnlyRecordCount++
      }
      [void]$seenInputs.Add([string]$record.inputName)
    } catch {
      Add-Check -Name "raw-comment-record-$($record.lineNumber)" -Passed $false -Detail $_.Exception.Message
    }
  }
  $summary.inputs = @($seenInputs | Sort-Object)
  $summary.extractedDictionaryRows = $rawDictionaryRows.Count
  Add-Check -Name "d-11-raw-separators" -Passed ($rawDictionaryRows.Count -gt 0 -and $summary.dictionaryRecordCount -gt 0) -Detail "Validated $($summary.dictionaryRecordCount) dictionary records, $($summary.jyutpingOnlyRecordCount) Jyutping-only records, and $($rawDictionaryRows.Count) \r-delimited dictionary rows."
}

if ($csvRows.Count -gt 0 -and $rawDictionaryRows.Count -gt 0) {
  $missingRows = @($rawDictionaryRows | Where-Object { $csvRows -notcontains $_ })
  $extraRows = @($csvRows | Where-Object { $rawDictionaryRows -notcontains $_ })
  Add-Check -Name "csv-rows-match-raw-comments" -Passed ($missingRows.Count -eq 0 -and $extraRows.Count -eq 0) -Detail "Raw dictionary rows: $($rawDictionaryRows.Count); CSV rows: $($csvRows.Count); missing=$($missingRows.Count); extra=$($extraRows.Count)."
}

$dictionaryText = ($rawDictionaryRows -join "`n")
Add-Check -Name "dictionary-backed-fields" -Passed (
  $dictionaryText -match "you \(singular\)" -and
  $dictionaryText -match "adj\|adv" -and
  $dictionaryText -match "拜托" -and
  $dictionaryText -match "kamu"
) -Detail "Dictionary rows include English meaning, part-of-speech, reading/register notes, and enabled-language meanings."

$generatedAtUtc = [System.DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ", [System.Globalization.CultureInfo]::InvariantCulture)
$status = "failed"
if ($failures.Count -eq 0) {
  $status = "passed"
}
$checksArray = $checks.ToArray()
$failuresArray = $failures.ToArray()
$report = [ordered] @{
  schema = "typeduck-lookup-filter-validation-v1"
  generatedAtUtc = $generatedAtUtc
  inputs = [ordered] @{
    typingProofPath = $typingProofFull
    rawCommentsPath = $rawCommentsFull
    csvRowsPath = $csvRowsFull
  }
  expectedHeader = $ExpectedHeader
  summary = $summary
  checks = $checksArray
  status = $status
  failures = $failuresArray
}
Write-ValidationReport -Path $validationFull -Report $report

if ($failures.Count -gt 0) {
  Write-Error ("TypeDuck lookup payload validation failed:`n - " + ($failures -join "`n - "))
  exit 1
}

Write-Host "OK: TypeDuck lookup payload validation passed."
