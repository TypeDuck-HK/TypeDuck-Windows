param(
  [string] $RepoRoot = ".",
  [string] $BackendRoot = "D:\VSProjects\moqi-ime",
  [switch] $Strict,
  [ValidateSet("", "DiagnosticsPrivacy")]
  [string] $ExpectRed = ""
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

function Get-FileText([string] $Path) {
  Assert-True (Test-Path -LiteralPath $Path -PathType Leaf) "Missing required file: $Path"
  return Get-Content -Raw -Encoding UTF8 -LiteralPath $Path
}

function Add-Violation([System.Collections.Generic.List[string]] $Violations, [string] $Category, [string] $Message) {
  $Violations.Add("[$Category] $Message")
}

function Get-SourceFiles([string] $Root, [string[]] $RelativePaths) {
  foreach ($relative in $RelativePaths) {
    $path = Join-Path $Root $relative
    [pscustomobject]@{
      RelativePath = $relative
      FullPath = $path
      Text = Get-FileText $path
    }
  }
}

function Get-DiagnosticLines([string] $Text) {
  $lines = $Text -split "\r?\n"
  for ($i = 0; $i -lt $lines.Count; $i++) {
    $line = $lines[$i]
    if ($line -match 'logger(_|->|\(\)->)|OutputDebugString|printf|wprintf|fprintf|fwprintf|appendRpcGuardLog|appendQuotePairLog|DebugLog|TsfLog') {
      [pscustomobject]@{
        Number = $i + 1
        Text = $line
      }
    }
  }
}

function Assert-PatternAbsent([System.Collections.Generic.List[string]] $Violations, [string] $Category, [string] $Text, [string] $Pattern, [string] $Message) {
  $matches = [regex]::Matches($Text, $Pattern)
  foreach ($match in $matches) {
    Add-Violation $Violations $Category "$Message Pattern '$Pattern' matched '$($match.Value)'."
  }
}

$repo = Resolve-FullPath $RepoRoot
$backend = Resolve-FullPath $BackendRoot
Assert-True (Test-Path -LiteralPath $repo -PathType Container) "RepoRoot does not exist: $repo"
Assert-True (Test-Path -LiteralPath $backend -PathType Container) "BackendRoot does not exist: $backend"

$runtimeFiles = @(
  "MoqLauncher/Utils.cpp",
  "MoqLauncher/Utils.h",
  "MoqLauncher/PipeServer.cpp",
  "MoqLauncher/PipeServer.h",
  "MoqLauncher/BackendServer.cpp",
  "MoqLauncher/BackendServer.h",
  "MoqiTextService/MoqiClient.cpp",
  "MoqiTextService/MoqiCandidateWindow.cpp",
  "MoqiTextService/MoqiTextService.cpp",
  "MoqiTextService/TsfLog.cpp",
  "MoqiTextService/TsfLog.h",
  "MoqiTextService/DllEntry.cpp",
  "libIME2/src/DebugLogConfig.cpp",
  "libIME2/src/TextService.cpp",
  "proto/moqi.proto"
)

$sources = @(Get-SourceFiles $repo $runtimeFiles)
$joinedRuntimeText = ($sources | ForEach-Object { $_.Text }) -join "`n"
$violations = [System.Collections.Generic.List[string]]::new()

# D-01 and D-24 allowance: internal source identifiers can still contain Moqi
# scaffold names. This guard targets runtime paths, visible/callable features,
# protocol fields, and diagnostics, not file names, class names, namespaces, or
# CMake target identifiers.

if ($joinedRuntimeText -notmatch 'TypeDuckIME') {
  Add-Violation $violations "paths" "Runtime source does not contain the TypeDuck-owned data directory name TypeDuckIME."
}
if ($joinedRuntimeText -notmatch 'TypeDuckLauncher\.json') {
  Add-Violation $violations "paths" "Runtime source does not contain the TypeDuck-owned launcher config TypeDuckLauncher.json."
}
if ($joinedRuntimeText -notmatch 'TypeDuckLauncher\.log') {
  Add-Violation $violations "paths" "Runtime source does not contain the TypeDuck-owned launcher log TypeDuckLauncher.log."
}

foreach ($source in $sources) {
  Assert-PatternAbsent $violations "legacy-path" $source.Text '\\MoqiIM(\\|")' "$($source.RelativePath) still references legacy local TypeDuck/Moqi scaffold path"
  Assert-PatternAbsent $violations "legacy-path" $source.Text '\\Moqi(\\|")' "$($source.RelativePath) still references legacy roaming Moqi state"
  Assert-PatternAbsent $violations "legacy-path" $source.Text 'MoqiLauncher\.json|MoqiLauncher\.log|getMoqiAppDataDir' "$($source.RelativePath) still references legacy launcher config/log or roaming helper"
}

$legacyStatePatterns = @(
  '\\MoqiIM',
  '\\Moqi\\',
  'Software\\.*Moqi',
  'MoqiIM-ReRegisterTSF'
)
$destructivePatterns = @(
  'Remove-Item',
  'DeleteFile',
  'RemoveDirectory',
  'RegDelete',
  'SHFileOperation',
  'TerminateProcess',
  'Stop-Process',
  'schtasks.*Delete',
  'Unregister'
)
foreach ($source in $sources) {
  foreach ($legacyPattern in $legacyStatePatterns) {
    foreach ($destructivePattern in $destructivePatterns) {
      if ($source.Text -match $legacyPattern -and $source.Text -match $destructivePattern) {
        Add-Violation $violations "legacy-coexistence" "$($source.RelativePath) combines legacy Moqi state pattern '$legacyPattern' with cleanup/repair verb '$destructivePattern'. TypeDuck cleanup must leave Legacy Moqi alone."
      }
    }
  }
}

$cloudPatterns = @(
  'AddClipboardFormatListener',
  'RemoveClipboardFormatListener',
  'WM_CLIPBOARDUPDATE',
  'ID_CLIPBOARD_DEBOUNCE_TIMER',
  'WM_FLUSH_CLIPBOARD_UPLOAD',
  'cloud_clipboard',
  'Cloud clipboard',
  'METHOD_CLOUD_CLIPBOARD_UPLOAD',
  'cloud_clipboard_text',
  'uploadCloudClipboardText',
  'processClipboardUploadOnUvThread',
  'readClipboardUtf8',
  'scheduleClipboardUpload',
  'pendingClipboardText_',
  'client_id\(\)\s*==\s*"clipboard"'
)
foreach ($source in $sources) {
  foreach ($pattern in $cloudPatterns) {
    Assert-PatternAbsent $violations "off-scope-cloud" $source.Text $pattern "$($source.RelativePath) still exposes cloud clipboard/frontend clipboard upload surface."
  }
}

$offScopePatterns = @(
  'WebDAV|webdav',
  '\bfcitx\b|fcitx5',
  'ai_config|api_key|base_url|chatgpt|openai'
)
foreach ($source in $sources) {
  foreach ($pattern in $offScopePatterns) {
    Assert-PatternAbsent $violations "off-scope-surface" $source.Text $pattern "$($source.RelativePath) still exposes off-scope WebDAV/fcitx/AI surface."
  }
}

foreach ($source in $sources) {
  foreach ($line in Get-DiagnosticLines $source.Text) {
    if ($line.Text -match '[\p{IsCJKUnifiedIdeographs}]') {
      Add-Violation $violations "english-diagnostics" "$($source.RelativePath):$($line.Number) diagnostic output contains CJK text: $($line.Text.Trim())"
    }
    if ($line.Text -match 'compositionString|candidate(List|Entries)?|rawLookupComment|clipboard|utf8Text|commitString|serialized(Request|Response|Reply)|payload') {
      Add-Violation $violations "raw-content-diagnostics" "$($source.RelativePath):$($line.Number) diagnostic output may include typed, candidate, clipboard, or raw protocol content: $($line.Text.Trim())"
    }
  }
}

if ($Strict) {
  $scriptText = Get-FileText (Join-Path $repo "scripts/Test-TypeDuckPrivacySecurityCleanup.ps1")
  foreach ($required in @(
      'source identifiers can still contain Moqi',
      '\$legacyStatePatterns',
      '\$destructivePatterns',
      'TypeDuck cleanup must leave Legacy Moqi alone',
      'DiagnosticsPrivacy')) {
    if ($scriptText -notmatch $required) {
      Add-Violation $violations "guard-contract" "Guard is missing required D-18/D-19 or source-identifier assertion: $required"
    }
  }
}

if ($ExpectRed -eq "DiagnosticsPrivacy") {
  Assert-True ($violations.Count -gt 0) "Expected RED DiagnosticsPrivacy, but no privacy/security cleanup violations were detected."
  $hasLegacy = ($violations | Where-Object { $_ -match '\[legacy-path\]|\[legacy-coexistence\]' } | Select-Object -First 1) -ne $null
  $hasCloud = ($violations | Where-Object { $_ -match '\[off-scope-cloud\]' } | Select-Object -First 1) -ne $null
  Assert-True $hasLegacy "Expected RED DiagnosticsPrivacy to catch legacy path or Legacy Moqi coexistence risks."
  Assert-True $hasCloud "Expected RED DiagnosticsPrivacy to catch cloud clipboard/frontend upload risks."
  Write-Host "PASS RED: DiagnosticsPrivacy caught $($violations.Count) cleanup gap(s)."
  foreach ($violation in $violations) {
    Write-Host "  $violation"
  }
  exit 0
}

if ($violations.Count -gt 0) {
  throw "TypeDuck privacy/security cleanup guard failed:`n$($violations -join "`n")"
}

Write-Host "[PASS] TypeDuck privacy/security cleanup guard passed."
