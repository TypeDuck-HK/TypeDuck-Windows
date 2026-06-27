#Requires -Version 5.1
<#
.SYNOPSIS
  Validates TypeDuck backend diagnostics and generated runtime package surfaces.

.PARAMETER RepoRoot
  Root of moqi-im-windows.

.PARAMETER BackendRoot
  Root of the sibling moqi-ime backend runtime.

.PARAMETER Strict
  Enables stricter generated package assertions when package trees exist.

.PARAMETER ExpectRed
  Reverses the guard for known failing pre-implementation states.
#>
param(
    [string] $RepoRoot = ".",
    [string] $BackendRoot = "D:\VSProjects\moqi-ime",
    [switch] $Strict,
    [ValidateSet("", "BackendDiagnostics")]
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

function Get-RelativePath {
    param(
        [string] $Root,
        [string] $Path
    )
    return $Path.Substring($Root.Length).TrimStart('\', '/')
}

function Test-HanText {
    param([string] $Text)
    return $Text -match '[\u3400-\u9FFF\uF900-\uFAFF]'
}

function Test-ExcludedSourcePath {
    param([string] $RelativePath)
    $normalized = $RelativePath -replace '/', '\'
    foreach ($pattern in @(
        '(^|\\)\.git(\\|$)',
        '^scripts\\build(\\|$)',
        '^rime-frost(\\|$)',
        '^input_methods\\fcitx5(\\|$)',
        '^input_methods\\moqi(\\|$)',
        '^input_methods\\rime\\data\\rime-frost(\\|$)',
        '^input_methods\\rime\\data\\opencc(\\|$)',
        '^input_methods\\rime\\data\\dicts(\\|$)',
        '^input_methods\\rime\\data\\schema(\\|$)',
        '^input_methods\\rime\\data\\schemas(\\|$)'
    )) {
        if ($normalized -match $pattern) {
            return $true
        }
    }
    return $false
}

function Test-BannedRuntimeText {
    param(
        [string] $FullPath,
        [string] $RelativePath
    )
    $extension = [System.IO.Path]::GetExtension($FullPath).ToLowerInvariant()
    if (($RelativePath -replace '/', '\') -match '^input_methods\\rime\\data\\') {
        return @()
    }

    $textExtensions = @(".json", ".yaml", ".yml", ".txt", ".md", ".ini", ".toml", ".ps1", ".bat", ".cmd", ".go", ".proto")
    if ($textExtensions -notcontains $extension) {
        return @()
    }

    $text = Get-Content -Raw -Encoding UTF8 -LiteralPath $FullPath -ErrorAction SilentlyContinue
    if ($null -eq $text) {
        return @()
    }

    $violations = @()
    foreach ($entry in @(
        @{ Pattern = '(?i)\bWebDAV\b'; Label = "WebDAV" },
        @{ Pattern = '(?i)cloud[_ -]?clipboard|云剪贴板|雲剪貼簿'; Label = "cloud clipboard" },
        @{ Pattern = '(?i)\bfcitx5?\b'; Label = "fcitx" },
        @{ Pattern = '(?i)scheme[_ -]?download|download scheme|下载方案|方案集下载'; Label = "scheme download" },
        @{ Pattern = '(?i)\bAI\b|ai_config|api[_-]?key|base[_-]?url|prompt|写好评'; Label = "AI/config prompt" },
        @{ Pattern = 'MoqiIM|moqi-ime\.log'; Label = "legacy backend log path" }
    )) {
        if ($text -match $entry["Pattern"]) {
            $violations += "$($entry["Label"]) text in $RelativePath"
        }
    }
    return $violations
}

function Test-RuntimeTree {
    param(
        [System.Collections.Generic.List[string]] $Violations,
        [string] $Root,
        [string] $Label
    )

    if (-not (Test-Path -LiteralPath $Root -PathType Container)) {
        return
    }

    $files = @(Get-ChildItem -LiteralPath $Root -Recurse -Force -File -ErrorAction SilentlyContinue)
    $directories = @(Get-ChildItem -LiteralPath $Root -Recurse -Force -Directory -ErrorAction SilentlyContinue)

    foreach ($directory in $directories) {
        $relativePath = Get-RelativePath -Root $Root -Path $directory.FullName
        $normalized = $relativePath -replace '/', '\'
        foreach ($entry in @(
            @{ Pattern = '^input_methods\\rime\\android($|\\)'; Label = "Android runtime directory" },
            @{ Pattern = '^input_methods\\rime\\cloudclipboard($|\\)'; Label = "cloud clipboard runtime directory" },
            @{ Pattern = '^input_methods\\rime\\templates($|\\)'; Label = "template runtime directory" },
            @{ Pattern = '^input_methods\\rime\\test($|\\)'; Label = "test fixture runtime directory" },
            @{ Pattern = '^input_methods\\fcitx5($|\\)'; Label = "fcitx runtime directory" },
            @{ Pattern = '^input_methods\\moqi($|\\)'; Label = "Moqi runtime directory" }
        )) {
            if ($normalized -match $entry.Pattern) {
                Add-Violation $Violations "$Label contains banned $($entry.Label): $relativePath"
            }
        }
    }

    foreach ($file in $files) {
        $relativePath = Get-RelativePath -Root $Root -Path $file.FullName
        $normalized = $relativePath -replace '/', '\'
        foreach ($entry in @(
            @{ Pattern = '^input_methods\\rime\\ai_config\.json$'; Label = "AI config" },
            @{ Pattern = '^input_methods\\rime\\ime\.json$'; Label = "runtime profile metadata" },
            @{ Pattern = '^input_methods\\rime\\data\\appearance_themes\.json$'; Label = "duplicate data-path appearance themes" },
            @{ Pattern = '^backends\.json$'; Label = "staged backend manifest" },
            @{ Pattern = '^backends\.moqi-ime\.json$'; Label = "legacy backend snippet" },
            @{ Pattern = '\.go$'; Label = "Go source file" }
        )) {
            if ($normalized -match $entry.Pattern) {
                Add-Violation $Violations "$Label contains banned $($entry.Label): $relativePath"
            }
        }
        foreach ($textViolation in (Test-BannedRuntimeText -FullPath $file.FullName -RelativePath $relativePath)) {
            Add-Violation $Violations "$Label contains banned shipped text: $textViolation"
        }
    }
}

function Test-BackendDiagnostics {
    param(
        [System.Collections.Generic.List[string]] $Violations,
        [string] $Backend
    )

    $sourceFiles = @(Get-ChildItem -LiteralPath $Backend -Recurse -Force -File -Include *.go,*.ps1 -ErrorAction SilentlyContinue | Where-Object {
        $relativePath = Get-RelativePath -Root $Backend -Path $_.FullName
        -not (Test-ExcludedSourcePath -RelativePath $relativePath)
    })

    $callPattern = '(?s)(?<callee>\blog\.(?:Printf|Println|Print|Fatal|Fatalf|Panic|Panicf)|\bdebugLogf|\btraceLogf|\bfmt\.Errorf|\berrors\.New|\bpanic)\s*\((?<args>.*?)\)'
    $linePattern = '(Write-(?:Host|Warning|Error)|\bthrow\b).*$'

    foreach ($file in $sourceFiles) {
        $relativePath = Get-RelativePath -Root $Backend -Path $file.FullName
        $text = Get-Content -Raw -Encoding UTF8 -LiteralPath $file.FullName

        foreach ($match in [regex]::Matches($text, $callPattern)) {
            $args = $match.Groups["args"].Value
            if (Test-HanText -Text $args) {
                $lineNumber = ($text.Substring(0, $match.Index) -split "`r?`n").Count
                Add-Violation $Violations "${relativePath}:$lineNumber contains Han text in $($match.Groups["callee"].Value) diagnostic arguments"
            }
        }

        $lines = $text -split "`r?`n"
        for ($i = 0; $i -lt $lines.Count; $i++) {
            $line = $lines[$i]
            if (($line -match $linePattern) -and (Test-HanText -Text $line)) {
                Add-Violation $Violations "${relativePath}:$($i + 1) contains Han text in PowerShell diagnostic line"
            }
            if ($line -match 'MoqiIM|moqi-ime\.log') {
                Add-Violation $Violations "${relativePath}:$($i + 1) uses legacy backend log path contract"
            }
            if ($relativePath -match '_test\.go$' -and (Test-HanText -Text $line) -and
                $line -match '(?i)(error|log|diagnostic|MoqiIM|moqi-ime|未初始化|未知|失败|失敗|錯誤|错误)') {
                Add-Violation $Violations "${relativePath}:$($i + 1) appears to assert a non-English diagnostic string"
            }
        }
    }
}

$repo = Resolve-FullPath -Path $RepoRoot
$backend = Resolve-FullPath -Path $BackendRoot
$violations = [System.Collections.Generic.List[string]]::new()

if (-not (Test-Path -LiteralPath $backend -PathType Container)) {
    throw "BackendRoot does not exist: $backend"
}

Test-BackendDiagnostics -Violations $violations -Backend $backend

$backendPackageRoot = Join-Path $backend "scripts\build\TypeDuckRuntime"
Test-RuntimeTree -Violations $violations -Root $backendPackageRoot -Label "Backend TypeDuckRuntime package"

$stagePackageRoot = Join-Path $repo "installer\stage\win32\TypeDuckIME\TypeDuckRuntime"
Test-RuntimeTree -Violations $violations -Root $stagePackageRoot -Label "Windows staged TypeDuckRuntime"

if ($ExpectRed -eq "BackendDiagnostics") {
    if ($violations.Count -eq 0) {
        throw "Expected RED BackendDiagnostics, but no backend diagnostic or package violations were found."
    }
    Write-Host "PASS RED: BackendDiagnostics caught backend diagnostic/package gaps:"
    foreach ($violation in $violations) {
        Write-Host " - $violation"
    }
    exit 0
}

if ($violations.Count -gt 0) {
    throw "Rejected backend diagnostics or package surfaces found: $($violations -join '; ')"
}

Write-Host "[PASS] TypeDuck backend diagnostics guard passed."
