#Requires -Version 5.1
<#
.SYNOPSIS
  Collects VM-backed TypeDuck installer registration and uninstall evidence.

.DESCRIPTION
  This script is intentionally host-safe: it never runs the TypeDuck installer on
  the host machine. Automation mode uses Hyper-V plus PowerShell Direct to copy
  the installer into a named VM, create or record a checkpoint before install,
  run install/uninstall inside the guest, and write evidence artifacts.

  When guest automation is unavailable, use -ManualChecklistOnly to generate a
  checklist/evidence packet without touching the host or VM.
#>
param(
    [string] $VmName = "",
    [string] $InstallerPath = "",
    [string] $GuestUser = "",
    [securestring] $GuestPassword,
    [string] $CheckpointName = "",
    [string] $EvidenceRoot = "",
    [switch] $ManualChecklistOnly,
    [switch] $ReleaseInstallEvidence,
    [switch] $SkipCheckpoint
)

$ErrorActionPreference = "Stop"

Set-StrictMode -Version 2.0

$kExpectedInstallerName = "typeduck-windows-ime-setup.exe"
$kTypeDuckClsid = "{7D92985A-BC53-47B5-A5CC-6E47F86B9D18}"
$kTypeDuckProfileGuid = "{C6E8F5DF-6504-44F9-B7CF-17A195373A83}"
$kTypeDuckDisplayName = "TypeDuck 粵語輸入法 / TypeDuck Cantonese IME"
$kTaskName = "TypeDuckIME-ReRegisterTSF"
$kLauncherValue = "TypeDuckLauncher"
$kGuestWorkDir = "C:\Users\Public\TypeDuckVmVerify"
$kGuestInstallerPath = Join-Path $kGuestWorkDir $kExpectedInstallerName
$kGuestInstallLog = Join-Path $kGuestWorkDir "install.log"
$kGuestUninstallLog = Join-Path $kGuestWorkDir "uninstall.log"
$kJsonEvidenceName = "vm-install-registration-uninstall.json"
$kNotesName = "verification-notes.md"
$kBeforeName = "registry-before.json"
$kAfterInstallName = "registry-after-install.json"
$kAfterUninstallName = "registry-after-uninstall.json"

# TYPE_DUCK_PHASE07_INSTALL_EVIDENCE_BEGIN
$kPhase07EvidenceRootRelative = ".planning\product\release-fixtures\phase-07"
$kPhase07InstallTemplateRelative = ".planning\product\release-fixtures\phase-07\install-verification-template.json"
$kPhase07InstallNotesName = "install-verification-notes.md"
$kPhase07ScenarioIds = @(
    "clean-install",
    "reinstall-upgrade",
    "uninstall-cleanup",
    "reboot-required-registration",
    "bitness-win32-x64",
    "artifact-name-and-sha256"
)
$kPhase07RequiredEvidence = @(
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
)
$kPhase07HostSafety = [ordered]@{
    installerRunsOnHost = $false
    defaultMode = "ManualChecklistOnly"
    installer = "typeduck-windows-ime-setup.exe"
    dll = "TypeDuckTextService.dll"
    scheduledTask = "TypeDuckIME-ReRegisterTSF"
}
# TYPE_DUCK_PHASE07_INSTALL_EVIDENCE_END

function Format-UtcTimestamp {
    param([datetime] $Value = [datetime]::UtcNow)

    return $Value.ToUniversalTime().ToString(
        "yyyy-MM-dd'T'HH':'mm':'ss'Z'",
        [System.Globalization.CultureInfo]::InvariantCulture)
}

function Resolve-RepoRoot {
    return [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
}

function Resolve-EvidenceRoot {
    param([string] $RequestedRoot)

    if ([string]::IsNullOrWhiteSpace($RequestedRoot)) {
        $RequestedRoot = Join-Path (Resolve-RepoRoot) ".planning\product\installer-fixtures\phase-03"
    }
    $fullPath = [System.IO.Path]::GetFullPath($RequestedRoot)
    New-Item -ItemType Directory -Path $fullPath -Force | Out-Null
    return $fullPath
}

function Test-IsPhase07InstallEvidenceMode {
    param([string] $EvidenceRootPath)

    if ($ReleaseInstallEvidence.IsPresent) {
        return $true
    }
    if ([string]::IsNullOrWhiteSpace($EvidenceRootPath)) {
        return $false
    }
    $normalized = [System.IO.Path]::GetFullPath($EvidenceRootPath).TrimEnd('\', '/')
    $expected = [System.IO.Path]::GetFullPath((Join-Path (Resolve-RepoRoot) $kPhase07EvidenceRootRelative)).TrimEnd('\', '/')
    return [string]::Equals($normalized, $expected, [System.StringComparison]::OrdinalIgnoreCase)
}

function Write-JsonFile {
    param(
        [Parameter(Mandatory = $true)] [object] $Value,
        [Parameter(Mandatory = $true)] [string] $Path,
        [int] $Depth = 16
    )

    $json = $Value | ConvertTo-Json -Depth $Depth
    Set-Content -LiteralPath $Path -Value $json -Encoding UTF8
}

function Get-Sha256 {
    param([string] $Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return $null
    }
    return (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToLowerInvariant()
}

function ConvertTo-PlainCredential {
    param(
        [string] $UserName,
        [securestring] $Password
    )

    if ([string]::IsNullOrWhiteSpace($UserName) -or $null -eq $Password) {
        return $null
    }
    return [pscredential]::new($UserName, $Password)
}

function New-ManualChecklistText {
    param(
        [string] $Reason,
        [string] $Vm,
        [string] $Installer,
        [string] $InstallerHash,
        [string] $StartedAt
    )

    $installerDisplay = if ($Installer) { $Installer } else { "<path to typeduck-windows-ime-setup.exe>" }
    $vmDisplay = if ($Vm) { $Vm } else { "<Hyper-V VM name>" }
    return @"
# TypeDuck Phase 03 VM Installer Verification Notes

**Mode:** manual checklist
**Reason:** $Reason
**Started:** $StartedAt
**VM:** $vmDisplay
**Installer:** $installerDisplay
**Installer SHA-256:** $InstallerHash

## Safety

- Do not install on the host machine.
    - Use Hyper-V VM $vmDisplay.
    - Create a checkpoint before install unless using an already recorded clean checkpoint.
    - No screenshots are required.
    - Observe installer/input settings surfaces directly in the VM and record notes only; avoid personal typed content.

## Manual Checklist

1. Host checkpoint:
   - Run ``Checkpoint-VM -Name "$vmDisplay" -SnapshotName "TypeDuck-Phase03-BeforeInstall"`` or record the existing clean checkpoint name/id.
   - Record VM Windows version from the guest.

2. installer UI text:
   - Launch $installerDisplay inside the VM.
   - Confirm TypeDuck branding is visible.
   - Confirm TypeDuck-controlled text is bilingual Traditional Hong Kong Chinese / English.

3. Chinese (Traditional, Hong Kong) input settings appearance:
   - Open Windows Settings language/input pages in the VM.
   - Confirm ``TypeDuck 粵語輸入法 / TypeDuck Cantonese IME`` appears under Chinese (Traditional, Hong Kong) / ``zh-HK``.
   - Record direct observations in this notes file.

4. Win32/x64 DLL registration:
   - Confirm ``C:\Windows\SysWOW64\TypeDuckTextService.dll`` exists and record SHA-256.
   - Confirm ``C:\Windows\System32\TypeDuckTextService.dll`` exists and record SHA-256.
   - Confirm COM/TSF registration contains CLSID $kTypeDuckClsid.
   - Confirm profile GUID $kTypeDuckProfileGuid and display text $kTypeDuckDisplayName are present.

5. startup entry:
   - Confirm ``HKCU\Software\Microsoft\Windows\CurrentVersion\Run\TypeDuckLauncher`` points to ``TypeDuckLauncher.exe``.

6. scheduled task:
   - Confirm task $kTaskName is absent after normal install, or present only when restart-required fallback was documented.

7. uninstall cleanup:
   - Uninstall TypeDuck from Apps & Features or run the Inno uninstaller with ``/VERYSILENT /SUPPRESSMSGBOXES /NORESTART``.
   - Confirm TypeDuck disappears from Chinese (Traditional, Hong Kong) input methods.
   - Confirm TypeDuck CLSID/profile keys, startup entry, scheduled task, install directory, SysWOW64 DLL, and System32 DLL are removed.
   - If locked DLLs require reboot, reboot the VM and repeat cleanup checks.

## Expected Evidence Files

- ``registry-before.json``
- ``registry-after-install.json``
- ``registry-after-uninstall.json``
- ``vm-install-registration-uninstall.json``
- Human observation notes and limitations in this Markdown file.

## Source Audit

GOAL Phase 3: COVERED by Plans 03-01, 03-02, and VM evidence in 03-03.
INST-01: Bilingual TypeDuck-branded installer, covered by installer skeleton and VM UI evidence.
INST-02: Select TypeDuck under Chinese (Traditional, Hong Kong), covered by VM settings/profile evidence.
INST-03: Deterministic CLSID/profile GUID/zh-HK/display text, covered by source constants and VM registry evidence.
INST-04: Win32 and x64 TSF DLL registration, covered by SysWOW64/System32 evidence.
INST-05: TypeDuck-owned uninstall cleanup, covered by after-uninstall VM evidence.
D-01 through D-16: Covered by Plans 03-01 and 03-02 source changes.
D-17 through D-19: Covered only when this packet is completed against a disposable VM; static checks alone are insufficient.

## Limitations

- Manual checklist mode does not prove VM installation by itself.
- Mark Plan 03-03 complete only after the VM evidence files above contain actual guest snapshots.
"@
}

function New-Phase07InstallVerificationNotesText {
    param(
        [string] $Reason,
        [string] $Vm,
        [string] $Installer,
        [string] $InstallerHash,
        [string] $InstallerLength,
        [string] $StartedAt
    )

    $installerDisplay = if ($Installer) { $Installer } else { "<path to typeduck-windows-ime-setup.exe>" }
    $vmDisplay = if ($Vm) { $Vm } else { "<disposable Windows 10/11 VM name>" }
    $hashDisplay = if ($InstallerHash) { $InstallerHash } else { "<record SHA-256 before guest copy>" }
    $lengthDisplay = if ($InstallerLength) { $InstallerLength } else { "<record byte size before guest copy>" }
    return @"
# TypeDuck Phase 7 Install Verification Notes

**Mode:** manual checklist
**Reason:** $Reason
**Started:** $StartedAt
**VM:** $vmDisplay
**Installer:** $installerDisplay
**Installer SHA-256:** $hashDisplay
**Installer byte size:** $lengthDisplay
**Template:** .planning/product/release-fixtures/phase-07/install-verification-template.json

## Safety

- Do not install on the host machine.
- Use a disposable Windows 10/11 VM or equivalent checkpointed guest.
- Runtime VM credentials are supplied only at execution time and must not be written into evidence.
- Record command output, logs, hashes, registry/file/task state, Windows version, checkpoint identity, and human notes only.

## Session Identity

| Field | Value |
|-------|-------|
| Tester | |
| UTC completed | |
| VM name | $vmDisplay |
| VM id | |
| Checkpoint name/id | |
| Windows version/build | |
| Host-safe statement | Installer was not run on the host |

## Artifact Evidence

| Field | Value |
|-------|-------|
| Expected installer name | typeduck-windows-ime-setup.exe |
| Actual installer path | $installerDisplay |
| SHA-256 command | Get-FileHash -Algorithm SHA256 -LiteralPath <installer> |
| SHA-256 | $hashDisplay |
| Byte size | $lengthDisplay |
| Generated-at UTC | |

## Lifecycle Cases

| Case ID | Required evidence | Pass/Fail/Skipped | Notes |
|---------|-------------------|-------------------|-------|
| clean-install | install command, install.log, after-install registry/file/task snapshot | pending | |
| reinstall-upgrade | second install command, reinstall.log, after-reinstall registry/file/task snapshot | pending | |
| uninstall-cleanup | uninstall command, uninstall.log, after-uninstall cleanup snapshot | pending | |
| reboot-required-registration | setup-helper exit 2 or locked-DLL condition, TypeDuckIME-ReRegisterTSF state, reboot/reregister result if exercised | pending | |
| bitness-win32-x64 | SysWOW64 and System32 TypeDuckTextService.dll existence plus SHA-256 | pending | |
| artifact-name-and-sha256 | typeduck-windows-ime-setup.exe name, byte size, SHA-256, command record | pending | |

## Command Evidence

| Step | Command or guest action | Exit code | Log path | Notes |
|------|-------------------------|-----------|----------|-------|
| Copy installer to guest | | | | |
| Clean install | | | | |
| Reinstall/upgrade | | | | |
| Scheduled task query | schtasks /Query /TN TypeDuckIME-ReRegisterTSF | | | |
| Uninstall | | | | |
| Post-uninstall cleanup query | | | | |

## Expected TypeDuck State

| Item | Expected path/value | Evidence |
|------|---------------------|----------|
| CLSID | {7D92985A-BC53-47B5-A5CC-6E47F86B9D18} | |
| Profile GUID | {C6E8F5DF-6504-44F9-B7CF-17A195373A83} | |
| Locale | zh-HK | |
| Display text | TypeDuck 粵語輸入法 / TypeDuck Cantonese IME | |
| Win32 DLL | C:\Windows\SysWOW64\TypeDuckTextService.dll | |
| x64 DLL | C:\Windows\System32\TypeDuckTextService.dll | |
| Startup entry | HKCU\Software\Microsoft\Windows\CurrentVersion\Run\TypeDuckLauncher | |
| Re-registration task | TypeDuckIME-ReRegisterTSF | |

## Human Observation Notes

Record direct VM observations here. Visual/DPI and host-app judgement belongs to the Phase 7 interactive checklist and notes files; this installer packet records only install lifecycle facts and notes.

## Limitations

- Manual checklist mode does not prove VM installation by itself.
- Mark the case complete only after the rows above contain actual guest command and state evidence.
- If a case is not applicable, record why and keep the skipped state explicit.
"@
}

function Write-Phase07ManualPacket {
    param(
        [string] $EvidenceRootPath,
        [string] $Reason,
        [string] $StartedAt,
        [string] $InstallerFullPath = ""
    )

    $installerHash = if ($InstallerFullPath) { Get-Sha256 -Path $InstallerFullPath } else { $null }
    $installerLength = ""
    if ($InstallerFullPath -and (Test-Path -LiteralPath $InstallerFullPath)) {
        $installerLength = [string] (Get-Item -LiteralPath $InstallerFullPath).Length
    }
    Set-Content -LiteralPath (Join-Path $EvidenceRootPath $kPhase07InstallNotesName) `
        -Value (New-Phase07InstallVerificationNotesText `
            -Reason $Reason `
            -Vm $VmName `
            -Installer $InstallerFullPath `
            -InstallerHash $installerHash `
            -InstallerLength $installerLength `
            -StartedAt $StartedAt) `
        -Encoding UTF8
}

function Write-ManualPacket {
    param(
        [string] $EvidenceRootPath,
        [string] $Reason,
        [string] $StartedAt,
        [string] $InstallerFullPath = ""
    )

    $installerHash = if ($InstallerFullPath) { Get-Sha256 -Path $InstallerFullPath } else { $null }
    $packet = [ordered]@{
        mode = "manual-checklist"
        status = "needs-human-vm-verification"
        reason = $Reason
        startedAt = $StartedAt
        endedAt = Format-UtcTimestamp
        vm = [ordered]@{
            name = $VmName
            checkpointRequired = -not $SkipCheckpoint.IsPresent
        }
        installer = [ordered]@{
            expectedFileName = $kExpectedInstallerName
            path = $InstallerFullPath
            sha256 = $installerHash
        }
        requirements = @("INST-01", "INST-02", "INST-03", "INST-04", "INST-05")
        expected = [ordered]@{
            clsid = $kTypeDuckClsid
            profileGuid = $kTypeDuckProfileGuid
            locale = "zh-HK"
            displayName = $kTypeDuckDisplayName
            win32Dll = "C:\Windows\SysWOW64\TypeDuckTextService.dll"
            x64Dll = "C:\Windows\System32\TypeDuckTextService.dll"
            startupValue = $kLauncherValue
            scheduledTask = $kTaskName
        }
    }
    Write-JsonFile -Value $packet -Path (Join-Path $EvidenceRootPath $kJsonEvidenceName)
    Set-Content -LiteralPath (Join-Path $EvidenceRootPath $kNotesName) `
        -Value (New-ManualChecklistText -Reason $Reason -Vm $VmName -Installer $InstallerFullPath -InstallerHash $installerHash -StartedAt $StartedAt) `
        -Encoding UTF8
}

function Get-HyperVCheckpointMetadata {
    param(
        [string] $Vm,
        [string] $Name,
        [bool] $Skip
    )

    if ($Skip) {
        return [ordered]@{
            skipped = $true
            name = $Name
            id = $null
            creationTime = $null
        }
    }

    if ([string]::IsNullOrWhiteSpace($Name)) {
        $Name = "TypeDuck-Phase03-BeforeInstall-{0}" -f (Get-Date -Format "yyyyMMdd-HHmmss")
    }

    $snapshot = Checkpoint-VM -Name $Vm -SnapshotName $Name -Passthru
    return [ordered]@{
        skipped = $false
        name = $snapshot.Name
        id = $snapshot.Id.ToString()
        creationTime = Format-UtcTimestamp -Value $snapshot.CreationTime
    }
}

function New-GuestSession {
    param(
        [string] $Vm,
        [pscredential] $Credential
    )

    if ($null -ne $Credential) {
        return New-PSSession -VMName $Vm -Credential $Credential
    }
    return New-PSSession -VMName $Vm
}

function Invoke-GuestScript {
    param(
        [System.Management.Automation.Runspaces.PSSession] $Session,
        [scriptblock] $ScriptBlock,
        [object[]] $ArgumentList = @()
    )

    return Invoke-Command -Session $Session -ScriptBlock $ScriptBlock -ArgumentList $ArgumentList
}

function Save-GuestSnapshot {
    param(
        [System.Management.Automation.Runspaces.PSSession] $Session,
        [string] $EvidenceRootPath,
        [string] $FileName,
        [string] $Label
    )

    $snapshot = Invoke-GuestScript -Session $Session -ScriptBlock {
        param(
            [string] $Label,
            [string] $Clsid,
            [string] $ProfileGuid,
            [string] $TaskName,
            [string] $LauncherValue
        )

        function Format-UtcTimestamp {
            param([datetime] $Value = [datetime]::UtcNow)

            return $Value.ToUniversalTime().ToString(
                "yyyy-MM-dd'T'HH':'mm':'ss'Z'",
                [System.Globalization.CultureInfo]::InvariantCulture)
        }

        function Get-RegistryNode {
            param([string] $Path)

            if (-not (Test-Path -LiteralPath $Path)) {
                return [ordered]@{
                    path = $Path
                    exists = $false
                    values = @{}
                    children = @()
                }
            }

            $item = Get-Item -LiteralPath $Path
            $props = Get-ItemProperty -LiteralPath $Path
            $values = [ordered]@{}
            foreach ($property in $props.PSObject.Properties) {
                if ($property.Name -notin @("PSPath", "PSParentPath", "PSChildName", "PSDrive", "PSProvider")) {
                    $values[$property.Name] = [string] $property.Value
                }
            }
            $children = @()
            foreach ($child in Get-ChildItem -LiteralPath $Path -ErrorAction SilentlyContinue) {
                $children += Get-RegistryNode -Path $child.PSPath
            }
            return [ordered]@{
                path = $Path
                exists = $true
                values = $values
                children = $children
            }
        }

        function Get-FileEvidence {
            param([string] $Path)

            if (-not (Test-Path -LiteralPath $Path)) {
                return [ordered]@{
                    path = $Path
                    exists = $false
                    sha256 = $null
                    length = $null
                    lastWriteTimeUtc = $null
                }
            }
            $item = Get-Item -LiteralPath $Path
            return [ordered]@{
                path = $Path
                exists = $true
                sha256 = (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToLowerInvariant()
                length = $item.Length
                lastWriteTimeUtc = Format-UtcTimestamp -Value $item.LastWriteTimeUtc
            }
        }

        function Get-UninstallEntries {
            $roots = @(
                "HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall",
                "HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall",
                "HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall"
            )
            $entries = @()
            foreach ($root in $roots) {
                if (-not (Test-Path -LiteralPath $root)) {
                    continue
                }
                foreach ($item in Get-ChildItem -LiteralPath $root -ErrorAction SilentlyContinue) {
                    $props = Get-ItemProperty -LiteralPath $item.PSPath -ErrorAction SilentlyContinue
                    if ($props.DisplayName -like "*TypeDuck*") {
                        $entries += [ordered]@{
                            key = $item.Name
                            displayName = [string] $props.DisplayName
                            uninstallString = [string] $props.UninstallString
                            quietUninstallString = [string] $props.QuietUninstallString
                        }
                    }
                }
            }
            return $entries
        }

        $os = Get-CimInstance Win32_OperatingSystem
        $languageList = @()
        try {
            $languageList = Get-WinUserLanguageList | ForEach-Object {
                [ordered]@{
                    languageTag = $_.LanguageTag
                    inputMethodTips = @($_.InputMethodTips)
                }
            }
        } catch {
            $languageList = @([ordered]@{ error = $_.Exception.Message })
        }

        $task = $null
        try {
            $taskObj = Get-ScheduledTask -TaskName $TaskName -ErrorAction Stop
            $task = [ordered]@{
                exists = $true
                taskName = $taskObj.TaskName
                state = [string] $taskObj.State
                taskPath = $taskObj.TaskPath
            }
        } catch {
            $task = [ordered]@{
                exists = $false
                taskName = $TaskName
            }
        }

        $runValue = $null
        try {
            $runProps = Get-ItemProperty -LiteralPath "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run" -ErrorAction Stop
            $runValue = [string] $runProps.$LauncherValue
        } catch {
            $runValue = $null
        }

        $programFilesX86 = [Environment]::GetFolderPath("ProgramFilesX86")
        [ordered]@{
            label = $Label
            capturedAt = Format-UtcTimestamp
            os = [ordered]@{
                caption = $os.Caption
                version = $os.Version
                buildNumber = $os.BuildNumber
                osArchitecture = $os.OSArchitecture
            }
            expected = [ordered]@{
                clsid = $Clsid
                profileGuid = $ProfileGuid
                locale = "zh-HK"
                displayName = "TypeDuck 粵語輸入法 / TypeDuck Cantonese IME"
            }
            registry = [ordered]@{
                hklmTip = Get-RegistryNode -Path "HKLM:\SOFTWARE\Microsoft\CTF\TIP\$Clsid"
                hkcuTip = Get-RegistryNode -Path "HKCU:\Software\Microsoft\CTF\TIP\$Clsid"
                hkcrClsid = Get-RegistryNode -Path "Registry::HKEY_CLASSES_ROOT\CLSID\$Clsid"
                hkcuClsid = Get-RegistryNode -Path "HKCU:\Software\Classes\CLSID\$Clsid"
                runTypeDuckLauncher = $runValue
            }
            files = [ordered]@{
                installDir = [ordered]@{
                    path = (Join-Path $programFilesX86 "TypeDuckIME")
                    exists = (Test-Path -LiteralPath (Join-Path $programFilesX86 "TypeDuckIME"))
                }
                win32Dll = Get-FileEvidence -Path "C:\Windows\SysWOW64\TypeDuckTextService.dll"
                x64Dll = Get-FileEvidence -Path "C:\Windows\System32\TypeDuckTextService.dll"
            }
            scheduledTask = $task
            languageList = $languageList
            uninstallEntries = Get-UninstallEntries
        }
    } -ArgumentList @($Label, $kTypeDuckClsid, $kTypeDuckProfileGuid, $kTaskName, $kLauncherValue)

    Write-JsonFile -Value $snapshot -Path (Join-Path $EvidenceRootPath $FileName) -Depth 32
    return $snapshot
}

function Invoke-GuestInstaller {
    param([System.Management.Automation.Runspaces.PSSession] $Session)

    return Invoke-GuestScript -Session $Session -ScriptBlock {
        param([string] $WorkDir, [string] $InstallerPath, [string] $InstallLog)

        New-Item -ItemType Directory -Path $WorkDir -Force | Out-Null
        if (-not (Test-Path -LiteralPath $InstallerPath)) {
            throw "Installer not found inside guest: $InstallerPath"
        }
        $args = "/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /LOG=`"$InstallLog`""
        $process = Start-Process -FilePath $InstallerPath -ArgumentList $args -Wait -PassThru
        return [ordered]@{
            exitCode = $process.ExitCode
            logPath = $InstallLog
            logTail = if (Test-Path -LiteralPath $InstallLog) { (Get-Content -LiteralPath $InstallLog -Tail 80) -join "`n" } else { "" }
        }
    } -ArgumentList @($kGuestWorkDir, $kGuestInstallerPath, $kGuestInstallLog)
}

function Invoke-GuestUninstaller {
    param([System.Management.Automation.Runspaces.PSSession] $Session)

    return Invoke-GuestScript -Session $Session -ScriptBlock {
        param([string] $UninstallLog)

        $roots = @(
            "HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall",
            "HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall",
            "HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall"
        )
        $uninstallString = $null
        foreach ($root in $roots) {
            if (-not (Test-Path -LiteralPath $root)) {
                continue
            }
            foreach ($item in Get-ChildItem -LiteralPath $root -ErrorAction SilentlyContinue) {
                $props = Get-ItemProperty -LiteralPath $item.PSPath -ErrorAction SilentlyContinue
                if ($props.DisplayName -like "*TypeDuck*" -and $props.UninstallString) {
                    $uninstallString = [string] $props.UninstallString
                    break
                }
            }
            if ($uninstallString) {
                break
            }
        }
        if (-not $uninstallString) {
            throw "TypeDuck uninstall entry not found."
        }

        $exe = $uninstallString.Trim()
        if ($exe.StartsWith('"')) {
            $endQuote = $exe.IndexOf('"', 1)
            $exePath = $exe.Substring(1, $endQuote - 1)
        } else {
            $exePath = ($exe -split "\s+", 2)[0]
        }
        if (-not (Test-Path -LiteralPath $exePath)) {
            throw "Uninstaller executable not found: $exePath"
        }
        $args = "/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /LOG=`"$UninstallLog`""
        $process = Start-Process -FilePath $exePath -ArgumentList $args -Wait -PassThru
        return [ordered]@{
            exitCode = $process.ExitCode
            executable = $exePath
            logPath = $UninstallLog
            logTail = if (Test-Path -LiteralPath $UninstallLog) { (Get-Content -LiteralPath $UninstallLog -Tail 80) -join "`n" } else { "" }
        }
    } -ArgumentList @($kGuestUninstallLog)
}

function Test-InstallSnapshot {
    param([object] $Snapshot)

    $failures = @()
    if (-not $Snapshot.files.win32Dll.exists) { $failures += "Win32 SysWOW64 TypeDuckTextService.dll missing after install." }
    if (-not $Snapshot.files.x64Dll.exists) { $failures += "x64 System32 TypeDuckTextService.dll missing after install." }
    if (-not ($Snapshot.registry.hklmTip.exists -or $Snapshot.registry.hkcuTip.exists)) { $failures += "TypeDuck CTF TIP key missing after install." }
    if (-not ($Snapshot.registry.hkcrClsid.exists -or $Snapshot.registry.hkcuClsid.exists)) { $failures += "TypeDuck CLSID key missing after install." }
    if ([string]::IsNullOrWhiteSpace([string] $Snapshot.registry.runTypeDuckLauncher) -or ([string] $Snapshot.registry.runTypeDuckLauncher) -notlike "*TypeDuckLauncher.exe*") {
        $failures += "TypeDuckLauncher startup entry missing after install."
    }
    return $failures
}

function Test-UninstallSnapshot {
    param([object] $Snapshot)

    $failures = @()
    if ($Snapshot.files.installDir.exists) { $failures += "TypeDuck install directory remains after uninstall." }
    if ($Snapshot.files.win32Dll.exists) { $failures += "Win32 SysWOW64 TypeDuckTextService.dll remains after uninstall." }
    if ($Snapshot.files.x64Dll.exists) { $failures += "x64 System32 TypeDuckTextService.dll remains after uninstall." }
    if ($Snapshot.registry.hklmTip.exists -or $Snapshot.registry.hkcuTip.exists) { $failures += "TypeDuck CTF TIP key remains after uninstall." }
    if ($Snapshot.registry.hkcrClsid.exists -or $Snapshot.registry.hkcuClsid.exists) { $failures += "TypeDuck CLSID key remains after uninstall." }
    if (-not [string]::IsNullOrWhiteSpace([string] $Snapshot.registry.runTypeDuckLauncher)) { $failures += "TypeDuckLauncher startup entry remains after uninstall." }
    if ($Snapshot.scheduledTask.exists) { $failures += "TypeDuck re-registration scheduled task remains after uninstall." }
    return $failures
}

function Write-AutomationNotes {
    param(
        [string] $EvidenceRootPath,
        [object] $Evidence,
        [string[]] $InstallFailures,
        [string[]] $UninstallFailures
    )

    $status = if ($Evidence.status -eq "complete") { "complete" } else { "attention-needed" }
    $limitations = if ($Evidence.limitations.Count -gt 0) { ($Evidence.limitations | ForEach-Object { "- $_" }) -join "`n" } else { "None" }
    $installFailureText = if ($InstallFailures.Count -gt 0) { ($InstallFailures | ForEach-Object { "- $_" }) -join "`n" } else { "None" }
    $uninstallFailureText = if ($UninstallFailures.Count -gt 0) { ($UninstallFailures | ForEach-Object { "- $_" }) -join "`n" } else { "None" }

    $notes = @"
# TypeDuck Phase 03 VM Installer Verification Notes

**Mode:** automated Hyper-V / PowerShell Direct
**Status:** $status
**VM:** $($Evidence.vm.name)
**Checkpoint:** $($Evidence.vm.checkpoint.name) / $($Evidence.vm.checkpoint.id)
**Started:** $($Evidence.startedAt)
**Completed:** $($Evidence.endedAt)
**Installer SHA-256:** $($Evidence.installer.sha256)

## Verification Results

- Installer exit code: $($Evidence.install.exitCode)
- Uninstaller exit code: $($Evidence.uninstall.exitCode)
- CLSID: $kTypeDuckClsid
- Profile GUID: $kTypeDuckProfileGuid
- Display text: $kTypeDuckDisplayName
- Win32 DLL: `C:\Windows\SysWOW64\TypeDuckTextService.dll`
- x64 DLL: `C:\Windows\System32\TypeDuckTextService.dll`
- Startup value: $kLauncherValue
- Scheduled task: $kTaskName

## After-Install Failures

$installFailureText

## After-Uninstall Failures

$uninstallFailureText

## Human Visual Notes

- Automation collected registry, file, task, language-list, and uninstall-entry snapshots.
- If product review needs visual judgement, observe Windows Settings directly in the VM and record notes only.
- Observation notes must avoid personal typed content.

## Limitations

$limitations

## Source Audit

GOAL Phase 3: COVERED by Plans 03-01, 03-02, and this VM evidence packet.
INST-01: Bilingual TypeDuck-branded installer, covered by Plan 03-02 source plus installer execution evidence and direct UI observation notes.
INST-02: Select TypeDuck under Chinese (Traditional, Hong Kong), covered by TypeDuck zh-HK profile registry/language evidence and direct Settings observation notes.
INST-03: Deterministic CLSID/profile GUID/zh-HK/display text, covered by Plan 03-01 constants and VM registry snapshots.
INST-04: Win32 and x64 TSF DLL registration, covered by SysWOW64/System32 file hashes and regsvr-created registry snapshots.
INST-05: TypeDuck-owned uninstall cleanup, covered by after-uninstall snapshots.
D-01 through D-04: Covered by Plan 03-01 first-party TypeDuck profile registration.
D-05 through D-16: Covered by Plan 03-02 installer/setup/staging changes.
D-17 through D-19: Covered by this VM checkpoint and install/uninstall evidence; no host install was attempted.
"@
    Set-Content -LiteralPath (Join-Path $EvidenceRootPath $kNotesName) -Value $notes -Encoding UTF8
}

$startedAt = Format-UtcTimestamp
$resolvedEvidenceRoot = Resolve-EvidenceRoot -RequestedRoot $EvidenceRoot

if ($ManualChecklistOnly) {
    $resolvedInstaller = ""
    if (-not [string]::IsNullOrWhiteSpace($InstallerPath)) {
        $resolvedInstaller = [System.IO.Path]::GetFullPath($InstallerPath)
    }
    if (Test-IsPhase07InstallEvidenceMode -EvidenceRootPath $resolvedEvidenceRoot) {
        Write-Phase07ManualPacket -EvidenceRootPath $resolvedEvidenceRoot -Reason "ManualChecklistOnly was specified for Phase 7 install evidence." -StartedAt $startedAt -InstallerFullPath $resolvedInstaller
    } else {
        Write-ManualPacket -EvidenceRootPath $resolvedEvidenceRoot -Reason "ManualChecklistOnly was specified." -StartedAt $startedAt -InstallerFullPath $resolvedInstaller
    }
    Write-Host "Manual verification packet written to: $resolvedEvidenceRoot"
    exit 0
}

if ([string]::IsNullOrWhiteSpace($VmName) -or [string]::IsNullOrWhiteSpace($InstallerPath)) {
    Write-ManualPacket -EvidenceRootPath $resolvedEvidenceRoot -Reason "Automation requires -VmName and -InstallerPath." -StartedAt $startedAt
    throw "Refusing to run without -VmName and -InstallerPath. Use -ManualChecklistOnly to generate the manual packet."
}

$resolvedInstallerPath = [System.IO.Path]::GetFullPath($InstallerPath)
if (-not (Test-Path -LiteralPath $resolvedInstallerPath)) {
    Write-ManualPacket -EvidenceRootPath $resolvedEvidenceRoot -Reason "Installer was not found: $resolvedInstallerPath" -StartedAt $startedAt -InstallerFullPath $resolvedInstallerPath
    throw "Installer not found: $resolvedInstallerPath"
}

if ((Split-Path -Leaf $resolvedInstallerPath) -ne $kExpectedInstallerName) {
    Write-Warning "Installer filename is not ${kExpectedInstallerName}: $resolvedInstallerPath"
}

$vm = Get-VM -Name $VmName -ErrorAction Stop
if ($vm.State -ne "Running") {
    Write-ManualPacket -EvidenceRootPath $resolvedEvidenceRoot -Reason "VM is not running: $VmName ($($vm.State))" -StartedAt $startedAt -InstallerFullPath $resolvedInstallerPath
    throw "VM must be running for PowerShell Direct: $VmName ($($vm.State))"
}

$checkpoint = Get-HyperVCheckpointMetadata -Vm $VmName -Name $CheckpointName -Skip:$SkipCheckpoint.IsPresent
$credential = ConvertTo-PlainCredential -UserName $GuestUser -Password $GuestPassword
$session = $null

try {
    try {
        $session = New-GuestSession -Vm $VmName -Credential $credential
    } catch {
        Write-ManualPacket -EvidenceRootPath $resolvedEvidenceRoot -Reason ("PowerShell Direct session failed: " + $_.Exception.Message) -StartedAt $startedAt -InstallerFullPath $resolvedInstallerPath
        throw "PowerShell Direct session failed. Manual VM verification is required. $($_.Exception.Message)"
    }

    Invoke-GuestScript -Session $session -ScriptBlock {
        param([string] $WorkDir)
        New-Item -ItemType Directory -Path $WorkDir -Force | Out-Null
    } -ArgumentList @($kGuestWorkDir) | Out-Null

    Copy-Item -LiteralPath $resolvedInstallerPath -Destination $kGuestInstallerPath -ToSession $session -Force

    $beforeSnapshot = Save-GuestSnapshot -Session $session -EvidenceRootPath $resolvedEvidenceRoot -FileName $kBeforeName -Label "before-install"
    $installResult = Invoke-GuestInstaller -Session $session
    $afterInstallSnapshot = Save-GuestSnapshot -Session $session -EvidenceRootPath $resolvedEvidenceRoot -FileName $kAfterInstallName -Label "after-install"
    $installFailures = @(Test-InstallSnapshot -Snapshot $afterInstallSnapshot)

    $uninstallResult = Invoke-GuestUninstaller -Session $session
    $afterUninstallSnapshot = Save-GuestSnapshot -Session $session -EvidenceRootPath $resolvedEvidenceRoot -FileName $kAfterUninstallName -Label "after-uninstall"
    $uninstallFailures = @(Test-UninstallSnapshot -Snapshot $afterUninstallSnapshot)

    $limitations = @()
    if ($afterInstallSnapshot.scheduledTask.exists) {
        $limitations += "Scheduled task $kTaskName existed after install; this is acceptable only when restart-required fallback is documented by install logs."
    }
    $zhHkLanguage = @($afterInstallSnapshot.languageList | Where-Object { $_.languageTag -eq "zh-HK" })
    if ($zhHkLanguage.Count -eq 0) {
        $limitations += "Get-WinUserLanguageList did not report zh-HK for the guest user; rely on CTF/TIP registry evidence and direct Settings observation notes if needed."
    }

    $endedAt = Format-UtcTimestamp
    $status = if ($installFailures.Count -eq 0 -and $uninstallFailures.Count -eq 0 -and $installResult.exitCode -eq 0 -and $uninstallResult.exitCode -eq 0) { "complete" } else { "failed" }

    $evidence = [ordered]@{
        mode = "automated-hyperv-powershell-direct"
        status = $status
        startedAt = $startedAt
        endedAt = $endedAt
        vm = [ordered]@{
            name = $VmName
            id = $vm.Id.ToString()
            state = [string] $vm.State
            checkpoint = $checkpoint
            os = $afterInstallSnapshot.os
        }
        installer = [ordered]@{
            path = $resolvedInstallerPath
            guestPath = $kGuestInstallerPath
            expectedFileName = $kExpectedInstallerName
            sha256 = Get-Sha256 -Path $resolvedInstallerPath
        }
        expected = [ordered]@{
            clsid = $kTypeDuckClsid
            profileGuid = $kTypeDuckProfileGuid
            locale = "zh-HK"
            displayName = $kTypeDuckDisplayName
            win32Dll = "C:\Windows\SysWOW64\TypeDuckTextService.dll"
            x64Dll = "C:\Windows\System32\TypeDuckTextService.dll"
            startupValue = $kLauncherValue
            scheduledTask = $kTaskName
        }
        install = $installResult
        uninstall = $uninstallResult
        snapshots = [ordered]@{
            before = $kBeforeName
            afterInstall = $kAfterInstallName
            afterUninstall = $kAfterUninstallName
        }
        checks = [ordered]@{
            afterInstallFailures = $installFailures
            afterUninstallFailures = $uninstallFailures
        }
        limitations = $limitations
        requirements = @("INST-01", "INST-02", "INST-03", "INST-04", "INST-05")
    }

    Write-JsonFile -Value $evidence -Path (Join-Path $resolvedEvidenceRoot $kJsonEvidenceName) -Depth 32
    Write-AutomationNotes -EvidenceRootPath $resolvedEvidenceRoot -Evidence $evidence -InstallFailures $installFailures -UninstallFailures $uninstallFailures

    if ($status -ne "complete") {
        throw "VM verification completed with failures. See $resolvedEvidenceRoot."
    }

    Write-Host "VM installer verification evidence written to: $resolvedEvidenceRoot"
}
finally {
    if ($null -ne $session) {
        Remove-PSSession -Session $session
    }
}
