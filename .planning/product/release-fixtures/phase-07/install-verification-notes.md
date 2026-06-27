# TypeDuck Phase 7 Install Verification Notes

**Mode:** manual checklist
**Reason:** ManualChecklistOnly was specified for Phase 7 install evidence.
**Started:** 2026-06-27T12:11:00Z
**VM:** <disposable Windows 10/11 VM name>
**Installer:** <path to typeduck-windows-ime-setup.exe>
**Installer SHA-256:** <record SHA-256 before guest copy>
**Installer byte size:** <record byte size before guest copy>
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
| VM name | <disposable Windows 10/11 VM name> |
| VM id | |
| Checkpoint name/id | |
| Windows version/build | |
| Host-safe statement | Installer was not run on the host |

## Artifact Evidence

| Field | Value |
|-------|-------|
| Expected installer name | typeduck-windows-ime-setup.exe |
| Actual installer path | <path to typeduck-windows-ime-setup.exe> |
| SHA-256 command | Get-FileHash -Algorithm SHA256 -LiteralPath <installer> |
| SHA-256 | <record SHA-256 before guest copy> |
| Byte size | <record byte size before guest copy> |
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
