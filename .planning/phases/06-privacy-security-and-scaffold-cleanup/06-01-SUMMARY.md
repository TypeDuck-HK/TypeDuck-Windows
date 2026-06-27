---
phase: 06-privacy-security-and-scaffold-cleanup
plan: 01
subsystem: installer
tags: [inno-setup, installer, cleanup, localization, typeduck]

requires:
  - phase: 05-candidate-dictionary-settings-and-about-ui-parity
    provides: Phase 5 About copy and Installer.bmp resource used by the installer welcome page
provides:
  - TypeDuck-owned installer welcome bitmap and bilingual Phase 5 About text
  - TypeDuckIME Start Menu folder and bilingual Settings/About/Uninstall shortcuts
  - Removal of the Simplified Chinese Inno translation submodule dependency
  - TypeDuck-only process cleanup and uninstall cleanup guard coverage
affects: [phase-06, installer, release-verification]

tech-stack:
  added: []
  patterns:
    - Static PowerShell guard assertions for installer-visible TypeDuck contracts
    - Inno Setup code keeps process cleanup allowlisted and TypeDuck-owned

key-files:
  created:
    - .planning/phases/06-privacy-security-and-scaffold-cleanup/06-01-SUMMARY.md
  modified:
    - .gitmodules
    - installer/MoqiTsf.iss
    - scripts/Test-TypeDuckInstallerSkeleton.ps1
    - installer/Inno-Setup-Chinese-Simplified-Translation

key-decisions:
  - "Removed Legacy Moqi registry/startup cleanup from the TypeDuck installer so TypeDuck uninstall does not touch separately installed Moqi state."
  - "Kept TypeDuck runtime server cleanup path-scoped to the TypeDuck app directory because server.exe is a generic process name."
  - "Rendered restart guidance on final installer/uninstaller surfaces with reopen-apps-first wording instead of separate technical popups."

patterns-established:
  - "Installer guards now reject the removed Simplified Inno translation dependency and Legacy Moqi cleanup operations."
  - "TypeDuck installer process cleanup uses allowlisted product executable names plus app-dir-scoped runtime process cleanup."

requirements-completed: [IDEN-01, IDEN-03, LANG-01, LANG-02, SEC-05, VER-02]

duration: 7 min
completed: 2026-06-27
status: complete
---

# Phase 06 Plan 01: Installer Cleanup Summary

**TypeDuck installer cleanup with product-owned bilingual welcome copy, Start Menu entries, Simplified Inno dependency removal, and TypeDuck-only process/uninstall cleanup guards**

## Performance

- **Duration:** 7 min
- **Started:** 2026-06-27T10:03:21Z
- **Completed:** 2026-06-27T10:09:48Z
- **Tasks:** 3
- **Files modified:** 4

## Accomplishments

- Removed the Simplified Chinese Inno translation submodule from `.gitmodules` and deleted its tracked gitlink.
- Updated `installer/MoqiTsf.iss` to use `Installer.bmp`, the exact Phase 5 About text, EdUHK publisher copy, and the required `TypeDuckIME` Start Menu shortcuts.
- Disabled Inno running-application close pages, automated narrow TypeDuck process cleanup, removed Legacy Moqi registry/startup cleanup, and added TypeDuck-owned install/data cleanup.
- Extended `scripts/Test-TypeDuckInstallerSkeleton.ps1` so strict mode fails on the removed language dependency, unsafe Legacy Moqi cleanup, broad process cleanup, popup restart guidance, and missing Phase 6 installer contracts.

## Task Commits

1. **Task 1: Remove the legacy installer language dependency** - `5cd96f7` (chore)
2. **Task 2: Replace installer page text and Start Menu entries** - `baf9d6d` (feat)
3. **Task 3: Automate TypeDuck process cleanup without a user-running-process page** - `cc1dada` (feat)
4. **Guard fix: Preserve D-24 source-identifier allowance for packaging scripts** - `0dacbbb` (fix)

**Plan metadata:** `0ecf80a` (docs), final metadata refresh pending

## Files Created/Modified

- `.gitmodules` - Removed the Simplified Chinese Inno translation submodule entry.
- `installer/Inno-Setup-Chinese-Simplified-Translation` - Removed the tracked submodule gitlink as an intentional deletion.
- `installer/MoqiTsf.iss` - Updated welcome bitmap/text, publisher, Start Menu entries, TypeDuck-only cleanup, TypeDuck state removal, and final-page guidance.
- `scripts/Test-TypeDuckInstallerSkeleton.ps1` - Added strict assertions for Phase 6 installer localization, process cleanup, Legacy Moqi coexistence, and final guidance contracts.
- `.planning/phases/06-privacy-security-and-scaffold-cleanup/06-01-SUMMARY.md` - This execution summary.

## Decisions Made

- Removed the prior Legacy Moqi cleanup function and startup value deletion because D-19 requires TypeDuck uninstall to leave Legacy Moqi registrations, files, tasks, startup entries, and processes alone.
- Used direct `taskkill` only for TypeDuck-unique executables and a PowerShell path-filtered cleanup for `server.exe` because that filename is generic.
- Used final installer/uninstaller surfaces for restart guidance and removed separate restart popups that exposed implementation terms.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical] Preserved explicit installer-side DLL identity for guard coverage**
- **Found during:** Task 3
- **Issue:** Removing dead overwrite-detection code also removed the only literal `TypeDuckTextService.dll` occurrence from the installer script, which weakened the existing static guard for installer identity.
- **Fix:** Added an explicit `TypeDuckTextServiceDllName` constant in `installer/MoqiTsf.iss`.
- **Files modified:** `installer/MoqiTsf.iss`
- **Verification:** `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckInstallerSkeleton.ps1 -RepoRoot . -Strict`
- **Committed in:** `cc1dada`

**2. [Rule 1 - Bug] Fixed over-strict legacy identifier guard for packaging scripts**
- **Found during:** Final verification after concurrent Phase 06 package-script commits landed
- **Issue:** `Assert-NarrowLegacyMoqiAllowlist` treated non-visible source/build identifiers in `scripts/install.ps1` and `scripts/_all_in_package.ps1` as user-facing leakage, contrary to D-24.
- **Fix:** Added a guard allowance for those packaging scripts while preserving strict installer/setup surface assertions.
- **Files modified:** `scripts/Test-TypeDuckInstallerSkeleton.ps1`
- **Verification:** `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckInstallerSkeleton.ps1 -RepoRoot . -Strict`
- **Committed in:** `0dacbbb`

---

**Total deviations:** 2 auto-fixed (1 missing critical, 1 bug)
**Impact on plan:** The fixes preserved guard coverage without changing installer behavior or broadening scope.

## Issues Encountered

- `git rm` required the `.gitmodules` edit to be staged before removing the submodule gitlink. Staged only `.gitmodules`, removed only `installer/Inno-Setup-Chinese-Simplified-Translation`, and continued.
- A regex typo in the stub scan command was corrected and re-run. The only match was `$RepoRoot = ""` in a script parameter default, which is not a UI/data stub.
- Concurrent Phase 06 package-script commits caused the strict guard to fail after the first metadata commit. Fixed the 06-01 guard to honor D-24 source-identifier allowances without editing out-of-scope package scripts.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckInstallerSkeleton.ps1 -RepoRoot . -Strict` - PASS
- Task 1 acceptance probes: `.gitmodules` no longer references the removed submodule path; `installer/Inno-Setup-Chinese-Simplified-Translation` is absent - PASS
- Task 2 acceptance probes: installer contains `WizardImageFile=..\TypeDuckSettings\resources\Installer.bmp`, EdUHK publisher, required Start Menu entries, and Phase 5 About text - PASS
- Task 3 acceptance probes: installer contains `CloseApplications=no`, TypeDuck process cleanup, app-dir-scoped server cleanup, TypeDuck startup/task/app/state cleanup, final-page guidance, and no Legacy Moqi cleanup marker - PASS

## Known Stubs

None.

## Threat Flags

| Flag | File | Description |
|------|------|-------------|
| threat_flag: process-cleanup | installer/MoqiTsf.iss | Installer process termination remains an elevated OS cleanup surface, now allowlisted to TypeDuck executables plus app-dir-scoped `server.exe` and covered by strict guard assertions. |
| threat_flag: uninstall-cleanup | installer/MoqiTsf.iss | Uninstall deletes TypeDuck-owned app and data directories only; strict guard rejects Legacy Moqi cleanup paths. |

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Ready for `06-02-PLAN.md` runtime package pruning. No blockers from 06-01.

## Self-Check: PASSED

- Found `.planning/phases/06-privacy-security-and-scaffold-cleanup/06-01-SUMMARY.md` on disk.
- Found task commits `5cd96f7`, `baf9d6d`, `cc1dada`, and `0dacbbb` in git history.
- Re-ran the strict installer skeleton guard successfully.

---
*Phase: 06-privacy-security-and-scaffold-cleanup*
*Completed: 2026-06-27*
