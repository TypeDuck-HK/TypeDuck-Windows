# TypeDuck Phase 03 VM Installer Verification Notes

**Mode:** manual checklist
**Reason:** ManualChecklistOnly was specified.
**Started:** 2026-06-23T16 時 31 時 36Z
**VM:** My Virtual Machine
**Installer:** D:\VSProjects\moqi-im-windows\installer\dist\typeduck-windows-ime-setup.exe
**Installer SHA-256:** 7d72748067d029af188078d27683928124f3c75ad7d3cc3bc7945ea80176bac4

## Safety

- Do not install on the host machine.
- Use Hyper-V VM My Virtual Machine.
- Create a checkpoint before install unless using an already recorded clean checkpoint.
- Capture screenshots only of installer/input settings surfaces; avoid personal typed content.

## Manual Checklist

1. Host checkpoint:
   - Run `Checkpoint-VM -Name "My Virtual Machine" -SnapshotName "TypeDuck-Phase03-BeforeInstall"` or record the existing clean checkpoint name/id.
   - Record VM Windows version from the guest.

2. installer UI text:
   - Launch D:\VSProjects\moqi-im-windows\installer\dist\typeduck-windows-ime-setup.exe inside the VM.
   - Confirm TypeDuck branding is visible.
   - Confirm TypeDuck-controlled text is bilingual Traditional Hong Kong Chinese / English.
   - Save screenshot as `installer-ui.png` if possible.

3. Chinese (Traditional, Hong Kong) input settings appearance:
   - Open Windows Settings language/input pages in the VM.
   - Confirm `TypeDuck 粵語輸入法 / TypeDuck Cantonese IME` appears under Chinese (Traditional, Hong Kong) / `zh-HK`.
   - Save screenshot as `zh-hk-input-settings.png` if possible.

4. Win32/x64 DLL registration:
   - Confirm `C:\Windows\SysWOW64\TypeDuckTextService.dll` exists and record SHA-256.
   - Confirm `C:\Windows\System32\TypeDuckTextService.dll` exists and record SHA-256.
   - Confirm COM/TSF registration contains CLSID {7D92985A-BC53-47B5-A5CC-6E47F86B9D18}.
   - Confirm profile GUID {C6E8F5DF-6504-44F9-B7CF-17A195373A83} and display text TypeDuck 粵語輸入法 / TypeDuck Cantonese IME are present.

5. startup entry:
   - Confirm `HKCU\Software\Microsoft\Windows\CurrentVersion\Run\TypeDuckLauncher` points to `TypeDuckLauncher.exe`.

6. scheduled task:
   - Confirm task TypeDuckIME-ReRegisterTSF is absent after normal install, or present only when restart-required fallback was documented.

7. uninstall cleanup:
   - Uninstall TypeDuck from Apps & Features or run the Inno uninstaller with `/VERYSILENT /SUPPRESSMSGBOXES /NORESTART`.
   - Confirm TypeDuck disappears from Chinese (Traditional, Hong Kong) input methods.
   - Confirm TypeDuck CLSID/profile keys, startup entry, scheduled task, install directory, SysWOW64 DLL, and System32 DLL are removed.
   - If locked DLLs require reboot, reboot the VM and repeat cleanup checks.

## Expected Evidence Files

- `registry-before.json`
- `registry-after-install.json`
- `registry-after-uninstall.json`
- `vm-install-registration-uninstall.json`
- Optional screenshots/limitations: `installer-ui.png`, `zh-hk-input-settings.png`

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
