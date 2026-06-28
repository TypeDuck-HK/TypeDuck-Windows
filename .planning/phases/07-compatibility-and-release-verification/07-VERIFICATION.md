---
phase: 07-compatibility-and-release-verification
verified: 2026-06-28T08:33:34Z
status: complete
score: 4/4 must-haves verified
human_verification: complete
installer_artifact:
  path: installer\dist\typeduck-windows-ime-setup.exe
  sha256: B5EFBCFC8620E83B2DD9E83B0D8D647F685B3882B4D793510A80BA3610C378CE
  byte_size: 20942974
regressions_closed:
  - setup-helper failure did not leave success-only final installer text
  - launcher/settings first-run ordering
  - missing `ime.json` after runtime pruning
  - AppData/Roaming preference location
  - accepted Rime deploy first-run behavior
  - candidate mouse-click commit
  - Explorer/high-DPI candidate popup sizing
  - Chinese font fallback in candidate and definition text
  - uninstall user-data prompt and cleanup
  - appearance-theme `source` metadata
  - Windows system IME menu icon packaging
---

# Phase 7 Verification Report

**Phase Goal:** Developer can produce a TypeDuck v1 installer artifact backed by repeatable Windows installation, typing, UI, protocol, and release evidence.  
**Verified:** 2026-06-28T08:33:34Z  
**Status:** complete

## Goal Achievement

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | Developer can verify clean install, upgrade/reinstall, uninstall, and reboot-required registration behavior on Windows 10/11 or equivalent test VMs. | VERIFIED | `.planning/product/release-fixtures/phase-07/install-verification-notes.md`; repeated VM install/reinstall/uninstall iterations; installer skeleton and release evidence guards. |
| 2 | Developer can verify TypeDuck typing and candidate UI in representative host apps including Notepad, browsers, Office-like apps, terminal/console contexts, elevated apps, and high-DPI setups. | VERIFIED | `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md`; `.planning/product/release-fixtures/phase-07/host-app-dpi-notes.md`; user completed all host/DPI rows as pass. |
| 3 | Developer can verify protocol and engine behavior with golden tests for normal input, dictionary lookup, reverse lookup, malformed frames, timeouts, and backend restart. | VERIFIED | `ProtocolRecovery_test`; `scripts\Invoke-TypeDuckProtocolRecoveryProbe.ps1`; `.planning/product/release-fixtures/phase-07/protocol-recovery-results.json`. |
| 4 | Developer can produce a v1 installer artifact with TypeDuck naming and documented verification evidence. | VERIFIED | `installer\dist\typeduck-windows-ime-setup.exe`; SHA-256 `B5EFBCFC8620E83B2DD9E83B0D8D647F685B3882B4D793510A80BA3610C378CE`; release artifact/workflow guards. |

## Automated Verification

| Check | Command / Artifact | Result |
|---|---|---|
| Release install evidence schema | `scripts\Test-TypeDuckReleaseInstallEvidence.ps1 -Strict` | PASS |
| Protocol recovery guard | `scripts\Test-TypeDuckProtocolRecovery.ps1 -Strict` | PASS |
| Protocol recovery probe | `scripts\Invoke-TypeDuckProtocolRecoveryProbe.ps1 -Strict` | PASS |
| Protocol recovery C++ tests | `ProtocolRecovery_test` | PASS |
| Release artifact guard | `scripts\Test-TypeDuckReleaseArtifacts.ps1 -Strict` | PASS |
| Aggregate release verification | `scripts\Invoke-TypeDuckReleaseVerification.ps1`; `scripts\Test-TypeDuckReleaseVerification.ps1 -Strict` | PASS before final interactive closeout; release fixture updated to complete afterward. |
| Phase 6 privacy/security baseline | `.planning/product/privacy-security/phase-06-guard-results.json` | PASS |
| Interactive checklist guard | `scripts\Test-TypeDuckInteractiveReleaseChecklist.ps1 -Strict` | PASS |
| Icon packaging guard | `scripts\Test-TypeDuckIconPackaging.ps1 -Strict` | PASS after system IME icon repair. |
| Runtime package pruning guard | `scripts\Test-TypeDuckRuntimePackagePruning.ps1 -Strict` | PASS after preserving only `resources\TypeDuck_Small.ico`. |

## Human VM Verification

The user completed direct VM judgement without screenshot capture.

| Area | Result | Evidence |
|---|---|---|
| Host apps | PASS | Notepad, browser text field, Office-like app, terminal/console context, elevated app, and awkward TSF host rows are all pass in `interactive-vm-checklist.md`. |
| Typing smoke | PASS | Cantonese composition, candidate list, dictionary detail, reverse lookup where supported, settings redeploy, backend restart/degraded recovery, and TypeDuck log path checks are all pass. |
| DPI | PASS | 100%, 140% if available, 175%, and 200% rows are all pass. |
| Multi-monitor disposition | ACCEPTED | Phase 5 already human verified multi-monitor behavior; Phase 7 did not reintroduce screenshot automation. |

## Regression Closure Detail

| Regression / issue found during Phase 7 iteration | Fix | Verification |
|---|---|---|
| TypeDuck could not type after runtime pruning because backend looked for removed `ime.json` and returned `unknown_input_method`. | Backend registers TypeDuck Rime directly with the fixed profile GUID; frontend guard covers fixed profile registration. | VM typing works; commits `2e01b67`, `36e74ff`. |
| Installer reported success after setup-helper/TSF registration failure. | Installer failure flow no longer claims installed on failure; final text avoids technical TSF/DLL wording. | VM install iterations; commits `a927d03`, `c814860`. |
| Install-time settings failed first run because launcher was not running. | Installer starts launcher and uses a dedicated launcher flag for install settings before invoking settings application. | VM install works; commits `498854c`, `1f05b4f`, `5158f10`. |
| Rime prebuilt build-copy optimization did not work reliably. | Reverted to accepted first-run/full-check deployment behavior; no copying shortcut remains. | Commits `ba7f07d`, backend `cfa3509`. |
| Preferences and custom YAML were not created in `%APPDATA%\TypeDuckIME`; logs were separate local state. | Preferences/custom YAML moved to Roaming AppData; logs remain Local AppData. | VM path observation; backend commit `490949c`. |
| Candidate mouse click selected internally but did not output text. | Candidate click now commits through the frontend path and resets UI correctly. | User verified typing/click commit; commit `efa2092`. |
| Candidate popup was too small in Explorer compared with Notepad/Chrome. | Candidate popup DPI sizing uses host-aware scaling. | User verified Explorer; commit `35c7bb2`. |
| Candidate/definition Chinese text fell back to the wrong font. | Candidate popup preserves the accepted Chinese font fallback for candidate and definition text. | User verified; commit `1a1ff65`. |
| Uninstaller custom prompt raised layout/runtime issues and did not delete user data when checked. | Prompt was rebuilt with stable controls/layout and optional `%APPDATA%\TypeDuckIME` deletion. | VM uninstall iterations; commits `79cc11f`, `2868406`, `68fe074`. |
| Appearance themes included development `source` metadata. | Removed metadata and added guard. | Backend commit `c0686de`; frontend commit `0f591a6`. |
| Windows system IME menu showed `繁體` instead of the TypeDuck icon after icon-folder pruning. | TSF profile icon lookup now uses `resources\TypeDuck_Small.ico`; staging copies only that raw icon. | `scripts\Test-TypeDuckIconPackaging.ps1 -Strict`; commit `96f3453`. |

## Release Artifact

| Field | Value |
|---|---|
| Installer | `installer\dist\typeduck-windows-ime-setup.exe` |
| SHA-256 | `B5EFBCFC8620E83B2DD9E83B0D8D647F685B3882B4D793510A80BA3610C378CE` |
| Byte size | `20942974` |
| Evidence notes | `.planning/product/release-fixtures/phase-07/verification-notes.md` |

## Requirements Coverage

| Requirement | Status | Evidence |
|---|---|---|
| VER-03 | SATISFIED | Install/reinstall/uninstall notes plus VM iterations. |
| VER-04 | SATISFIED | Interactive host-app/DPI checklist completed by user. |
| VER-05 | SATISFIED | Protocol recovery probe/results and C++ tests. |
| VER-06 | SATISFIED | Current TypeDuck installer artifact and workflow/release guards. |

## Gaps Summary

No open Phase 7 gaps remain. Screenshot-driven verification remains intentionally excluded; user judgement was direct and interactive.
