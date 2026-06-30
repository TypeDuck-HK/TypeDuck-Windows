#Requires -Version 5.1
<#
.SYNOPSIS
  Compile the TypeDuck Windows IME installer with Inno Setup 6.

.PARAMETER StageDir
  Root of the staged installer tree. Expected layout:
    win32\TypeDuckIME\...
    x64\TypeDuckIME\...

.PARAMETER IssPath
  Optional path to MoqiTsf.iss (default: installer dir next to this script).
  Legacy scaffold source compatibility: the script filename is not user-facing.

.EXAMPLE
  pwsh -NoProfile -ExecutionPolicy Bypass -File .\installer\build-installer.ps1 -StageDir D:\typeduck-windows-ime\installer\stage
#>
param(
    [Parameter(Mandatory = $true)]
    [string] $StageDir,
    [string] $IssPath = ''
)

$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $StageDir)) {
    Write-Error "StageDir not found: $StageDir"
}
$StageDir = (Resolve-Path -LiteralPath $StageDir).Path

$win32Root = Join-Path $StageDir 'win32\TypeDuckIME'
$x64Root = Join-Path $StageDir 'x64\TypeDuckIME'
if (-not (Test-Path -LiteralPath $win32Root)) {
    Write-Error "Stage win32 payload not found: $win32Root"
}
if (-not (Test-Path -LiteralPath $x64Root)) {
    Write-Error "Stage x64 payload not found: $x64Root"
}

$requiredPaths = @(
    (Join-Path $win32Root 'TypeDuckLauncher.exe'),
    (Join-Path $win32Root 'TypeDuckSetupHelper.exe'),
    (Join-Path $win32Root 'TypeDuckTextService.dll'),
    (Join-Path $win32Root 'THIRD_PARTY_NOTICES.txt'),
    (Join-Path $win32Root 'x64\TypeDuckTextService.dll'),
    (Join-Path $x64Root 'TypeDuckTextService.dll')
)
foreach ($path in $requiredPaths) {
    if (-not (Test-Path -LiteralPath $path)) {
        Write-Error "Required staged file not found: $path"
    }
}

if ([string]::IsNullOrWhiteSpace($IssPath)) {
    $IssPath = Join-Path $PSScriptRoot 'MoqiTsf.iss'
}
if (-not (Test-Path -LiteralPath $IssPath)) {
    Write-Error "ISS not found: $IssPath"
}

$candidates = @(
    (Join-Path $env:LOCALAPPDATA 'Programs\Inno Setup 6\ISCC.exe'),
    (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
    (Join-Path $env:ProgramFiles 'Inno Setup 6\ISCC.exe'),
    'ISCC.exe'
)
$iscc = $null
foreach ($c in $candidates) {
    if ($c -eq 'ISCC.exe') {
        $cmd = Get-Command ISCC.exe -ErrorAction SilentlyContinue
        if ($cmd) { $iscc = $cmd.Path; break }
    }
    elseif (Test-Path -LiteralPath $c) {
        $iscc = $c
        break
    }
}
if (-not $iscc) {
    Write-Error @"
Inno Setup 6 compiler (ISCC.exe) not found.
Install: https://jrsoftware.org/isdl.php
Then re-run this script.
"@
}

$argStage = '/DStageDir=' + $StageDir
Write-Host "ISCC: $iscc"
Write-Host "Args: `"$IssPath`" $argStage"
$p = Start-Process -FilePath $iscc -ArgumentList @("`"$IssPath`"", $argStage) -Wait -PassThru -NoNewWindow
if ($p.ExitCode -ne 0) {
    Write-Error "ISCC failed with exit code $($p.ExitCode)"
}

$dist = Join-Path $PSScriptRoot 'dist'
Write-Host "Output: $(Join-Path $dist 'typeduck-windows-ime-setup.exe')"
exit 0
