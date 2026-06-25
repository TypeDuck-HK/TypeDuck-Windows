#Requires -Version 5.1
<#
.SYNOPSIS
  Stage the pinned TypeDuck-HK runtime assets for Phase 2 engine proof.

.PARAMETER RepoRoot
  Root of moqi-im-windows.

.PARAMETER StageRoot
  Destination runtime tree. The script recreates this directory.

.PARAMETER CacheRoot
  Local cache for the pinned release archive and extraction directory.

.PARAMETER SchemaSubmodulePath
  TypeDuck-HK/schema submodule path.

.PARAMETER ManifestPath
  Machine-readable runtime evidence manifest path.

.PARAMETER Force
  Re-download the pinned release archive even when a cached copy exists.
#>
param(
  [string] $RepoRoot = ".",
  [string] $StageRoot = ".\runtime\typeduck-phase02",
  [string] $CacheRoot = ".cache\typeduck-phase02",
  [string] $SchemaSubmodulePath = "third_party\typeduck-schema",
  [string] $ManifestPath = ".planning\product\engine-runtime-fixtures\phase-02\runtime-manifest.json",
  [switch] $Force
)

$ErrorActionPreference = "Stop"

$ReleaseUrl = "https://github.com/TypeDuck-HK/librime/releases/download/v1.1.4/rime-TypeDuck-v1.1.4-Windows-msvc-x64.7z"
$ReleaseArchiveName = "rime-TypeDuck-v1.1.4-Windows-msvc-x64.7z"
$SchemaUrl = "https://github.com/TypeDuck-HK/schema"
$SchemaBranch = "aap2-alpha"
$LookupFilterSource = "https://github.com/TypeDuck-HK/rime-dictionary-lookup-filter"

function Resolve-StagePath {
  param(
    [string] $Root,
    [string] $Path
  )

  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path $Root $Path))
}

function Assert-InsideRepo {
  param(
    [string] $RepoRoot,
    [string] $Path,
    [string] $Label
  )

  $repo = [System.IO.Path]::GetFullPath($RepoRoot).TrimEnd('\', '/')
  $full = [System.IO.Path]::GetFullPath($Path).TrimEnd('\', '/')
  if ($full -ne $repo -and -not $full.StartsWith($repo + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "$Label must stay inside RepoRoot. $Label=$full RepoRoot=$repo"
  }
}

function New-CleanDirectory {
  param([string] $Path)

  if (Test-Path -LiteralPath $Path) {
    Remove-Item -LiteralPath $Path -Recurse -Force
  }
  New-Item -ItemType Directory -Path $Path -Force | Out-Null
}

function Get-RelativePath {
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

function Get-Sha256 {
  param([string] $Path)
  return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
}

function Get-FileEvidence {
  param(
    [string] $BasePath,
    [System.IO.FileInfo] $File,
    [string] $Kind
  )

  [ordered] @{
    path = Get-RelativePath -BasePath $BasePath -Path $File.FullName
    kind = $Kind
    bytes = $File.Length
    sha256 = Get-Sha256 -Path $File.FullName
  }
}

function Copy-DirectoryContents {
  param(
    [string] $SourceRoot,
    [string] $DestinationRoot
  )

  New-Item -ItemType Directory -Path $DestinationRoot -Force | Out-Null
  Get-ChildItem -Path $SourceRoot -Recurse -Force -Directory | ForEach-Object {
    $relative = Get-RelativePath -BasePath $SourceRoot -Path $_.FullName
    New-Item -ItemType Directory -Path (Join-Path $DestinationRoot $relative) -Force | Out-Null
  }
  Get-ChildItem -Path $SourceRoot -Recurse -Force -File | Where-Object {
    $_.FullName -notmatch '[\\/]\.git(?:[\\/]|$)'
  } | ForEach-Object {
    $relative = Get-RelativePath -BasePath $SourceRoot -Path $_.FullName
    $destination = Join-Path $DestinationRoot $relative
    $destinationDir = Split-Path -Parent $destination
    if (-not (Test-Path -LiteralPath $destinationDir)) {
      New-Item -ItemType Directory -Path $destinationDir -Force | Out-Null
    }
    Copy-Item -LiteralPath $_.FullName -Destination $destination -Force
  }
}

function Resolve-Extractor {
  $commands = @(
    "7z.exe",
    "C:\Program Files\7-Zip\7z.exe",
    "tar.exe"
  )

  foreach ($command in $commands) {
    if ([System.IO.Path]::IsPathRooted($command) -and (Test-Path -LiteralPath $command -PathType Leaf)) {
      return $command
    }
    $resolved = Get-Command $command -ErrorAction SilentlyContinue
    if ($resolved) {
      return $resolved.Source
    }
  }
  throw "No extractor found. Install 7-Zip or provide a Windows tar.exe capable of extracting .7z."
}

function Expand-Archive7z {
  param(
    [string] $ArchivePath,
    [string] $DestinationPath
  )

  $extractor = Resolve-Extractor
  New-CleanDirectory -Path $DestinationPath
  if ((Split-Path -Leaf $extractor).ToLowerInvariant() -eq "tar.exe") {
    & $extractor -xf $ArchivePath -C $DestinationPath
  } else {
    & $extractor x $ArchivePath "-o$DestinationPath" -y | Out-Null
  }
  if ($LASTEXITCODE -ne 0) {
    throw "Archive extraction failed with exit code $LASTEXITCODE."
  }
  return $extractor
}

function Invoke-RimeDeployer {
  param(
    [string] $DeployerPath,
    [string] $EngineBinPath,
    [string] $EngineLibPath,
    [string] $UserPath,
    [string] $SharedPath,
    [string] $BuildPath
  )

  New-Item -ItemType Directory -Path $UserPath, $BuildPath -Force | Out-Null
  $psi = New-Object System.Diagnostics.ProcessStartInfo
  $psi.FileName = $DeployerPath
  $psi.Arguments = "--build `"$UserPath`" `"$SharedPath`" `"$BuildPath`""
  $psi.WorkingDirectory = $SharedPath
  $psi.UseShellExecute = $false
  $psi.RedirectStandardOutput = $true
  $psi.RedirectStandardError = $true
  $psi.EnvironmentVariables["PATH"] = "$EngineLibPath;$EngineBinPath;$($psi.EnvironmentVariables["PATH"])"

  $process = New-Object System.Diagnostics.Process
  $process.StartInfo = $psi
  [void] $process.Start()
  $stdout = $process.StandardOutput.ReadToEnd()
  $stderr = $process.StandardError.ReadToEnd()
  $process.WaitForExit()

  return [ordered] @{
    command = "$DeployerPath $($psi.Arguments)"
    exitCode = $process.ExitCode
    stdout = $stdout
    stderr = $stderr
  }
}

$RepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)
$StageRootFull = Resolve-StagePath -Root $RepoRoot -Path $StageRoot
$CacheRootFull = Resolve-StagePath -Root $RepoRoot -Path $CacheRoot
$SchemaRootFull = Resolve-StagePath -Root $RepoRoot -Path $SchemaSubmodulePath
$ManifestFull = Resolve-StagePath -Root $RepoRoot -Path $ManifestPath

Assert-InsideRepo -RepoRoot $RepoRoot -Path $StageRootFull -Label "StageRoot"
Assert-InsideRepo -RepoRoot $RepoRoot -Path $CacheRootFull -Label "CacheRoot"
Assert-InsideRepo -RepoRoot $RepoRoot -Path $ManifestFull -Label "ManifestPath"

if (-not (Test-Path -LiteralPath $SchemaRootFull -PathType Container)) {
  throw "TypeDuck schema submodule not found: $SchemaRootFull"
}

$schemaBranchActual = (& git -C $SchemaRootFull rev-parse --abbrev-ref HEAD).Trim()
if ($schemaBranchActual -ne $SchemaBranch) {
  throw "TypeDuck schema submodule branch is $schemaBranchActual, expected $SchemaBranch."
}
$schemaCommit = (& git -C $SchemaRootFull rev-parse HEAD).Trim()

New-Item -ItemType Directory -Path $CacheRootFull -Force | Out-Null
$archivePath = Join-Path $CacheRootFull $ReleaseArchiveName
if ($Force -or -not (Test-Path -LiteralPath $archivePath -PathType Leaf)) {
  Write-Host "Downloading $ReleaseUrl"
  Invoke-WebRequest -Uri $ReleaseUrl -OutFile $archivePath
} else {
  Write-Host "Using cached archive: $archivePath"
}

$extractRoot = Join-Path $CacheRootFull "extracted"
$extractor = Expand-Archive7z -ArchivePath $archivePath -DestinationPath $extractRoot

$extractedRimeDll = Get-ChildItem -Path $extractRoot -Recurse -File -Filter "rime.dll" |
  Where-Object { $_.FullName -match '[\\/]dist[\\/]lib[\\/]rime\.dll$' } |
  Select-Object -First 1
if (-not $extractedRimeDll) {
  throw "Pinned release archive did not contain dist\lib\rime.dll."
}

$versionInfo = Get-ChildItem -Path $extractRoot -Recurse -File -Filter "version-info.txt" | Select-Object -First 1
if (-not $versionInfo) {
  throw "Pinned release archive did not contain version-info.txt for lookup-filter evidence."
}
$versionText = Get-Content -Raw -LiteralPath $versionInfo.FullName
$lookupCommit = ""
if ($versionText -match "rime-dictionary-lookup-filter\s+([a-fA-F0-9]{7,40})") {
  $lookupCommit = $Matches[1]
} else {
  throw "version-info.txt did not record rime-dictionary-lookup-filter provenance."
}

New-CleanDirectory -Path $StageRootFull
$rimeRuntimeRoot = Join-Path $StageRootFull "input_methods\rime"
$rimeBuildRoot = Join-Path $rimeRuntimeRoot "build"
$rimeUserRoot = Join-Path $rimeRuntimeRoot "user"
$engineRoot = Join-Path $StageRootFull "engine"
$engineBinRoot = Join-Path $engineRoot "bin"
$engineLibRoot = Join-Path $engineRoot "lib"
$engineIncludeRoot = Join-Path $engineRoot "include"
New-Item -ItemType Directory -Path $rimeRuntimeRoot, $engineBinRoot, $engineLibRoot, $engineIncludeRoot -Force | Out-Null

Copy-DirectoryContents -SourceRoot $SchemaRootFull -DestinationRoot $rimeRuntimeRoot
Copy-Item -LiteralPath $extractedRimeDll.FullName -Destination (Join-Path $rimeRuntimeRoot "rime.dll") -Force
Copy-Item -LiteralPath $versionInfo.FullName -Destination (Join-Path $rimeRuntimeRoot "version-info.txt") -Force

$distRoot = Join-Path $extractRoot "dist"
Copy-DirectoryContents -SourceRoot (Join-Path $distRoot "bin") -DestinationRoot $engineBinRoot
Copy-DirectoryContents -SourceRoot (Join-Path $distRoot "lib") -DestinationRoot $engineLibRoot
Copy-DirectoryContents -SourceRoot (Join-Path $distRoot "include") -DestinationRoot $engineIncludeRoot

$deployerPath = Join-Path $engineBinRoot "rime_deployer.exe"
if (-not (Test-Path -LiteralPath $deployerPath -PathType Leaf)) {
  throw "rime_deployer.exe missing from pinned release archive."
}

$deployEvidence = Invoke-RimeDeployer `
  -DeployerPath $deployerPath `
  -EngineBinPath $engineBinRoot `
  -EngineLibPath $engineLibRoot `
  -UserPath $rimeUserRoot `
  -SharedPath $rimeRuntimeRoot `
  -BuildPath $rimeBuildRoot
if ($deployEvidence.exitCode -ne 0) {
  throw "rime_deployer.exe failed with exit code $($deployEvidence.exitCode)."
}

$schemaFiles = @(Get-ChildItem -Path $rimeRuntimeRoot -File | Where-Object {
  $_.Name -like "*.schema.yaml" -or $_.Name -in @("default.yaml", "default.custom.yaml", "common.yaml", "common.custom.yaml", "include.yaml", "weasel.yaml", "template.yaml")
} | Sort-Object FullName | ForEach-Object { Get-FileEvidence -BasePath $StageRootFull -File $_ -Kind "schema" })

$dictionaryFiles = @(Get-ChildItem -Path $rimeRuntimeRoot -File | Where-Object {
  $_.Name -like "*.dict.yaml" -or $_.Name -eq "essay.txt"
} | Sort-Object FullName | ForEach-Object { Get-FileEvidence -BasePath $StageRootFull -File $_ -Kind "dictionary" })

$openccFiles = @(Get-ChildItem -Path (Join-Path $rimeRuntimeRoot "opencc") -Recurse -File | Sort-Object FullName | ForEach-Object {
  Get-FileEvidence -BasePath $StageRootFull -File $_ -Kind "opencc"
})

$builtSchemaFiles = @(Get-ChildItem -Path $rimeBuildRoot -Recurse -File | Where-Object {
  $_.Extension -eq ".bin" -or $_.Name -like "*.schema.yaml" -or $_.Name -eq "default.yaml"
} | Sort-Object FullName | ForEach-Object { Get-FileEvidence -BasePath $StageRootFull -File $_ -Kind "built-schema" })

if ($schemaFiles.Count -eq 0) { throw "No TypeDuck schema files were staged." }
if ($dictionaryFiles.Count -eq 0) { throw "No TypeDuck dictionary files were staged." }
if ($openccFiles.Count -eq 0) { throw "No TypeDuck OpenCC files were staged." }
if ($builtSchemaFiles.Count -eq 0) { throw "No built schema artifacts were produced." }

$rimeDllEvidence = Get-FileEvidence -BasePath $StageRootFull -File (Get-Item -LiteralPath (Join-Path $rimeRuntimeRoot "rime.dll")) -Kind "engine-dll"
$manifest = [ordered] @{
  manifestVersion = 1
  generatedAtUtc = (Get-Date).ToUniversalTime().ToString("yyyy-MM-dd'T'HH':'mm':'ss'Z'", [System.Globalization.CultureInfo]::InvariantCulture)
  stageRoot = $StageRootFull
  sources = [ordered] @{
    librimeRelease = [ordered] @{
      url = $ReleaseUrl
      archivePath = Get-RelativePath -BasePath $RepoRoot -Path $archivePath
      archiveSha256 = Get-Sha256 -Path $archivePath
      extractor = $extractor
    }
    schema = [ordered] @{
      url = $SchemaUrl
      branch = $SchemaBranch
      commit = $schemaCommit
      path = Get-RelativePath -BasePath $RepoRoot -Path $SchemaRootFull
    }
  }
  runtime = [ordered] @{
    compatibilityShape = "input_methods\\rime\\rime.dll"
    rimeDll = $rimeDllEvidence
    versionInfo = Get-FileEvidence -BasePath $StageRootFull -File (Get-Item -LiteralPath (Join-Path $rimeRuntimeRoot "version-info.txt")) -Kind "version-info"
  }
  lookupFilterPlugin = [ordered] @{
    path = $rimeDllEvidence.path
    kind = "lookup-filter-static-runtime"
    bytes = $rimeDllEvidence.bytes
    sha256 = $rimeDllEvidence.sha256
    source = $LookupFilterSource
    commit = $lookupCommit
    relation = "Pinned TypeDuck-HK librime release version-info.txt records rime-dictionary-lookup-filter at this commit; the staged runtime artifact is the shipped rime.dll."
  }
  assets = [ordered] @{
    schemaFiles = $schemaFiles
    dictionaries = $dictionaryFiles
    opencc = $openccFiles
    builtSchemas = $builtSchemaFiles
  }
  commands = [ordered] @{
    deployer = $deployEvidence
  }
}

$manifestDir = Split-Path -Parent $ManifestFull
if (-not (Test-Path -LiteralPath $manifestDir)) {
  New-Item -ItemType Directory -Path $manifestDir -Force | Out-Null
}
$manifest | ConvertTo-Json -Depth 12 | Out-File -LiteralPath $ManifestFull -Encoding utf8

Write-Host "TypeDuck runtime staged at: $StageRootFull"
Write-Host "Runtime manifest written: $ManifestFull"
