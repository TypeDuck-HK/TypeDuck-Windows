---
phase: 06-privacy-security-and-scaffold-cleanup
plan: 04
subsystem: privacy-security
tags: [typeduck, diagnostics, privacy, cloud-removal, protobuf, tsf]

requires:
  - phase: 06-03
    provides: Fixed TypeDuck runtime bridge and identity baseline used by the diagnostics cleanup.
provides:
  - TypeDuck-owned local diagnostics paths under %LOCALAPPDATA%\TypeDuckIME.
  - TypeDuck launcher config/log names using TypeDuckLauncher.json and TypeDuckLauncher.log.
  - Strict privacy/security cleanup guard for diagnostics, banned frontend surfaces, and Legacy Moqi coexistence.
  - Removed Windows frontend cloud clipboard listener, backend upload method, and protocol upload fields.
affects: [06-05-ipc-hardening, diagnostics, launcher, tsf, protocol]

tech-stack:
  added: []
  patterns:
    - Conservative diagnostics paths and English-only runtime logging.
    - Removed protocol numbers are reserved rather than reused.

key-files:
  created:
    - scripts/Test-TypeDuckPrivacySecurityCleanup.ps1
  modified:
    - MoqLauncher/Utils.cpp
    - MoqLauncher/Utils.h
    - MoqLauncher/PipeServer.cpp
    - MoqLauncher/PipeServer.h
    - MoqLauncher/BackendServer.cpp
    - MoqLauncher/BackendServer.h
    - MoqiTextService/MoqiClient.cpp
    - MoqiTextService/MoqiCandidateWindow.cpp
    - MoqiTextService/MoqiTextService.cpp
    - MoqiTextService/TsfLog.cpp
    - MoqiTextService/DllEntry.cpp
    - libIME2/src/DebugLogConfig.cpp
    - libIME2/src/TextService.cpp
    - proto/moqi.proto

key-decisions:
  - "TypeDuck diagnostics use %LOCALAPPDATA%\\TypeDuckIME and TypeDuckLauncher.json; Legacy Moqi local/roaming paths are neither migrated nor touched."
  - "Cloud clipboard protocol method 19 and request field 26 are removed from v1 and reserved in proto/moqi.proto."
  - "Routine quote-pair debug logging records redacted lengths rather than raw committed text."

patterns-established:
  - "TypeDuck runtime state helpers live in MoqLauncher/Utils and distinguish local versus roaming TypeDuck-owned data."
  - "Frontend privacy cleanup is guarded by scripts/Test-TypeDuckPrivacySecurityCleanup.ps1 before build verification."

requirements-completed: [IDEN-01, IDEN-04, LANG-02, SEC-01, SEC-02]

duration: 12min
completed: 2026-06-27
status: complete
---

# Phase 06 Plan 04: Diagnostics Privacy and Scaffold Cleanup Summary

**TypeDuck diagnostics now use TypeDuck-owned paths, avoid routine raw typed-content logs, and no longer expose the v1 cloud clipboard frontend/protocol path.**

## Performance

- **Duration:** 12 min
- **Started:** 2026-06-27T11:00:00Z
- **Completed:** 2026-06-27T11:12:00Z
- **Tasks:** 3
- **Files modified:** 16

## Accomplishments

- Added a strict PowerShell cleanup guard covering TypeDuck paths, diagnostics privacy, banned frontend surfaces, and Legacy Moqi coexistence.
- Replaced Moqi runtime diagnostics paths with `%LOCALAPPDATA%\TypeDuckIME`, `TypeDuckLauncher.json`, and `TypeDuckLauncher.log` without migrating or deleting Legacy Moqi state.
- Removed the Windows frontend cloud clipboard listener/upload path and reserved the removed protobuf method/field numbers.

## Task Commits

Each task was committed atomically:

1. **Task 1: Add TypeDuck path and diagnostics privacy guards** - `4cdcfe8` (test)
2. **Task 2: Rename data, config, and log paths to TypeDuck-owned locations** - `bfd4607` (fix), with `libIME2@8b68411`
3. **Task 3: Remove cloud clipboard and other off-scope frontend paths** - `bc1c1ee` (fix)

## Files Created/Modified

- `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1` - Strict guard for TypeDuck diagnostics/privacy cleanup and banned frontend surface checks.
- `MoqLauncher/Utils.cpp`, `MoqLauncher/Utils.h` - TypeDuck local/roaming data path helpers.
- `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeServer.h` - TypeDuck config/log path usage and cloud clipboard listener removal.
- `MoqLauncher/BackendServer.cpp`, `MoqLauncher/BackendServer.h` - Cloud clipboard upload method removal.
- `MoqiTextService/MoqiClient.cpp` - TypeDuck launcher config path usage and redacted quote-pair diagnostics.
- `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/TsfLog.cpp`, `MoqiTextService/DllEntry.cpp` - TypeDuck log/config path usage.
- `libIME2/src/DebugLogConfig.cpp`, `libIME2/src/TextService.cpp` - TypeDuck debug-log config path usage inside the submodule.
- `proto/moqi.proto` - Removed v1 cloud clipboard request surface and reserved removed numbers.

## Decisions Made

- TypeDuck v1 keeps Legacy Moqi state separate: no cleanup code migrates, repairs, unregisters, kills, or deletes `%APPDATA%\Moqi`, `%LOCALAPPDATA%\MoqiIM`, Moqi registry keys, scheduled tasks, install folders, or processes.
- Direct English diagnostics were kept for technical logs, while routine user-facing technical details remain out of UI unless typing is unrecoverable or tamper suspicion is explicit.
- The cloud clipboard protobuf method and field were removed for v1 and reserved so future protocol changes cannot accidentally reuse those numbers.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Verification Sequencing] Scoped Task 2 guard checks before Task 3 cloud removal**
- **Found during:** Task 2 (Rename data, config, and log paths to TypeDuck-owned locations)
- **Issue:** The strict guard covered Task 3 cloud clipboard removal before Task 3 had executed, which blocked Task 2's intended path/diagnostics verification.
- **Fix:** Temporarily scoped the guard so Task 2 verified path and diagnostics cleanup, then restored full cloud/WebDAV/AI/fcitx checks during Task 3.
- **Files modified:** `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1`
- **Verification:** Final strict guard includes cloud checks and passes after Task 3.
- **Committed in:** `bfd4607` and `bc1c1ee`

---

**Total deviations:** 1 auto-fixed (Rule 3)
**Impact on plan:** Verification sequencing only; final guard coverage matches the plan and no scope was reduced.

## Issues Encountered

- `libIME2` is a git submodule, so TypeDuck debug-log config changes were committed inside the submodule as `8b68411` and the superproject pointer was committed in `bfd4607`.
- The focused CMake builds pass with existing MSVC STL4043 deprecation warnings from dependency/spdlog formatting code.
- An initial stub scan command had bad regex quoting; it was rerun with explicit `rg -e` patterns. The matches are existing FIXME comments and one debug string containing `null`, not new UI/data stubs.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckPrivacySecurityCleanup.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict -ExpectRed DiagnosticsPrivacy` - PASS
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckPrivacySecurityCleanup.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS
- `cmake --build build-vs32 --config Debug --target MoqiLauncher MoqiTextService -- /m:1` - PASS
- `cmake --build build-vs32 --config Debug --target MoqiLauncher ProtoFraming_test -- /m:1` - PASS
- `cmake --build build-vs32 --config Debug --target MoqiLauncher MoqiTextService ProtoFraming_test -- /m:1` - PASS
- `rg` cloud clipboard/frontend surface acceptance scan across `MoqLauncher`, `MoqiTextService`, and `proto` - PASS (`NO_CLOUD_FRONTEND_MATCHES`)

## Known Stubs

None. Stub scan found only pre-existing FIXME comments and a diagnostic `target_window=null` string; neither prevents this plan's goal.

## Threat Flags

None. The plan removed a sensitive clipboard/protocol surface and moved diagnostics to TypeDuck-owned local paths; it did not add new endpoints, auth paths, or file trust boundaries beyond the planned local diagnostics path contract.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Phase 06 Plan 05 can harden IPC ACL/server identity on top of the cleaned TypeDuck diagnostics and removed cloud clipboard frontend surface. No blockers remain for this plan.

## Self-Check: PASSED

- Found `.planning/phases/06-privacy-security-and-scaffold-cleanup/06-04-SUMMARY.md`.
- Found task commits `4cdcfe8`, `bfd4607`, and `bc1c1ee`.
- Found submodule commit `libIME2@8b68411`.

---
*Phase: 06-privacy-security-and-scaffold-cleanup*
*Completed: 2026-06-27*
