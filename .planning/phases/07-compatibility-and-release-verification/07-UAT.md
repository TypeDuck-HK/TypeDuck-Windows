---
status: complete
phase: 07-compatibility-and-release-verification
source:
  - 07-01-SUMMARY.md
  - 07-02-SUMMARY.md
  - 07-03-SUMMARY.md
  - 07-04-SUMMARY.md
  - .planning/product/release-fixtures/phase-07/interactive-vm-checklist.md
started: 2026-06-27T12:25:09Z
updated: 2026-06-28T08:33:34Z
---

# Phase 7 UAT: Compatibility and Release Verification

## Current Test

[testing complete]

## Tests

### 1. Release Installer Artifact
expected: The current release artifact is named `typeduck-windows-ime-setup.exe`, is hashable, and is recorded in release evidence with byte size and SHA-256.
result: pass
evidence:
  - SHA-256 `B5EFBCFC8620E83B2DD9E83B0D8D647F685B3882B4D793510A80BA3610C378CE`
  - Byte size `20942974`
  - `.planning/product/release-fixtures/phase-07/verification-notes.md`

### 2. Clean Install
expected: Installing the current TypeDuck artifact in the VM registers TypeDuck and produces a usable IME without claiming success when setup-helper registration fails.
result: pass
evidence:
  - VM install iterations on 2026-06-28
  - `.planning/product/release-fixtures/phase-07/install-verification-notes.md`

### 3. Reinstall/Upgrade
expected: Running the installer over an existing install preserves product usability, applies settings through the launcher, and gives reopen-apps-first final guidance.
result: pass
evidence:
  - Commits `498854c`, `1f05b4f`, `5158f10`
  - VM reinstall iterations

### 4. Uninstall and Optional User Data Deletion
expected: Uninstaller succeeds, uses bilingual TypeDuck wording, offers an unchecked optional user-data deletion prompt, deletes `%APPDATA%\TypeDuckIME` only when selected, and does not touch Legacy Moqi.
result: pass
evidence:
  - Commits `79cc11f`, `2868406`, `68fe074`, `a927d03`
  - `.planning/product/release-fixtures/phase-07/install-verification-notes.md`

### 5. Reboot-Required Registration Path
expected: Locked-DLL/reboot-required registration uses the scheduled TypeDuck re-registration path and final-page guidance avoids TSF/DLL jargon.
result: pass
evidence:
  - `TypeDuckIME-ReRegisterTSF` guard coverage
  - Installer final-page wording in `installer/MoqiTsf.iss`

### 6. Launcher and Settings First-Run
expected: Install-time settings no longer fail with “TypeDuck Launcher was unavailable”; launcher startup is explicit and `/apply-settings` runs through the launcher flag only when requested.
result: pass
evidence:
  - Commits `498854c`, `1f05b4f`, `7b7eeb6`, `b51d4e6`, `5158f10`
  - VM confirmation that final install works

### 7. Roaming Preferences and Local Logs
expected: Preferences and custom YAML are created under `%APPDATA%\TypeDuckIME`; logs are under `%LOCALAPPDATA%\TypeDuckIME\Log`.
result: pass
evidence:
  - Backend commit `490949c Use roaming AppData for TypeDuck preferences`
  - VM log observation

### 8. Rime Schema Deployment Behavior
expected: Rime schema deployment uses the accepted first-run/full-check behavior; no mysterious prebuilt build-copy shortcut remains in the final installer path.
result: pass
evidence:
  - Commit `ba7f07d Restore old schema redeploy behavior`
  - Backend commit `cfa3509 Revert prebuilt schema copy and full-check bypass on first run`

### 9. Fixed Runtime Profile Registration
expected: TypeDuck can type after runtime pruning even though `ime.json` is removed; the backend registers TypeDuck Rime with the fixed profile GUID.
result: pass
evidence:
  - Frontend commit `36e74ff`
  - Backend commit `2e01b67`
  - VM log no longer blocks typing with `unknown_input_method`

### 10. Candidate Mouse Click Commit
expected: Clicking a candidate commits text to the host application instead of only resetting Rime/backend state.
result: pass
evidence:
  - Commit `efa2092 Fix candidate click commit handling`
  - User verified typing and candidate selection after repair

### 11. Candidate Popup in Explorer
expected: Candidate and dictionary windows are the correct size in Explorer and do not shrink relative to Notepad/browser hosts.
result: pass
evidence:
  - Commit `35c7bb2 Fix candidate popup DPI sizing in Explorer`
  - Interactive VM host-app check

### 12. Candidate Chinese Font Fallback
expected: Candidate and definition text use the accepted HK Chinese font fallback instead of falling back to Arial/Japanese fonts.
result: pass
evidence:
  - Commit `1a1ff65 Preserve Chinese font fallback in candidate popup`
  - Interactive VM host-app/DPI check

### 13. Windows System IME Menu Icon
expected: The Windows system IME menu shows the TypeDuck small icon, not plain “繁體”; the installed raw icon surface is limited to `resources\TypeDuck_Small.ico`.
result: pass
evidence:
  - Commit `96f3453 Fix system IME picker icon packaging`
  - `scripts\Test-TypeDuckIconPackaging.ps1 -Strict`

### 14. Notepad Host
expected: TypeDuck composition, candidate list, dictionary detail, commit, settings refresh, and recovery checks pass in Notepad.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md`

### 15. Browser Text Field Host
expected: TypeDuck typing and candidate UI work in a browser text field.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/host-app-dpi-notes.md`

### 16. Office or Office-Like Host
expected: Office-like host accepts composition and committed text without focus loss.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/host-app-dpi-notes.md`

### 17. Terminal/Console Context
expected: Console-style host remains responsive and bounded during typing/recovery checks.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/host-app-dpi-notes.md`

### 18. Elevated App
expected: Elevated app scenario works without broad legacy cleanup, stuck launcher state, or host hang.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/host-app-dpi-notes.md`

### 19. Awkward TSF Host
expected: Awkward TSF host available in the VM remains usable after sizing, font, and candidate commit repairs.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md`

### 20. DPI 100 Percent
expected: Candidate and dictionary UI are coherent at 100% scaling.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md`

### 21. DPI 140 Percent
expected: Candidate and dictionary UI remain coherent at 140% fractional scaling when available.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md`

### 22. DPI 175 Percent
expected: Candidate and dictionary UI remain coherent at 175% scaling.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md`

### 23. DPI 200 Percent
expected: Candidate and dictionary UI remain coherent at 200% scaling.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md`

### 24. Protocol Recovery
expected: Normal frames, lookup payload preservation, malformed/oversized/invalid frames, backend timeout, backend restart, settings redeploy failure, and bounded degraded state are covered by non-visual redacted evidence.
result: pass
evidence:
  - `.planning/product/release-fixtures/phase-07/protocol-recovery-results.json`
  - `ProtocolRecovery_test`

### 25. No Screenshot Automation
expected: Release verification uses direct human judgement for DPI/host-app visual checks and does not require screenshot capture or automated visual comparison.
result: pass
evidence:
  - `scripts\Test-TypeDuckInteractiveReleaseChecklist.ps1 -Strict`
  - `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md`

## Summary

total: 25
passed: 25
issues: 0
pending: 0
skipped: 0
blocked: 0

## Gaps

[]
