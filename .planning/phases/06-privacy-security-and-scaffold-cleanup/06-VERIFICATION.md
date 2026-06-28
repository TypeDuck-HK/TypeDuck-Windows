---
phase: 06-privacy-security-and-scaffold-cleanup
verified: 2026-06-28T08:33:34Z
status: complete
score: 6/6 must-haves verified
behavior_unverified: 0
overrides_applied: 0
phase7_followup:
  status: complete
  evidence:
    - .planning/product/release-fixtures/phase-07/interactive-vm-checklist.md
    - .planning/product/release-fixtures/phase-07/host-app-dpi-notes.md
    - .planning/product/release-fixtures/phase-07/install-verification-notes.md
regressions_closed:
  - backend menu path unreachable for TypeDuck v1
  - diagnostics and logs under TypeDuck paths
  - old AppData Moqi files pruned or inaccessible
  - Legacy Moqi cleanup removed
  - runtime package pruned without breaking TypeDuck typing
  - appearance theme source metadata removed
  - system IME icon packaged through resources\TypeDuck_Small.ico only
---

# Phase 6 Verification Report

**Phase Goal:** User sees only TypeDuck v1 behavior while local diagnostics, IPC, process cleanup, and user-facing copy meet TypeDuck privacy and security expectations.  
**Verified:** 2026-06-28T08:33:34Z
**Status:** complete

## Goal Achievement

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | User sees TypeDuck, not Moqi, in installer title text, installed app names, tray/menu surfaces, binary/resource metadata, logs, data paths, and release artifact names. | VERIFIED | Installer, Start Menu entries, release artifacts, runtime/log paths, and workflow names are TypeDuck-owned. Remaining Moqi identifiers are internal scaffold/source compatibility names only. |
| 2 | User-facing installer, settings, About, tray/menu, candidate UI, error, and status strings are bilingual in Traditional Hong Kong Chinese and English, with no Simplified-only wording. | VERIFIED | Installer and uninstaller text was replaced with bilingual TypeDuck copy; active backend/tray payloads are bilingual; stale Simplified backend menu payload is unreachable. |
| 3 | User-facing surfaces contain no visible Moqi, fcitx, WebDAV/cloud clipboard, AI, or other off-scope scaffold references. | VERIFIED | Runtime pruning, backend removed-surface tests, and aggregate guard pass; Legacy Moqi is not cleaned or modified. |
| 4 | Runtime logs and diagnostics use TypeDuck-owned paths and avoid raw typed content by default. | VERIFIED | Backend logs under `%LOCALAPPDATA%\TypeDuckIME\Log`; preferences/custom YAML under `%APPDATA%\TypeDuckIME`; routine diagnostics redacted or English-only. |
| 5 | Named pipe access, client/server identity checks, frame-size limits, first-party settings launch, and installer cleanup avoid obvious spoofing, oversized-frame abuse, arbitrary config-tool launch, and broad legacy process kills. | VERIFIED | Launcher protocol guard, protocol recovery tests, fixed first-party settings path, TypeDuck-only cleanup, and Phase 7 VM install/uninstall iterations. |
| 6 | Developer can run automated or scripted checks that fail on visible Moqi, fcitx, WebDAV/cloud clipboard, AI, or Simplified-only strings in user-facing resources. | VERIFIED | `scripts\Test-TypeDuckPrivacySecurityCleanup.ps1 -Strict` orchestrates focused installer, runtime, backend, launcher, settings/About, icon, and release workflow guards. |

## Verification Commands and Evidence

| Area | Evidence |
|---|---|
| Installer cleanup/localization | `scripts\Test-TypeDuckInstallerSkeleton.ps1 -Strict` |
| Runtime package pruning | `scripts\Test-TypeDuckRuntimePackagePruning.ps1 -Strict` |
| Manifest-free launcher bridge | `scripts\Test-TypeDuckLauncherProtocol.ps1 -Strict` |
| Backend diagnostics/removed surfaces | `scripts\Test-TypeDuckBackendDiagnostics.ps1 -Strict`; backend targeted Rime tests |
| Settings/About first-party surfaces | `scripts\Test-TypeDuckSettingsAboutUi.ps1 -Strict` |
| Icon/resource packaging | `scripts\Test-TypeDuckIconPackaging.ps1 -Strict` |
| Aggregate privacy/security evidence | `.planning/product/privacy-security/phase-06-guard-results.json` |
| Phase 7 live follow-up | `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md` |

## Delivered Feature and Cleanup Coverage

| Feature / cleanup | Status | Regression guard |
|---|---|---|
| Removed Simplified Inno translation dependency | VERIFIED | Installer skeleton guard rejects the removed submodule. |
| Installer final-page guidance avoids technical TSF/DLL terms | VERIFIED | Installer guard plus VM iteration. |
| TypeDuck-only process cleanup and uninstall cleanup | VERIFIED | Installer guard rejects Legacy Moqi cleanup and broad generic process kills. |
| Shipped runtime folder renamed to `TypeDuckRuntime` | VERIFIED | Runtime pruning guard. |
| Top-level `backends.json` removed from source/stage authority | VERIFIED | Runtime/launcher guards. |
| Backend discovery fixed to TypeDuck runtime bridge | VERIFIED | Launcher protocol guard. |
| TypeDuck diagnostics/log paths | VERIFIED | Backend diagnostics guard and VM log observation. |
| Cloud/WebDAV/AI/fcitx/scheme/custom/autopair active surfaces removed | VERIFIED | Backend removed-surface tests. |
| Backend `buildMenu` unreachable for v1 | VERIFIED | Backend menu reachability guard. |
| Named pipe DACL and client identity sanity | VERIFIED | Launcher protocol guard and protocol recovery tests. |
| TypeDuck release/nightly workflow naming | VERIFIED | Release artifact guard. |
| Appearance theme `source` metadata removed | VERIFIED | Appearance theme guard and backend/frontend commits. |
| Raw icon leakage controlled | VERIFIED | Only `resources\TypeDuck_Small.ico` is staged as a raw icon for the Windows system IME picker. |

## Deferred Items

None. The prior live Windows behavior follow-up has been closed by Phase 7 interactive VM verification and install/uninstall iterations.

## Requirements Coverage

| Requirement | Status |
|---|---|
| IDEN-01 | SATISFIED |
| IDEN-03 | SATISFIED |
| IDEN-04 | SATISFIED |
| LANG-01 | SATISFIED |
| LANG-02 | SATISFIED |
| SEC-01 | SATISFIED |
| SEC-02 | SATISFIED |
| SEC-03 | SATISFIED |
| SEC-04 | SATISFIED |
| SEC-05 | SATISFIED |
| VER-02 | SATISFIED |

## Gaps Summary

No open Phase 6 gaps remain.
