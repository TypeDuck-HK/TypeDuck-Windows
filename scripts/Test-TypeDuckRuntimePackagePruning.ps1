#Requires -Version 5.1
<#
.SYNOPSIS
  Validates that generated TypeDuck runtime packages do not ship scaffold/off-scope payload.

.PARAMETER RepoRoot
  Root of moqi-im-windows.

.PARAMETER BackendRoot
  Root of the sibling moqi-ime backend runtime.

.PARAMETER Strict
  Enables source-script and generated package assertions.

.PARAMETER ExpectRed
  Reverses the guard for the expected failing pre-implementation state.
#>
param(
    [string] $RepoRoot = ".",
    [string] $BackendRoot = "D:\VSProjects\moqi-ime",
    [switch] $Strict,
    [ValidateSet("", "RejectedRuntimePackage")]
    [string] $ExpectRed = ""
)

$ErrorActionPreference = "Stop"

function Resolve-FullPath {
    param([string] $Path)
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }
    return [System.IO.Path]::GetFullPath((Join-Path (Get-Location) $Path))
}

function Add-Violation {
    param(
        [System.Collections.Generic.List[string]] $Violations,
        [string] $Message
    )
    $Violations.Add($Message)
}

function Read-TextIfPresent {
    param([string] $Path)
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        return ""
    }
    return Get-Content -Raw -Encoding UTF8 -LiteralPath $Path
}

function Assert-SourceText {
    param(
        [System.Collections.Generic.List[string]] $Violations,
        [string] $Text,
        [string] $Pattern,
        [string] $Message
    )
    if ($Text -notmatch $Pattern) {
        Add-Violation $Violations $Message
    }
}

function Assert-SourceNotText {
    param(
        [System.Collections.Generic.List[string]] $Violations,
        [string] $Text,
        [string] $Pattern,
        [string] $Message
    )
    if ($Text -match $Pattern) {
        Add-Violation $Violations $Message
    }
}

function Get-RelativePath {
    param(
        [string] $Root,
        [string] $Path
    )
    return $Path.Substring($Root.Length).TrimStart('\', '/')
}

function Test-RelativePath {
    param(
        [string] $RelativePath,
        [string] $Pattern
    )
    return ($RelativePath -replace '/', '\') -match $Pattern
}

function Test-BannedText {
    param(
        [string] $FullPath,
        [string] $RelativePath
    )
    $textExtensions = @(".json", ".yaml", ".yml", ".txt", ".md", ".ini", ".toml", ".ps1", ".bat", ".cmd", ".go", ".proto")
    if ($textExtensions -notcontains ([System.IO.Path]::GetExtension($FullPath).ToLowerInvariant())) {
        return @()
    }

    $text = Get-Content -Raw -Encoding UTF8 -LiteralPath $FullPath -ErrorAction SilentlyContinue
    if ($null -eq $text) {
        return @()
    }

    $matches = @()
    foreach ($entry in @(
        @{ Pattern = '(?i)\bWebDAV\b'; Label = "WebDAV" },
        @{ Pattern = '(?i)cloud[_ -]?clipboard|云剪贴板'; Label = "cloud clipboard" },
        @{ Pattern = '(?i)\bfcitx5?\b'; Label = "fcitx" },
        @{ Pattern = '(?i)scheme[_ -]?download|download scheme|下载方案|方案集下载'; Label = "scheme download" },
        @{ Pattern = '(?i)\bAI\b|ai_config|api[_-]?key|base[_-]?url|model|prompt|写好评|翻译|问答'; Label = "AI/config prompt" }
    )) {
        if ($text -match $entry.Pattern) {
            $matches += "$($entry.Label) text in $RelativePath"
        }
    }
    return $matches
}

function Test-RuntimeTree {
    param(
        [System.Collections.Generic.List[string]] $Violations,
        [string] $Root,
        [string] $Label,
        [switch] $RequireExists
    )

    if (-not (Test-Path -LiteralPath $Root -PathType Container)) {
        if ($RequireExists) {
            Add-Violation $Violations "$Label is missing: $Root"
        }
        return
    }

    $serverExe = Join-Path $Root "server.exe"
    if (-not (Test-Path -LiteralPath $serverExe -PathType Leaf)) {
        Add-Violation $Violations "$Label is missing server.exe: $serverExe"
    }

    $canonicalTheme = Join-Path $Root "input_methods\rime\appearance_themes.json"
    if (-not (Test-Path -LiteralPath $canonicalTheme -PathType Leaf)) {
        Add-Violation $Violations "$Label is missing canonical TypeDuck appearance themes: $canonicalTheme"
    }

    $files = @(Get-ChildItem -LiteralPath $Root -Recurse -Force -File -ErrorAction SilentlyContinue)
    $directories = @(Get-ChildItem -LiteralPath $Root -Recurse -Force -Directory -ErrorAction SilentlyContinue)

    foreach ($directory in $directories) {
        $relativePath = Get-RelativePath -Root $Root -Path $directory.FullName
        foreach ($entry in @(
            @{ Pattern = '^input_methods\\rime\\android($|\\)'; Label = "Android runtime directory" },
            @{ Pattern = '^input_methods\\rime\\cloudclipboard($|\\)'; Label = "cloud clipboard runtime directory" },
            @{ Pattern = '^input_methods\\rime\\templates($|\\)'; Label = "template runtime directory" },
            @{ Pattern = '^input_methods\\rime\\test($|\\)'; Label = "test fixture runtime directory" },
            @{ Pattern = '^input_methods\\rime\\icons($|\\)'; Label = "duplicate Rime icon directory" },
            @{ Pattern = '^icons($|\\)'; Label = "duplicate root icon directory" },
            @{ Pattern = '^input_methods\\fcitx5($|\\)'; Label = "fcitx runtime directory" },
            @{ Pattern = '^input_methods\\moqi($|\\)'; Label = "Moqi runtime directory" }
        )) {
            if (Test-RelativePath -RelativePath $relativePath -Pattern $entry.Pattern) {
                Add-Violation $Violations "$Label contains banned $($entry.Label): $relativePath"
            }
        }
    }

    foreach ($file in $files) {
        $relativePath = Get-RelativePath -Root $Root -Path $file.FullName
        foreach ($entry in @(
            @{ Pattern = '^input_methods\\rime\\ai_config\.json$'; Label = "AI config" },
            @{ Pattern = '^input_methods\\rime\\ime\.json$'; Label = "runtime profile metadata" },
            @{ Pattern = '^input_methods\\rime\\data\\appearance_themes\.json$'; Label = "duplicate data-path appearance themes" },
            @{ Pattern = '^backends\.json$'; Label = "staged backend manifest" },
            @{ Pattern = '^backends\.moqi-ime\.json$'; Label = "legacy backend snippet" },
            @{ Pattern = '\.go$'; Label = "Go source file" }
        )) {
            if (Test-RelativePath -RelativePath $relativePath -Pattern $entry.Pattern) {
                Add-Violation $Violations "$Label contains banned $($entry.Label): $relativePath"
            }
        }
        foreach ($textViolation in (Test-BannedText -FullPath $file.FullName -RelativePath $relativePath)) {
            Add-Violation $Violations "$Label contains banned shipped text: $textViolation"
        }
    }
}

$repo = Resolve-FullPath -Path $RepoRoot
$backend = Resolve-FullPath -Path $BackendRoot
$violations = [System.Collections.Generic.List[string]]::new()

$backendBuildScriptPath = Join-Path $backend "scripts\build.ps1"
$installScriptPath = Join-Path $repo "scripts\install.ps1"
$packageScriptPath = Join-Path $repo "scripts\_all_in_package.ps1"
$installerBuildScriptPath = Join-Path $repo "installer\build-installer.ps1"

$backendBuildScript = Read-TextIfPresent $backendBuildScriptPath
$installScript = Read-TextIfPresent $installScriptPath
$packageScript = Read-TextIfPresent $packageScriptPath
$installerBuildScript = Read-TextIfPresent $installerBuildScriptPath

Assert-SourceText $violations $backendBuildScript '\$PackageDir\s*=\s*Join-Path\s+\$BuildRoot\s+"TypeDuckRuntime"' "Backend build default package output must be scripts/build/TypeDuckRuntime."
Assert-SourceNotText $violations $backendBuildScript 'backends\.moqi-ime\.json|Generate backends\.json snippet|workingDir\s*=\s*"moqi-ime"|command\s*=\s*"moqi-ime\\server\.exe"' "Backend build must not emit legacy backend snippets."
Assert-SourceNotText $violations $backendBuildScript '\$packageAppearanceThemesData\s*=|Packaged appearance theme compatibility copy|compatibility data path' "Backend build must not package input_methods/rime/data/appearance_themes.json."
Assert-SourceText $violations $backendBuildScript 'input_methods\\rime\\appearance_themes\.json' "Backend build must preserve canonical input_methods/rime/appearance_themes.json."

if ($Strict -and ($installScript -match 'TypeDuckRuntime' -or (Test-Path -LiteralPath (Join-Path $repo "installer\stage\win32\TypeDuckIME\TypeDuckRuntime")))) {
    Assert-SourceText $violations $installScript 'TypeDuckRuntime' "Windows staging must use TypeDuckRuntime as the installed runtime folder."
    Assert-SourceNotText $violations $installScript 'Copy-Item\s+-LiteralPath\s+\$backends|backends\.json"\)' "Windows staging must not copy backends.json."
    Assert-SourceText $violations $installScript 'Copy-TypeDuckRuntime|Copy-MoqiImeRuntime' "Windows staging must retain a filtered runtime copy function."
    Assert-SourceText $violations $packageScript 'scripts\\build\\TypeDuckRuntime' "All-in package must consume the sibling TypeDuckRuntime package output."
    Assert-SourceNotText $violations $installerBuildScript "backends\.json" "Installer builder must not require staged backends.json."
}

$backendTypeDuckPackage = Join-Path $backend "scripts\build\TypeDuckRuntime"
Test-RuntimeTree -Violations $violations -Root $backendTypeDuckPackage -Label "Backend TypeDuckRuntime package" -RequireExists:($Strict -and ($ExpectRed -ne "RejectedRuntimePackage") -and (Test-Path -LiteralPath (Join-Path $backend "scripts\build")))

$legacyBackendPackage = Join-Path $backend "scripts\build\moqi-ime"
if (Test-Path -LiteralPath $legacyBackendPackage -PathType Container) {
    Add-Violation $violations "Legacy backend package output must not be the active TypeDuck runtime: $legacyBackendPackage"
}

$legacyBackendSnippet = Join-Path $backend "scripts\build\backends.moqi-ime.json"
if (Test-Path -LiteralPath $legacyBackendSnippet -PathType Leaf) {
    Add-Violation $violations "Legacy backend snippet must not be generated: $legacyBackendSnippet"
}

$stageRoot = Join-Path $repo "installer\stage\win32\TypeDuckIME"
$stageTypeDuckRuntime = Join-Path $stageRoot "TypeDuckRuntime"
if (Test-Path -LiteralPath $stageTypeDuckRuntime -PathType Container) {
    Test-RuntimeTree -Violations $violations -Root $stageTypeDuckRuntime -Label "Windows staged TypeDuckRuntime" -RequireExists

    $stagedBackends = Join-Path $stageRoot "backends.json"
    if (Test-Path -LiteralPath $stagedBackends -PathType Leaf) {
        Add-Violation $violations "Windows staging must not include top-level backends.json: $stagedBackends"
    }

    $stagedLegacyRuntime = Join-Path $stageRoot "moqi-ime"
    if (Test-Path -LiteralPath $stagedLegacyRuntime -PathType Container) {
        Add-Violation $violations "Windows staging must not include legacy moqi-ime runtime folder: $stagedLegacyRuntime"
    }
}

if ($ExpectRed -eq "RejectedRuntimePackage") {
    if ($violations.Count -eq 0) {
        throw "Expected RED RejectedRuntimePackage, but no runtime package pruning violations were found."
    }
    Write-Host "PASS RED: RejectedRuntimePackage caught runtime package pruning gaps:"
    foreach ($violation in $violations) {
        Write-Host " - $violation"
    }
    exit 0
}

if ($violations.Count -gt 0) {
    throw "Rejected runtime package behavior found: $($violations -join '; ')"
}

Write-Host "[PASS] TypeDuck runtime package pruning guard passed."
