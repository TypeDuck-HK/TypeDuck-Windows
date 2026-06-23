#Requires -Version 5.1
<#
.SYNOPSIS
  Run or validate the Phase 2 TypeDuck backend compatibility proof.

.PARAMETER RepoRoot
  Root of moqi-im-windows.

.PARAMETER StageRoot
  Staged TypeDuck runtime root produced by scripts\Stage-TypeDuckRuntime.ps1.

.PARAMETER MoqiImeRoot
  Canonical moqi-ime adapter checkout. Defaults to ..\moqi-ime.

.PARAMETER ProbeExe
  TypeduckBackendProbe executable path.

.PARAMETER OutputPath
  JSON proof output path.

.PARAMETER FrameLogPath
  NDJSON request/response frame log path.

.PARAMETER CheckOnly
  Validate required inputs and existing proof evidence without running the probe.
#>
param(
  [string] $RepoRoot = ".",
  [string] $StageRoot = ".\runtime\typeduck-phase02",
  [string] $MoqiImeRoot = "..\moqi-ime",
  [string] $ProbeExe = ".\build-vs32\Tools\TypeduckBackendProbe\Release\TypeduckBackendProbe.exe",
  [string] $OutputPath = ".planning\product\engine-runtime-fixtures\phase-02\typing-proof.json",
  [string] $FrameLogPath = ".planning\product\engine-runtime-fixtures\phase-02\backend-stdio-frames.ndjson",
  [switch] $CheckOnly
)

$ErrorActionPreference = "Stop"

$CanonicalAdapterUrl = "https://github.com/gaboolic/moqi-ime"
$ExpectedSiblingAdapter = "..\moqi-ime"
$RuntimeManifestPath = ".planning\product\engine-runtime-fixtures\phase-02\runtime-manifest.json"
$BannedLegacyEvidence = @(
  "Moqi fallback",
  "legacy Moqi fallback",
  "simple_pinyin",
  "fcitx fallback"
)

$ProofInputs = @(
  [ordered] @{
    name = "nei"
    keys = @("n", "e", "i")
    expectedCandidateCodePoints = @("4f60", "5c3c", "5462")
    expectedJyutping = @("nei5", "nei4", "nei1")
  },
  [ordered] @{
    name = "housam"
    keys = @("h", "o", "u", "s", "a", "m")
    expectedCandidateCodePoints = @("597d 5fc3", "597d 5fc3 4f60")
    expectedJyutping = @("hou2", "sam1")
  }
)

function Resolve-ProofPath {
  param(
    [string] $BasePath,
    [string] $Path
  )

  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path $BasePath $Path))
}

function Get-RelativeProofPath {
  param(
    [string] $BasePath,
    [string] $Path
  )

  $base = [System.IO.Path]::GetFullPath($BasePath)
  if (-not $base.EndsWith([System.IO.Path]::DirectorySeparatorChar)) {
    $base += [System.IO.Path]::DirectorySeparatorChar
  }
  $target = [System.IO.Path]::GetFullPath($Path)
  $baseUri = New-Object System.Uri($base)
  $targetUri = New-Object System.Uri($target)
  return [System.Uri]::UnescapeDataString($baseUri.MakeRelativeUri($targetUri).ToString()).Replace("/", "\")
}

function Assert-File {
  param(
    [string] $Path,
    [string] $Label
  )

  if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
    throw "$Label missing: $Path"
  }
}

function Assert-Directory {
  param(
    [string] $Path,
    [string] $Label
  )

  if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
    throw "$Label missing: $Path"
  }
}

function Resolve-MoqiImeRoot {
  param([string] $Path)

  $full = Resolve-ProofPath -BasePath $script:RepoRootFull -Path $Path
  if (-not (Test-Path -LiteralPath $full -PathType Container)) {
    throw "Missing moqi-ime adapter checkout. Clone $CanonicalAdapterUrl to $ExpectedSiblingAdapter or pass -MoqiImeRoot to scripts\Invoke-TypeDuckEngineProof.ps1. Resolved path: $full"
  }
  Assert-File -Path (Join-Path $full "go.mod") -Label "moqi-ime go.mod"
  Assert-File -Path (Join-Path $full "server.go") -Label "moqi-ime server.go"
  return $full
}

function Read-JsonFile {
  param([string] $Path)

  Assert-File -Path $Path -Label "JSON file"
  return Get-Content -Raw -LiteralPath $Path | ConvertFrom-Json
}

function Test-AnyTextContains {
  param(
    [object] $Value,
    [string[]] $Needles
  )

  $json = $Value | ConvertTo-Json -Depth 20 -Compress
  foreach ($needle in $Needles) {
    if ($json -match [regex]::Escape($needle)) {
      return $true
    }
  }
  return $false
}

function Assert-TypingProof {
  param(
    [string] $ProofPath,
    [string] $FramesPath
  )

  $proof = Read-JsonFile -Path $ProofPath
  if (-not $proof.runtime_manifest) {
    throw "typing proof is missing runtime_manifest evidence."
  }
  if (-not $proof.adapter -or -not $proof.adapter.server_exe) {
    throw "typing proof is missing adapter server.exe evidence."
  }
  if (-not $proof.inputs -or $proof.inputs.Count -eq 0) {
    throw "typing proof is missing controlled input evidence."
  }
  foreach ($input in $ProofInputs) {
    $observed = @($proof.inputs | Where-Object { $_.name -eq $input.name } | Select-Object -First 1)
    if ($observed.Count -eq 0) {
      throw "typing proof is missing controlled input '$($input.name)'."
    }
    $entry = $observed[0]
    if (-not $entry.composition -or -not $entry.candidates -or $entry.candidates.Count -eq 0) {
      throw "typing proof input '$($input.name)' lacks composition or candidates."
    }
    if (-not $entry.jyutping_or_comments -or $entry.jyutping_or_comments.Count -eq 0) {
      throw "typing proof input '$($input.name)' lacks Jyutping/comment evidence."
    }
    if (-not $entry.commit) {
      throw "typing proof input '$($input.name)' lacks commit output."
    }
    if (-not $entry.page -or $null -eq $entry.page.show_candidates) {
      throw "typing proof input '$($input.name)' lacks candidate page state."
    }
  }
  if (Test-AnyTextContains -Value $proof -Needles $BannedLegacyEvidence) {
    throw "typing proof contains only or includes banned legacy Moqi fallback evidence."
  }
  Assert-File -Path $FramesPath -Label "backend stdio frame log"
  $frameLines = @(Get-Content -LiteralPath $FramesPath)
  if ($frameLines.Count -eq 0) {
    throw "backend stdio frame log is empty."
  }
}

$RepoRootFull = Resolve-ProofPath -BasePath (Get-Location).Path -Path $RepoRoot
Assert-Directory -Path $RepoRootFull -Label "RepoRoot"

$StageRootFull = Resolve-ProofPath -BasePath $RepoRootFull -Path $StageRoot
$ManifestFull = Resolve-ProofPath -BasePath $RepoRootFull -Path $RuntimeManifestPath
$ProbeExeFull = Resolve-ProofPath -BasePath $RepoRootFull -Path $ProbeExe
$OutputFull = Resolve-ProofPath -BasePath $RepoRootFull -Path $OutputPath
$FrameLogFull = Resolve-ProofPath -BasePath $RepoRootFull -Path $FrameLogPath
$MoqiImeRootFull = Resolve-MoqiImeRoot -Path $MoqiImeRoot

Assert-Directory -Path $StageRootFull -Label "StageRoot"
Assert-File -Path $ManifestFull -Label "Plan 02-01 runtime manifest"
Assert-File -Path (Join-Path $StageRootFull "input_methods\rime\rime.dll") -Label "staged TypeDuck rime.dll"
Assert-File -Path (Join-Path $StageRootFull "input_methods\rime\jyut6ping3.schema.yaml") -Label "staged TypeDuck Jyutping schema"
Assert-File -Path $ProbeExeFull -Label "TypeduckBackendProbe executable"

$manifest = Read-JsonFile -Path $ManifestFull
if (-not $manifest.runtime -or -not $manifest.runtime.rimeDll -or -not $manifest.lookupFilterPlugin) {
  throw "runtime manifest is invalid: missing rime.dll or lookup-filter evidence."
}

if ($CheckOnly) {
  Assert-TypingProof -ProofPath $OutputFull -FramesPath $FrameLogFull
  Write-Host "OK: existing TypeDuck typing proof is valid."
  exit 0
}

throw "Proof execution is not implemented in this RED contract yet. Controlled inputs: $($ProofInputs.name -join ', ')."
