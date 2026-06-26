param(
  [string] $RepoRoot = ".",
  [string] $BackendRoot = "D:\VSProjects\moqi-ime",
  [switch] $Strict,
  [ValidateSet("", "RejectedUatBehavior")]
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
      $payloads += , $payload
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

function Add-Violation([System.Collections.Generic.List[string]] $Violations, [string] $Message) {
  $Violations.Add($Message)
}

$repo = Resolve-FullPath $RepoRoot
$backend = Resolve-FullPath $BackendRoot

$assetRoot = Join-Path $repo "TypeDuckSettings/assets"
$resourceRoot = Join-Path $repo "TypeDuckSettings/resources"
$backendIconRoot = Join-Path $backend "icons"
$transparent = Join-Path $assetRoot "TypeDuck_Transparent.ico"
$small = Join-Path $assetRoot "TypeDuck_Small.ico"
$product = Join-Path $assetRoot "TypeDuck.ico"
$aboutBanner = Join-Path $resourceRoot "About_Banner.bmp"
$creditLogos = Join-Path $resourceRoot "Credit_Logos.bmp"
$installerBitmap = Join-Path $resourceRoot "Installer.bmp"
$backendTransparent = Join-Path $backendIconRoot "TypeDuck_Transparent.ico"
$backendSmall = Join-Path $backendIconRoot "TypeDuck_Small.ico"
$backendProduct = Join-Path $backendIconRoot "TypeDuck.ico"

Assert-SameFileHash $transparent $backendTransparent "TypeDuck_Transparent.ico must be staged from the locked backend source."
Assert-SameFileHash $small $backendSmall "TypeDuck_Small.ico must be staged from the locked backend source."
Assert-SameFileHash $product $backendProduct "TypeDuck.ico must be staged from the locked backend source."

$settingsRc = Get-FileText (Join-Path $repo "TypeDuckSettings/TypeDuckSettings.rc")
$aboutRc = Get-FileText (Join-Path $repo "TypeDuckSettings/TypeDuckAbout.rc")
$settingsResource = Get-FileText (Join-Path $repo "TypeDuckSettings/resource.h")
$textServiceRc = Get-FileText (Join-Path $repo "MoqiTextService/MoqiTextService.rc.in")
$typeDuckProfile = Get-FileText (Join-Path $repo "MoqiTextService/TypeDuckProfile.cpp")
$launcherRc = Get-FileText (Join-Path $repo "MoqLauncher/MoqiLauncher.rc")
$setupHelperCmake = Get-FileText (Join-Path $repo "SetupHelper/CMakeLists.txt")
$setupHelperRc = Get-FileText (Join-Path $repo "SetupHelper/SetupHelper.rc")
$installer = Get-FileText (Join-Path $repo "installer/MoqiTsf.iss")
$installScript = Get-FileText (Join-Path $repo "scripts/install.ps1")
$packageScript = Get-FileText (Join-Path $repo "scripts/_all_in_package.ps1")
$backendBuildScript = Get-FileText (Join-Path $backend "scripts/build.ps1")
$about = Get-FileText (Join-Path $repo "TypeDuckSettings/TypeDuckAboutDialog.cpp")

Assert-Text $settingsResource "IDI_TYPEDUCK_SETTINGS" "Settings icon resource id must remain addressable."
Assert-Text $settingsRc "IDI_TYPEDUCK_SETTINGS\s+ICON\s+`"assets/TypeDuck_Transparent\.ico`"" "TypeDuckSettings executable must use TypeDuck_Transparent.ico."
Assert-Text $aboutRc "IDI_TYPEDUCK_SETTINGS\s+ICON\s+`"assets/TypeDuck_Transparent\.ico`"" "TypeDuckAbout executable must use TypeDuck_Transparent.ico."
Assert-True (Test-Path -LiteralPath $aboutBanner) "About_Banner.bmp must be frontend-owned under TypeDuckSettings/resources."
Assert-True (Test-Path -LiteralPath $creditLogos) "Credit_Logos.bmp must be frontend-owned under TypeDuckSettings/resources."
Assert-True (Test-Path -LiteralPath $installerBitmap) "Installer.bmp must be frontend-owned under TypeDuckSettings/resources."
Assert-Text $settingsRc "resources/About_Banner\.bmp" "TypeDuckSettings About banner resource must come from TypeDuckSettings/resources."
Assert-Text $settingsRc "resources/Credit_Logos\.bmp" "TypeDuckSettings credit logos resource must come from TypeDuckSettings/resources."
Assert-Text $aboutRc "resources/About_Banner\.bmp" "TypeDuckAbout banner resource must come from TypeDuckSettings/resources."
Assert-Text $aboutRc "resources/Credit_Logos\.bmp" "TypeDuckAbout credit logos resource must come from TypeDuckSettings/resources."
Assert-Text $launcherRc "IDI_MOQI_LAUNCHER\s+ICON\s+`"\.\./TypeDuckSettings/assets/TypeDuck_Transparent\.ico`"" "TypeDuckLauncher executable must use TypeDuck_Transparent.ico."
Assert-Text $setupHelperCmake "SetupHelper\.rc" "TypeDuckSetupHelper must compile a resource script."
Assert-Text $setupHelperRc "TypeDuck_Transparent\.ico" "TypeDuckSetupHelper executable must use TypeDuck_Transparent.ico."
Assert-Text $installScript '\$transparentIcon\s*=\s*Join-Path\s+\$iconSourceRoot\s+"TypeDuck_Transparent\.ico"' "Staging must resolve TypeDuck_Transparent.ico from product assets."
Assert-Text $installScript '\$resourceSourceRoot\s*=\s*Join-Path\s+\$RepoRoot\s+"TypeDuckSettings\\resources"' "Staging must resolve bitmap resources from TypeDuckSettings/resources."
Assert-Text $installScript '\$stageResourceRoot\s*=\s*Join-Path\s+\$stageWin32Root\s+"resources"' "Staging must copy frontend bitmap resources to a dedicated product resources folder."
Assert-Text $installScript 'Copy-IfExists\s+-Source\s+\$aboutBanner\s+-Destination\s+\(Join-Path\s+\$stageResourceRoot\s+"About_Banner\.bmp"\)' "Staging must copy About_Banner.bmp into the product resources folder."
Assert-Text $installScript 'Copy-IfExists\s+-Source\s+\$creditLogos\s+-Destination\s+\(Join-Path\s+\$stageResourceRoot\s+"Credit_Logos\.bmp"\)' "Staging must copy Credit_Logos.bmp into the product resources folder."
Assert-Text $installScript 'Copy-IfExists\s+-Source\s+\$installerBitmap\s+-Destination\s+\(Join-Path\s+\$stageResourceRoot\s+"Installer\.bmp"\)' "Staging must copy Installer.bmp into the product resources folder."
Assert-NotText $installScript 'Copy-IfExists\s+-Source\s+\$(transparentIcon|smallIcon|productIcon)\s+-Destination\s+\(Join-Path\s+\$stageWin32Root\s+"TypeDuck[^"]*\.ico"\)' "Staging must not copy raw TypeDuck icon files into the installed app root."
Assert-Text $installScript 'Set-WindowsExecutableIcon\s+-ExecutablePath\s+\(Join-Path\s+\$stageWin32Root\s+"TypeDuckLauncher\.exe"\)\s+-IconPath\s+\$transparentIcon' "Staging must stamp TypeDuckLauncher.exe with TypeDuck_Transparent.ico."
Assert-Text $installScript 'Set-WindowsExecutableIcon\s+-ExecutablePath\s+\(Join-Path\s+\$stageWin32Root\s+"TypeDuckSetupHelper\.exe"\)\s+-IconPath\s+\$transparentIcon' "Staging must stamp TypeDuckSetupHelper.exe with TypeDuck_Transparent.ico."
Assert-Text $installScript 'Set-WindowsExecutableIcon\s+-ExecutablePath\s+\(Join-Path\s+\$stageWin32Root\s+"TypeDuckSettings\.exe"\)\s+-IconPath\s+\$transparentIcon' "Staging must stamp TypeDuckSettings.exe with TypeDuck_Transparent.ico."
Assert-Text $installScript 'Set-WindowsExecutableIcon\s+-ExecutablePath\s+\(Join-Path\s+\$stageWin32Root\s+"TypeDuckAbout\.exe"\)\s+-IconPath\s+\$transparentIcon' "Staging must stamp TypeDuckAbout.exe with TypeDuck_Transparent.ico."
Assert-Text $installScript 'Set-WindowsExecutableIcon\s+-ExecutablePath\s+\$backendServer\s+-IconPath\s+\$transparentIcon' "Staging must stamp packaged moqi-ime/server.exe with TypeDuck_Transparent.ico."

Assert-Text $textServiceRc "IDI_TYPEDUCK_PROFILE\s+ICON\s+`"\.\./TypeDuckSettings/assets/TypeDuck_Small\.ico`"" "TSF DLL profile resource must use TypeDuck_Small.ico."
Assert-Text $typeDuckProfile 'moqi-ime\\\\icons\\\\TypeDuck_Small\.ico' "First-party TypeDuck profile must prefer the packaged TypeDuck_Small.ico used by the system IME menu."
Assert-Text $typeDuckProfile "preferredSmallIconPath" "Profile icon lookup must prefer the packaged small icon with a DLL resource fallback."
Assert-NotText $typeDuckProfile "lockedSmallIconPath" "Profile icon lookup must not depend on the old raw app-root icon helper."

Assert-Text $installer "SetupIconFile=\.\.\\TypeDuckSettings\\assets\\TypeDuck\.ico" "Installer setup icon must use TypeDuck.ico."
Assert-Text $installer "UninstallDisplayIcon=\{uninstallexe\}" "Uninstaller display icon must come from the compiled uninstaller icon, not a raw app-root TypeDuck.ico file."
Assert-Text $packageScript "scripts\\install\.ps1" "All-in package must continue routing final packaging through scripts/install.ps1."
Assert-Text $backendBuildScript '\$ServerIcon\s*=\s*Join-Path\s+\$IconsDir\s+"TypeDuck_Transparent\.ico"' "Backend server.exe must be stamped from TypeDuck_Transparent.ico."
Assert-Text $backendBuildScript 'Remove-IfExists\s+-Path\s+\(Join-Path\s+\$PackageRimeDir\s+"icon\.ico"\)' "Backend package build must remove the legacy input_methods/rime/icon.ico runtime file."
Assert-NotText $backendBuildScript '\$ServerIcon\s*=\s*Join-Path\s+\$IconsDir\s+"mo\.ico"' "Backend server.exe must not use the legacy Moqi executable icon."

Assert-Ordered $about @(
  "TypeDuck Windows IME 版本 Version",
  "TypeDuck-HK librime fork",
  "TypeDuck-HK schema",
  "aboutLinks"
) "About version and attribution order"
Assert-Ordered $about @(
  "TypeDuck 網站 Website",
  "https://typeduck\.hk",
  "LearnDuck 粵拼打字入門 Introduction to Jyutping Typing",
  "https://learn\.typeduck\.hk",
  "粵拼方案 Jyutping Scheme",
  "https://lshk\.org/jyutping-scheme/",
  "TypeDuck 原始碼 Source Code",
  "https://github\.com/TypeDuck-HK/TypeDuck-Windows"
) "About D-24 link labels and URLs"
Assert-Ordered $about @(
  "kIntroText",
  "kContactText",
  "kCreditText",
  "attributionText",
  "aboutLinks"
) "About D-23/D-27 packaged resource order"

$installScriptForBannedScan = $installScript -replace '(?s)\$bannedLegacyIconNames\s*=\s*@\([\s\S]*?\)', ''
$packagingText = @($settingsRc, $textServiceRc, $typeDuckProfile, $launcherRc, $setupHelperRc, $installer, $installScriptForBannedScan, $packageScript, $about) -join "`n"
foreach ($banned in @("moqi\.png", "mo\.ico", "mo\.png", "moqi\.ico")) {
  Assert-NotText $packagingText $banned "Banned legacy Moqi image reference found in Phase 5 resource packaging: $banned"
}

$stageRoot = Join-Path $repo "installer/stage/win32/TypeDuckIME"
$rejectedBehavior = [System.Collections.Generic.List[string]]::new()
if (Test-Path -LiteralPath $stageRoot -PathType Container) {
  $rawStageIcons = @(Get-ChildItem -LiteralPath $stageRoot -File -Filter "*.ico" -ErrorAction SilentlyContinue)
  foreach ($icon in $rawStageIcons) {
    Add-Violation $rejectedBehavior "Raw standalone icon is staged under installed product root: $($icon.FullName)"
  }

  $legacyRimeIcon = Join-Path $stageRoot "moqi-ime/input_methods/rime/icon.ico"
  if (Test-Path -LiteralPath $legacyRimeIcon -PathType Leaf) {
    Add-Violation $rejectedBehavior "Legacy runtime icon is packaged: $legacyRimeIcon"
  }
}

if ($ExpectRed -eq "RejectedUatBehavior") {
  Assert-True ($rejectedBehavior.Count -gt 0) "Expected RED RejectedUatBehavior, but rejected icon packaging behavior was not present."
  Write-Host "PASS RED: RejectedUatBehavior caught icon packaging gaps: $($rejectedBehavior -join '; ')"
  exit 0
}

Assert-True ($rejectedBehavior.Count -eq 0) "Rejected UAT icon packaging behavior found: $($rejectedBehavior -join '; ')"

if (Test-Path -LiteralPath $stageRoot -PathType Container) {
  foreach ($resourceName in @("About_Banner.bmp", "Credit_Logos.bmp", "Installer.bmp")) {
    Assert-True (Test-Path -LiteralPath (Join-Path $stageRoot "resources/$resourceName")) "Missing staged frontend resource: resources/$resourceName"
  }
  Assert-ExecutableContainsIcon (Join-Path $stageRoot "TypeDuckLauncher.exe") $transparent "Staged TypeDuckLauncher.exe does not contain TypeDuck_Transparent.ico image data."
  Assert-ExecutableContainsIcon (Join-Path $stageRoot "TypeDuckSetupHelper.exe") $transparent "Staged TypeDuckSetupHelper.exe does not contain TypeDuck_Transparent.ico image data."
  Assert-ExecutableContainsIcon (Join-Path $stageRoot "TypeDuckSettings.exe") $transparent "Staged TypeDuckSettings.exe does not contain TypeDuck_Transparent.ico image data."
  Assert-ExecutableContainsIcon (Join-Path $stageRoot "TypeDuckAbout.exe") $transparent "Staged TypeDuckAbout.exe does not contain TypeDuck_Transparent.ico image data."
  Assert-ExecutableContainsIcon (Join-Path $stageRoot "TypeDuckTextService.dll") $small "Staged TypeDuckTextService.dll does not contain TypeDuck_Small.ico image data for the input picker."
  Assert-ExecutableContainsIcon (Join-Path $stageRoot "moqi-ime/server.exe") $transparent "Staged moqi-ime/server.exe does not contain TypeDuck_Transparent.ico image data."
  foreach ($bannedName in @("moqi.png", "mo.ico", "mo.png", "moqi.ico")) {
    $bannedStageFile = Get-ChildItem -LiteralPath $stageRoot -Recurse -Force -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -ieq $bannedName } |
    Select-Object -First 1
    Assert-True ($null -eq $bannedStageFile) "Banned legacy Moqi image was staged: $($bannedStageFile.FullName)"
  }
}

$installerOutput = Join-Path $repo "installer/dist/typeduck-windows-ime-setup.exe"
if (Test-Path -LiteralPath $installerOutput) {
  Assert-ExecutableContainsIcon $installerOutput $product "Installer output does not contain TypeDuck.ico image data."
}

if ($Strict) {
  Assert-True (-not (Test-Path -LiteralPath (Join-Path $backendIconRoot "About_Banner.bmp"))) "Backend source must not own About_Banner.bmp."
  Assert-True (-not (Test-Path -LiteralPath (Join-Path $backendIconRoot "Credit_Logos.bmp"))) "Backend source must not own Credit_Logos.bmp."
  Assert-True (-not (Test-Path -LiteralPath (Join-Path $backendIconRoot "Installer.bmp"))) "Backend source must not own Installer.bmp."
}

Write-Host "[PASS] TypeDuck icon packaging guard passed."
