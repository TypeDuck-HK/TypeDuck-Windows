---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 07
subsystem: settings-launch
tags: [typeduck, installer, win32, settings, tsf, launcher, powershell]

requires:
  - phase: 05-03
    provides: TypeDuckPreferences JSON source of truth and shared settings apply path.
  - phase: 05-05
    provides: Native first-party TypeDuckSettings.exe settings/About executable.
provides:
  - Installer-first-run TypeDuckSettings.exe launch after successful setup-helper registration.
  - Post-install fixed TypeDuckSettings.exe entry points from TSF Configure and launcher tray menu.
  - Guard coverage proving settings launch paths avoid backend-declared configTool metadata.
affects: [05-06-vm-ui-proof, 05-08-resource-packaging, phase-06-scaffold-cleanup]

tech-stack:
  added: []
  patterns:
    - Fixed first-party settings launch paths root at the installed TypeDuck directory.
    - Installer staging resolves TypeDuckSettings.exe as a required payload artifact.
    - Static settings/About guard covers installer and post-install entry points.

key-files:
  created: []
  modified:
    - MoqLauncher/PipeServer.cpp
    - MoqLauncher/PipeServer.h
    - MoqiTextService/MoqiImeModule.cpp
    - installer/MoqiTsf.iss
    - scripts/install.ps1
    - scripts/Test-TypeDuckSettingsAboutUi.ps1

key-decisions:
  - "Install-time and post-install settings entry points launch the fixed first-party TypeDuckSettings.exe."
  - "Backend-declared configTool metadata is no longer used for the Phase 5 settings surface."
  - "Installer staging treats TypeDuckSettings.exe as a required payload and selects the newest built launcher artifact during verification."

patterns-established:
  - "Use scripts/Test-TypeDuckSettingsAboutUi.ps1 to guard both settings UI content and settings launch reachability."
  - "Use bilingual Traditional Hong Kong Chinese / English error text for settings launch failures."

requirements-completed: [INST-06, SET-01, SET-02]

duration: 46min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 07: Settings Launch Wiring Summary

**Installer and post-install TypeDuck entry points now open the same first-party TypeDuckSettings.exe dialog.**

## Performance

- **Duration:** 46 min
- **Started:** 2026-06-24T04:16:00Z
- **Completed:** 2026-06-24T05:01:46Z
- **Tasks:** 2/2
- **Files modified:** 6

## Accomplishments

- Staged `TypeDuckSettings.exe` into the installer payload and added a post-install settings launch before launcher startup.
- Replaced TSF Configure backend `configTool` metadata launching with a fixed installed `TypeDuckSettings.exe` path.
- Added a launcher tray menu item, `設定 / Settings`, that opens the same installed settings executable.
- Extended `scripts/Test-TypeDuckSettingsAboutUi.ps1` to prove installer staging, installer run order, TSF Configure launch, launcher launch, bilingual labels, and absence of backend config-tool metadata.

## Task Commits

Each task was committed atomically:

1. **Task 1: Wire installer-first-run settings launch** - `976fc09` (`feat(05-07): launch settings during install`)
2. **Task 2: Wire post-install settings entry point** - `3bb78a8` (`feat(05-07): add fixed settings entry points`)
3. **Verification fix: stage newest launcher artifact** - `0c196ee` (`fix(05-07): stage newest launcher artifact`)

## Files Created/Modified

- `installer/MoqiTsf.iss` - Adds a bilingual post-install `TypeDuckSettings.exe` run entry gated by successful setup-helper registration.
- `scripts/install.ps1` - Stages `TypeDuckSettings.exe` and resolves Debug/Release launcher/settings artifacts for package verification.
- `scripts/Test-TypeDuckSettingsAboutUi.ps1` - Guards installer launch wiring, post-install launch wiring, and no backend config-tool dependency.
- `MoqiTextService/MoqiImeModule.cpp` - Routes TSF Configure to installed `TypeDuckSettings.exe` with bilingual failure text.
- `MoqLauncher/PipeServer.cpp` - Adds launcher tray settings menu item and fixed settings executable launch helper.
- `MoqLauncher/PipeServer.h` - Declares the launcher settings launch helper.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsAboutUi.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -MoqiImeSource D:\VSProjects\moqi-ime\scripts\build\moqi-ime` - PASS; staged `TypeDuckSettings.exe`, staged updated Debug `TypeDuckLauncher.exe`, and compiled `installer\dist\typeduck-windows-ime-setup.exe`.
- `cmake --build build-vs32-settings-ui --config Debug --target TypeDuckSettings -- /m:1` - PASS.
- `cmake --build build-vs32 --config Debug --target MoqiLauncher -- /m:1` - PASS, with existing third-party MSVC STL deprecation warnings.
- `cmake --build build-vs32 --config Debug --target MoqiTextService -- /m:1` - PASS, with existing third-party CMake deprecation warnings.

## Decisions Made

- Used `TypeDuckSettings.exe` as the only settings executable for installer and post-install entry points, matching D-17 and SET-02.
- Kept launch roots local to the installed TypeDuck directory rather than trusting backend metadata.
- Preserved installer launcher startup while opening settings first through the Inno `[Run]` order.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Made installer staging find the built settings executable**
- **Found during:** Task 1 verification.
- **Issue:** The plan verification command runs `scripts/install.ps1` directly, while this workspace had `TypeDuckSettings.exe` built under `build-vs32-settings-ui`.
- **Fix:** Added `TypeDuckSettings.exe` staging and Debug/settings-UI artifact candidates so verification and local packaging can locate the existing first-party executable.
- **Files modified:** `scripts/install.ps1`, `scripts/Test-TypeDuckSettingsAboutUi.ps1`
- **Verification:** Strict settings/About guard and installer build passed.
- **Committed in:** `976fc09`

**2. [Rule 3 - Blocking] Packaged the launcher build that contains the new settings entry**
- **Found during:** Plan-level verification.
- **Issue:** `scripts/install.ps1` preferred Release launcher candidates only, so verification could package an older launcher without the new tray settings entry after a Debug build.
- **Fix:** Added Debug launcher artifact candidates, preserving newest-artifact selection.
- **Files modified:** `scripts/install.ps1`
- **Verification:** Installer build selected `build-vs32\MoqLauncher\Debug\TypeDuckLauncher.exe` and passed.
- **Committed in:** `0c196ee`

---

**Total deviations:** 2 auto-fixed (Rule 3 blocking)
**Impact on plan:** Both fixes were necessary for the plan's verification commands to prove the intended launch wiring in the current workspace. No product-scope expansion.

## Issues Encountered

- Initial guard assertions for the new install staging checks used brittle PowerShell regex strings. They were corrected before the first task commit.
- Builds emitted existing CMake and MSVC deprecation warnings from third-party/vendored dependencies; no new build errors remained.
- The staged transitional backend still includes legacy scaffold assets and config files. That is pre-existing Phase 6 cleanup scope, not new launch wiring behavior.

## Known Stubs

None introduced. Stub scan found one pre-existing `FIXME` comment in `MoqLauncher/PipeServer.cpp` near tray menu translation and ordinary empty-string/null/handle initializers; none are new placeholder UI data from this plan.

## Threat Flags

None beyond the plan threat model. The planned local executable launch surface now uses fixed first-party `TypeDuckSettings.exe` paths and guard coverage rejects backend `configTool` metadata for this settings surface.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 05-06 can verify install-time and post-install settings launch in the Windows VM. Plan 05-08 can package final TypeDuck icons/resources knowing `TypeDuckSettings.exe` is now a required staged installer payload.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-07-SUMMARY.md`.
- Modified production files exist: `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeServer.h`, `MoqiTextService/MoqiImeModule.cpp`, `installer/MoqiTsf.iss`, `scripts/install.ps1`, `scripts/Test-TypeDuckSettingsAboutUi.ps1`.
- Commits found: `976fc09`, `3bb78a8`, `0c196ee`.
- Required verification commands passed.

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
