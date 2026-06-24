---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 11
subsystem: ui
tags: [win32, settings, about, installer, uat-gap-closure]
requires:
  - phase: 05-candidate-dictionary-settings-and-about-ui-parity
    provides: "Phase 5 settings/About foundation, installer settings entry point, and rejected UAT gap inventory."
provides:
  - "Settings layout/copy guard closure for UAT gaps 6 and 7."
  - "Separate TypeDuckAbout.exe build/staging/installer entry point for UAT gap 8."
  - "Static guard coverage for settings copy, page-size ticks, About target wiring, Start Menu shortcuts, and postinstall About launch."
affects: [phase-05, settings-ui, about-ui, installer, uat]
tech-stack:
  added: []
  patterns:
    - "Separate Win32 executable target for About while preserving the existing native About dialog implementation."
    - "Static PowerShell guards for UAT-rejected settings/About copy and wiring."
key-files:
  created:
    - TypeDuckSettings/TypeDuckAboutMain.cpp
    - TypeDuckSettings/TypeDuckAbout.rc
    - TypeDuckSettings/TypeDuckAboutVersion.rc.in
  modified:
    - TypeDuckSettings/TypeDuckSettingsWindow.cpp
    - TypeDuckSettings/CMakeLists.txt
    - scripts/install.ps1
    - installer/MoqiTsf.iss
    - scripts/Test-TypeDuckSettingsAboutUi.ps1
key-decisions:
  - "About is now a separate first-party Win32 executable and is no longer linked into the Settings executable."
  - "The About Start Menu entry is represented as two concrete shortcuts, TypeDuck About and TypeDuck 關於, instead of a single slash-delimited filename."
  - "05-11 stages TypeDuckAbout.exe but does not add new icon stamping rules; executable icon packaging remains 05-12 scope."
patterns-established:
  - "Settings UI copy must avoid internal persistence filenames and unsupported-state placeholders."
  - "About installer access must be guarded through both Start Menu shortcut and non-silent postinstall launch entries."
requirements-completed: [INST-06, SET-01, SET-02, SET-03, SET-04, SET-05, SET-06, SET-07, SET-08, SET-09, SET-10, CAND-04, CAND-05, LANG-03]
duration: 45min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 11: Settings/About Gap Closure Summary

**Native settings layout and copy now match the accepted TypeDuck contract, and About is packaged as a separate first-party executable with installer-visible entry points.**

## Performance

- **Duration:** 45 min
- **Started:** 2026-06-24T15:24:00+08:00
- **Completed:** 2026-06-24T16:09:13+08:00
- **Tasks:** 2
- **Files modified:** 8

## Accomplishments

- Corrected settings hierarchy/copy: dialog-colored background, bold section headings, aligned display/main-language group labels, fixed page-size tick labels 4 through 10, and no user-facing `TypeDuckPreferences.json` or unsupported-state copy.
- Split About out of Settings by removing About sources/buttons from `TypeDuckSettings.exe` and adding `TypeDuckAbout.exe` with its own entry point, resources, and version metadata.
- Added About staging plus installer Start Menu shortcuts and a non-silent postinstall finish action, with guard coverage for all new entry points.

## Task Commits

1. **Task 1: Correct settings hierarchy, grouping, tick labels, and product copy** - `2237be1` (fix)
2. **Task 2: Move About to a separate executable and remove Settings-panel button** - `be933e2` (feat)

## Files Created/Modified

- `TypeDuckSettings/TypeDuckSettingsWindow.cpp` - Settings layout, background, section headings, main-language alignment, tick labels, and product copy.
- `TypeDuckSettings/CMakeLists.txt` - Split `TypeDuckSettings` and `TypeDuckAbout` targets.
- `TypeDuckSettings/TypeDuckAboutMain.cpp` - Separate About executable entry point.
- `TypeDuckSettings/TypeDuckAbout.rc` - About executable resources.
- `TypeDuckSettings/TypeDuckAboutVersion.rc.in` - About-specific Windows version metadata.
- `scripts/install.ps1` - Stages `TypeDuckAbout.exe` without adding 05-12 icon-stamping scope.
- `installer/MoqiTsf.iss` - Adds About Start Menu shortcuts and postinstall launch option.
- `scripts/Test-TypeDuckSettingsAboutUi.ps1` - Guards rejected settings/About UAT behavior and new About executable wiring.

## Decisions Made

- About now lives in `TypeDuckAbout.exe`; Settings no longer links or launches the About dialog.
- The installer creates two concrete shortcuts, `TypeDuck About` and `TypeDuck 關於`, because slash-delimited shortcut filenames are not reliable Windows shortcut names.
- 05-11 does not add new icon stamping for `TypeDuckAbout.exe`; plan 05-12 owns executable icon packaging/stamping.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical Functionality] Represented the requested bilingual About shortcut as two concrete Start Menu shortcuts**
- **Found during:** Task 2
- **Issue:** A single shortcut literally named `TypeDuck About / TypeDuck 關於` would use a slash in a Windows shortcut filename, which is not a reliable product entry point.
- **Fix:** Added `TypeDuck About` and `TypeDuck 關於` shortcuts, both targeting `TypeDuckAbout.exe`.
- **Files modified:** `installer/MoqiTsf.iss`, `scripts/Test-TypeDuckSettingsAboutUi.ps1`
- **Verification:** `scripts\Test-TypeDuckSettingsAboutUi.ps1 -Strict` and Inno compile passed.
- **Committed in:** `be933e2`

---

**Total deviations:** 1 auto-fixed missing critical functionality.
**Impact on plan:** The user-visible About entry point is more concrete and avoids invalid shortcut naming. Scope stayed settings/About only.

## Issues Encountered

- The settings/About guard initially could not prove generated page-size tick labels; the implementation now uses explicit `L"4"` through `L"10"` labels.
- A guard regex for the Settings target initially crossed into the new About target; it now checks only the `TypeDuckSettings` CMake block.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsAboutUi.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- `cmake --build build-vs32-settings-ui --config Debug --target TypeDuckSettings -- /m:1`
- `cmake --build build-vs32-settings-ui --config Debug --target TypeDuckAbout -- /m:1`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -MoqiImeSource D:\VSProjects\moqi-ime\scripts\build\moqi-ime`

## Known Stubs

None. Stub scan found only intentional guard strings and About implementation identifiers, not user-facing placeholders in the settings UI.

## Threat Flags

None. About links and installer/About entry points were already listed in the plan threat model and are covered by the guard.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

05-11 is ready for follow-on Phase 5 proof work. VM visual evidence for the new About executable remains in the later verification plan; icon stamping/packaging for `TypeDuckAbout.exe` remains 05-12 scope.

## Self-Check: PASSED

- FOUND: `TypeDuckSettings/TypeDuckAboutMain.cpp`
- FOUND: `TypeDuckSettings/TypeDuckAbout.rc`
- FOUND: `TypeDuckSettings/TypeDuckAboutVersion.rc.in`
- FOUND: `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-11-SUMMARY.md`
- FOUND: `2237be1`
- FOUND: `be933e2`

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
