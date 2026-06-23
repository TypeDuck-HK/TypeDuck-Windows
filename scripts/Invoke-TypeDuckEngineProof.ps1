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

.PARAMETER TypeDuckSchemaRoot
  Optional TypeDuck Web schema checkout used to overlay current desktop schema files for proof execution.

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
  [string] $TypeDuckSchemaRoot = "I:\GitHub\TypeDuck-Web\schema",
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
    expectedCandidates = @("你", "尼", "呢")
    expectedJyutping = @("nei5", "nei4", "nei1")
  },
  [ordered] @{
    name = "housam"
    keys = @("h", "o", "u", "s", "a", "m")
    expectedCandidates = @("好心", "好心你")
    expectedJyutping = @("hou2sam1", "hou2sam1nei5")
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

function Assert-InsideRoot {
  param(
    [string] $Root,
    [string] $Path,
    [string] $Label
  )

  $rootFull = [System.IO.Path]::GetFullPath($Root).TrimEnd('\', '/')
  $pathFull = [System.IO.Path]::GetFullPath($Path).TrimEnd('\', '/')
  if ($pathFull -ne $rootFull -and -not $pathFull.StartsWith($rootFull + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "$Label must stay inside $rootFull. Resolved path: $pathFull"
  }
}

function New-CleanDirectory {
  param([string] $Path)

  Assert-InsideRoot -Root $script:RepoRootFull -Path $Path -Label "Clean directory"
  if (Test-Path -LiteralPath $Path) {
    Remove-Item -LiteralPath $Path -Recurse -Force
  }
  New-Item -ItemType Directory -Path $Path -Force | Out-Null
}

function Get-Sha256 {
  param([string] $Path)

  Assert-File -Path $Path -Label "hash input"
  return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
}

function Copy-DirectoryContents {
  param(
    [string] $SourceRoot,
    [string] $DestinationRoot,
    [string[]] $ExcludeNames = @()
  )

  Assert-Directory -Path $SourceRoot -Label "copy source"
  New-Item -ItemType Directory -Path $DestinationRoot -Force | Out-Null
  Get-ChildItem -LiteralPath $SourceRoot -Force | Where-Object {
    $ExcludeNames -notcontains $_.Name
  } | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination $DestinationRoot -Recurse -Force
  }
}

function Overlay-TypeDuckSchema {
  param(
    [string] $SchemaRoot,
    [string] $RimeDataRoot,
    [string] $WorkRoot
  )

  if ([string]::IsNullOrWhiteSpace($SchemaRoot)) {
    return
  }

  $schemaFull = Resolve-ProofPath -BasePath $script:RepoRootFull -Path $SchemaRoot
  if (-not (Test-Path -LiteralPath $schemaFull -PathType Container)) {
    return
  }

  Assert-File -Path (Join-Path $schemaFull "default.yaml") -Label "TypeDuck Web default.yaml"
  Assert-File -Path (Join-Path $schemaFull "jyut6ping3.schema.yaml") -Label "TypeDuck Web Jyutping schema"
  Assert-File -Path (Join-Path $schemaFull "jyut6ping3.dict.yaml") -Label "TypeDuck Web Jyutping dictionary"

  Copy-DirectoryContents -SourceRoot $schemaFull -DestinationRoot $RimeDataRoot -ExcludeNames @(".git", ".gitignore", "common.custom.yaml", "default.custom.yaml", "trime.yaml", "weasel.yaml")

  $schemaBuildRoot = Join-Path $schemaFull "build"
  if (Test-Path -LiteralPath $schemaBuildRoot -PathType Container) {
    $userBuildRoot = Join-Path $WorkRoot "appdata\Moqi\Rime\build"
    New-Item -ItemType Directory -Path $userBuildRoot -Force | Out-Null
    Copy-DirectoryContents -SourceRoot $schemaBuildRoot -DestinationRoot $userBuildRoot
  }
}

function Invoke-External {
  param(
    [string] $FilePath,
    [string[]] $ArgumentList,
    [string] $WorkingDirectory = $script:RepoRootFull
  )

  Write-Host ">> $FilePath $($ArgumentList -join ' ')"
  Push-Location $WorkingDirectory
  try {
    & $FilePath @ArgumentList
    if ($LASTEXITCODE -ne 0) {
      throw "Command failed with exit code ${LASTEXITCODE}: $FilePath"
    }
  }
  finally {
    Pop-Location
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

function Apply-AdapterPatches {
  param([string] $AdapterRoot)

  function Test-AdapterPatchAlreadyApplied {
    param(
      [string] $PatchName,
      [string] $AdapterRoot
    )

    $librimePath = Join-Path $AdapterRoot "input_methods\rime\librime.go"
    if (-not (Test-Path -LiteralPath $librimePath -PathType Leaf)) {
      return $false
    }
    $librimeText = Get-Content -Raw -LiteralPath $librimePath
    if ($PatchName -eq "0001-typeduck-librime-1.1.3-candidate-abi.patch") {
      return $librimeText -match "Quality\s+float64"
    }
    if ($PatchName -eq "0002-typeduck-librime-1.1.3-candidate-abi-x86-padding.patch") {
      return $librimeText -match "Padding\s+uint32"
    }
    return $false
  }

  $patchRoot = Join-Path $script:RepoRootFull "tools\typeduck-moqi-ime-adapter\patches"
  if (-not (Test-Path -LiteralPath $patchRoot -PathType Container)) {
    return
  }

  $patches = Get-ChildItem -LiteralPath $patchRoot -Filter "*.patch" -File | Sort-Object Name
  foreach ($patch in $patches) {
    & git -C $AdapterRoot apply --check $patch.FullName 2>$null
    if ($LASTEXITCODE -eq 0) {
      & git -C $AdapterRoot apply $patch.FullName
      if ($LASTEXITCODE -ne 0) {
        throw "Failed to apply adapter patch: $($patch.Name)"
      }
      continue
    }

    & git -C $AdapterRoot apply --reverse --check $patch.FullName 2>$null
    if ($LASTEXITCODE -ne 0 -and -not (Test-AdapterPatchAlreadyApplied -PatchName $patch.Name -AdapterRoot $AdapterRoot)) {
      throw "Adapter patch is neither applicable nor already applied: $($patch.Name)"
    }
  }
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
  foreach ($proofInput in $ProofInputs) {
    $observed = @($proof.inputs | Where-Object { $_.name -eq $proofInput.name } | Select-Object -First 1)
    if ($observed.Count -eq 0) {
      throw "typing proof is missing controlled input '$($proofInput.name)'."
    }
    $entry = $observed[0]
    if (-not $entry.composition -or -not $entry.candidates -or $entry.candidates.Count -eq 0) {
      throw "typing proof input '$($proofInput.name)' lacks composition or candidates."
    }
    if (-not $entry.jyutping_or_comments -or $entry.jyutping_or_comments.Count -eq 0) {
      throw "typing proof input '$($proofInput.name)' lacks Jyutping/comment evidence."
    }
    if (-not $entry.commit) {
      throw "typing proof input '$($proofInput.name)' lacks commit output."
    }
    if (-not $entry.page -or $null -eq $entry.page.show_candidates) {
      throw "typing proof input '$($proofInput.name)' lacks candidate page state."
    }
    $candidateValues = @($entry.candidates | ForEach-Object { [string] $_.text })
    $candidateText = ($candidateValues -join "`n")
    $textJson = ($entry.candidates | ConvertTo-Json -Depth 8 -Compress)
    if ($candidateText -notmatch '[^\x00-\x7F]') {
      throw "typing proof input '$($proofInput.name)' lacks TypeDuck CJK candidate text."
    }
    foreach ($expected in $proofInput.expectedCandidates) {
      if ($candidateValues -notcontains $expected) {
        throw "typing proof input '$($proofInput.name)' lacks expected candidate '$expected'."
      }
    }
    foreach ($expected in $proofInput.expectedJyutping) {
      if ($textJson -notmatch [regex]::Escape($expected)) {
        throw "typing proof input '$($proofInput.name)' lacks expected Jyutping/comment token '$expected'."
      }
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

function Build-AdapterServer {
  param(
    [string] $AdapterRoot,
    [string] $ServerExe
  )

  if (-not (Get-Command go -ErrorAction SilentlyContinue)) {
    throw "Go is required to build the moqi-ime adapter server."
  }
  $serverDir = Split-Path -Parent $ServerExe
  New-Item -ItemType Directory -Path $serverDir -Force | Out-Null

  $oldGoos = $env:GOOS
  $oldGoarch = $env:GOARCH
  $oldCgo = $env:CGO_ENABLED
  try {
    $env:GOOS = "windows"
    $env:GOARCH = "386"
    $env:CGO_ENABLED = "0"
    Invoke-External -FilePath "go" -ArgumentList @("build", "-o", $ServerExe, ".") -WorkingDirectory $AdapterRoot
  }
  finally {
    $env:GOOS = $oldGoos
    $env:GOARCH = $oldGoarch
    $env:CGO_ENABLED = $oldCgo
  }
}

function Prepare-AdapterWorkDir {
  param(
    [string] $StageRoot,
    [string] $AdapterRoot,
    [string] $TypeDuckSchemaRoot
  )

  $workRoot = Join-Path $StageRoot "adapter-work"
  New-CleanDirectory -Path $workRoot
  $rimeRoot = Join-Path $workRoot "input_methods\rime"
  $rimeDataRoot = Join-Path $rimeRoot "data"
  New-Item -ItemType Directory -Path $rimeRoot, $rimeDataRoot -Force | Out-Null

  $sourceRimeRoot = Join-Path $StageRoot "input_methods\rime"
  Assert-Directory -Path $sourceRimeRoot -Label "staged TypeDuck rime runtime"
  Copy-Item -LiteralPath (Join-Path $sourceRimeRoot "rime.dll") -Destination (Join-Path $rimeRoot "rime.dll") -Force
  Copy-DirectoryContents -SourceRoot $sourceRimeRoot -DestinationRoot $rimeDataRoot -ExcludeNames @("rime.dll", "user", "common.custom.yaml", "default.custom.yaml")
  Overlay-TypeDuckSchema -SchemaRoot $TypeDuckSchemaRoot -RimeDataRoot $rimeDataRoot -WorkRoot $workRoot

  Copy-Item -LiteralPath (Join-Path $AdapterRoot "input_methods\rime\ime.json") -Destination (Join-Path $rimeRoot "ime.json") -Force
  $appearanceThemes = Join-Path $AdapterRoot "input_methods\rime\appearance_themes.json"
  if (Test-Path -LiteralPath $appearanceThemes -PathType Leaf) {
    Copy-Item -LiteralPath $appearanceThemes -Destination (Join-Path $rimeRoot "appearance_themes.json") -Force
    Copy-Item -LiteralPath $appearanceThemes -Destination (Join-Path $rimeDataRoot "appearance_themes.json") -Force
  }

  return $workRoot
}

function Invoke-TypeDuckProbe {
  param(
    [string] $ProbePath,
    [string] $ServerExe,
    [string] $WorkingDirectory,
    [string] $ManifestPath,
    [string] $AdapterRoot,
    [string] $OutputPath,
    [string] $FramesPath
  )

  $adapterCommit = (& git -C $AdapterRoot rev-parse HEAD).Trim()
  if ($LASTEXITCODE -ne 0) {
    throw "Failed to read moqi-ime adapter commit."
  }

  $proofAppData = Join-Path $WorkingDirectory "appdata"
  $proofLocalAppData = Join-Path $WorkingDirectory "localappdata"
  New-Item -ItemType Directory -Path $proofAppData, $proofLocalAppData -Force | Out-Null

  $oldAppData = $env:APPDATA
  $oldLocalAppData = $env:LOCALAPPDATA
  try {
    $env:APPDATA = $proofAppData
    $env:LOCALAPPDATA = $proofLocalAppData
    Invoke-External -FilePath $ProbePath -ArgumentList @(
      "--server-exe", $ServerExe,
      "--working-dir", $WorkingDirectory,
      "--runtime-manifest", $ManifestPath,
      "--runtime-manifest-sha256", (Get-Sha256 -Path $ManifestPath),
      "--adapter-root", $AdapterRoot,
      "--adapter-commit", $adapterCommit,
      "--adapter-server-sha256", (Get-Sha256 -Path $ServerExe),
      "--output", $OutputPath,
      "--frames", $FramesPath,
      "--timeout-ms", "60000"
    )
  }
  finally {
    $env:APPDATA = $oldAppData
    $env:LOCALAPPDATA = $oldLocalAppData
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
Apply-AdapterPatches -AdapterRoot $MoqiImeRootFull

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

$adapterWorkRoot = Prepare-AdapterWorkDir -StageRoot $StageRootFull -AdapterRoot $MoqiImeRootFull -TypeDuckSchemaRoot $TypeDuckSchemaRoot
$adapterServerExe = Join-Path $adapterWorkRoot "server.exe"
Build-AdapterServer -AdapterRoot $MoqiImeRootFull -ServerExe $adapterServerExe

$outputDir = Split-Path -Parent $OutputFull
$frameDir = Split-Path -Parent $FrameLogFull
New-Item -ItemType Directory -Path $outputDir, $frameDir -Force | Out-Null
Invoke-TypeDuckProbe `
  -ProbePath $ProbeExeFull `
  -ServerExe $adapterServerExe `
  -WorkingDirectory $adapterWorkRoot `
  -ManifestPath $ManifestFull `
  -AdapterRoot $MoqiImeRootFull `
  -OutputPath $OutputFull `
  -FramesPath $FrameLogFull

Assert-TypingProof -ProofPath $OutputFull -FramesPath $FrameLogFull
Write-Host "OK: TypeDuck engine proof written to $(Get-RelativeProofPath -BasePath $RepoRootFull -Path $OutputFull)"
