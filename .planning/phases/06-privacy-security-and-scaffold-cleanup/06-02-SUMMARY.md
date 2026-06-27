---
phase: 06-privacy-security-and-scaffold-cleanup
plan: 02
subsystem: packaging
tags: [typeduck-runtime, powershell, installer-staging, privacy-cleanup, backend-package]

requires:
  - phase: 05-candidate-dictionary-settings-and-about-ui-parity
    provides: TypeDuck icons, appearance themes, settings/About executables, and package guards
provides:
  - TypeDuckRuntime backend package output with banned scaffold payload pruned
  - Windows installer staging under TypeDuckIME\TypeDuckRuntime
  - Runtime package guard covering backend package output and staged payload output
affects: [phase-06-runtime-discovery, phase-06-aggregate-guards, phase-07-release-verification]

tech-stack:
  added: []
  patterns:
    - PowerShell guard scripts enforce package-level banned surface absence
    - Backend package is pruned before Windows staging consumes it

key-files:
  created:
    - scripts/Test-TypeDuckRuntimePackagePruning.ps1
  modified:
    - scripts/install.ps1
    - scripts/_all_in_package.ps1
    - installer/build-installer.ps1
    - scripts/Test-TypeDuckIconPackaging.ps1
    - scripts/Test-TypeDuckAppearanceTheme.ps1
    - D:/VSProjects/moqi-ime/scripts/build.ps1

key-decisions:
  - "Use TypeDuckRuntime as the shipped runtime folder while preserving legacy -MoqiImeSource/-MoqiImeRoot parameter names for caller compatibility."
  - "Do not stage backends.json; Plan 06-03 owns the fixed in-code TypeDuck runtime bridge."
  - "Keep only canonical input_methods/rime/appearance_themes.json in package output; do not ship the former data-path copy."

patterns-established:
  - "Runtime package guards scan generated package trees only when they exist, so backend packaging and Windows staging can be verified independently."
  - "Windows staging mirrors backend pruning as defense in depth."

requirements-completed: [IDEN-01, IDEN-04, SEC-02, VER-02]

duration: 70 min
completed: 2026-06-27
status: complete
---

# Phase 06 Plan 02: Runtime Package Pruning Summary

**TypeDuckRuntime packaging now excludes cloud/AI/fcitx/scaffold payloads before and during Windows installer staging.**

## Performance

- **Duration:** 70 min
- **Started:** 2026-06-27T09:04:00Z
- **Completed:** 2026-06-27T10:14:25Z
- **Tasks:** 3
- **Files modified:** 7

## Accomplishments

- Added `scripts/Test-TypeDuckRuntimePackagePruning.ps1`, including expected-red support and strict checks for backend package and staged runtime payloads.
- Updated the sibling backend build to produce `scripts/build/TypeDuckRuntime`, remove banned runtime paths/files, keep only canonical `input_methods/rime/appearance_themes.json`, and stop generating legacy backend snippets.
- Updated Windows staging to copy the pruned runtime into `installer/stage/win32/TypeDuckIME/TypeDuckRuntime` and stop staging top-level `backends.json`.

## Task Commits

1. **Task 1: Add a runtime package pruning guard** - `a84044f` (test)
2. **Task 2: Prune the sibling TypeDuck runtime at package build time** - `860226a` in `D:/VSProjects/moqi-ime` (feat)
3. **Task 2 guard fix** - `38ca18a` (fix)
4. **Task 3: Stage the pruned runtime under the TypeDuck installed tree** - `a54aa6c` (feat)

## Files Created/Modified

- `scripts/Test-TypeDuckRuntimePackagePruning.ps1` - New strict guard for banned TypeDuck runtime package and staged payload surfaces.
- `scripts/install.ps1` - Stages `TypeDuckRuntime`, applies defense-in-depth runtime filtering, and no longer copies `backends.json`.
- `scripts/_all_in_package.ps1` - Consumes sibling `scripts/build/TypeDuckRuntime`.
- `installer/build-installer.ps1` - No longer requires staged `backends.json`.
- `scripts/Test-TypeDuckIconPackaging.ps1` - Checks staged `TypeDuckRuntime/server.exe` and accepts the new backend pruning helper.
- `scripts/Test-TypeDuckAppearanceTheme.ps1` - Checks `TypeDuckRuntime` output and rejects the old packaged data-path theme copy.
- `D:/VSProjects/moqi-ime/scripts/build.ps1` - Builds/prunes the TypeDuck runtime package at source.

## Decisions Made

- Preserved legacy PowerShell parameter names only as caller compatibility; all produced runtime paths and user-facing script output now use TypeDuck wording.
- Removed backend snippet generation because Plan 06-03 will replace manifest-driven runtime discovery.
- Did not edit `installer/MoqiTsf.iss` during Plan 02 execution; a follow-up integration fix in the Plan 01-owned installer script repaired the Inno compile issue found by Plan 02 staging verification.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Fixed PowerShell guard accumulator collision**
- **Found during:** Task 2 strict pruning guard verification
- **Issue:** The new guard used `$matches`, colliding with PowerShell's automatic `$Matches` hashtable.
- **Fix:** Renamed the accumulator and used explicit hashtable key access.
- **Files modified:** `scripts/Test-TypeDuckRuntimePackagePruning.ps1`
- **Verification:** `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckRuntimePackagePruning.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- **Committed in:** `38ca18a`

**2. [Rule 1 - Bug] Narrowed banned-text scanning away from legitimate Rime dictionaries**
- **Found during:** Task 2 strict pruning guard verification
- **Issue:** The guard treated ordinary dictionary words containing `ai` as banned AI text.
- **Fix:** Excluded `input_methods/rime/data` dictionary/schema content from text scanning while retaining explicit banned path checks.
- **Files modified:** `scripts/Test-TypeDuckRuntimePackagePruning.ps1`
- **Verification:** strict pruning guard passed
- **Committed in:** `38ca18a`

**Total deviations:** 2 auto-fixed (2 Rule 1 bugs)
**Impact on plan:** Guard fixes were required for accurate package verification and did not expand runtime package scope.

## Issues Encountered

- Initial staging exposed an Inno compile issue in the Plan 01-owned installer About text block, where a line beginning with `#13#10` was parsed as a preprocessor directive. Follow-up integration verification changed those blank lines to string literals and reran the full staging/installer build successfully.

## Verification

- PASS: `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckRuntimePackagePruning.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict -ExpectRed RejectedRuntimePackage`
- PASS: `pwsh -NoProfile -ExecutionPolicy Bypass -File D:\VSProjects\moqi-ime\scripts\build.ps1 -RepoRoot D:\VSProjects\moqi-ime -RimeDataSource I:\GitHub\TypeDuck-Web\schema`
- PASS: `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckRuntimePackagePruning.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- PASS: `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckAppearanceTheme.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- PASS: `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckIconPackaging.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- PASS after integration fix: `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -MoqiImeSource D:\VSProjects\moqi-ime\scripts\build\TypeDuckRuntime`

## Known Stubs

None. Stub scan only found intentional PowerShell empty-string parameter defaults and empty version-resource metadata fields.

## Threat Flags

None. This plan reduces shipped runtime surface; it does not add new network endpoints, auth paths, file access trust boundaries, or schema changes beyond the planned package/staging trust boundary.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 06-03 can replace manifest-driven backend discovery with a fixed TypeDuck runtime bridge targeting `TypeDuckRuntime`. Runtime staging and installer compilation are green after the follow-up installer syntax fix.

## Self-Check: PASSED

- Found `scripts/Test-TypeDuckRuntimePackagePruning.ps1`.
- Found `D:/VSProjects/moqi-ime/scripts/build/TypeDuckRuntime/server.exe`.
- Found `D:/VSProjects/moqi-ime/scripts/build/TypeDuckRuntime/input_methods/rime/appearance_themes.json`.
- Found `installer/stage/win32/TypeDuckIME/TypeDuckRuntime/server.exe`.
- Confirmed `installer/stage/win32/TypeDuckIME/backends.json` is absent.
- Confirmed commits: `a84044f`, `38ca18a`, `a54aa6c`, and sibling backend commit `860226a`.

---
*Phase: 06-privacy-security-and-scaffold-cleanup*
*Completed: 2026-06-27*
