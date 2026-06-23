---
phase: 03-zh-hk-tsf-registration-and-installer-skeleton
verified: 2026-06-23T17:10:00Z
status: passed
score: 5/5 must-haves verified
behavior_unverified: 0
overrides_applied: 0
behavior_unverified_items: []
human_verification: complete
---

# Phase 3: zh-HK TSF Registration and Installer Skeleton Verification Report

**Phase Goal:** User can install and remove TypeDuck as a Chinese (Traditional, Hong Kong) Windows IME with deterministic first-party registration.  
**Verified:** 2026-06-23T17:10:00Z  
**Status:** passed  
**Re-verification:** Yes - human UAT completed the two original behavior-unverified checks

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | User can run a TypeDuck-branded installer worded in bilingual Traditional Hong Kong Chinese and English. | VERIFIED | `installer/MoqiTsf.iss` uses TypeDuck app name/publisher/output and bilingual product-controlled strings; `scripts\Test-TypeDuckInstallerSkeleton.ps1 -Strict` passed; VM installer exited 0. |
| 2 | User can select TypeDuck under Windows Chinese (Traditional, Hong Kong) language/input settings after installation. | VERIFIED | After-install registry has `LanguageProfile\0x00000c04\{C6E8F5DF-...}` with display text, `HiddenInSettingUI=0`, and `Enable=1`; human UAT confirmed Settings UI selectability in the VM. |
| 3 | TypeDuck appears with deterministic TypeDuck-owned CLSID/profile GUID, zh-HK locale metadata, TypeDuck icon, and bilingual display text. | VERIFIED | `TypeDuckProfile.cpp` centralizes CLSID `{7D92985A-...}`, profile `{C6E8F5DF-...}`, `zh-HK`, display text, and DLL name; after-install HKLM TIP registry contains description and `IconFile=C:\Windows\System32\TypeDuckTextService.dll`. |
| 4 | TypeDuck works from both 32-bit and 64-bit host applications after installer registration. | VERIFIED | SetupHelper copies/registers SysWOW64 and System32 DLLs through matching regsvr32 paths; VM snapshot shows both DLLs present; human UAT confirmed 32-bit and 64-bit host activation without missing-COM or missing-DLL errors. |
| 5 | User can uninstall TypeDuck without leaving broken TypeDuck TSF registrations, startup entries, install files, or runtime registry/profile residue. | VERIFIED | VM uninstall exited 0; after-uninstall snapshot shows HKLM/HKCU TIP false, HKCR CLSID false, Run value empty, install dir false, both system DLLs false, scheduled task false. |

**Score:** 5/5 truths verified

### Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `MoqiTextService/TypeDuckProfile.*` | First-party TypeDuck CLSID/profile/locale/display/DLL metadata | VERIFIED | Header/source exist and are compiled by `MoqiTextService/CMakeLists.txt`; constants match the Phase 1/3 identity contract. |
| `MoqiTextService/DllEntry.cpp` | Required TypeDuck profile seeded before backend metadata | VERIFIED | `DllRegisterServer` pushes `Moqi::TypeDuck::makeLangProfile(...)` before backend `ime.json` scanning and filters backend attempts to duplicate/override the required profile. |
| `SetupHelper/SetupHelper.cpp` | TypeDuck dual-bitness copy/register/unregister helper | VERIFIED | Uses `TypeDuckTextService.dll`, `TYPEDUCK_PROGRAM_DIR`, `TypeDuckIME-ReRegisterTSF`, SysWOW64/System32 paths, and 32/64 regsvr32. |
| `installer/MoqiTsf.iss` | TypeDuck installer skeleton and cleanup | VERIFIED | TypeDuck AppId/name/install dir/output/startup are present; uninstall purges TypeDuck CLSID/TIP/startup residue and narrow legacy Moqi residue. |
| `scripts/Invoke-TypeDuckVmInstallerVerification.ps1` | Host-safe Hyper-V evidence harness | VERIFIED | Harness contract test passed; captured before/install/uninstall snapshots and aggregate evidence. |
| `.planning/product/installer-fixtures/phase-03/*.json` | VM install/register/uninstall evidence | VERIFIED WITH WARNING | Aggregate status is `complete` with no harness failures. Warning: `registry-before.json` already had HKCU TypeDuck TIP residue, so it is not a fully clean per-user baseline. |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `TypeDuckProfile.cpp` | `TYPEDUCK-IDENTITY-CONTRACT.md` | Shared CLSID/profile/locale/display/DLL constants | WIRED | Static identity guard passed. |
| `DllEntry.cpp` | `libIME2/src/ImeModule.cpp` | `registerServer(..., langProfiles.data(), langProfiles.size())` | WIRED | Uses existing `RegisterProfile`, `EnableLanguageProfile`, and default-enable flow. |
| `SetupHelper.cpp` | system DLL registration | copy then `regsvr32.exe` for SysWOW64/System32 | WIRED | VM shows both DLLs installed; source uses distinct 32/64 regsvr32 paths. |
| `installer/MoqiTsf.iss` | `SetupHelper.cpp` | runs `TypeDuckSetupHelper.exe /i` and `/u` | WIRED | Installer log shows install/uninstall success; cleanup code ran and after-uninstall snapshot is clean. |
| VM harness | `installer/dist/typeduck-windows-ime-setup.exe` | copy into Hyper-V guest and execute install/uninstall | WIRED | Evidence includes installer SHA-256 and guest path. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|---|---|---|---|---|
| `DllEntry.cpp` | `langProfiles` | First-party `TypeDuckProfile` plus optional backend scan | Yes | FLOWING |
| `SetupHelper.cpp` | DLL source/destination paths | `{app}` plus system-directory APIs | Yes | FLOWING |
| `Invoke-TypeDuckVmInstallerVerification.ps1` | evidence snapshots | live VM registry/files/tasks/language-list queries | Yes | FLOWING, with noted HKCU baseline limitation |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|---|---|---|---|
| Static TSF identity contract | `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckTsfIdentity.ps1 -RepoRoot . -Strict` | `TypeDuck TSF identity check passed.` | PASS |
| Static installer skeleton contract | `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckInstallerSkeleton.ps1 -RepoRoot . -Strict` | `TypeDuck installer skeleton check passed.` | PASS |
| VM harness contract | `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckVmInstallerVerification.ps1 -RepoRoot .` | `TypeDuck VM installer verification harness contract passed.` | PASS |
| VM evidence assertions | JSON check over aggregate/install/uninstall fixtures | `VM evidence assertion passed` | PASS |

### Probe Execution

| Probe | Command | Result | Status |
|---|---|---|---|
| None discovered | `Get-ChildItem scripts -Recurse -Filter 'probe-*.sh'` | No probe files found | SKIP |

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|---|---|---|---|---|
| INST-01 | 03-02, 03-03 | Bilingual TypeDuck-branded installer | SATISFIED | Static installer guard passed; installer executed successfully with TypeDuck name and bilingual controlled messages. |
| INST-02 | 03-01, 03-03 | Select TypeDuck under Chinese (Traditional, Hong Kong) settings | SATISFIED | CTF/TIP registry is present under `0x00000c04`; human UAT confirmed Settings UI selectability in the VM. |
| INST-03 | 03-01, 03-02, 03-03 | Deterministic GUIDs, zh-HK metadata, icon, bilingual text | SATISFIED | Source constants and VM HKLM TIP/profile snapshot match. |
| INST-04 | 03-02, 03-03 | Register both 32-bit and 64-bit TSF DLLs | SATISFIED | Both DLLs and regsvr paths are evidenced; human UAT confirmed real 32-bit/64-bit host activation. |
| INST-05 | 03-02, 03-03 | Clean uninstall without broken residue | SATISFIED | After-uninstall snapshot shows TypeDuck registry/file/startup/task cleanup complete. |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| `.planning/product/installer-fixtures/phase-03/registry-before.json` | n/a | HKCU TypeDuck TIP existed before install | WARNING | VM was disposable/checkpointed but not fully clean for per-user TIP state; it weakens proof of fresh Settings/user-language enablement. |
| `.planning/product/installer-fixtures/phase-03/vm-install-registration-uninstall.json` | n/a | Inno Start Menu directory creation warning from slash in bilingual app name | WARNING | Install/register/uninstall still succeeded; Start Menu shortcut polish should be fixed later but is not a Phase 3 registration blocker. |

### Human Verification Completed

#### 1. Windows Settings Selectability

**Test:** Open Windows Settings in the VM after install and inspect Chinese (Traditional, Hong Kong) input methods.  
**Expected:** `TypeDuck 粵語輸入法 / TypeDuck Cantonese IME` is selectable under Chinese (Traditional, Hong Kong).  
**Result:** Pass. Human UAT confirmed the Settings user flow in the VM.

#### 2. Dual-Bitness Host Activation

**Test:** Activate/select TypeDuck from one 32-bit host and one 64-bit host after install.  
**Expected:** Both host bitnesses can load/select the TypeDuck TSF profile without missing-COM or missing-DLL errors.  
**Result:** Pass. Human UAT confirmed both host bitnesses can select TypeDuck without missing-COM or missing-DLL errors. ASCII output was observed but is outside this phase's registration/installer skeleton scope.

### Gaps Summary

No blocking implementation gaps were found. Automated source checks, VM fixture assertions, and human UAT support install, deterministic registration metadata, Settings selectability, dual-bitness host activation, and uninstall cleanup.

### Non-Blocking Follow-Ups

- Installer language picker showed duplicate English entries; keep as installer-localization follow-up.
- Inno Start Menu directory creation warning from the slash in the bilingual app name remains a known follow-up for a later phase.

---

_Verified: 2026-06-23T17:10:00Z_  
_Verifier: the agent (gsd-verifier)_
