---
phase: 06-privacy-security-and-scaffold-cleanup
verified: 2026-06-27T11:59:01Z
status: human_needed
score: 5/6 must-haves verified
behavior_unverified: 1
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 3/6
  gaps_closed:
    - "Backend `onMenu`/`buildMenu` payloads now return bilingual Traditional Hong Kong Chinese and English menu labels instead of the prior Simplified-only labels."
    - "Strict backend diagnostics guard now scans active backend UI/menu payload literals and fails on the prior Simplified-only menu labels."
  gaps_remaining: []
  regressions: []
deferred:
  - truth: "Live spoofed-pipe, denied-process-inspection, clean install/uninstall, reboot-required registration, and elevated cleanup scenarios are not exercised by this static Phase 6 verification."
    addressed_in: "Phase 7"
    evidence: "Phase 7 success criteria cover clean install/upgrade/uninstall/reboot behavior, protocol/engine release evidence, and final release artifact verification."
behavior_unverified_items:
  - truth: "Named pipe access, client/server identity checks, frame-size limits, first-party settings launch, and installer cleanup avoid obvious spoofing, oversized-frame abuse, arbitrary config-tool launch, and broad legacy process kills."
    test: "Exercise a live spoofed same-user pipe server, denied-process-inspection pipe server, and install/uninstall cleanup path in the Phase 7 Windows release verification environment."
    expected: "Inspectable wrong launcher processes are rejected; denied inspection remains compatibility-tolerant; installer cleanup touches only TypeDuck-owned process/state/registration paths."
    why_human: "Static guards, source inspection, and frame tests verify code shape and parser bounds, but this re-verification intentionally did not run live pipe spoofing, installation, screenshot automation, or elevated cleanup."
human_verification:
  - test: "Run Phase 7 live Windows release verification for spoofed pipe, denied inspection, clean install/uninstall, reboot-required registration, and elevated cleanup behavior."
    expected: "The installed product rejects obvious spoofed launcher processes when inspectable, remains compatible when inspection is denied, and only cleans TypeDuck-owned process/state/registration paths."
    why_human: "Requires live Windows processes/elevation/install state; out of scope for this Phase 6 static/product-code re-verification."
---

# Phase 6: Privacy, Security, and Scaffold Cleanup Verification Report

**Phase Goal:** User sees only TypeDuck v1 behavior while local diagnostics, IPC, process cleanup, and user-facing copy meet TypeDuck privacy and security expectations.  
**Verified:** 2026-06-27T11:59:01Z  
**Status:** human_needed  
**Re-verification:** Yes - after backend menu localization repair

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | User sees TypeDuck, not Moqi, in installer title text, installed app names, tray/menu surfaces, binary/resource metadata, logs, data paths, and release artifact names. | VERIFIED | Installer uses `TypeDuckIME`, TypeDuck shortcuts, EdUHK publisher, and `typeduck-windows-ime-setup`; CI workflows use TypeDuck checkout paths and artifact names. Internal scaffold identifiers remain covered by D-01/D-24 allowances. |
| 2 | User-facing installer, settings, About, tray/menu, candidate UI, error, and status strings are bilingual in Traditional Hong Kong Chinese and English, with no Simplified-only wording. | VERIFIED | Prior backend blocker is fixed in `D:/VSProjects/moqi-ime@c968cf4`: `onMenu()` returns `buildMenu()`, and `buildMenu()` now emits labels such as `切換方案集 / Switch Scheme Set`, `輸入方案(&I) / Input Schema`, `外觀(&A) / Appearance`, `輸入設定 / Input Settings`, and `說明文件(&H) / Help`; `ButtonInfo` text/tooltips are also bilingual. |
| 3 | User-facing surfaces contain no visible Moqi, fcitx, WebDAV/cloud clipboard, AI, or other off-scope scaffold references. | VERIFIED | Aggregate guard evidence records `status: pass`; backend removed-surface tests cover menu text, command IDs, unhandled removed commands, and omitted CustomizeUI payloads. |
| 4 | Runtime logs and diagnostics use TypeDuck-owned paths and avoid raw typed content by default. | VERIFIED | Frontend paths use `%LOCALAPPDATA%\\TypeDuckIME`, `TypeDuckLauncher.json`, and `TypeDuckLauncher.log`; backend log candidates use `TypeDuckIME\\Log\\TypeDuckBackend.log`; backend diagnostics guard passed in the recorded evidence. |
| 5 | Named pipe access, client/server identity checks, frame-size limits, first-party settings launch, and installer cleanup avoid obvious spoofing, oversized-frame abuse, arbitrary config-tool launch, and broad legacy process kills. | PRESENT_BEHAVIOR_UNVERIFIED | Source shows `\\TypeDuckIME\\Launcher`, explicit pipe rights, PID/image sanity checks, and fixed `TypeDuckSettings.exe` launch; `ProtoFraming_test.exe` evidence covers the 1 MiB frame caps. Live spoofed-pipe and elevated cleanup scenarios are deferred to Phase 7 release verification. |
| 6 | Developer can run automated or scripted checks that fail on visible Moqi, fcitx, WebDAV/cloud clipboard, AI, or Simplified-only strings in user-facing resources. | VERIFIED | `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1` invokes `scripts/Test-TypeDuckBackendDiagnostics.ps1`; `Test-BackendUserFacingPayloads` now rejects the prior Simplified-only backend menu label patterns and requires bilingual Traditional/English labels. Guard evidence records aggregate pass. |

**Score:** 5/6 truths verified (1 present, behavior-unverified)

### Deferred Items

Items not yet exercised but explicitly addressed in later milestone phases.

| # | Item | Addressed In | Evidence |
|---|---|---|---|
| 1 | Live spoofed-pipe, denied-process-inspection, clean install/uninstall, reboot-required registration, and elevated cleanup scenarios | Phase 7 | Phase 7 success criteria cover Windows install/upgrade/uninstall/reboot behavior, protocol/engine evidence, and final release artifact verification. |

### Required Artifacts

| Artifact | Expected | Status | Details |
|---|---|---|---|
| `D:/VSProjects/moqi-ime/input_methods/rime/rime.go` | Active backend menu/button payloads are bilingual Traditional/English | VERIFIED | Commit `c968cf4` localizes `buildMenu()` labels and `ButtonInfo` text/tooltips; `onMenu()` still returns `ime.buildMenu()` for the visible settings/menu buttons. |
| `D:/VSProjects/moqi-ime/input_methods/rime/removed_surfaces_test.go` | Removed-surface and menu-localization tests | VERIFIED | Adds `TestTypeDuckV1MenuLabelsAreBilingualTraditional`, which fails on the prior Simplified-only labels and requires representative bilingual labels. |
| `scripts/Test-TypeDuckBackendDiagnostics.ps1` | Backend diagnostics/package/UI-payload guard | VERIFIED | `Test-BackendUserFacingPayloads` scans `input_methods/rime/rime.go`, rejects the prior Simplified-only label literals, and requires bilingual labels. |
| `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1` | Aggregate Phase 6 guard | VERIFIED | Invokes installer, runtime, backend diagnostics, launcher protocol, settings/About, and icon guards; writes compact JSON evidence. |
| `.planning/product/privacy-security/phase-06-guard-results.json` | Strict guard result evidence | VERIFIED | JSON records `status: pass`, UTC timestamp, command list, roots, and categories including `backend-diagnostics`, `backend-package`, `installer-cleanup`, `language`, and `ipc`. |
| `MoqiTextService/MoqiClient.cpp` | Frontend wiring renders backend menu JSON into native menus | VERIFIED | `Client::onMenu()` calls backend `onMenu`; `menuFromJson()` and Win32 `AppendMenu` consume returned `text` values. This is the same visibility path that made the prior gap real. |
| `MoqLauncher/PipeSecurity.cpp` / `MoqiTextService/MoqiClient.cpp` | IPC hardening code shape | VERIFIED | Static source contains explicit pipe rights and PID/image checks; live spoof/elevation behavior remains behavior-unverified. |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| `D:/VSProjects/moqi-ime/input_methods/rime/rime.go:onMenu` | `buildMenu()` | `resp.ReturnData = ime.buildMenu()` | WIRED | The active backend `onMenu` payload now uses the localized `buildMenu()` output. |
| `D:/VSProjects/moqi-ime/input_methods/rime/rime.go` | `MoqiTextService/MoqiClient.cpp` | backend JSON return data -> native `ITfMenu`/Win32 menu rendering | WIRED | Frontend still renders returned `text` fields, so the repaired labels are user-facing through the same path as the prior blocker. |
| `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1` | `scripts/Test-TypeDuckBackendDiagnostics.ps1` | aggregate focused-guard invocation | WIRED | Aggregate guard invokes backend diagnostics with `RepoRoot`, `BackendRoot`, and `Strict`. |
| `scripts/Test-TypeDuckBackendDiagnostics.ps1` | `D:/VSProjects/moqi-ime/input_methods/rime/rime.go` | `Test-BackendUserFacingPayloads` source scan | WIRED | Rejects the old Simplified-only label patterns and requires representative bilingual labels. |
| `D:/VSProjects/moqi-ime/input_methods/rime/removed_surfaces_test.go` | `rime.go:buildMenu()` | flattened menu item test helper | WIRED | Test directly exercises the menu payload object produced by `buildMenu()`. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|---|---|---|---|---|
| Backend menu labels | `resp.ReturnData` | `onMenu()` calls `ime.buildMenu()` | Yes - localized maps returned to frontend | VERIFIED |
| Backend button labels | `resp.AddButton` / `resp.ChangeButton` | `addButtons()` and update paths in `rime.go` | Yes - bilingual `ButtonInfo` text/tooltips | VERIFIED |
| Guard result evidence | `status`, `commands`, `categories` | aggregate guard JSON writer | Yes - compact metadata only | VERIFIED |
| IPC/elevated cleanup behavior | Live Windows process/install state | Static source plus later Phase 7 VM checks | Not exercised live here | PRESENT_BEHAVIOR_UNVERIFIED |

### Behavioral Spot-Checks

These are recorded/already-run verification results supplied for this re-verification; no screenshot automation or installation was run here.

| Behavior | Command | Result | Status |
|---|---|---|---|
| Aggregate Phase 6 guard | `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\\Test-TypeDuckPrivacySecurityCleanup.ps1 -RepoRoot . -BackendRoot D:\\VSProjects\\moqi-ime -Strict` | `.planning/product/privacy-security/phase-06-guard-results.json` records `status: pass` | PASS |
| Backend diagnostics guard | `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\\Test-TypeDuckBackendDiagnostics.ps1 -RepoRoot . -BackendRoot D:\\VSProjects\\moqi-ime -Strict` | Provided evidence says backend diagnostics guard passed; source now includes active UI/menu payload checks | PASS |
| Backend root tests | `go test -timeout 60s .` in `D:\\VSProjects\\moqi-ime` | Provided evidence says backend root tests passed | PASS |
| Targeted Rime TypeDuck v1 tests | `go test -timeout 60s ./input_methods/rime -run "TestTypeDuckV1|TestOnCommandSyncUserData|TestRimeLogDir|TestDeployTypeDuckFromLauncher"` | Provided evidence says targeted tests passed; the pattern includes `TestTypeDuckV1MenuLabelsAreBilingualTraditional` | PASS |

### Probe Execution

| Probe | Command | Result | Status |
|---|---|---|---|
| Phase 6 strict guard suite | `scripts\\Test-TypeDuckPrivacySecurityCleanup.ps1 -Strict` | Recorded aggregate guard evidence is `status: pass` | PASS |
| Backend strict diagnostics guard | `scripts\\Test-TypeDuckBackendDiagnostics.ps1 -Strict` | Provided evidence says pass; source inspection confirms it now covers active backend UI/menu payload labels | PASS |

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|---|---|---|---|---|
| IDEN-01 | 06-01/02/03/06 | TypeDuck identity in visible surfaces and artifacts | SATISFIED | Installer, logs, runtime folder, CI artifacts, and staged payload use TypeDuck-owned names. |
| IDEN-03 | 06-01/03/06 | No visible Moqi product strings | SATISFIED | Guards pass; remaining Moqi identifiers are internal scaffold/source compatibility names allowed by D-01/D-24. |
| IDEN-04 | 06-02/04/06/07 | No visible fcitx/WebDAV/cloud/AI references | SATISFIED | Removed-surface tests and guards cover menu IDs, command handling, package output, and frontend surfaces. |
| LANG-01 | 06-01/06 | Every user-facing string bilingual | SATISFIED | Prior backend menu blocker fixed by `c968cf4`; menu/button payloads are bilingual Traditional/English. |
| LANG-02 | 06-01/04/06/07 | Traditional Hong Kong wording, not Simplified-only | SATISFIED | `buildMenu()` uses Traditional labels such as `切換`, `輸入`, `外觀`, `設定`, `說明`; tests and guard reject prior Simplified-only labels. |
| SEC-01 | 06-04/05/06/07 | Logs/diagnostics TypeDuck-owned and avoid raw typed content | SATISFIED | TypeDuck log paths and diagnostics guards are present; backend guard pass evidence supplied. |
| SEC-02 | 06-02/04/06/07 | Cloud/WebDAV removed or gated | SATISFIED | Frontend protocol numbers reserved; backend removed-surface tests and runtime pruning guard evidence pass. |
| SEC-03 | 06-03/06 | No arbitrary backend config-tool launch | SATISFIED | Frontend settings paths are fixed to `TypeDuckSettings.exe`; settings/About guard remains wired. |
| SEC-04 | 06-05/06 | Pipe access/identity/frame bounds hardened | PRESENT_BEHAVIOR_UNVERIFIED | Static source and frame tests are present; live spoofed-pipe/elevated cleanup scenarios are deferred to Phase 7. |
| SEC-05 | 06-01/03/06 | Installer avoids broad legacy process kills | SATISFIED by static guard/source | Installer guard confirms TypeDuck-only cleanup and Legacy Moqi coexistence; elevated runtime exercise belongs to Phase 7. |
| VER-02 | 06-01/02/04/06/07 | Automated checks fail on visible banned/Simplified strings | SATISFIED | Backend diagnostics guard now fails on the exact old backend menu label patterns and aggregate guard invokes it. |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---:|---|---|---|
| `MoqLauncher/PipeServer.cpp` | 774 | `FIXME: make this translatable later` | WARNING | Pre-existing marker in a phase-touched file; tray menu copy is currently bilingual, but the marker should be formally tracked or removed later. |
| `MoqiTextService/MoqiClient.cpp` | 948, 1030, 1084, 1096, 1110, 1271, 1607 | `FIXME` markers | WARNING | Legacy debt remains in touched scaffold code; not introduced by the backend localization repair and not blocking the repaired Phase 6 gaps. |
| `MoqLauncher/PipeSecurity.cpp` | 93 | `FIXME` marker | WARNING | Legacy SID-freeing comment remains without formal follow-up reference. |

### Human Verification Required

#### 1. Phase 7 Live IPC and Cleanup Verification

**Test:** Run the Phase 7 non-screenshot Windows verification for a live spoofed same-user pipe server, denied-process-inspection pipe server, clean install/uninstall, reboot-required registration, and elevated cleanup path.  
**Expected:** Obvious inspectable wrong launcher processes are rejected; denied inspection remains compatibility-tolerant; installer cleanup touches only TypeDuck-owned process/state/registration paths.  
**Why human:** Requires live Windows process identity/elevation/install state and is explicitly covered by Phase 7 release verification.

### Gaps Summary

No Phase 6 static/product-code blockers remain from the previous verification. The prior backend menu localization gap is closed, and the strict guard blind spot is closed. The only remaining item is live Windows behavior evidence for spoofed-pipe and elevated cleanup scenarios, recorded above as behavior-unverified Phase 7 release verification debt rather than a Phase 6 blocker.

---

_Verified: 2026-06-27T11:59:01Z_  
_Verifier: the agent (gsd-verifier)_
