---
phase: 06-privacy-security-and-scaffold-cleanup
plan: 03
subsystem: runtime-discovery
tags: [typeduck, launcher, tsf, runtime, security, powershell]

requires:
  - phase: 06-02
    provides: TypeDuckRuntime is the shipped runtime folder and staged backends.json is removed.
provides:
  - Fixed in-code launcher bridge for TypeDuckRuntime/server.exe.
  - Manifest-free TSF runtime directory discovery under TypeDuckRuntime.
  - Guard coverage rejecting source and staged backends.json as TypeDuck runtime authority.
affects: [06-04, 06-05, 06-06, 06-07, phase-07-release-verification]

tech-stack:
  added: []
  patterns:
    - Launcher constructs the single typeduck-runtime-bridge BackendServer config in code.
    - TSF module scans optional compatibility metadata only under a fixed TypeDuckRuntime directory.
    - Source and staged backends.json are rejected by guard scripts.

key-files:
  created:
    - .planning/phases/06-privacy-security-and-scaffold-cleanup/06-03-SUMMARY.md
  modified:
    - MoqLauncher/PipeServer.cpp
    - MoqiTextService/MoqiImeModule.cpp
    - MoqiTextService/TypeDuckProfile.cpp
    - scripts/Test-TypeDuckLauncherProtocol.ps1
    - scripts/Test-TypeDuckSettingsAboutUi.ps1
    - scripts/Test-TypeDuckRuntimePackagePruning.ps1
    - .planning/codebase/ARCHITECTURE.md
    - .planning/codebase/CONCERNS.md
    - .planning/codebase/INTEGRATIONS.md
    - .planning/codebase/STACK.md
    - backends.json (deleted)

key-decisions:
  - "Launcher backend discovery is no longer file-driven; it always constructs one typeduck-runtime-bridge for TypeDuckRuntime/server.exe."
  - "Optional backend ime.json scanning remains compatibility-only and runs after first-party TypeDuck profile mapping."
  - "TSF Configure and launcher Settings stay fixed to TypeDuckSettings.exe and guards continue to reject backend configTool metadata."

patterns-established:
  - "Use TypeDuckRuntime as the only frontend-known runtime folder."
  - "Reject source and staged backends.json in static guards."
  - "Preserve first-party profile authority before any optional runtime metadata scan."

requirements-completed: [IDEN-01, IDEN-03, SEC-03, SEC-05]

duration: 13 min
completed: 2026-06-27
status: complete
---

# Phase 06 Plan 03: Fixed TypeDuck Runtime Bridge Summary

**Manifest-driven backend discovery replaced with a fixed TypeDuckRuntime bridge and guarded TSF runtime scanning.**

## Performance

- **Duration:** 13 min
- **Started:** 2026-06-27T10:18:00Z
- **Completed:** 2026-06-27T10:31:09Z
- **Tasks:** 3/3
- **Files modified:** 11

## Accomplishments

- Deleted top-level `backends.json` and moved the single `typeduck-runtime-bridge` definition into `MoqLauncher/PipeServer.cpp`.
- Pointed the launcher backend command and working directory at `TypeDuckRuntime\server.exe` and `TypeDuckRuntime`.
- Replaced TSF module runtime-dir discovery with a fixed `TypeDuckRuntime` list while preserving first-party profile authority.
- Updated TypeDuck profile icon preference from `moqi-ime` to `TypeDuckRuntime`, with the existing DLL resource fallback preserved.
- Extended guards so source/staged `backends.json` fail and settings launch remains independent of backend `configTool` metadata.

## Task Commits

Each task was committed atomically:

1. **Task 1: Add RED guards for manifest-free runtime discovery** - `51f868d` (test)
2. **Task 2: Inline the fixed TypeDuck runtime bridge** - `62af53b` (feat)
3. **Task 3: Remove TSF module backend manifest dependency** - `6864cf2` (feat)

## Files Created/Modified

- `MoqLauncher/PipeServer.cpp` - Constructs the fixed TypeDuck runtime bridge in code and scans optional metadata under the bridge working directory.
- `MoqiTextService/MoqiImeModule.cpp` - Uses a fixed `TypeDuckRuntime` runtime directory list instead of parsing `backends.json`.
- `MoqiTextService/TypeDuckProfile.cpp` - Prefers `TypeDuckRuntime\icons\TypeDuck_Small.ico` when available, with DLL fallback.
- `scripts/Test-TypeDuckLauncherProtocol.ps1` - Requires the fixed bridge and rejects source/staged `backends.json`.
- `scripts/Test-TypeDuckSettingsAboutUi.ps1` - Aligns Start Menu checks with Phase 6 labels while preserving config-tool rejection.
- `scripts/Test-TypeDuckRuntimePackagePruning.ps1` - Rejects source `backends.json`.
- `.planning/codebase/ARCHITECTURE.md`, `.planning/codebase/CONCERNS.md`, `.planning/codebase/INTEGRATIONS.md`, `.planning/codebase/STACK.md` - Update current-state map references from manifest-driven backend discovery to the fixed `TypeDuckRuntime` bridge.
- `backends.json` - Deleted intentionally.

## Decisions Made

- Launcher code, not a writable/staged manifest, owns the TypeDuck runtime process contract.
- Runtime metadata may still be scanned only under fixed `TypeDuckRuntime`, and `backendMap_.insert` preserves the seeded first-party TypeDuck profile mapping.
- The settings entry points remain fixed first-party launches; backend-declared `configTool`, `configToolParams`, and `configToolDir` are still rejected by guard coverage.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Updated stale settings guard shortcut expectations**
- **Found during:** Task 1 (Add RED guards for manifest-free runtime discovery)
- **Issue:** `scripts/Test-TypeDuckSettingsAboutUi.ps1` still expected the older Phase 5 About shortcut names and failed before proving the config-tool rejection assertions.
- **Fix:** Updated the guard to match the Phase 6 Start Menu labels already present in `installer/MoqiTsf.iss`: `輸入法設定 IME Settings`, `關於 About TypeDuck…`, and `解除安裝 Uninstall`.
- **Files modified:** `scripts/Test-TypeDuckSettingsAboutUi.ps1`
- **Verification:** `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsAboutUi.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- **Committed in:** `51f868d`

---

**Total deviations:** 1 auto-fixed (Rule 3 blocking)
**Impact on plan:** The fix kept guard coverage aligned with prior Phase 6 installer changes and did not modify installer behavior.

## Issues Encountered

- Running `MoqiLauncher` and `MoqiTextService` MSBuild targets in parallel caused a transient `.tlog` file lock in the shared `build-vs32` dependency tree. `MoqiTextService` completed, and rerunning `MoqiLauncher` sequentially passed.
- Builds continue to emit existing third-party MSVC STL deprecation warnings from vendored dependencies; no new build errors remain.

## Validation Commands

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckLauncherProtocol.ps1 -RepoRoot . -Strict` - PASS after production changes; failed as expected before Task 2.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckTsfIdentity.ps1 -RepoRoot . -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsAboutUi.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckRuntimePackagePruning.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `cmake --build build-vs32 --config Debug --target MoqiLauncher -- /m:1` - PASS after sequential rerun.
- `cmake --build build-vs32 --config Debug --target MoqiTextService -- /m:1` - PASS.

## Known Stubs

- `MoqLauncher/PipeServer.cpp:792` contains a pre-existing `FIXME` about tray menu translation. This plan did not introduce it and did not change tray menu copy.

## Threat Flags

None beyond the plan threat model. The planned trust-boundary reduction removes `backends.json` as runtime authority and keeps settings launch fixed to `TypeDuckSettings.exe`.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 06-04 can assume the frontend no longer trusts `backends.json` and that TypeDuck runtime paths are rooted at `TypeDuckRuntime`. Plan 06-05 can build pipe hardening on the existing fixed bridge and first-party profile mapping.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/06-privacy-security-and-scaffold-cleanup/06-03-SUMMARY.md`.
- Modified production files exist: `MoqLauncher/PipeServer.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `MoqiTextService/TypeDuckProfile.cpp`.
- Modified guard files exist: `scripts/Test-TypeDuckLauncherProtocol.ps1`, `scripts/Test-TypeDuckSettingsAboutUi.ps1`, `scripts/Test-TypeDuckRuntimePackagePruning.ps1`.
- Deleted file confirmed absent: `backends.json`.
- Commits found: `51f868d`, `62af53b`, `6864cf2`.
- Required verification commands passed.

---
*Phase: 06-privacy-security-and-scaffold-cleanup*
*Completed: 2026-06-27*
