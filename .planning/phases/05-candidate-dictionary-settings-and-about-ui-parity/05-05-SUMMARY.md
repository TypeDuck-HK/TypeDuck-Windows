---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 05
subsystem: settings-about-ui
tags: [typeduck, win32, settings, about-dialog, resources, powershell, cmake]

requires:
  - phase: 05-01
    provides: TypeDuck semantic theme and bilingual UI conventions
  - phase: 05-03
    provides: TypeDuckPreferences JSON source of truth and Apply path
provides:
  - Native Win32 TypeDuckSettings executable target
  - Web-alpha-ordered two-column settings window over TypeDuckPreferences
  - Native About dialog with required bitmap assets, exact text, version, attribution, and links
  - Static settings/About guard for layout, resources, version, attribution, links, and banned Installer.bmp dependency
affects: [05-06-vm-ui-proof, 05-07-installer-settings-launch, 05-08-resource-packaging, phase-06-scaffold-cleanup]

tech-stack:
  added: [Win32 settings executable, PowerShell static guard]
  patterns:
    - Native settings/About UI is built as a first-party Win32 executable without Qt.
    - Settings UI reuses MoqLauncher/TypeDuckPreferences.* instead of creating a parallel persistence contract.
    - About resources live under TypeDuckSettings/assets and are compiled into the TypeDuckSettings resource module.

key-files:
  created:
    - TypeDuckSettings/CMakeLists.txt
    - TypeDuckSettings/main.cpp
    - TypeDuckSettings/TypeDuckSettingsWindow.h
    - TypeDuckSettings/TypeDuckSettingsWindow.cpp
    - TypeDuckSettings/TypeDuckAboutDialog.h
    - TypeDuckSettings/TypeDuckAboutDialog.cpp
    - TypeDuckSettings/TypeDuckSettings.rc
    - TypeDuckSettings/resource.h
    - TypeDuckSettings/TypeDuckSettingsVersion.h.in
    - TypeDuckSettings/TypeDuckSettingsVersion.rc.in
    - TypeDuckSettings/assets/About_Banner.bmp
    - TypeDuckSettings/assets/Credit_Logos.bmp
    - scripts/Test-TypeDuckSettingsAboutUi.ps1
  modified:
    - CMakeLists.txt

key-decisions:
  - "TypeDuckSettings is native Win32 and does not use Qt; the UI is sufficient with standard controls and custom layout anchors."
  - "The settings executable applies through MoqLauncher/TypeDuckPreferences.* and saves TypeDuckPreferences.json, preserving the Plan 05-03 source-of-truth contract."
  - "About dialog attribution uses concise product copy: TypeDuck Windows IME 版本 Version, TypeDuck-HK librime fork v1.1.4, and TypeDuck-HK schema v2.0.0."

patterns-established:
  - "Use scripts/Test-TypeDuckSettingsAboutUi.ps1 as the focused guard for settings order, bilingual labels, About assets/text/link order, and banned Installer.bmp usage."
  - "Generate TypeDuckSettingsVersion.h from the same CMake version value used by resource metadata when native UI needs product version text."

requirements-completed: [SET-03, SET-04, SET-05, SET-06, SET-07, SET-08, SET-09, SET-10, CAND-04, CAND-05, LANG-03]

duration: 16min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 05: Native Settings and About UI Summary

**Native TypeDuckSettings Win32 executable with Web-alpha settings order and exact TypeDuck About content/assets.**

## Performance

- **Duration:** 16 min
- **Started:** 2026-06-24T04:32:51Z
- **Completed:** 2026-06-24T04:48:30Z
- **Tasks:** 2/2
- **Files modified:** 14

## Accomplishments

- Added a first-party `TypeDuckSettings.exe` Win32 target, wired into the top-level CMake graph.
- Built the settings window around the Plan 05-03 `TypeDuckPreferences` contract with Display Languages first, Web-alpha setting order, 4-10 page-size range, bilingual Apply/Cancel copy, and visible capability-gating explanation.
- Copied `About_Banner.bmp` and `Credit_Logos.bmp` from the sibling backend icons directory into `TypeDuckSettings/assets` and compiled them as resources.
- Implemented the About dialog sequence: banner, exact D-27 bilingual text block, credit logos, product version, TypeDuck-HK engine/schema attribution, and the four exact D-24 links opened through `ShellExecuteW`.
- Added `scripts/Test-TypeDuckSettingsAboutUi.ps1` to guard settings order, bilingual labels, shared Apply path, no Qt/configTool use, About resource/text/link order, version source, attribution, and no `Installer.bmp` dependency.

## Task Commits

Each task was committed atomically:

1. **Task 1: Add native TypeDuckSettings executable and two-column settings layout** - `62fff91` (`feat(05-05): add native TypeDuck settings UI`)
2. **Task 2: Implement exact About dialog assets, text, version, attribution, and links** - `b775339` (`feat(05-05): add TypeDuck about dialog`)

## Files Created/Modified

- `CMakeLists.txt` - Adds `TypeDuckSettings` to the build graph and strips `version.txt` before generated version use.
- `TypeDuckSettings/CMakeLists.txt` - Defines the Win32 target, generated version header/resource, jsoncpp linkage, and TypeDuckPreferences reuse.
- `TypeDuckSettings/main.cpp` - Native executable entry point.
- `TypeDuckSettings/TypeDuckSettingsWindow.*` - Two-column settings window with Web-alpha controls and shared Apply path.
- `TypeDuckSettings/TypeDuckAboutDialog.*` - About dialog assets, text, attribution, and link handling.
- `TypeDuckSettings/TypeDuckSettings.rc`, `TypeDuckSettings/resource.h` - Icon, version, and bitmap resources.
- `TypeDuckSettings/TypeDuckSettingsVersion.h.in`, `TypeDuckSettings/TypeDuckSettingsVersion.rc.in` - Version text/resource generation from the product version source.
- `TypeDuckSettings/assets/About_Banner.bmp` - Required About banner copied from `D:/VSProjects/moqi-ime/icons`.
- `TypeDuckSettings/assets/Credit_Logos.bmp` - Required credit logos copied from `D:/VSProjects/moqi-ime/icons`.
- `scripts/Test-TypeDuckSettingsAboutUi.ps1` - Static acceptance guard for this plan.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsAboutUi.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `cmake -S . -B build-vs32-settings-ui -G "Visual Studio 17 2022" -A Win32` - PASS, with existing third-party CMake deprecation warnings.
- `cmake --build build-vs32-settings-ui --config Debug --target TypeDuckSettings -- /m:1` - PASS; produced `build-vs32-settings-ui\TypeDuckSettings\Debug\TypeDuckSettings.exe`.

## Decisions Made

- Used native Win32 controls and resources for the settings/About executable per D-03; no Qt dependency was added.
- Kept the settings Apply behavior batched and shared with `MoqLauncher/TypeDuckPreferences.*`, saving `TypeDuckPreferences.json` rather than introducing a new settings store.
- Used the Phase 2 runtime manifest/version-info evidence for About attribution because it is the available packaged runtime metadata in this repo.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Replaced compiler-definition version string with a generated header**
- **Found during:** Task 1 verification
- **Issue:** Passing `TYPEDUCK_VERSION_TEXT=L"${MOQI_VERSION}"` through MSVC compile definitions corrupted the compiler command line.
- **Fix:** Generated `TypeDuckSettingsVersion.h` from CMake instead of passing a quoted wide string on the command line.
- **Files modified:** `TypeDuckSettings/CMakeLists.txt`, `TypeDuckSettings/TypeDuckSettingsVersion.h.in`, `scripts/Test-TypeDuckSettingsAboutUi.ps1`
- **Verification:** Strict settings/About guard, CMake configure, and TypeDuckSettings build passed.
- **Committed in:** `62fff91`

**2. [Rule 3 - Blocking] Stripped trailing newline from version.txt before generating native version text**
- **Found during:** Task 2 verification
- **Issue:** `version.txt` includes a trailing newline; the generated C++ version header produced an unterminated wide string literal.
- **Fix:** Added `string(STRIP "${MOQI_VERSION}" MOQI_VERSION)` immediately after reading `version.txt`.
- **Files modified:** `CMakeLists.txt`
- **Verification:** Strict settings/About guard and TypeDuckSettings build passed.
- **Committed in:** `b775339`

---

**Total deviations:** 2 auto-fixed (Rule 3 blocking)
**Impact on plan:** Both fixes were required to make the planned version-source About UI build correctly. No scope expansion.

## Issues Encountered

- The guard initially used a malformed regex for the CMake version definition. It was corrected before the Task 1 commit.
- The final build emitted existing third-party CMake deprecation warnings from spdlog, jsoncpp, libIME2, and googletest; they did not block the target build.

## Known Stubs

None. Stub scan found no TODO/FIXME/placeholder user-facing content. The `nullptr` matches are Win32 handle initializers, not placeholder data flowing to UI rendering.

## Threat Flags

None beyond the plan's threat model. The only new external-boundary behavior is the planned About link opening; links are hard-coded, guard-validated, and opened through `ShellExecuteW`.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 05-07 can launch the same `TypeDuckSettings.exe` from installer and post-install entry points. Plan 05-06 can verify the native settings/About surfaces in the VM, and Plan 05-08 can package the executable/icon resources.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-05-SUMMARY.md`.
- Created files exist under `TypeDuckSettings/` and `scripts/Test-TypeDuckSettingsAboutUi.ps1`.
- Task commits found: `62fff91`, `b775339`.
- Required verification commands passed.

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
