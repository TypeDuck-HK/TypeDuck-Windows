#Requires -Version 5.1
<#
.SYNOPSIS
  Verifies the TypeDuck Phase 7 release install evidence contract.

.PARAMETER RepoRoot
  Root of the TypeDuck Windows IME checkout.

.PARAMETER Strict
  Enables release-blocking checks for host-safe, non-visual-artifact evidence.
#>
param(
    [string] $RepoRoot = "",
    [switch] $Strict
)

$ErrorActionPreference = "Stop"

function Resolve-RepoRoot {
    param([string] $RequestedRoot)

    if ([string]::IsNullOrWhiteSpace($RequestedRoot)) {
        return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
    }
    return [System.IO.Path]::GetFullPath($RequestedRoot)
}

function Add-Failure {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Message
    )

    $Failures.Add($Message) | Out-Null
}

function Read-RequiredText {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Root,
        [string] $RelativePath
    )

    $path = Join-Path $Root $RelativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        Add-Failure $Failures "Required file not found: $RelativePath"
        return ""
    }
    return [System.IO.File]::ReadAllText($path, [System.Text.Encoding]::UTF8)
}

function Read-RequiredJson {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Root,
        [string] $RelativePath
    )

    $text = Read-RequiredText -Failures $Failures -Root $Root -RelativePath $RelativePath
    if ([string]::IsNullOrWhiteSpace($text)) {
        return $null
    }
    try {
        return $text | ConvertFrom-Json
    } catch {
        Add-Failure $Failures "Invalid JSON in ${RelativePath}: $($_.Exception.Message)"
        return $null
    }
}

function Assert-ContainsAll {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [object[]] $Actual,
        [string[]] $Expected,
        [string] $Context
    )

    $actualStrings = @($Actual | ForEach-Object { [string] $_ })
    foreach ($expectedValue in $Expected) {
        if ($actualStrings -notcontains $expectedValue) {
            Add-Failure $Failures "$Context is missing required value '$expectedValue'."
        }
    }
}

function Assert-TextContainsAll {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Text,
        [string[]] $Patterns,
        [string] $Context
    )

    foreach ($pattern in $Patterns) {
        if ($Text -notmatch $pattern) {
            Add-Failure $Failures "$Context is missing required pattern: $pattern"
        }
    }
}

function Get-DelimitedSection {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Text,
        [string] $BeginMarker,
        [string] $EndMarker,
        [string] $Context
    )

    $begin = $Text.IndexOf($BeginMarker, [System.StringComparison]::Ordinal)
    $end = $Text.IndexOf($EndMarker, [System.StringComparison]::Ordinal)
    if ($begin -lt 0 -or $end -lt 0 -or $end -le $begin) {
        Add-Failure $Failures "$Context must contain release evidence markers $BeginMarker and $EndMarker."
        return ""
    }
    return $Text.Substring($begin, $end - $begin + $EndMarker.Length)
}

function Assert-NoRequiredVisualArtifacts {
    param(
        [System.Collections.Generic.List[string]] $Failures,
        [string] $Text,
        [string] $Context
    )

    $blockingPatterns = @(
        "(?i)\b(require|required|requires|save|capture|compare|comparison|manifest)\b.{0,80}\bscreenshot\b",
        "(?i)\bscreenshot\b.{0,80}\b(require|required|requires|save|capture|compare|comparison|manifest|file|path)\b",
        "(?i)\bpreviewScreenshots\b",
        "(?i)\bscreenshotManifest\b",
        "(?i)\bscreenshotCapture\b",
        "(?i)\bautomatedScreenshot\b"
    )
    foreach ($pattern in $blockingPatterns) {
        if ($Text -match $pattern) {
            Add-Failure $Failures "$Context must not require visual artifact automation. Matched pattern: $pattern"
        }
    }
}

$root = Resolve-RepoRoot -RequestedRoot $RepoRoot
$failures = [System.Collections.Generic.List[string]]::new()

$templateRelativePath = ".planning\product\release-fixtures\phase-07\install-verification-template.json"
$harnessRelativePath = "scripts\Invoke-TypeDuckVmInstallerVerification.ps1"

$template = Read-RequiredJson -Failures $failures -Root $root -RelativePath $templateRelativePath
$templateText = Read-RequiredText -Failures $failures -Root $root -RelativePath $templateRelativePath
$harnessText = Read-RequiredText -Failures $failures -Root $root -RelativePath $harnessRelativePath
$releaseHarnessSection = Get-DelimitedSection `
    -Failures $failures `
    -Text $harnessText `
    -BeginMarker "TYPE_DUCK_PHASE07_INSTALL_EVIDENCE_BEGIN" `
    -EndMarker "TYPE_DUCK_PHASE07_INSTALL_EVIDENCE_END" `
    -Context $harnessRelativePath

if ($null -ne $template) {
    Assert-ContainsAll $failures @($template.requiredScenarioIds) @(
        "clean-install",
        "reinstall-upgrade",
        "uninstall-cleanup",
        "reboot-required-registration",
        "bitness-win32-x64",
        "artifact-name-and-sha256"
    ) "Phase 7 install evidence scenario list"

    Assert-ContainsAll $failures @($template.requiredEvidence) @(
        "command-outcomes",
        "install-log-path",
        "uninstall-log-path",
        "sha256",
        "byte-size",
        "registry-state",
        "file-state",
        "scheduled-task-state",
        "human-notes-path",
        "vm-checkpoint",
        "windows-version"
    ) "Phase 7 install evidence requiredEvidence"

    if ($template.installer.expectedFileName -ne "typeduck-windows-ime-setup.exe") {
        Add-Failure $failures "Installer expectedFileName must be typeduck-windows-ime-setup.exe."
    }
    if ($template.expected.win32Dll -ne "C:\Windows\SysWOW64\TypeDuckTextService.dll") {
        Add-Failure $failures "Win32 DLL evidence must target C:\Windows\SysWOW64\TypeDuckTextService.dll."
    }
    if ($template.expected.x64Dll -ne "C:\Windows\System32\TypeDuckTextService.dll") {
        Add-Failure $failures "x64 DLL evidence must target C:\Windows\System32\TypeDuckTextService.dll."
    }
    if ($template.expected.scheduledTask -ne "TypeDuckIME-ReRegisterTSF") {
        Add-Failure $failures "Reboot-required evidence must target TypeDuckIME-ReRegisterTSF."
    }
    if ($template.hostSafety.installerRunsOnHost -ne $false) {
        Add-Failure $failures "hostSafety.installerRunsOnHost must be false."
    }
}

Assert-TextContainsAll $failures $releaseHarnessSection @(
    "phase-07",
    "clean-install",
    "reinstall-upgrade",
    "uninstall-cleanup",
    "reboot-required-registration",
    "bitness-win32-x64",
    "artifact-name-and-sha256",
    "typeduck-windows-ime-setup\.exe",
    "TypeDuckTextService\.dll",
    "TypeDuckIME-ReRegisterTSF",
    "ManualChecklistOnly",
    'installerRunsOnHost\s*=\s*\$false'
) "Phase 7 release evidence harness section"

Assert-NoRequiredVisualArtifacts $failures $templateText $templateRelativePath
Assert-NoRequiredVisualArtifacts $failures $releaseHarnessSection "Phase 7 release evidence harness section"

if ($Strict) {
    Assert-TextContainsAll $failures $templateText @(
        "commands",
        "logs",
        "sha256",
        "registry",
        "files",
        "scheduledTask",
        "notesPath"
    ) "Strict Phase 7 install evidence template"
}

if ($failures.Count -gt 0) {
    Write-Error ("TypeDuck release install evidence check failed:`n - " + ($failures -join "`n - "))
}

Write-Host "TypeDuck release install evidence check passed."
exit 0
