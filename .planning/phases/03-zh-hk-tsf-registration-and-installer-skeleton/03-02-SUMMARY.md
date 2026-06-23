---
phase: 03-zh-hk-tsf-registration-and-installer-skeleton
plan: 02
subsystem: installer-registration
tags: [windows-tsf, inno-setup, powershell, cmake, setup-helper, zh-hk]

requires:
  - phase: 03-zh-hk-tsf-registration-and-installer-skeleton
    provides: TypeDuckTextService.dll output name, first-party CLSID, and zh-HK profile metadata from Plan 03-01
provides:
  - TypeDuck-branded installer skeleton with TypeDuck AppId, CLSID, install directory, output filename, startup entry, and cleanup logic
  - TypeDuck setup helper registration paths for Win32 SysWOW64 and x64 System32 TSF DLL deployment
  - TypeDuck staging pipeline producing TypeDuckLauncher.exe, TypeDuckSetupHelper.exe, and TypeDuckTextService.dll under TypeDuckIME roots
  - Static installer/setup/staging contract guard for TypeDuck identity alignment
affects: [phase-03-vm-verification, phase-06-cleanup, phase-07-release]

tech-stack:
  added: [PowerShell installer skeleton guard, Inno Setup per-user ISCC discovery, TypeDuck launcher icon asset]
  patterns: [deployed TypeDuck names over scaffold target names, narrow legacy migration cleanup, bilingual product-controlled setup messages]

key-files:
  created:
    - scripts/Test-TypeDuckInstallerSkeleton.ps1
    - MoqLauncher/TypeDuckLauncher.ico
  modified:
    - SetupHelper/SetupHelper.cpp
    - SetupHelper/CMakeLists.txt
    - installer/MoqiTsf.iss
    - installer/build-installer.ps1
    - scripts/install.ps1
    - scripts/_all_in_package.ps1
    - MoqLauncher/CMakeLists.txt
    - MoqLauncher/MoqiLauncher.rc
    - MoqLauncher/version.rc.in

key-decisions:
  - "Keep internal scaffold target/source names for now, but deploy TypeDuckLauncher.exe, TypeDuckSetupHelper.exe, TypeDuckTextService.dll, TypeDuckIME paths, and typeduck-windows-ime-setup.exe."
  - "Use TYPEDUCK_PROGRAM_DIR as the setup-helper registration environment variable, with MOQI_PROGRAM_DIR retained only as a transition-only compatibility alias."
  - "Use local TypeDuck-controlled bilingual installer/setup messages; standard Inno wizard chrome falls back to English when a vetted Traditional Chinese language pack is unavailable."

patterns-established:
  - "Installer contract guard pattern: scripts/Test-TypeDuckInstallerSkeleton.ps1 statically asserts TypeDuck installer/setup/staging identity before VM verification."
  - "Staging copy-name pattern: CMake target names may remain scaffolded, while install.ps1 copies artifacts under deployed TypeDuck names."
  - "Narrow migration cleanup pattern: TypeDuck uninstall cleanup owns TypeDuck state and only allowlists specific legacy Moqi scaffold CLSID/startup residue."

requirements-completed: ["INST-01", "INST-03", "INST-04", "INST-05"]

duration: 17 min wall-clock across checkpoint
completed: 2026-06-24
status: complete
---

# Phase 03 Plan 02: TypeDuck Installer Skeleton Summary

**TypeDuck installer skeleton now stages, builds, registers, and cleans TypeDuck-named dual-bitness TSF payloads with bilingual product-controlled setup copy.**

## Performance

- **Duration:** 17 min wall-clock across checkpoint
- **Started:** 2026-06-23T16:00:12Z
- **Completed:** 2026-06-23T16:17:20Z
- **Tasks:** 3
- **Files modified:** 12

## Accomplishments

- Added `scripts/Test-TypeDuckInstallerSkeleton.ps1`, a strict static guard for TypeDuck installer AppId, CLSID, output filename, staged payload roots, binary names, setup-helper constants, bilingual copy, and uninstall cleanup.
- Updated `SetupHelper` to copy/register/unregister `TypeDuckTextService.dll` for Win32 and x64 TSF paths, use `TypeDuckIME-ReRegisterTSF`, and show bilingual Traditional Hong Kong Chinese / English helper messages.
- Updated the Inno installer skeleton to use TypeDuck AppId, publisher/name, `TypeDuckIME` install directory, `typeduck-windows-ime-setup.exe`, `TypeDuckLauncher` startup value, TypeDuck setup-helper invocation, and TypeDuck/narrow legacy cleanup.
- Updated staging and package scripts to deploy `TypeDuckLauncher.exe`, `TypeDuckSetupHelper.exe`, and `TypeDuckTextService.dll` under `win32\TypeDuckIME` and `x64\TypeDuckIME`.
- Added a stable TypeDuck launcher icon/resource path so Release builds no longer depend on staged backend icon files.

## Task Commits

1. **Task 1: Add the installer skeleton contract check** - `37ea0f9` (test)
2. **Task 2: Update SetupHelper for TypeDuck dual-bitness registration** - `6e87a70` (feat)
3. **Task 3: Update installer script and staging pipeline for TypeDuck payloads** - `9a900b6` (feat)

**Plan metadata:** pending final docs commit

## Files Created/Modified

- `scripts/Test-TypeDuckInstallerSkeleton.ps1` - Static TypeDuck installer/setup/staging contract guard.
- `SetupHelper/SetupHelper.cpp` - TypeDuck TSF DLL names, env vars, scheduled task name, dual-bitness registration paths, and bilingual setup messages.
- `SetupHelper/CMakeLists.txt` - Emits `TypeDuckSetupHelper.exe`.
- `installer/MoqiTsf.iss` - TypeDuck Inno installer skeleton, startup entry, run entry, bilingual product-controlled messages, and cleanup hooks.
- `installer/build-installer.ps1` - Validates TypeDuck stage roots and finds per-user Inno Setup installs.
- `scripts/install.ps1` - Stages TypeDuck-named payloads under `TypeDuckIME` roots.
- `scripts/_all_in_package.ps1` - Uses `pwsh` and expects `typeduck-windows-ime-setup.exe`.
- `MoqLauncher/CMakeLists.txt` - Emits `TypeDuckLauncher.exe`.
- `MoqLauncher/MoqiLauncher.rc` - Uses a stable checked-in TypeDuck launcher icon and zh-HK resource language.
- `MoqLauncher/version.rc.in` - TypeDuck launcher resource metadata.
- `MoqLauncher/TypeDuckLauncher.ico` - Generated TypeDuck launcher icon asset.

## Decisions Made

- Kept source filenames such as `installer/MoqiTsf.iss` and internal CMake target names as scaffold compatibility details, while replacing deployed/user-facing installer and binary names.
- Retained `MOQI_PROGRAM_DIR` only as a transition-only alias during `regsvr32` because Plan 03-01 still supports the legacy env var fallback.
- Used Inno's bundled English wizard resource for the `chinesetraditional` language entry when this local Inno Setup install did not provide `ChineseTraditional.isl`; TypeDuck-controlled installer and setup-helper strings remain bilingual.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Added per-user Inno Setup compiler discovery**
- **Found during:** Task 3 continuation verification
- **Issue:** `ISCC.exe` was installed at `C:\Users\User\AppData\Local\Programs\Inno Setup 6\ISCC.exe`, but `installer/build-installer.ps1` only checked Program Files and PATH.
- **Fix:** Added `$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe` to the compiler search list.
- **Files modified:** `installer/build-installer.ps1`
- **Verification:** `scripts\install.ps1 -SkipMoqiImeCopy` found ISCC at the per-user path and compiled the installer.
- **Committed in:** `9a900b6`

**2. [Rule 3 - Blocking] Replaced staged-backend launcher icon dependency**
- **Found during:** Plan-level Release build after TypeDuck staging paths were introduced
- **Issue:** `MoqLauncher/MoqiLauncher.rc` still referenced `..\installer\stage\win32\MoqiIM\moqi-ime\icons\moqi.ico`, which fails after staging is cleaned/rebuilt under `TypeDuckIME`.
- **Fix:** Generated `MoqLauncher/TypeDuckLauncher.ico`, pointed the resource script at the checked-in icon, and aligned launcher version metadata to TypeDuck.
- **Files modified:** `MoqLauncher/MoqiLauncher.rc`, `MoqLauncher/version.rc.in`, `MoqLauncher/TypeDuckLauncher.ico`
- **Verification:** Release build produced `build-vs32\MoqLauncher\Release\TypeDuckLauncher.exe`.
- **Committed in:** `9a900b6`

**3. [Rule 3 - Blocking] Fell back from missing Traditional Inno language pack**
- **Found during:** Task 3 continuation verification
- **Issue:** The local Inno Setup 6 install did not include `Languages\ChineseTraditional.isl`, so the installer compile failed when referenced directly.
- **Fix:** Removed the missing file dependency and used Inno's bundled `Default.isl` for the `chinesetraditional` language entry, with an explicit comment documenting that TypeDuck-controlled strings remain bilingual until a vetted Traditional pack is bundled.
- **Files modified:** `installer/MoqiTsf.iss`
- **Verification:** Inno Setup compiled `installer\dist\typeduck-windows-ime-setup.exe`.
- **Committed in:** `9a900b6`

---

**Total deviations:** 3 auto-fixed (3 blocking)
**Impact on plan:** All fixes were required to complete the installer skeleton and keep build/install verification reproducible. No install-time settings flow or broader cleanup scope was added.

## Issues Encountered

- The first execution stopped at a human-action checkpoint because Inno Setup 6 was not installed. After the user installed Inno Setup 6, execution resumed from Task 3.
- `scripts\install.ps1 -SkipMoqiImeCopy` intentionally warns that the transitional backend is not copied. This is acceptable for the Phase 3 installer skeleton and VM registration plan; final engine packaging remains later work.
- Inno Setup emitted non-blocking warnings about deprecated `x64` architecture identifier substitution and HKCU registry writes in an admin installer. Phase 3 keeps the existing x64-only installer model and startup entry behavior for VM verification.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckInstallerSkeleton.ps1 -RepoRoot . -Strict`  
  **Result:** PASS - `TypeDuck installer skeleton check passed.`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\build.ps1 -RepoRoot . -Configuration Release`  
  **Result:** PASS - Win32 full solution and x64 `MoqiTextService` target built successfully.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -SkipMoqiImeCopy`  
  **Result:** PASS - staged TypeDuck payload roots and compiled `installer\dist\typeduck-windows-ime-setup.exe`.

## Known Stubs

None - no placeholder or unwired stub patterns were found in files modified by this plan.

## Threat Flags

None - new or modified security-relevant surfaces were the installer/setup trust boundaries already covered by the plan threat model.

## User Setup Required

None - no external service configuration required. Local installer compilation requires Inno Setup 6, now available in this workspace at the per-user install path.

## Next Phase Readiness

Plan 03-03 can verify the generated TypeDuck installer in a clean Windows VM: install, zh-HK profile appearance, Win32/x64 TSF DLL registration, startup entry, reboot-required scheduling behavior where applicable, and uninstall cleanup.

## Self-Check: PASSED

- Created files exist: `scripts/Test-TypeDuckInstallerSkeleton.ps1`, `MoqLauncher/TypeDuckLauncher.ico`.
- Task commits exist: `37ea0f9`, `6e87a70`, `9a900b6`.
- Final verification commands passed.
- Only remaining dirty status is the pre-existing `jsoncpp` submodule marker.

---
*Phase: 03-zh-hk-tsf-registration-and-installer-skeleton*
*Completed: 2026-06-24*
