---
phase: 03-zh-hk-tsf-registration-and-installer-skeleton
verified: 2026-06-23T17:10:00Z
status: human_needed
score: 3/5 must-haves verified
behavior_unverified: 2
overrides_applied: 0
behavior_unverified_items:
  - truth: "User can select TypeDuck under Windows Chinese (Traditional, Hong Kong) language/input settings after installation."
    test: "Open Windows Settings in the VM and inspect Chinese (Traditional, Hong Kong) input methods after install."
    expected: "TypeDuck 粵語輸入法 / TypeDuck Cantonese IME is selectable under Chinese (Traditional, Hong Kong), not only present in CTF/TIP registry."
    why_human: "The VM packet proves CTF/TIP registry state under 0x00000c04, but Get-WinUserLanguageList remained en-US and no Settings screenshot was captured."
  - truth: "TypeDuck works from both 32-bit and 64-bit host applications after installer registration."
    test: "In the VM, activate/select TypeDuck from one 32-bit host and one 64-bit host after install."
    expected: "Both host bitnesses can load/select the TypeDuck TSF profile without missing-COM or missing-DLL errors."
    why_human: "Source and fixture evidence show both DLL copies and regsvr32 paths, but no host-application activation smoke test was captured."
human_verification:
  - test: "Open Windows Settings in the VM and inspect Chinese (Traditional, Hong Kong) input methods after install."
    expected: "TypeDuck 粵語輸入法 / TypeDuck Cantonese IME is selectable under Chinese (Traditional, Hong Kong)."
    why_human: "Get-WinUserLanguageList did not report zh-HK; registry evidence is strong but does not visually prove Settings selectability."
  - test: "Activate/select TypeDuck from one 32-bit host and one 64-bit host after install."
    expected: "Both host bitnesses can load/select the TypeDuck TSF profile without missing-COM or missing-DLL errors."
    why_human: "The harness verified files and registry, not real host activation."
---

# Phase 3: zh-HK TSF Registration and Installer Skeleton Verification Report

**Phase Goal:** User can install and remove TypeDuck as a Chinese (Traditional, Hong Kong) Windows IME with deterministic first-party registration.  
**Verified:** 2026-06-23T17:10:00Z  
**Status:** human_needed  
**Re-verification:** No - initial verification

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | User can run a TypeDuck-branded installer worded in bilingual Traditional Hong Kong Chinese and English. | VERIFIED | `installer/MoqiTsf.iss` uses TypeDuck app name/publisher/output and bilingual product-controlled strings; `scripts\Test-TypeDuckInstallerSkeleton.ps1 -Strict` passed; VM installer exited 0. |
| 2 | User can select TypeDuck under Windows Chinese (Traditional, Hong Kong) language/input settings after installation. | PRESENT_BEHAVIOR_UNVERIFIED | After-install registry has `LanguageProfile\0x00000c04\{C6E8F5DF-...}` with display text, `HiddenInSettingUI=0`, and `Enable=1`; however `Get-WinUserLanguageList` stayed `en-US` and no Settings screenshot was captured. |
| 3 | TypeDuck appears with deterministic TypeDuck-owned CLSID/profile GUID, zh-HK locale metadata, TypeDuck icon, and bilingual display text. | VERIFIED | `TypeDuckProfile.cpp` centralizes CLSID `{7D92985A-...}`, profile `{C6E8F5DF-...}`, `zh-HK`, display text, and DLL name; after-install HKLM TIP registry contains description and `IconFile=C:\Windows\System32\TypeDuckTextService.dll`. |
| 4 | TypeDuck works from both 32-bit and 64-bit host applications after installer registration. | PRESENT_BEHAVIOR_UNVERIFIED | SetupHelper copies/registers SysWOW64 and System32 DLLs through matching regsvr32 paths; VM snapshot shows both DLLs present. No 32-bit and 64-bit host activation smoke test was captured. |
| 5 | User can uninstall TypeDuck without leaving broken TypeDuck TSF registrations, startup entries, install files, or runtime registry/profile residue. | VERIFIED | VM uninstall exited 0; after-uninstall snapshot shows HKLM/HKCU TIP false, HKCR CLSID false, Run value empty, install dir false, both system DLLs false, scheduled task false. |

**Score:** 3/5 truths verified (2 present, behavior-unverified)

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
| INST-02 | 03-01, 03-03 | Select TypeDuck under Chinese (Traditional, Hong Kong) settings | HUMAN NEEDED | CTF/TIP registry is present under `0x00000c04`, but Settings UI selectability was not visually captured and language list stayed `en-US`. |
| INST-03 | 03-01, 03-02, 03-03 | Deterministic GUIDs, zh-HK metadata, icon, bilingual text | SATISFIED | Source constants and VM HKLM TIP/profile snapshot match. |
| INST-04 | 03-02, 03-03 | Register both 32-bit and 64-bit TSF DLLs | HUMAN NEEDED | Both DLLs and regsvr paths are evidenced; real 32-bit/64-bit host activation was not captured. |
| INST-05 | 03-02, 03-03 | Clean uninstall without broken residue | SATISFIED | After-uninstall snapshot shows TypeDuck registry/file/startup/task cleanup complete. |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| `.planning/product/installer-fixtures/phase-03/registry-before.json` | n/a | HKCU TypeDuck TIP existed before install | WARNING | VM was disposable/checkpointed but not fully clean for per-user TIP state; it weakens proof of fresh Settings/user-language enablement. |
| `.planning/product/installer-fixtures/phase-03/vm-install-registration-uninstall.json` | n/a | Inno Start Menu directory creation warning from slash in bilingual app name | WARNING | Install/register/uninstall still succeeded; Start Menu shortcut polish should be fixed later but is not a Phase 3 registration blocker. |

### Human Verification Required

#### 1. Windows Settings Selectability

**Test:** Open Windows Settings in the VM after install and inspect Chinese (Traditional, Hong Kong) input methods.  
**Expected:** `TypeDuck 粵語輸入法 / TypeDuck Cantonese IME` is selectable under Chinese (Traditional, Hong Kong).  
**Why human:** `Get-WinUserLanguageList` did not report `zh-HK`; CTF/TIP registry evidence does not visually prove the Settings user flow.

#### 2. Dual-Bitness Host Activation

**Test:** Activate/select TypeDuck from one 32-bit host and one 64-bit host after install.  
**Expected:** Both host bitnesses can load/select the TypeDuck TSF profile without missing-COM or missing-DLL errors.  
**Why human:** The harness verified files and registry, not real host activation.

### Gaps Summary

No blocking implementation gaps were found. Automated source checks and VM fixture assertions support install, deterministic registration metadata, dual-bitness registration wiring, and uninstall cleanup. The phase should not be marked fully passed until the two user-flow checks above are confirmed.

---

_Verified: 2026-06-23T17:10:00Z_  
_Verifier: the agent (gsd-verifier)_
