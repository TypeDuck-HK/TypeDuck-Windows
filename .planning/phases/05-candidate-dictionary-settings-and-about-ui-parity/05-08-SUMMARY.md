---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 08
subsystem: resource-packaging
tags: [typeduck, icons, installer, resources, powershell, win32]

requires:
  - phase: 05-05
    provides: Native TypeDuckSettings About dialog with D-23/D-24/D-27 assets, text, links, version, and attribution.
  - phase: 05-07
    provides: Installer and post-install launch paths for TypeDuckSettings.exe.
provides:
  - Locked TypeDuck icon assets staged under TypeDuckSettings/assets.
  - Source resource assignments for executable, profile picker, setup-helper, and installer icon surfaces.
  - Installer staging that stamps prebuilt executables and the packaged backend server with TypeDuck_Transparent.ico.
  - Strict icon packaging guard covering About content, exact links, attribution, staged executable icon resources, installer icon, and banned legacy Moqi image exclusions.
affects: [05-06-vm-ui-proof, phase-06-scaffold-cleanup, phase-07-release-verification]

tech-stack:
  added: [PowerShell PE resource icon updater, Win32 resource scripts]
  patterns:
    - Staged installer payloads are restamped from locked product assets so verification does not depend on stale prebuilt binary resources.
    - Banned legacy image filenames are filtered during transitional backend runtime copying.

key-files:
  created:
    - TypeDuckSettings/assets/TypeDuck.ico
    - TypeDuckSettings/assets/TypeDuck_Small.ico
    - TypeDuckSettings/assets/TypeDuck_Transparent.ico
    - SetupHelper/SetupHelper.rc
    - scripts/Test-TypeDuckIconPackaging.ps1
  modified:
    - TypeDuckSettings/TypeDuckSettings.rc
    - MoqLauncher/MoqiLauncher.rc
    - MoqiTextService/MoqiTextService.rc.in
    - MoqiTextService/TypeDuckProfile.cpp
    - MoqiTextService/resource.h
    - SetupHelper/CMakeLists.txt
    - installer/MoqiTsf.iss
    - scripts/install.ps1

key-decisions:
  - "Use TypeDuck_Transparent.ico for staged executable surfaces, including TypeDuckLauncher.exe, TypeDuckSetupHelper.exe, TypeDuckSettings.exe, and packaged moqi-ime/server.exe."
  - "Use TypeDuck_Small.ico for the first-party TSF profile/input picker through both a DLL resource fallback and a staged installed icon path."
  - "Use TypeDuck.ico for Inno setup/uninstall branding and copy it into the installed payload."
  - "Filter legacy moqi.png, mo.ico, mo.png, and moqi.ico from the transitional backend package during installer staging."

patterns-established:
  - "scripts/Test-TypeDuckIconPackaging.ps1 is the focused guard for TypeDuck icon assignments, staged About resources, exact D-24 links, attribution, and banned legacy icon files."
  - "scripts/install.ps1 can apply locked icon resources to staged PE files with UpdateResource before compiling the installer."

requirements-completed: [SET-10]

duration: 13 min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 08: Icon and About Resource Packaging Summary

**Locked TypeDuck icon assets and package guards now enforce executable, input-picker, installer, and About resource branding.**

## Performance

- **Duration:** 13 min
- **Started:** 2026-06-24T05:07:38Z
- **Completed:** 2026-06-24T05:20:30Z
- **Tasks:** 2/2
- **Files modified:** 13

## Accomplishments

- Copied `TypeDuck_Transparent.ico`, `TypeDuck_Small.ico`, and `TypeDuck.ico` from `D:\VSProjects\moqi-ime\icons` into `TypeDuckSettings/assets`.
- Assigned `TypeDuck_Transparent.ico` to executable resources and staging-time PE icon stamping for launcher, setup helper, settings, and packaged backend server.
- Assigned `TypeDuck_Small.ico` to the TSF profile picker path with a staged installed-icon preference and DLL resource fallback.
- Assigned `TypeDuck.ico` to Inno setup/uninstall branding and staged app payload branding.
- Added `scripts/Test-TypeDuckIconPackaging.ps1` to guard icon hashes, resource assignments, D-23/D-27 About ordering, D-24 links, version/engine/schema attribution, staged executable icons, installer icon, and banned legacy Moqi image exclusions.

## Task Commits

Each task was committed atomically:

1. **Task 1: Stage TypeDuck icon assets into product resources** - `de20f16` (`feat`)
2. **Task 2: Assign icons and verify staged About resources** - `d760cf8` (`feat`)

## Files Created/Modified

- `TypeDuckSettings/assets/TypeDuck_Transparent.ico` - Locked executable icon source copied from the backend icon source directory.
- `TypeDuckSettings/assets/TypeDuck_Small.ico` - Locked input-picker icon source copied from the backend icon source directory.
- `TypeDuckSettings/assets/TypeDuck.ico` - Locked installer/broad-branding icon source copied from the backend icon source directory.
- `TypeDuckSettings/TypeDuckSettings.rc` - Points the settings/About executable at `TypeDuck_Transparent.ico`.
- `MoqLauncher/MoqiLauncher.rc` - Points the launcher executable resource at `TypeDuck_Transparent.ico`.
- `SetupHelper/CMakeLists.txt`, `SetupHelper/SetupHelper.rc` - Adds setup-helper resource compilation with `TypeDuck_Transparent.ico`.
- `MoqiTextService/MoqiTextService.rc.in`, `MoqiTextService/resource.h`, `MoqiTextService/TypeDuckProfile.cpp` - Adds the small profile icon resource and installed `TypeDuck_Small.ico` profile preference.
- `installer/MoqiTsf.iss` - Uses `TypeDuck.ico` for setup and uninstall display branding.
- `scripts/install.ps1` - Copies product icons into the app payload, stamps staged PE icon resources, and filters banned legacy Moqi icon images from the transitional backend copy.
- `scripts/Test-TypeDuckIconPackaging.ps1` - Strict package guard for SET-10 icon/About packaging.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckIconPackaging.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -MoqiImeSource D:\VSProjects\moqi-ime\scripts\build\moqi-ime` - PASS; staged icons, stamped staged executables, filtered banned Moqi icon images, and compiled `installer\dist\typeduck-windows-ime-setup.exe`.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckIconPackaging.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict && pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -MoqiImeSource D:\VSProjects\moqi-ime\scripts\build\moqi-ime` - PASS.
- Post-install-stage guard rerun: `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckIconPackaging.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.

The Inno compiler emitted existing warnings about deprecated `x64` architecture identifier usage and admin install mode touching HKCU. These warnings predate this plan and did not block the installer compile.

## Decisions Made

- Used the three locked icon files exactly as specified by D-28 through D-30.
- Kept icon assets in `TypeDuckSettings/assets` as the semantic product resource location for this plan.
- Stamped staged PE files during installer packaging so verification reflects the packaged output even when existing build artifacts were produced before resource-file edits.
- Removed only the four D-31 legacy Moqi image filenames from the transitional backend copy; other backend runtime icons remain for current engine/tray behavior and Phase 6 cleanup scope.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical] Filtered banned legacy Moqi images from staged backend payload**
- **Found during:** Task 2 verification.
- **Issue:** `scripts/install.ps1` copied the transitional backend `icons` directory wholesale, so `moqi.png`, `mo.ico`, `mo.png`, and `moqi.ico` were still compressed into the installer payload.
- **Fix:** Added a banned filename filter in `Copy-MoqiImeRuntime` and extended the icon packaging guard to fail if any of the four D-31 filenames appear under `installer/stage/win32/TypeDuckIME`.
- **Files modified:** `scripts/install.ps1`, `scripts/Test-TypeDuckIconPackaging.ps1`
- **Verification:** Rebuilt the installer stage and reran the strict guard; the banned files were absent and verification passed.
- **Committed in:** `d760cf8`

---

**Total deviations:** 1 auto-fixed (Rule 2 missing critical)
**Impact on plan:** The fix directly enforces the planned D-31 banned image exclusion without broadening product scope.

## Issues Encountered

- The initial guard assertion matched its own banned-filename exclusion list. The scan now strips that explicit filter declaration before checking resource/package references while still checking staged files.
- `SetupHelper/SetupHelper.rc` did not exist before this plan, so the setup-helper icon assignment required adding that resource file and compiling it through `SetupHelper/CMakeLists.txt`.

## Known Stubs

None. Stub scan found only PowerShell optional string parameter defaults and Win32 buffer/handle initializers; no placeholder UI data or TODO/FIXME stubs were introduced.

## Threat Flags

None beyond the plan threat model. The new PE resource update path is local installer staging only and is guarded by exact source icon hashes and staged executable icon checks.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 05-06 can now verify the packaged TypeDuck settings/About/icon surfaces in the Windows VM. Phase 6 still owns broader scaffold cleanup for legacy runtime filenames, templates, AI config, and off-scope feature surfaces not targeted by this icon packaging plan.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-08-SUMMARY.md`.
- Created files exist: `TypeDuckSettings/assets/TypeDuck_Transparent.ico`, `TypeDuckSettings/assets/TypeDuck_Small.ico`, `TypeDuckSettings/assets/TypeDuck.ico`, `SetupHelper/SetupHelper.rc`, and `scripts/Test-TypeDuckIconPackaging.ps1`.
- Task commits found: `de20f16`, `d760cf8`.
- Required verification commands passed.

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
