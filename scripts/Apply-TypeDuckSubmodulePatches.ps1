#Requires -Version 5.1
<#
.SYNOPSIS
  Apply TypeDuck-owned patches to third-party submodules.

.PARAMETER RepoRoot
  Root of TypeDuck-Windows. Defaults to the parent directory of this script.

.PARAMETER CheckOnly
  Validate that each patch is either applicable or already applied.
#>
param(
  [string] $RepoRoot = "",
  [switch] $CheckOnly
)

$ErrorActionPreference = "Stop"

function Invoke-Git {
  param(
    [string] $WorkingDirectory,
    [string[]] $ArgumentList
  )

  Write-Host ">> git -C $WorkingDirectory $($ArgumentList -join ' ')"
  & git -C $WorkingDirectory @ArgumentList
  if ($LASTEXITCODE -ne 0) {
    throw "git failed with exit code ${LASTEXITCODE}: git -C $WorkingDirectory $($ArgumentList -join ' ')"
  }
}

function Test-GitApply {
  param(
    [string] $WorkingDirectory,
    [string] $PatchPath,
    [switch] $Reverse
  )

  $arguments = @("apply", "--check")
  if ($Reverse) {
    $arguments += "--reverse"
  }
  $arguments += $PatchPath

  & git -C $WorkingDirectory @arguments *> $null
  return $LASTEXITCODE -eq 0
}

function Invoke-SubmodulePatch {
  param(
    [string] $RepoRoot,
    [string] $SubmodulePath,
    [string] $PatchPath,
    [string] $Name,
    [switch] $CheckOnly
  )

  $submoduleRoot = Join-Path $RepoRoot $SubmodulePath
  $patchFullPath = Join-Path $RepoRoot $PatchPath

  if (-not (Test-Path -LiteralPath $patchFullPath -PathType Leaf)) {
    throw "Missing patch file: $patchFullPath"
  }

  if (-not (Test-Path -LiteralPath (Join-Path $submoduleRoot ".git"))) {
    Invoke-Git -WorkingDirectory $RepoRoot -ArgumentList @("submodule", "update", "--init", "--recursive", $SubmodulePath)
  }

  if (Test-GitApply -WorkingDirectory $submoduleRoot -PatchPath $patchFullPath) {
    if ($CheckOnly) {
      Write-Host "[OK] $Name can be applied."
      return
    }

    Invoke-Git -WorkingDirectory $submoduleRoot -ArgumentList @("apply", $patchFullPath)
    Write-Host "[OK] Applied $Name."
    return
  }

  if (Test-GitApply -WorkingDirectory $submoduleRoot -PatchPath $patchFullPath -Reverse) {
    Write-Host "[OK] $Name is already applied."
    return
  }

  throw @"
Cannot apply $Name, and it does not look already applied.
Reset the submodule to the recorded checkout, then rerun this script:
  git submodule update --init --checkout --recursive $SubmodulePath
  pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Apply-TypeDuckSubmodulePatches.ps1
"@
}

$scriptRepoRoot = Join-Path $PSScriptRoot ".."
if (-not $RepoRoot) { $RepoRoot = $scriptRepoRoot }
$RepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)

$patches = @(
  @{
    Name = "libIME2 TypeDuck diagnostics path patch"
    SubmodulePath = "libIME2"
    PatchPath = "patches\libIME2\0001-use-typeduck-diagnostics-paths.patch"
  }
)

foreach ($patch in $patches) {
  Invoke-SubmodulePatch `
    -RepoRoot $RepoRoot `
    -SubmodulePath $patch.SubmodulePath `
    -PatchPath $patch.PatchPath `
    -Name $patch.Name `
    -CheckOnly:$CheckOnly
}

Write-Host "OK: TypeDuck submodule patches are ready."
