#Requires -Version 5.1
<#
.SYNOPSIS
  Verifies the TypeDuck VM installer verification harness contract.

.PARAMETER RepoRoot
  Root of the TypeDuck Windows IME checkout.
#>
param(
    [string] $RepoRoot = ""
)

$ErrorActionPreference = "Stop"

if (-not $RepoRoot) {
    $RepoRoot = Join-Path $PSScriptRoot ".."
}
$RepoRoot = [System.IO.Path]::GetFullPath($RepoRoot)
$harness = Join-Path $RepoRoot "scripts\Invoke-TypeDuckVmInstallerVerification.ps1"

function Assert-True {
    param(
        [bool] $Condition,
        [string] $Message
    )
    if (-not $Condition) {
        throw $Message
    }
}

Assert-True (Test-Path -LiteralPath $harness) "Harness script not found: $harness"

$content = Get-Content -Raw -LiteralPath $harness
foreach ($requiredParameter in @(
        "VmName",
        "InstallerPath",
        "GuestUser",
        "GuestPassword",
        "CheckpointName",
        "EvidenceRoot",
        "ManualChecklistOnly",
        "SkipCheckpoint"
    )) {
    Assert-True ($content -match "\`$$requiredParameter\b") "Missing parameter: $requiredParameter"
}

$tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("typeduck-vm-harness-test-" + [System.Guid]::NewGuid().ToString("N"))
New-Item -ItemType Directory -Path $tempRoot -Force | Out-Null
try {
    & $harness -ManualChecklistOnly -EvidenceRoot $tempRoot

    $notesPath = Join-Path $tempRoot "verification-notes.md"
    $jsonPath = Join-Path $tempRoot "vm-install-registration-uninstall.json"
    Assert-True (Test-Path -LiteralPath $notesPath) "Manual checklist notes were not created."
    Assert-True (Test-Path -LiteralPath $jsonPath) "Evidence JSON was not created."

    $notes = Get-Content -Raw -LiteralPath $notesPath
    foreach ($expectedText in @(
            "installer UI text",
            "Chinese (Traditional, Hong Kong)",
            "Win32/x64 DLL registration",
            "startup entry",
            "scheduled task",
            "uninstall cleanup",
            "screenshots"
        )) {
        Assert-True ($notes -like "*$expectedText*") "Checklist missing expected text: $expectedText"
    }

    $evidence = Get-Content -Raw -LiteralPath $jsonPath | ConvertFrom-Json
    Assert-True ($evidence.mode -eq "manual-checklist") "Unexpected evidence mode: $($evidence.mode)"
    Assert-True ($evidence.installer.expectedFileName -eq "typeduck-windows-ime-setup.exe") "Installer filename contract missing."
    Assert-True ($evidence.requirements.Count -ge 5) "Requirement coverage was not recorded."
}
finally {
    if (Test-Path -LiteralPath $tempRoot) {
        Remove-Item -LiteralPath $tempRoot -Recurse -Force
    }
}

Write-Host "TypeDuck VM installer verification harness contract passed."
