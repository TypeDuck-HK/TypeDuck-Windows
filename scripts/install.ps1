#Requires -Version 5.1
<#
.SYNOPSIS
  Stage TypeDuck Windows IME binaries and invoke the installer builder.

  Does not install files into Program Files directly. Instead it prepares an
  installer stage tree and calls installer\build-installer.ps1 to produce the
  setup executable.

.PARAMETER RepoRoot
  Root of the TypeDuck Windows IME scaffold checkout (defaults to the parent directory of this script).

.PARAMETER Win32BuildDir
  CMake Win32 build directory (default: RepoRoot\build-vs32).

.PARAMETER X64BuildDir
  CMake x64 build directory (default: RepoRoot\build-vs64).

.PARAMETER MoqiImeSource
  Legacy Moqi scaffold compatibility: path to the transitional moqi-ime runtime tree to copy as backend.
  Default detection order:
    1. sibling ..\moqi-ime\scripts\build\moqi-ime
    2. sibling ..\moqi-ime

.PARAMETER SkipMoqiImeCopy
  If set, do not include the transitional backend tree in the staged installer payload.

.PARAMETER StageDir
  Installer staging directory (default: RepoRoot\installer\stage).

.PARAMETER IssPath
  Optional path to the Inno Setup script (default: RepoRoot\installer\MoqiTsf.iss).
#>
param(
    [string] $RepoRoot = "",
    [string] $Win32BuildDir = "",
    [string] $X64BuildDir = "",
    [string] $MoqiImeSource = "",
    [switch] $SkipMoqiImeCopy,
    [string] $StageDir = "",
    [string] $IssPath = ""
)

$ErrorActionPreference = "Stop"

function New-CleanDirectory {
    param([string] $Path)

    if (Test-Path -LiteralPath $Path) {
        Remove-Item -LiteralPath $Path -Recurse -Force
    }
    New-Item -ItemType Directory -Path $Path -Force | Out-Null
}

function Copy-IfExists {
    param(
        [string] $Source,
        [string] $Destination
    )

    if (-not (Test-Path -LiteralPath $Source)) {
        throw "Required file not found: $Source"
    }
    Copy-Item -LiteralPath $Source -Destination $Destination -Force
}

function Resolve-ArtifactPath {
    param(
        [string[]] $Candidates,
        [string] $Label
    )

    $existingCandidates = foreach ($candidate in $Candidates) {
        if (Test-Path -LiteralPath $candidate) {
            Get-Item -LiteralPath $candidate
        }
    }

    if (-not $existingCandidates) {
        throw "$Label not found. Checked: $($Candidates -join ', ')"
    }

    $selected = $existingCandidates |
    Sort-Object LastWriteTime -Descending |
    Select-Object -First 1

    Write-Host ("Using {0}: {1} ({2})" -f $Label, $selected.FullName, $selected.LastWriteTime)
    return $selected.FullName
}

function Initialize-ResourceUpdater {
    if ("TypeDuck.ResourceUpdater.NativeMethods" -as [type]) {
        return
    }

    Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;

namespace TypeDuck.ResourceUpdater {
  public static class NativeMethods {
    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    public static extern IntPtr BeginUpdateResource(string pFileName, bool bDeleteExistingResources);

    [DllImport("kernel32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool UpdateResource(IntPtr hUpdate, IntPtr lpType, IntPtr lpName, ushort wLanguage, byte[] lpData, uint cbData);

    [DllImport("kernel32.dll", SetLastError = true)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool EndUpdateResource(IntPtr hUpdate, bool fDiscard);
  }
}
"@
}

function Get-ResourceIntPtr {
    param([int] $Value)
    return [IntPtr]::new($Value)
}

function Convert-IcoToResourcePayloads {
    param([string] $IconPath)

    $bytes = [System.IO.File]::ReadAllBytes($IconPath)
    $stream = [System.IO.MemoryStream]::new($bytes)
    $reader = [System.IO.BinaryReader]::new($stream)
    try {
        $reserved = $reader.ReadUInt16()
        $type = $reader.ReadUInt16()
        $count = $reader.ReadUInt16()
        if ($reserved -ne 0 -or $type -ne 1 -or $count -le 0) {
            throw "Invalid .ico file: $IconPath"
        }

        $entries = @()
        for ($i = 0; $i -lt $count; $i++) {
            $entry = [ordered]@{
                Width = $reader.ReadByte()
                Height = $reader.ReadByte()
                ColorCount = $reader.ReadByte()
                Reserved = $reader.ReadByte()
                Planes = $reader.ReadUInt16()
                BitCount = $reader.ReadUInt16()
                BytesInResource = $reader.ReadUInt32()
                ImageOffset = $reader.ReadUInt32()
                ResourceId = $i + 1
            }
            $entries += [pscustomobject]$entry
        }

        $images = @()
        foreach ($entry in $entries) {
            $image = New-Object byte[] ([int] $entry.BytesInResource)
            [Array]::Copy($bytes, [int] $entry.ImageOffset, $image, 0, [int] $entry.BytesInResource)
            $images += [pscustomobject]@{
                Id = [int] $entry.ResourceId
                Data = $image
            }
        }

        $groupStream = [System.IO.MemoryStream]::new()
        $writer = [System.IO.BinaryWriter]::new($groupStream)
        try {
            $writer.Write([UInt16]0)
            $writer.Write([UInt16]1)
            $writer.Write([UInt16]$count)
            foreach ($entry in $entries) {
                $writer.Write([byte] $entry.Width)
                $writer.Write([byte] $entry.Height)
                $writer.Write([byte] $entry.ColorCount)
                $writer.Write([byte] 0)
                $writer.Write([UInt16] $entry.Planes)
                $writer.Write([UInt16] $entry.BitCount)
                $writer.Write([UInt32] $entry.BytesInResource)
                $writer.Write([UInt16] $entry.ResourceId)
            }
            $writer.Flush()
            return [pscustomobject]@{
                Group = $groupStream.ToArray()
                Images = $images
            }
        }
        finally {
            $writer.Dispose()
            $groupStream.Dispose()
        }
    }
    finally {
        $reader.Dispose()
        $stream.Dispose()
    }
}

function Set-WindowsExecutableIcon {
    param(
        [string] $ExecutablePath,
        [string] $IconPath
    )

    if (-not (Test-Path -LiteralPath $ExecutablePath)) {
        throw "Executable not found for icon update: $ExecutablePath"
    }
    if (-not (Test-Path -LiteralPath $IconPath)) {
        throw "Icon file not found for icon update: $IconPath"
    }

    Initialize-ResourceUpdater
    $payload = Convert-IcoToResourcePayloads -IconPath $IconPath
    $handle = [TypeDuck.ResourceUpdater.NativeMethods]::BeginUpdateResource($ExecutablePath, $false)
    if ($handle -eq [IntPtr]::Zero) {
        throw "BeginUpdateResource failed for $ExecutablePath (Win32 $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))"
    }

    $committed = $false
    try {
        foreach ($image in $payload.Images) {
            $ok = [TypeDuck.ResourceUpdater.NativeMethods]::UpdateResource(
                $handle,
                (Get-ResourceIntPtr 3),
                (Get-ResourceIntPtr $image.Id),
                0,
                $image.Data,
                [uint32] $image.Data.Length)
            if (-not $ok) {
                throw "UpdateResource RT_ICON failed for $ExecutablePath (Win32 $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))"
            }
        }

        $groupOk = [TypeDuck.ResourceUpdater.NativeMethods]::UpdateResource(
            $handle,
            (Get-ResourceIntPtr 14),
            (Get-ResourceIntPtr 1),
            0,
            $payload.Group,
            [uint32] $payload.Group.Length)
        if (-not $groupOk) {
            throw "UpdateResource RT_GROUP_ICON failed for $ExecutablePath (Win32 $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))"
        }

        if (-not [TypeDuck.ResourceUpdater.NativeMethods]::EndUpdateResource($handle, $false)) {
            throw "EndUpdateResource failed for $ExecutablePath (Win32 $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))"
        }
        $committed = $true
        Write-Host "Applied icon: $IconPath -> $ExecutablePath"
    }
    finally {
        if (-not $committed -and $handle -ne [IntPtr]::Zero) {
            [void] [TypeDuck.ResourceUpdater.NativeMethods]::EndUpdateResource($handle, $true)
        }
    }
}

function Resolve-MoqiImeSource {
    param(
        [string] $RepoRoot,
        [string] $RequestedSource
    )

    if (-not [string]::IsNullOrWhiteSpace($RequestedSource)) {
        return [System.IO.Path]::GetFullPath($RequestedSource)
    }

    $candidates = @(
        (Join-Path $RepoRoot "..\moqi-ime\scripts\build\moqi-ime"),
        (Join-Path $RepoRoot "..\moqi-ime")
    )

    foreach ($candidate in $candidates) {
        $fullPath = [System.IO.Path]::GetFullPath($candidate)
        if (Test-Path -LiteralPath (Join-Path $fullPath "server.exe")) {
            return $fullPath
        }
    }

    return [System.IO.Path]::GetFullPath((Join-Path $RepoRoot "..\moqi-ime\scripts\build\moqi-ime"))
}

function Copy-MoqiImeRuntime {
    param(
        [string] $SourceRoot,
        [string] $DestinationRoot
    )

    $serverExe = Join-Path $SourceRoot "server.exe"
    if (-not (Test-Path -LiteralPath $serverExe)) {
        throw "moqi-ime server.exe not found: $serverExe"
    }

    New-Item -ItemType Directory -Path $DestinationRoot -Force | Out-Null

    $directories = Get-ChildItem -Path $SourceRoot -Recurse -Force -Directory |
    Where-Object { $_.FullName -notmatch '[\\/]\.git(?:[\\/]|$)' }
    foreach ($directory in $directories) {
        $relativePath = $directory.FullName.Substring($SourceRoot.Length).TrimStart('\', '/')
        $targetDir = Join-Path $DestinationRoot $relativePath
        New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
    }

    $bannedLegacyIconNames = @("moqi.png", "mo.ico", "mo.png", "moqi.ico")
    $files = Get-ChildItem -Path $SourceRoot -Recurse -Force -File | Where-Object {
        $_.Extension -ne ".go" -and
        $_.FullName -notmatch '[\\/]\.git(?:[\\/]|$)' -and
        ($bannedLegacyIconNames -notcontains $_.Name.ToLowerInvariant())
    }
    foreach ($file in $files) {
        $relativePath = $file.FullName.Substring($SourceRoot.Length).TrimStart('\', '/')
        $targetPath = Join-Path $DestinationRoot $relativePath
        $targetDir = Split-Path -Parent $targetPath
        if (-not (Test-Path -LiteralPath $targetDir)) {
            New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
        }
        Copy-Item -LiteralPath $file.FullName -Destination $targetPath -Force
    }
}

$scriptRepoRoot = Join-Path $PSScriptRoot ".."
if (-not $RepoRoot) { $RepoRoot = $scriptRepoRoot }
$RepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)

if (-not $Win32BuildDir) { $Win32BuildDir = Join-Path $RepoRoot "build-vs32" }
if (-not $X64BuildDir) { $X64BuildDir = Join-Path $RepoRoot "build-vs64" }
$MoqiImeSource = Resolve-MoqiImeSource -RepoRoot $RepoRoot -RequestedSource $MoqiImeSource
if (-not $StageDir) { $StageDir = Join-Path $RepoRoot "installer\stage" }
if (-not $IssPath) { $IssPath = Join-Path $RepoRoot "installer\MoqiTsf.iss" }
$Win32BuildDir = [System.IO.Path]::GetFullPath($Win32BuildDir)
$X64BuildDir = [System.IO.Path]::GetFullPath($X64BuildDir)
$StageDir = [System.IO.Path]::GetFullPath($StageDir)
$IssPath = [System.IO.Path]::GetFullPath($IssPath)

$stageWin32Root = Join-Path $StageDir "win32\TypeDuckIME"
$stageX64Root = Join-Path $StageDir "x64\TypeDuckIME"
$stageWin32X64Root = Join-Path $stageWin32Root "x64"
$iconSourceRoot = Join-Path $RepoRoot "TypeDuckSettings\assets"
$transparentIcon = Join-Path $iconSourceRoot "TypeDuck_Transparent.ico"
$smallIcon = Join-Path $iconSourceRoot "TypeDuck_Small.ico"
$productIcon = Join-Path $iconSourceRoot "TypeDuck.ico"
New-CleanDirectory -Path $StageDir
New-Item -ItemType Directory -Path $stageWin32Root -Force | Out-Null
New-Item -ItemType Directory -Path $stageX64Root -Force | Out-Null
New-Item -ItemType Directory -Path $stageWin32X64Root -Force | Out-Null

Copy-IfExists -Source $transparentIcon -Destination (Join-Path $stageWin32Root "TypeDuck_Transparent.ico")
Copy-IfExists -Source $smallIcon -Destination (Join-Path $stageWin32Root "TypeDuck_Small.ico")
Copy-IfExists -Source $productIcon -Destination (Join-Path $stageWin32Root "TypeDuck.ico")

$backends = Join-Path $RepoRoot "backends.json"
if (-not (Test-Path -LiteralPath $backends)) {
    throw "Missing backends.json at $backends"
}
Copy-Item -LiteralPath $backends -Destination (Join-Path $stageWin32Root "backends.json") -Force

$launcher = Resolve-ArtifactPath -Label "TypeDuckLauncher.exe" -Candidates @(
    (Join-Path $Win32BuildDir "TypeDuckLauncher.exe"),
    (Join-Path $Win32BuildDir "Debug\TypeDuckLauncher.exe"),
    (Join-Path $Win32BuildDir "Release\TypeDuckLauncher.exe"),
    (Join-Path $Win32BuildDir "MoqLauncher\Debug\TypeDuckLauncher.exe"),
    (Join-Path $Win32BuildDir "MoqLauncher\Release\TypeDuckLauncher.exe")
)
Copy-IfExists -Source $launcher -Destination (Join-Path $stageWin32Root "TypeDuckLauncher.exe")
Set-WindowsExecutableIcon -ExecutablePath (Join-Path $stageWin32Root "TypeDuckLauncher.exe") -IconPath $transparentIcon

$setupHelper = Resolve-ArtifactPath -Label "TypeDuckSetupHelper.exe" -Candidates @(
    (Join-Path $Win32BuildDir "TypeDuckSetupHelper.exe"),
    (Join-Path $Win32BuildDir "Debug\TypeDuckSetupHelper.exe"),
    (Join-Path $Win32BuildDir "Release\TypeDuckSetupHelper.exe"),
    (Join-Path $Win32BuildDir "SetupHelper\Debug\TypeDuckSetupHelper.exe"),
    (Join-Path $Win32BuildDir "SetupHelper\Release\TypeDuckSetupHelper.exe")
)
Copy-IfExists -Source $setupHelper -Destination (Join-Path $stageWin32Root "TypeDuckSetupHelper.exe")
Set-WindowsExecutableIcon -ExecutablePath (Join-Path $stageWin32Root "TypeDuckSetupHelper.exe") -IconPath $transparentIcon

$settingsExe = Resolve-ArtifactPath -Label "TypeDuckSettings.exe" -Candidates @(
    (Join-Path $Win32BuildDir "TypeDuckSettings.exe"),
    (Join-Path $Win32BuildDir "Debug\TypeDuckSettings.exe"),
    (Join-Path $Win32BuildDir "Release\TypeDuckSettings.exe"),
    (Join-Path $Win32BuildDir "TypeDuckSettings\Debug\TypeDuckSettings.exe"),
    (Join-Path $Win32BuildDir "TypeDuckSettings\Release\TypeDuckSettings.exe"),
    (Join-Path $RepoRoot "build-vs32-settings-ui\TypeDuckSettings\Debug\TypeDuckSettings.exe"),
    (Join-Path $RepoRoot "build-vs32-settings-ui\TypeDuckSettings\Release\TypeDuckSettings.exe")
)
Copy-IfExists -Source $settingsExe -Destination (Join-Path $stageWin32Root "TypeDuckSettings.exe")
Set-WindowsExecutableIcon -ExecutablePath (Join-Path $stageWin32Root "TypeDuckSettings.exe") -IconPath $transparentIcon

$aboutExe = Resolve-ArtifactPath -Label "TypeDuckAbout.exe" -Candidates @(
    (Join-Path $Win32BuildDir "TypeDuckAbout.exe"),
    (Join-Path $Win32BuildDir "Debug\TypeDuckAbout.exe"),
    (Join-Path $Win32BuildDir "Release\TypeDuckAbout.exe"),
    (Join-Path $Win32BuildDir "TypeDuckSettings\Debug\TypeDuckAbout.exe"),
    (Join-Path $Win32BuildDir "TypeDuckSettings\Release\TypeDuckAbout.exe"),
    (Join-Path $RepoRoot "build-vs32-settings-ui\TypeDuckSettings\Debug\TypeDuckAbout.exe"),
    (Join-Path $RepoRoot "build-vs32-settings-ui\TypeDuckSettings\Release\TypeDuckAbout.exe")
)
Copy-IfExists -Source $aboutExe -Destination (Join-Path $stageWin32Root "TypeDuckAbout.exe")

$dll32 = Resolve-ArtifactPath -Label "Win32 TypeDuckTextService.dll" -Candidates @(
    (Join-Path $Win32BuildDir "TypeDuckTextService.dll"),
    (Join-Path $Win32BuildDir "Debug\TypeDuckTextService.dll"),
    (Join-Path $Win32BuildDir "Release\TypeDuckTextService.dll"),
    (Join-Path $Win32BuildDir "MoqiTextService\Debug\TypeDuckTextService.dll"),
    (Join-Path $Win32BuildDir "MoqiTextService\Release\TypeDuckTextService.dll")
)
Copy-IfExists -Source $dll32 -Destination (Join-Path $stageWin32Root "TypeDuckTextService.dll")

$dll64 = Resolve-ArtifactPath -Label "x64 TypeDuckTextService.dll" -Candidates @(
    (Join-Path $X64BuildDir "TypeDuckTextService.dll"),
    (Join-Path $X64BuildDir "Release\TypeDuckTextService.dll"),
    (Join-Path $X64BuildDir "MoqiTextService\Release\TypeDuckTextService.dll")
)
Copy-IfExists -Source $dll64 -Destination (Join-Path $stageX64Root "TypeDuckTextService.dll")
Copy-IfExists -Source $dll64 -Destination (Join-Path $stageWin32X64Root "TypeDuckTextService.dll")

if (-not $SkipMoqiImeCopy) {
    if (-not (Test-Path -LiteralPath $MoqiImeSource)) {
        throw "Moqi IME source not found: $MoqiImeSource (use -MoqiImeSource or -SkipMoqiImeCopy)."
    }
    $imeDest = Join-Path $stageWin32Root "moqi-ime"
    Copy-MoqiImeRuntime -SourceRoot $MoqiImeSource -DestinationRoot $imeDest
    $backendServer = Join-Path $imeDest "server.exe"
    Set-WindowsExecutableIcon -ExecutablePath $backendServer -IconPath $transparentIcon
}
else {
    Write-Warning "Skipped copying transitional moqi-ime backend; ensure the final TypeDuck installer payload is sufficient for registration testing."
}

$installerScript = Join-Path $RepoRoot "installer\build-installer.ps1"
if (-not (Test-Path -LiteralPath $installerScript)) {
    throw "Installer builder script not found: $installerScript"
}
if (-not (Test-Path -LiteralPath $IssPath)) {
    throw "Installer ISS file not found: $IssPath"
}

Write-Host "Stage prepared at: $StageDir"
Write-Host "Win32 payload: $stageWin32Root"
Write-Host "x64 payload: $stageX64Root"

& $installerScript -StageDir $StageDir -IssPath $IssPath

Write-Host "Installer build finished."
