#Requires -Version 5.1
<#
.SYNOPSIS
  Validate the Phase 2 TypeDuck runtime staging contract.

.PARAMETER RepoRoot
  Root of moqi-im-windows.

.PARAMETER StageRoot
  Staged TypeDuck runtime tree produced by Stage-TypeDuckRuntime.ps1.

.PARAMETER ManifestPath
  Runtime manifest path produced by Stage-TypeDuckRuntime.ps1.
#>
param(
  [string] $RepoRoot = ".",
  [string] $StageRoot = ".\runtime\typeduck-phase02",
  [string] $ManifestPath = ".planning\product\engine-runtime-fixtures\phase-02\runtime-manifest.json"
)

$ErrorActionPreference = "Stop"

$ExpectedSchemaUrl = "https://github.com/TypeDuck-HK/schema"
$ExpectedSchemaBranch = "aap2-alpha"
$ExpectedReleaseUrl = "https://github.com/TypeDuck-HK/librime/releases/download/v1.1.3/rime-TypeDuck-v1.1.3-Windows-msvc-x86.7z"
$ExpectedLookupFilterSource = "https://github.com/TypeDuck-HK/rime-dictionary-lookup-filter"

$failures = New-Object System.Collections.Generic.List[string]

function Add-Failure {
  param([string] $Message)
  $failures.Add($Message) | Out-Null
}

function Resolve-ContractPath {
  param(
    [string] $Root,
    [string] $Path
  )

  if ([System.IO.Path]::IsPathRooted($Path)) {
    return [System.IO.Path]::GetFullPath($Path)
  }
  return [System.IO.Path]::GetFullPath((Join-Path $Root $Path))
}

function Get-ManifestValue {
  param(
    [object] $Object,
    [string] $PropertyPath
  )

  $current = $Object
  foreach ($part in $PropertyPath.Split(".")) {
    if ($null -eq $current) {
      return $null
    }
    $property = $current.PSObject.Properties[$part]
    if ($null -eq $property) {
      return $null
    }
    $current = $property.Value
  }
  return $current
}

function Test-HashEvidence {
  param(
    [object] $Entry,
    [string] $Label
  )

  $relativePath = Get-ManifestValue -Object $Entry -PropertyPath "path"
  $sha256 = Get-ManifestValue -Object $Entry -PropertyPath "sha256"

  if ([string]::IsNullOrWhiteSpace($relativePath)) {
    Add-Failure "$Label is missing a staged artifact path."
    return
  }
  if ([string]::IsNullOrWhiteSpace($sha256) -or $sha256 -notmatch "^[a-fA-F0-9]{64}$") {
    Add-Failure "$Label is missing a valid SHA-256 hash."
  }

  $fullPath = Resolve-ContractPath -Root $StageRootFull -Path $relativePath
  if (-not (Test-Path -LiteralPath $fullPath -PathType Leaf)) {
    Add-Failure "$Label staged artifact does not exist: $relativePath"
  }
}

function Test-AssetList {
  param(
    [object[]] $Entries,
    [string] $Label
  )

  if ($null -eq $Entries -or $Entries.Count -eq 0) {
    Add-Failure "$Label evidence is missing or empty."
    return
  }

  foreach ($entry in $Entries) {
    Test-HashEvidence -Entry $entry -Label $Label
  }
}

$RepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)
$StageRootFull = Resolve-ContractPath -Root $RepoRoot -Path $StageRoot
$ManifestFull = Resolve-ContractPath -Root $RepoRoot -Path $ManifestPath
$gitmodulesPath = Join-Path $RepoRoot ".gitmodules"
$schemaPath = Join-Path $RepoRoot "third_party\typeduck-schema"

if (-not (Test-Path -LiteralPath $gitmodulesPath -PathType Leaf)) {
  Add-Failure ".gitmodules is missing."
} else {
  $gitmodules = Get-Content -Raw -LiteralPath $gitmodulesPath
  if ($gitmodules -notmatch "path\s*=\s*third_party/typeduck-schema" -and $gitmodules -notmatch "path\s*=\s*third_party\\typeduck-schema") {
    Add-Failure ".gitmodules does not define third_party/typeduck-schema."
  }
  if ($gitmodules -notmatch [regex]::Escape("url = $ExpectedSchemaUrl")) {
    Add-Failure ".gitmodules does not pin TypeDuck schema URL $ExpectedSchemaUrl."
  }
  if ($gitmodules -notmatch [regex]::Escape("branch = $ExpectedSchemaBranch")) {
    Add-Failure ".gitmodules does not pin TypeDuck schema branch $ExpectedSchemaBranch."
  }
}

if (-not (Test-Path -LiteralPath $schemaPath -PathType Container)) {
  Add-Failure "third_party/typeduck-schema is missing."
} else {
  $branch = ""
  try {
    $branch = (& git -C $schemaPath rev-parse --abbrev-ref HEAD 2>$null).Trim()
  } catch {
    $branch = ""
  }
  if ($branch -and $branch -ne $ExpectedSchemaBranch) {
    Add-Failure "third_party/typeduck-schema is on branch $branch, expected $ExpectedSchemaBranch."
  }
}

if (-not (Test-Path -LiteralPath $ManifestFull -PathType Leaf)) {
  Add-Failure "Runtime manifest is missing: $ManifestPath."
} else {
  try {
    $manifest = Get-Content -Raw -LiteralPath $ManifestFull | ConvertFrom-Json
  } catch {
    Add-Failure "Runtime manifest is not valid JSON: $($_.Exception.Message)"
    $manifest = $null
  }

  if ($null -ne $manifest) {
    $releaseUrl = Get-ManifestValue -Object $manifest -PropertyPath "sources.librimeRelease.url"
    if ($releaseUrl -ne $ExpectedReleaseUrl) {
      Add-Failure "Manifest does not record exact D-06 DLL URL $ExpectedReleaseUrl."
    }

    $schemaUrl = Get-ManifestValue -Object $manifest -PropertyPath "sources.schema.url"
    $schemaBranch = Get-ManifestValue -Object $manifest -PropertyPath "sources.schema.branch"
    $schemaCommit = Get-ManifestValue -Object $manifest -PropertyPath "sources.schema.commit"
    if ($schemaUrl -ne $ExpectedSchemaUrl) {
      Add-Failure "Manifest schema source URL is missing or incorrect."
    }
    if ($schemaBranch -ne $ExpectedSchemaBranch) {
      Add-Failure "Manifest schema branch is missing or incorrect."
    }
    if ([string]::IsNullOrWhiteSpace($schemaCommit)) {
      Add-Failure "Manifest schema commit evidence is missing."
    }

    $lookupSource = Get-ManifestValue -Object $manifest -PropertyPath "lookupFilterPlugin.source"
    if ($lookupSource -ne $ExpectedLookupFilterSource) {
      Add-Failure "Lookup-filter evidence must cite $ExpectedLookupFilterSource."
    }
    Test-HashEvidence -Entry (Get-ManifestValue -Object $manifest -PropertyPath "runtime.rimeDll") -Label "rime.dll"
    Test-HashEvidence -Entry (Get-ManifestValue -Object $manifest -PropertyPath "lookupFilterPlugin") -Label "lookup-filter plugin"
    Test-AssetList -Entries @(Get-ManifestValue -Object $manifest -PropertyPath "assets.schemaFiles") -Label "TypeDuck schema file"
    Test-AssetList -Entries @(Get-ManifestValue -Object $manifest -PropertyPath "assets.dictionaries") -Label "TypeDuck dictionary"
    Test-AssetList -Entries @(Get-ManifestValue -Object $manifest -PropertyPath "assets.opencc") -Label "TypeDuck OpenCC asset"
    Test-AssetList -Entries @(Get-ManifestValue -Object $manifest -PropertyPath "assets.builtSchemas") -Label "built schema artifact"
  }
}

if ($failures.Count -gt 0) {
  Write-Error ("TypeDuck runtime contract failed:`n - " + ($failures -join "`n - "))
  exit 1
}

Write-Host "TypeDuck runtime contract passed."
