#Requires -Version 5.1
param(
    [string] $RepoRoot = ".",
    [switch] $Strict
)

$ErrorActionPreference = "Stop"

function Resolve-FullPath([string] $Path) {
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }
    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Path))
}

function Add-Failure([System.Collections.Generic.List[string]] $Failures, [string] $Message) {
    $Failures.Add($Message)
}

function Read-RequiredText([string] $Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Missing required file: $Path"
    }
    return Get-Content -Raw -Encoding UTF8 -LiteralPath $Path
}

function Assert-Text([System.Collections.Generic.List[string]] $Failures, [string] $Text, [string] $Pattern, [string] $Message) {
    if ($Text -notmatch $Pattern) {
        Add-Failure $Failures $Message
    }
}

function Assert-NoRequiredScreenshotAutomation([System.Collections.Generic.List[string]] $Failures, [string] $Text, [string] $Label) {
    $bannedRequiredPatterns = @(
        '(?im)^\s*[-*]\s*\[[ x]\]\s*.*screenshot',
        '(?im)^\s*[-*]\s*.*screenshot\s+(file|path|manifest|capture|comparison|compare)',
        '(?im)^\s*\|.*screenshot.*\|',
        '(?i)automated\s+screenshot|screenshot\s+comparison|capture\s+screenshot|screenshot\s+manifest|image\s+diff'
    )
    foreach ($pattern in $bannedRequiredPatterns) {
        if ($Text -match $pattern) {
            Add-Failure $Failures "$Label requires or automates screenshot evidence: pattern '$pattern'"
        }
    }
}

$repo = Resolve-FullPath $RepoRoot
$root = Join-Path $repo ".planning\product\release-fixtures\phase-07"
$checklistPath = Join-Path $root "interactive-vm-checklist.md"
$notesPath = Join-Path $root "host-app-dpi-notes.md"
$checklist = Read-RequiredText $checklistPath
$notes = Read-RequiredText $notesPath
$combined = $checklist + "`n" + $notes
$failures = [System.Collections.Generic.List[string]]::new()

foreach ($required in @(
    @{ Pattern = '(?i)Notepad'; Message = "Checklist must include Notepad host target." },
    @{ Pattern = '(?i)browser'; Message = "Checklist must include browser host target." },
    @{ Pattern = '(?i)Office|Office-like'; Message = "Checklist must include Office or Office-like host target." },
    @{ Pattern = '(?i)terminal|console'; Message = "Checklist must include terminal/console host target." },
    @{ Pattern = '(?i)elevated'; Message = "Checklist must include elevated app host target." },
    @{ Pattern = '(?i)awkward TSF|TSF host'; Message = "Checklist must include awkward TSF host target." },
    @{ Pattern = '100%'; Message = "Checklist must include 100% DPI judgement." },
    @{ Pattern = '140%.*if available|140% if available'; Message = "Checklist must include 140% DPI judgement if available." },
    @{ Pattern = '175%'; Message = "Checklist must include 175% DPI judgement." },
    @{ Pattern = '200%'; Message = "Checklist must include 200% DPI judgement." },
    @{ Pattern = 'No screenshots required'; Message = "Notes must explicitly say no screenshots are required." },
    @{ Pattern = '(?i)multi-monitor.*Phase 5|Phase 5.*multi-monitor'; Message = "Checklist must carry Phase 5 multi-monitor disposition." },
    @{ Pattern = '(?i)installer hash|SHA-256'; Message = "Notes must include installer hash evidence field." },
    @{ Pattern = '(?i)command|log'; Message = "Checklist/notes must point to command or log evidence." }
)) {
    Assert-Text $failures $combined $required.Pattern $required.Message
}

Assert-NoRequiredScreenshotAutomation $failures $checklist "interactive-vm-checklist.md"
Assert-NoRequiredScreenshotAutomation $failures $notes "host-app-dpi-notes.md"

if ($Strict) {
    Assert-Text $failures $checklist '(?i)direct human judgement|judge directly|directly observe' "Strict checklist must state visual checks use direct human judgement."
    Assert-Text $failures $notes '(?i)pass|fail|notes' "Strict notes file must include pass/fail/notes fields."
}

if ($failures.Count -gt 0) {
    throw "TypeDuck interactive release checklist guard failed:`n$($failures -join "`n")"
}

Write-Host "[PASS] TypeDuck interactive release checklist guard passed."
