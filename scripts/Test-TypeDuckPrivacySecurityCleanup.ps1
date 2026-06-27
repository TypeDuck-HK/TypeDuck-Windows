param(
  [string] $RepoRoot = ".",
  [string] $BackendRoot = "D:\VSProjects\moqi-ime",
  [string] $ResultPath = "",
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

function Invoke-FocusedGuard(
    [System.Collections.Generic.List[string]] $Violations,
    [System.Collections.Generic.List[object]] $Commands,
    [string] $Name,
    [string] $ScriptPath,
    [hashtable] $Parameters) {
  $argumentText = ($Parameters.GetEnumerator() | Sort-Object Name | ForEach-Object {
      if ($_.Value -is [bool]) {
        if ($_.Value) { "-$($_.Name)" } else { "" }
      }
      else {
        "-$($_.Name) `"$($_.Value)`""
      }
    }) -join " "
  $commandText = "pwsh -NoProfile -ExecutionPolicy Bypass -File `"$ScriptPath`" $argumentText"
  $Commands.Add([pscustomobject]@{
      name = $Name
      command = $commandText
    })

  try {
    $output = & $ScriptPath @Parameters 2>&1
    if ($LASTEXITCODE -ne 0) {
      Add-Violation $Violations "focused-guard" "$Name exited with code ${LASTEXITCODE}: $($output -join ' ')"
      return
    }
  }
  catch {
    Add-Violation $Violations "focused-guard" "$Name failed: $($_.Exception.Message)"
  }
}

function Write-GuardResult(
    [string] $Path,
    [string] $Repo,
    [string] $Backend,
    [System.Collections.Generic.List[object]] $Commands,
    [string[]] $Categories,
    [string] $Status) {
  $parent = Split-Path -Parent $Path
  if (-not [string]::IsNullOrWhiteSpace($parent)) {
    New-Item -ItemType Directory -Path $parent -Force | Out-Null
  }

  $result = [pscustomobject]@{
    generatedAt = [DateTime]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ", [Globalization.CultureInfo]::InvariantCulture)
    repoRoot = $Repo
    backendRoot = $Backend
    commands = @($Commands)
    status = $Status
    categories = @($Categories)
  }
  $result | ConvertTo-Json -Depth 5 | Set-Content -Encoding UTF8 -LiteralPath $Path
}

$repo = Resolve-FullPath $RepoRoot
$backend = Resolve-FullPath $BackendRoot
Assert-True (Test-Path -LiteralPath $repo -PathType Container) "RepoRoot does not exist: $repo"
Assert-True (Test-Path -LiteralPath $backend -PathType Container) "BackendRoot does not exist: $backend"

if ([string]::IsNullOrWhiteSpace($ResultPath)) {
  $ResultPath = Join-Path $repo ".planning\product\privacy-security\phase-06-guard-results.json"
}
else {
  $ResultPath = Resolve-FullPath $ResultPath
}

$guardCommands = [System.Collections.Generic.List[object]]::new()
$checkedCategories = @(
  "identity",
  "language",
  "diagnostics",
  "backend-diagnostics",
  "backend-package",
  "runtime",
  "installer",
  "installer-cleanup",
  "legacy-moqi-coexistence",
  "publisher",
  "restart-guidance",
  "ipc",
  "settings-about",
  "icon-packaging"
)

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

if ($Strict -and $ExpectRed -eq "") {
  Invoke-FocusedGuard $violations $guardCommands "installer" (Join-Path $repo "scripts\Test-TypeDuckInstallerSkeleton.ps1") @{ RepoRoot = $repo; Strict = $true }
  Invoke-FocusedGuard $violations $guardCommands "runtime" (Join-Path $repo "scripts\Test-TypeDuckRuntimePackagePruning.ps1") @{ RepoRoot = $repo; BackendRoot = $backend; Strict = $true }
  Invoke-FocusedGuard $violations $guardCommands "backend-diagnostics" (Join-Path $repo "scripts\Test-TypeDuckBackendDiagnostics.ps1") @{ RepoRoot = $repo; BackendRoot = $backend; Strict = $true }
  Invoke-FocusedGuard $violations $guardCommands "launcher-protocol" (Join-Path $repo "scripts\Test-TypeDuckLauncherProtocol.ps1") @{ RepoRoot = $repo; Strict = $true }
  Invoke-FocusedGuard $violations $guardCommands "settings-about" (Join-Path $repo "scripts\Test-TypeDuckSettingsAboutUi.ps1") @{ RepoRoot = $repo; BackendRoot = $backend; Strict = $true }
  Invoke-FocusedGuard $violations $guardCommands "icon-packaging" (Join-Path $repo "scripts\Test-TypeDuckIconPackaging.ps1") @{ RepoRoot = $repo; BackendRoot = $backend; Strict = $true }
}

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
    $isExplicitlyRedacted = $line.Text -match '(?i)redacted'
    if (-not $isExplicitlyRedacted -and
        $line.Text -match 'compositionString|candidate(List|Entries)?|rawLookupComment|clipboard|utf8Text|commitString|serialized(Request|Response|Reply)|payload') {
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
      'DiagnosticsPrivacy',
      'Test-TypeDuckInstallerSkeleton\.ps1',
      'Test-TypeDuckRuntimePackagePruning\.ps1',
      'Test-TypeDuckBackendDiagnostics\.ps1',
      'Test-TypeDuckLauncherProtocol\.ps1',
      'Test-TypeDuckSettingsAboutUi\.ps1',
      'Test-TypeDuckIconPackaging\.ps1',
      'WebDAV|webdav',
      'cloud_clipboard|Cloud clipboard',
      'fcitx',
      'ai_config|api_key|openai',
      'TypeDuck-owned registration cleanup',
      'TypeDuck startup-entry cleanup',
      'TypeDuck scheduled-task cleanup',
      'TypeDuck install-file cleanup',
      'TypeDuck-owned state cleanup',
      'Legacy Moqi coexistence',
      'Legacy Moqi deletion, migration, registry cleanup, scheduled-task cleanup, install-folder cleanup, and process termination',
      '香港教育大學 The Education University of Hong Kong',
      'restart guidance')) {
    if ($scriptText -notmatch $required) {
      Add-Violation $violations "guard-contract" "Guard is missing required D-18/D-19 or source-identifier assertion: $required"
    }
  }
}

# D-18/D-19 coverage notes for strict source audits:
# - TypeDuck-owned registration cleanup: focused installer guard checks setup-helper uninstall registration paths.
# - TypeDuck startup-entry cleanup: focused installer guard checks only the TypeDuckLauncher startup value.
# - TypeDuck scheduled-task cleanup: focused installer guard checks only TypeDuckIME-ReRegisterTSF cleanup.
# - TypeDuck install-file cleanup: focused installer guard checks the app directory cleanup.
# - TypeDuck-owned state cleanup: focused installer guard checks TypeDuckIME local and roaming state cleanup.
# - Legacy Moqi coexistence: this aggregate guard rejects Legacy Moqi cleanup/repair coupling.
# - Legacy Moqi deletion, migration, registry cleanup, scheduled-task cleanup, install-folder cleanup, and process termination are rejected.
# - D-28 publisher text must be 香港教育大學 The Education University of Hong Kong.
# - D-29 restart guidance must stay on the final page and avoid TSF/DLL/COM/internal terms.

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
  if ($ExpectRed -eq "") {
    Write-GuardResult $ResultPath $repo $backend $guardCommands $checkedCategories "fail"
  }
  throw "TypeDuck privacy/security cleanup guard failed:`n$($violations -join "`n")"
}

if ($ExpectRed -eq "") {
  Write-GuardResult $ResultPath $repo $backend $guardCommands $checkedCategories "pass"
}
Write-Host "[PASS] TypeDuck privacy/security cleanup guard passed."
