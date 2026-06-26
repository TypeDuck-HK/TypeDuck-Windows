---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 06
subsystem: ui-evidence
status: complete-by-explicit-user-divergence
completed: 2026-06-26
requirements-completed: [INST-06, CAND-01, CAND-02, CAND-03, CAND-04, CAND-05, CAND-06, CAND-07, SET-01, SET-02, SET-03, SET-04, SET-05, SET-06, SET-07, SET-08, SET-09, SET-10, LANG-03]
---

# Phase 05 Plan 06: Evidence Plan Closeout Summary

Plan 05-06 originally required an aggregate screenshot/VM evidence manifest under `.planning/product/ui-fixtures`. That evidence model was explicitly retired by user direction on 2026-06-26.

## Final Disposition

- **Closed as:** Complete by explicit user-approved divergence.
- **Reason:** The user explicitly replaced all Phase 5 screenshot tests with human verification and requested removal of the entire `.planning/product/ui-fixtures` tree.
- **Replacement evidence:** `05-VERIFICATION.md` and `05-UAT.md` record the human-verified feature set and regression notes.
- **Removed evidence path:** `.planning/product/ui-fixtures`.

## What Remains Valid From the Original Plan

- Focused static/build/package guards remain valid for regression prevention.
- Package/resource/icon settings and About guards remain valid and were rerun after the final resource move.
- The Phase 5 requirements are still covered, but by human verification plus static guards rather than screenshot-manifest acceptance.

## Human-Verified Behaviors Covered

- Candidate panel visual tone, measured sizing, input-buffer rendering, row heights, and page navigation.
- Structured candidate data, Jyutping/comments, multi-row candidate details, POS pills, multilingual rows, and dictionary panel layout/scrolling.
- Movement-triggered dictionary reveal and no passive pointer flicker while typing.
- Settings dialog layout, copy, radio grouping, page-size ticks, persistence, and Rime customization/reload.
- Installer-first-run and post-install settings entry points.
- Separate About executable, About tray entry, duplicate-window prevention, and About content.
- TypeDuck icon/resource packaging and removal of legacy Moqi images.

## Verification Commands Retained

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsAboutUi.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckIconPackaging.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckTsfIdentity.ps1 -RepoRoot . -Strict`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckLauncherProtocol.ps1 -RepoRoot . -Strict`

## Residual Risk

Release-grade screenshot/evidence capture is not part of Phase 5. If screenshots are needed again, they belong in Phase 7 compatibility/release verification and must be freshly captured from the then-current installer/runtime.

