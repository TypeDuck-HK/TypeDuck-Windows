---
phase: 06-privacy-security-and-scaffold-cleanup
plan: 05
subsystem: ipc-security
tags: [windows, tsf, named-pipe, acl, protocol-framing, typeduck]
requires:
  - phase: 06-04
    provides: TypeDuck-owned diagnostics paths and removed off-scope frontend surfaces
  - phase: 06-07
    provides: Backend privacy/security cleanup and diagnostics guard coverage
provides:
  - TypeDuck-owned launcher pipe hardening guard coverage
  - Narrower named-pipe DACL allow rights
  - Compatibility-tolerant launcher PID/image sanity checks
  - Verified 1 MiB protocol frame bounds
affects: [phase-06, phase-07, ipc, launcher, text-service]
tech-stack:
  added: []
  patterns:
    - Compatibility-tolerant Windows process inspection with trace-only fallback
    - Explicit pipe DACL read/write/synchronize rights instead of broad all-access allow ACEs
key-files:
  created:
    - .planning/phases/06-privacy-security-and-scaffold-cleanup/06-05-SUMMARY.md
  modified:
    - MoqLauncher/PipeSecurity.cpp
    - MoqLauncher/PipeServer.cpp
    - MoqiTextService/MoqiClient.cpp
    - Tests/TypeDuckProtocol/ProtoFraming_test.cpp
    - scripts/Test-TypeDuckLauncherProtocol.ps1
key-decisions:
  - "Launcher identity checks reject only inspectable obvious mismatches: non-TypeDuck-launcher-shaped executable names or paths outside the known TypeDuck program directory."
  - "Denied or unavailable Windows process inspection is accepted for compatibility and recorded only in trace logs."
  - "Pipe allow ACEs now use explicit file/pipe read-write-synchronize rights while preserving the network deny ACE and app-container compatibility."
patterns-established:
  - "IPC hardening guard: scripts/Test-TypeDuckLauncherProtocol.ps1 checks namespace, ACL rights, PID/image inspection, and rejects exact version/signature/hash checks."
requirements-completed: [SEC-04, SEC-01]
duration: 7 min
completed: 2026-06-27
status: complete
---

# Phase 06 Plan 05: IPC Hardening Summary

**TypeDuck launcher IPC now uses a TypeDuck pipe namespace, narrower pipe ACLs, bounded frame guards, and compatibility-tolerant PID/path sanity checks.**

## Performance

- **Duration:** 7 min
- **Started:** 2026-06-27T11:17:24Z
- **Completed:** 2026-06-27T11:24:30Z
- **Tasks:** 3/3
- **Files modified:** 5

## Accomplishments

- Added static IPC hardening checks for TypeDuck pipe namespace, pipe ACL rights, PID/image inspection, and non-strict identity compatibility.
- Replaced broad named-pipe allow ACEs with explicit `FILE_GENERIC_READ | FILE_GENERIC_WRITE | SYNCHRONIZE` rights.
- Renamed the launcher mutex and hidden window class to TypeDuck-owned identifiers.
- Implemented client-side launcher sanity checks with `GetNamedPipeServerProcessId`, `OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION)`, and `QueryFullProcessImageNameW`.
- Preserved compatibility by accepting denied/unavailable process inspection and logging technical detail only to trace diagnostics.

## Task Commits

1. **Task 1: Add IPC hardening guard assertions** - `d7dc7bd` (test)
2. **Task 2: Rename pipe namespace and narrow pipe ACL rights** - `22c5947` (fix)
3. **Task 3: Implement compatibility-tolerant launcher identity sanity checks** - `81ebafd` (fix)

## Files Created/Modified

- `scripts/Test-TypeDuckLauncherProtocol.ps1` - Adds IPC hardening guard assertions for namespace, ACL masks, PID/image checks, and no exact version/signature/hash enforcement.
- `Tests/TypeDuckProtocol/ProtoFraming_test.cpp` - Adds explicit 1 MiB client/backend frame-cap assertion.
- `MoqLauncher/PipeSecurity.cpp` - Uses explicit pipe-compatible allow rights and removes the process-all-access allow mask.
- `MoqLauncher/PipeServer.cpp` - Uses TypeDuck-owned launcher mutex and window class names.
- `MoqiTextService/MoqiClient.cpp` - Adds compatibility-tolerant TypeDuck launcher PID/image/path sanity checks.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckLauncherProtocol.ps1 -RepoRoot . -Strict` - PASS
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckPrivacySecurityCleanup.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS
- `cmake --build build-vs32 --config Debug --target ProtoFraming_test MoqiLauncher MoqiTextService -- /m:1` - PASS
- `.\build-vs32\Tests\TypeDuckProtocol\Debug\ProtoFraming_test.exe` - PASS, 5 tests

## Decisions Made

- Denied process inspection is not treated as tampering. The client accepts the pipe and records bounded English trace detail.
- Launcher executable identity is shape-based, not version/signature/hash-based: the basename must look like a TypeDuck launcher executable, preserving upgrade/downgrade/future-channel compatibility.
- A known configured TypeDuck program directory is used only as an obvious-mismatch check when the process image path is inspectable.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Task 2 guard depended on Task 3 implementation**
- **Found during:** Task 2
- **Issue:** The shared protocol guard intentionally covered both ACL and client identity hardening, so Task 2 verification could not pass until Task 3's client PID/path check existed.
- **Fix:** Implemented the Task 3 client check before committing Task 2, then staged and committed only Task 2-owned launcher files for the Task 2 commit.
- **Files modified:** `MoqiTextService/MoqiClient.cpp`, `scripts/Test-TypeDuckLauncherProtocol.ps1`
- **Verification:** Protocol guard and Debug launcher/text-service build passed before the Task 2 commit.
- **Committed in:** `22c5947` and `81ebafd`

**2. [Rule 1 - Bug] Guard rejected intended compatibility fallback branches**
- **Found during:** Task 3
- **Issue:** The initial "unconditional true" static assertion was too broad and matched legitimate compatibility fallback returns when Windows denies process inspection.
- **Fix:** Narrowed the guard to reject the old stub shape instead of compatibility-tolerant fallback branches.
- **Files modified:** `scripts/Test-TypeDuckLauncherProtocol.ps1`
- **Verification:** Protocol guard passed and still checks for PID/image inspection plus absence of exact version/signature/hash enforcement.
- **Committed in:** `81ebafd`

**Total deviations:** 2 auto-fixed (1 blocking, 1 bug)
**Impact on plan:** Both fixes preserved the plan intent and the user's compatibility constraint.

## Known Stubs

No goal-blocking stubs were introduced. The scan found pre-existing TODO/FIXME comments and ordinary null/empty initializers in touched legacy files; they do not feed UI rendering and do not block this IPC hardening goal.

## Threat Flags

None - all security-relevant changes were covered by the plan threat model for named-pipe spoofing, DACL hardening, and bounded frames.

## Issues Encountered

None beyond the auto-fixed sequencing and guard-pattern issues documented above.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Ready for Phase 6 Plan 06 aggregate guard coverage and TypeDuck CI/release artifact naming. Phase 7 should still perform release verification without screenshot automation.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/06-privacy-security-and-scaffold-cleanup/06-05-SUMMARY.md`.
- Task commits found: `d7dc7bd`, `22c5947`, `81ebafd`.
- Key modified files exist.
- Plan-level verification commands passed.

---
*Phase: 06-privacy-security-and-scaffold-cleanup*
*Completed: 2026-06-27*
