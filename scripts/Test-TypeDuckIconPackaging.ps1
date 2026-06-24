param(
  [string] $RepoRoot = ".",
  [string] $BackendRoot = "D:\VSProjects\moqi-ime",
  [switch] $Strict
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

function Assert-Text([string] $Text, [string] $Pattern, [string] $Message) {
  if ($Text -notmatch $Pattern) {
    throw $Message
  }
}

function Assert-NotText([string] $Text, [string] $Pattern, [string] $Message) {
  if ($Text -match $Pattern) {
    throw $Message
  }
}

function Assert-Ordered([string] $Text, [string[]] $Patterns, [string] $Label) {
  $position = -1
  foreach ($pattern in $Patterns) {
    $match = [regex]::Match($Text.Substring($position + 1), $pattern)
    if (-not $match.Success) {
      throw "$Label missing or out of order: $pattern"
    }
    $position = $position + 1 + $match.Index + $match.Length
  }
}

function Get-FileText([string] $Path) {
  Assert-True (Test-Path -LiteralPath $Path) "Missing required file: $Path"
  return Get-Content -Raw -Encoding UTF8 -LiteralPath $Path
}

function Assert-SameFileHash([string] $Actual, [string] $Expected, [string] $Message) {
  Assert-True (Test-Path -LiteralPath $Actual) "Missing file for hash check: $Actual"
  Assert-True (Test-Path -LiteralPath $Expected) "Missing source file for hash check: $Expected"
  $actualHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Actual).Hash
  $expectedHash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Expected).Hash
  Assert-True ($actualHash -eq $expectedHash) "$Message Expected $expectedHash but got $actualHash."
}

function Get-IcoImagePayloads([string] $IconPath) {
  $bytes = [System.IO.File]::ReadAllBytes($IconPath)
  $stream = [System.IO.MemoryStream]::new($bytes)
  $reader = [System.IO.BinaryReader]::new($stream)
  try {
    $reserved = $reader.ReadUInt16()
    $type = $reader.ReadUInt16()
    $count = $reader.ReadUInt16()
    Assert-True ($reserved -eq 0 -and $type -eq 1 -and $count -gt 0) "Invalid icon file: $IconPath"
    $entries = @()
    for ($i = 0; $i -lt $count; $i++) {
      $null = $reader.ReadByte()
      $null = $reader.ReadByte()
      $null = $reader.ReadByte()
      $null = $reader.ReadByte()
      $null = $reader.ReadUInt16()
      $null = $reader.ReadUInt16()
      $size = $reader.ReadUInt32()
      $offset = $reader.ReadUInt32()
      $entries += [pscustomobject]@{ Size = [int] $size; Offset = [int] $offset }
    }
    $payloads = @()
    foreach ($entry in $entries) {
      $payload = New-Object byte[] $entry.Size
      [Array]::Copy($bytes, $entry.Offset, $payload, 0, $entry.Size)
      $payloads += ,$payload
    }
    return $payloads
  }
  finally {
    $reader.Dispose()
    $stream.Dispose()
  }
}

function Test-ContainsByteSequence([byte[]] $Haystack, [byte[]] $Needle) {
  if ($Needle.Length -eq 0 -or $Haystack.Length -lt $Needle.Length) {
    return $false
  }
  $limit = $Haystack.Length - $Needle.Length
  for ($i = 0; $i -le $limit; $i++) {
    if ($Haystack[$i] -ne $Needle[0]) {
      continue
    }
    $matched = $true
    for ($j = 1; $j -lt $Needle.Length; $j++) {
      if ($Haystack[$i + $j] -ne $Needle[$j]) {
        $matched = $false
        break
      }
    }
    if ($matched) {
      return $true
    }
  }
  return $false
}

function Assert-ExecutableContainsIcon([string] $ExecutablePath, [string] $IconPath, [string] $Message) {
  Assert-True (Test-Path -LiteralPath $ExecutablePath) "Missing executable for icon check: $ExecutablePath"
  $exeBytes = [System.IO.File]::ReadAllBytes($ExecutablePath)
  foreach ($payload in (Get-IcoImagePayloads $IconPath)) {
    if (Test-ContainsByteSequence $exeBytes $payload) {
      return
    }
  }
  throw $Message
}

$repo = Resolve-FullPath $RepoRoot
$backend = Resolve-FullPath $BackendRoot

$assetRoot = Join-Path $repo "TypeDuckSettings/assets"
$backendIconRoot = Join-Path $backend "icons"
$transparent = Join-Path $assetRoot "TypeDuck_Transparent.ico"
$small = Join-Path $assetRoot "TypeDuck_Small.ico"
$product = Join-Path $assetRoot "TypeDuck.ico"
$backendTransparent = Join-Path $backendIconRoot "TypeDuck_Transparent.ico"
$backendSmall = Join-Path $backendIconRoot "TypeDuck_Small.ico"
$backendProduct = Join-Path $backendIconRoot "TypeDuck.ico"

Assert-SameFileHash $transparent $backendTransparent "TypeDuck_Transparent.ico must be staged from the locked backend source."
Assert-SameFileHash $small $backendSmall "TypeDuck_Small.ico must be staged from the locked backend source."
Assert-SameFileHash $product $backendProduct "TypeDuck.ico must be staged from the locked backend source."

$settingsRc = Get-FileText (Join-Path $repo "TypeDuckSettings/TypeDuckSettings.rc")
$settingsResource = Get-FileText (Join-Path $repo "TypeDuckSettings/resource.h")
$about = Get-FileText (Join-Path $repo "TypeDuckSettings/TypeDuckAboutDialog.cpp")

Assert-Text $settingsResource "IDI_TYPEDUCK_SETTINGS" "Settings icon resource id must remain addressable."
Assert-Text $settingsRc "IDI_TYPEDUCK_SETTINGS\s+ICON\s+`"assets/TypeDuck_Transparent\.ico`"" "TypeDuckSettings executable must use TypeDuck_Transparent.ico."

Assert-Ordered $about @(
  "kAboutBodyText",
  "TypeDuck Windows IME version",
  "TypeDuck-HK librime fork",
  "rime-dictionary-lookup-filter",
  "TypeDuck-HK schema",
  "aboutLinks"
) "About version and attribution order"
Assert-Ordered $about @(
  "TypeDuck 網站 Website",
  "https://typeduck\.hk",
  "LearnDuck 粵拼打字入門 Introduction to Cantonese Jyutping Typing",
  "https://learn\.typeduck\.hk",
  "粵拼方案 Jyutping Scheme",
  "https://lshk\.org/jyutping-scheme/",
  "TypeDuck 原始碼 Source Code",
  "https://github\.com/TypeDuck-HK/TypeDuck-Windows"
) "About D-24 link labels and URLs"
Assert-Ordered $about @(
  "IDB_TYPEDUCK_ABOUT_BANNER",
  "kAboutBodyText",
  "IDB_TYPEDUCK_CREDIT_LOGOS",
  "attributionText",
  "aboutLinks"
) "About D-23/D-27 packaged resource order"

$packagingText = @($settingsRc, $about) -join "`n"
foreach ($banned in @("moqi\.png", "mo\.ico", "mo\.png", "moqi\.ico")) {
  Assert-NotText $packagingText $banned "Banned legacy Moqi image reference found in Phase 5 resource packaging: $banned"
}

$stageRoot = Join-Path $repo "installer/stage/win32/TypeDuckIME"
$stageHasPlanIcons =
  (Test-Path -LiteralPath (Join-Path $stageRoot "TypeDuck_Transparent.ico")) -and
  (Test-Path -LiteralPath (Join-Path $stageRoot "TypeDuck_Small.ico")) -and
  (Test-Path -LiteralPath (Join-Path $stageRoot "TypeDuck.ico"))
if ($stageHasPlanIcons) {
  Assert-SameFileHash (Join-Path $stageRoot "TypeDuck_Transparent.ico") $transparent "Staged transparent icon must match source."
  Assert-SameFileHash (Join-Path $stageRoot "TypeDuck_Small.ico") $small "Staged small icon must match source."
  Assert-SameFileHash (Join-Path $stageRoot "TypeDuck.ico") $product "Staged product icon must match source."

  Assert-ExecutableContainsIcon (Join-Path $stageRoot "TypeDuckLauncher.exe") $transparent "Staged TypeDuckLauncher.exe does not contain TypeDuck_Transparent.ico image data."
  Assert-ExecutableContainsIcon (Join-Path $stageRoot "TypeDuckSetupHelper.exe") $transparent "Staged TypeDuckSetupHelper.exe does not contain TypeDuck_Transparent.ico image data."
  Assert-ExecutableContainsIcon (Join-Path $stageRoot "TypeDuckSettings.exe") $transparent "Staged TypeDuckSettings.exe does not contain TypeDuck_Transparent.ico image data."
  Assert-ExecutableContainsIcon (Join-Path $stageRoot "moqi-ime/server.exe") $transparent "Staged moqi-ime/server.exe does not contain TypeDuck_Transparent.ico image data."
}

$installerOutput = Join-Path $repo "installer/dist/typeduck-windows-ime-setup.exe"
if ($stageHasPlanIcons -and (Test-Path -LiteralPath $installerOutput)) {
  Assert-ExecutableContainsIcon $installerOutput $product "Installer output does not contain TypeDuck.ico image data."
}

if ($Strict) {
  Assert-True (Test-Path -LiteralPath (Join-Path $backendIconRoot "About_Banner.bmp")) "Backend source About_Banner.bmp is missing."
  Assert-True (Test-Path -LiteralPath (Join-Path $backendIconRoot "Credit_Logos.bmp")) "Backend source Credit_Logos.bmp is missing."
}

Write-Host "[PASS] TypeDuck icon packaging guard passed."
