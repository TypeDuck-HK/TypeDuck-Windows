---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 04
subsystem: candidate-dictionary-ui
tags: [typeduck, win32, gdi, tsf, candidate-ui, dictionary-ui, preview]

requires:
  - phase: 05-01
    provides: TypeDuck semantic theme/font roles
  - phase: 05-02
    provides: Native CandidateInfo/CandidateEntry parser and view model
  - phase: 05-03
    provides: TypeDuck settings preferences for display-language and presentation fields
provides:
  - Native Win32/GDI candidate rows and dictionary side panel rendered from CandidateInfo
  - Movement-triggered dictionary reveal without passive hover flicker
  - Work-area-clamped, no-activate candidate popup placement with fallback anchors
  - Candidate preview harness scenarios for nei, multilingual, housam, reverse lookup, edge, DPI, and fallback
affects: [05-05-settings-about-ui, 05-06-vm-ui-proof, phase-06-scaffold-cleanup]

tech-stack:
  added: [PowerShell guard script, CandidateInfo-backed preview capture mode]
  patterns:
    - Keep candidate/dictionary popup native Win32/GDI inside TSF host processes.
    - Render CandidateInfo at the popup boundary and keep transport layers raw.
    - Reveal dictionary detail only after actual pointer movement over a candidate.

key-files:
  created:
    - scripts/Test-TypeDuckCandidateWindow.ps1
    - .planning/product/ui-fixtures/phase-05/candidate-preview/capture-commands.md
  modified:
    - MoqiTextService/MoqiCandidateWindow.h
    - MoqiTextService/MoqiCandidateWindow.cpp
    - MoqiTextService/MoqiTextService.cpp
    - Preview/main.cpp
    - Preview/CMakeLists.txt

key-decisions:
  - "The TypeDuck candidate/dictionary popup remains native Win32/GDI and explicitly rejects Qt in the TSF popup path."
  - "Dictionary detail reveal follows the Web alpha movement threshold instead of passive pointer-rest hover."
  - "Preview screenshots remain iteration evidence only; Plan 05-06 owns exact VM host-app proof."

patterns-established:
  - "Use scripts/Test-TypeDuckCandidateWindow.ps1 as the strict guard for native popup rendering, banned toolkit usage, movement reveal, DPI, fallback, and no-activate placement."
  - "Use Preview/MoqiCandidatePreview.exe --sample ... --capture ... for repeatable layout captures."

requirements-completed: [CAND-01, CAND-02, CAND-03, CAND-04, CAND-05, CAND-06, CAND-07, LANG-03]

duration: 17 min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 04: Native Candidate and Dictionary Popup Summary

**Native TypeDuck candidate rows and movement-triggered dictionary side panel with focus-safe Win32 placement**

## Performance

- **Duration:** 17 min
- **Started:** 2026-06-24T04:13:38Z
- **Completed:** 2026-06-24T04:30:20Z
- **Tasks:** 3/3
- **Files modified:** 7

## Accomplishments

- Added `scripts/Test-TypeDuckCandidateWindow.ps1`, a strict static/rendering contract guard with RED mode for missing native rendering.
- Replaced the generic preview with CandidateInfo-backed TypeDuck scenarios for `nei`, multilingual Indonesian-main, `housam`, reverse lookup, edge clamp, high DPI, and fallback anchor capture.
- Reworked the native candidate popup into a measured TypeDuck layout: input buffer, page navigation, candidate rows, Jyutping, Chinese text, note/code, definitions, dictionary indicator, and side dictionary panel.
- Implemented actual movement-triggered dictionary reveal and reset behavior so a stationary pointer does not passively open or flicker the dictionary panel while typing.
- Hardened placement with caret/foreground fallback anchors, monitor work-area clamping, and `SWP_NOACTIVATE` topmost placement.

## Task Commits

1. **Task 1: Add native popup contract guard and preview cases** - `f321aaf` (test)
2. **Task 2: Render source-backed candidate and dictionary layout** - `be7eb87` (feat)
3. **Task 3: Harden placement, movement reveal, DPI, and fallback behavior** - `f1f7b1e` (fix)

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckCandidateWindow.ps1 -RepoRoot . -Strict -ExpectRed CandidateRenderingMissing` - PASS during Task 1 RED guard.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckCandidateWindow.ps1 -RepoRoot . -Strict` - PASS.
- `cmake --build build-vs32 --config Debug --target MoqiCandidatePreview -- /m:1` - PASS.
- `cmake --build build-vs32 --config Debug --target MoqiTextService -- /m:1` - PASS.

## Files Created/Modified

- `scripts/Test-TypeDuckCandidateWindow.ps1` - strict guard for native rendering anchors, Qt rejection, movement reveal, DPI, fallback, UI-less, and no-activate placement.
- `.planning/product/ui-fixtures/phase-05/candidate-preview/capture-commands.md` - documented preview capture commands for all planned scenarios.
- `Preview/main.cpp` - CandidateInfo-backed TypeDuck preview renderer and BMP capture mode.
- `Preview/CMakeLists.txt` - preview target now builds `TypeDuckCandidateInfo.cpp`.
- `MoqiTextService/MoqiCandidateWindow.h` - candidate popup dictionary state, movement tracking, layout, and drawing helpers.
- `MoqiTextService/MoqiCandidateWindow.cpp` - native TypeDuck candidate/dictionary drawing, movement reveal, and semantic palette usage.
- `MoqiTextService/MoqiTextService.cpp` - TypeDuck palette defaults plus work-area-clamped fallback/no-activate placement.

## Decisions Made

- Kept the TSF popup path native and focus-safe; no Qt or other heavyweight toolkit was introduced.
- Kept dictionary parsing/view-model consumption at the candidate-window boundary from Plan 05-02.
- Treated preview screenshots as development aids only; VM host-app proof remains deferred to Plan 05-06 per D-36.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Split strict guard scope across Task 2 and Task 3**
- **Found during:** Task 2
- **Issue:** The initial Task 1 guard required Task 3 placement anchors even though Task 2 verification also runs `-Strict`.
- **Fix:** Narrowed Task 2 strict checks to renderer anchors, then re-extended the guard in Task 3 for placement/fallback/UI-less checks.
- **Files modified:** `scripts/Test-TypeDuckCandidateWindow.ps1`
- **Verification:** Task 2 and Task 3 strict guard runs both passed.
- **Committed in:** `be7eb87`, `f1f7b1e`

**2. [Rule 3 - Blocking] Corrected preview CMake source-root path**
- **Found during:** Task 2 build verification
- **Issue:** `Preview/CMakeLists.txt` used `PROJECT_SOURCE_DIR` after declaring its own subproject, resolving the shared CandidateInfo source under `Preview/MoqiTextService/...`.
- **Fix:** Switched preview source/include references to `CMAKE_SOURCE_DIR`.
- **Files modified:** `Preview/CMakeLists.txt`
- **Verification:** `cmake --build build-vs32 --config Debug --target MoqiCandidatePreview -- /m:1` passed.
- **Committed in:** `be7eb87`

**3. [Rule 2 - Missing Critical Functionality] Replaced visible Moqi candidate UI description**
- **Found during:** Task 2 implementation
- **Issue:** The candidate UI element still exposed `Moqi candidate window`, conflicting with TypeDuck bilingual user-facing string rules.
- **Fix:** Replaced it with `TypeDuck 候選詞視窗 Candidate window`.
- **Files modified:** `MoqiTextService/MoqiCandidateWindow.cpp`
- **Verification:** `MoqiTextService` build passed and guard rejects TSF popup toolkit drift.
- **Committed in:** `be7eb87`

---

**Total deviations:** 3 auto-fixed (1 Rule 2, 2 Rule 3)  
**Impact on plan:** All fixes were required for correctness, verification, or product string compliance. No architectural scope change.

## Issues Encountered

- CMake reconfigured `build-vs32` during verification after CMake files changed; it completed successfully after the preview source-root fix.
- Build output still includes pre-existing third-party CMake deprecation/dev warnings from vendored dependencies; no project source changes were needed for them.

## Known Stubs

None. Stub scan found only pre-existing scaffold `FIXME` comments in unrelated font/message-window placement code and ordinary Win32 zero-initialization/null checks.

## Threat Flags

None beyond the plan threat model. The candidate popup continues to draw bounded in-process UI from parsed CandidateInfo data, rejects Qt in the TSF popup path, and clamps popup placement to the monitor work area.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Ready for Plan 05-05 settings/About executable work and Plan 05-06 VM host-app proof. The native candidate/dictionary popup now has code and preview coverage; exact focus and placement proof remains the planned VM acceptance step.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-04-SUMMARY.md`.
- Task commits exist in `moqi-im-windows`: `f321aaf`, `be7eb87`, `f1f7b1e`.
- Required final verification commands passed.

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
