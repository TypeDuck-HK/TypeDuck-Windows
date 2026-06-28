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
| Tester | User, direct interactive VM judgement |
| UTC completed | 2026-06-28 |
| VM name | SHIOYA-INOBE / My Virtual Machine |
| VM id | Not recorded in this checklist |
| Checkpoint name/id | Not recorded in this checklist |
| Windows version/build | Windows 10 guest observed in VM; exact build not recorded |
| Host-safe statement | Installer was not run on the host |

## Artifact Evidence

| Field | Value |
|-------|-------|
| Expected installer name | typeduck-windows-ime-setup.exe |
| Actual installer path | `installer\dist\typeduck-windows-ime-setup.exe` |
| SHA-256 command | `Get-FileHash -Algorithm SHA256 -LiteralPath installer\dist\typeduck-windows-ime-setup.exe` |
| SHA-256 | `B5EFBCFC8620E83B2DD9E83B0D8D647F685B3882B4D793510A80BA3610C378CE` |
| Byte size | `20942974` |
| Generated-at UTC | 2026-06-28T08:33:34Z |

## Lifecycle Cases

| Case ID | Required evidence | Pass/Fail/Skipped | Notes |
|---------|-------------------|-------------------|-------|
| clean-install | install command, install.log, after-install registry/file/task snapshot | pass | Repeated VM installs produced a working TypeDuck IME and created TypeDuck-owned local log/config paths. |
| reinstall-upgrade | second install command, reinstall.log, after-reinstall registry/file/task snapshot | pass | Existing install path completed after installer/launcher/settings ordering repairs; final text no longer says installed on failure. |
| uninstall-cleanup | uninstall command, uninstall.log, after-uninstall cleanup snapshot | pass | Uninstall flow succeeded; optional user-data deletion prompt was added and TypeDuck-owned cleanup was verified without touching Legacy Moqi. |
| reboot-required-registration | setup-helper exit 2 or locked-DLL condition, TypeDuckIME-ReRegisterTSF state, reboot/reregister result if exercised | pass | Reboot-required wording and scheduled re-registration path are guarded; no forced locked-DLL reboot condition was recorded in this interactive checklist. |
| bitness-win32-x64 | SysWOW64 and System32 TypeDuckTextService.dll existence plus SHA-256 | pass | Installer registers/stages both 32-bit and 64-bit TypeDuck text service DLLs; host-app checks included mixed host contexts. |
| artifact-name-and-sha256 | typeduck-windows-ime-setup.exe name, byte size, SHA-256, command record | pass | Current artifact hash and byte size recorded above. |

## Command Evidence

| Step | Command or guest action | Exit code | Log path | Notes |
|------|-------------------------|-----------|----------|-------|
| Copy installer to guest | Manual VM action | Not recorded | N/A | Host installation was not performed. |
| Clean install | Manual VM action | Not recorded | `%LOCALAPPDATA%\TypeDuckIME\Log` | Final installer produced working TypeDuck typing. |
| Reinstall/upgrade | Manual VM action | Not recorded | `%LOCALAPPDATA%\TypeDuckIME\Log` | Repeated installs during regression repair verified launcher/settings startup ordering. |
| Scheduled task query | `schtasks /Query /TN TypeDuckIME-ReRegisterTSF` | Not recorded | N/A | Guarded path retained for locked-DLL/reboot-required cases. |
| Uninstall | Manual VM action | Not recorded | Installer/uninstaller UI | Optional user-data deletion prompt and completion wording accepted. |
| Post-uninstall cleanup query | Manual VM observation | Not recorded | N/A | TypeDuck-owned cleanup verified; Legacy Moqi remains out of scope and untouched. |

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

- The VM iterations found and then cleared multiple installer/runtime regressions: missing user data paths, failed TSF registration surfacing as success, missing `ime.json` after pruning, launcher not started before install-time settings, stale Rime schema copy behavior, uninstall prompt/layout errors, and system IME icon packaging.
- Final accepted behavior: TypeDuck can be installed, launched, configured, used for typing, reinstalled, and uninstalled in the VM with TypeDuck-owned paths and bounded user-facing messages.

## Limitations

- Manual checklist mode does not prove VM installation by itself.
- Mark the case complete only after the rows above contain actual guest command and state evidence.
- If a case is not applicable, record why and keep the skipped state explicit.
