---
phase: 04-typeduck-protocol-and-typing-mvp
plan: 02
subsystem: launcher
tags: [typeduck, launcher, protobuf, recovery, ipc, windows]

requires:
  - phase: 04-typeduck-protocol-and-typing-mvp
    provides: "Plan 04-01 TypeDuck protobuf health/error fields and bounded ProtoFraming helpers"
provides:
  - "Bounded TypeDuck launcher client/backend frame handling"
  - "Explicit TypeDuck failed ServerResponse payloads for missing backend, spawn failure, timeout, restart, and malformed frames"
  - "First-party TypeDuck zh-HK profile mapping to the transitional runtime bridge"
  - "Launcher recovery fixture for Plan 04-04 runtime proof obligations"
affects: [04-03, 04-04, launcher, backend-probe, typeduck-runtime]

tech-stack:
  added: []
  patterns:
    - "Launcher writes TypeDuck typed error responses before restart/cleanup."
    - "Backend responses must match the pending client sequence before they can clear the timeout."
    - "Launcher normalizes GUID lookup keys with braces and lower-case text."

key-files:
  created:
    - scripts/Test-TypeDuckLauncherProtocol.ps1
    - .planning/product/protocol-fixtures/phase-04/launcher-recovery.json
  modified:
    - MoqLauncher/PipeClient.cpp
    - MoqLauncher/PipeClient.h
    - MoqLauncher/BackendServer.cpp
    - MoqLauncher/BackendServer.h
    - MoqLauncher/PipeServer.cpp
    - MoqLauncher/PipeServer.h
    - proto/ProtoFraming.h
    - backends.json

key-decisions:
  - "Kept the existing named-pipe and backend stdin/stdout framed protobuf transport."
  - "Named the transitional backend manifest entry typeduck-runtime-bridge while documenting that it is not product profile authority."
  - "Seeded the first-party TypeDuck profile mapping before optional backend ime.json compatibility scanning."
  - "Dropped stale backend responses whose seq_num does not match the active pending request."

patterns-established:
  - "Use PipeClient::writeTypeDuckErrorResponse for launcher-originated degraded/failure responses."
  - "Use PipeClient::writeBackendResponse for backend-originated responses so stale replies do not clear active timers."
  - "Use normalizeGuidKey before storing or looking up launcher language-profile backend mappings."

requirements-completed: [ENG-06, PROTO-02, PROTO-05, TYPE-05]

duration: 1h 30m
completed: 2026-06-24
status: complete
---

# Phase 4 Plan 02 Summary

**TypeDuck launcher recovery responses and first-party profile routing over the existing IPC bridge**

## Performance

- **Duration:** ~1h 30m
- **Started:** 2026-06-24T03:20:00+08:00
- **Completed:** 2026-06-24T04:49:10+08:00
- **Tasks:** 3
- **Files modified:** 10

## Accomplishments

- Added a static launcher recovery guard plus a Plan 04-04 runtime fixture for malformed frame, oversized backend stdout, missing backend, timeout, and restart proof cases.
- Bounded launcher client and backend frame parsing with explicit TypeDuck health/error responses instead of silent waits or unbounded buffering.
- Converted backend startup, stdin, timeout, malformed stdout, and read-error paths into deterministic failed `ServerResponse` payloads.
- Mapped the braced TypeDuck zh-HK profile GUID to `typeduck-runtime-bridge` before optional backend `ime.json` scanning.
- Made backend response routing sequence-aware so stale replies cannot clear the active request timeout or confuse the TSF client.

## Task Commits

1. **Task 1: Add RED launcher recovery guard** - `9962b3e` (test)
2. **Task 2/3: Return TypeDuck recovery errors and map TypeDuck runtime bridge** - `c8984dd` (fix)

## Files Created/Modified

- `scripts/Test-TypeDuckLauncherProtocol.ps1` - Static launcher recovery and mapping contract guard.
- `.planning/product/protocol-fixtures/phase-04/launcher-recovery.json` - Runtime proof obligations for Plan 04-04.
- `MoqLauncher/PipeClient.cpp` / `MoqLauncher/PipeClient.h` - Bounded client frame handling, TypeDuck error-response writer, pending sequence tracking, stale response drop.
- `MoqLauncher/BackendServer.cpp` / `MoqLauncher/BackendServer.h` - Spawn failure handling, bounded backend serialization/stdout parsing, degraded restart notifications, no restart-time client close loop.
- `MoqLauncher/PipeServer.cpp` / `MoqLauncher/PipeServer.h` - TypeDuck profile bridge seeding, GUID normalization, backend error fan-out.
- `backends.json` - TypeDuck-owned transitional runtime bridge manifest entry.
- `proto/ProtoFraming.h` - Macro-safe `numeric_limits::max` calls for Windows translation units.

## Decisions Made

- Preserved D-01 transport: TSF client to launcher still uses the same named pipe; launcher to backend still uses framed protobuf over backend stdin/stdout.
- Treated backend `ime.json` as optional compatibility metadata only; the TypeDuck profile mapping comes from the launcher-owned Phase 3 profile GUID.
- Returned bounded failed `ServerResponse` messages from launcher-managed failure paths rather than closing the client first.
- Used sequence-aware backend response delivery to avoid stale/out-of-order backend output completing a newer TSF request.

## Deviations from Plan

### Auto-fixed Issues

**1. Windows `max` macro collision in framing helper**
- **Found during:** Task 2 build verification
- **Issue:** `Windows.h` defines `max`, which broke `std::numeric_limits<...>::max()` in launcher translation units.
- **Fix:** Switched to the macro-safe `(std::numeric_limits<T>::max)()` spelling in `proto/ProtoFraming.h`.
- **Files modified:** `proto/ProtoFraming.h`
- **Verification:** `cmake --build build-vs32 --config Debug --target MoqiLauncher -- /m:1`
- **Committed in:** `c8984dd`

**2. Review-blocked GUID and stale response risks**
- **Found during:** Subagent code review
- **Issue:** First implementation seeded an unbraced TypeDuck profile GUID and allowed any backend response for a client to clear the pending timeout.
- **Fix:** Seeded/normalized braced lower-case GUID keys and routed backend payloads through `writeBackendResponse(response.seq_num(), ...)`.
- **Files modified:** `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeClient.cpp`, `MoqLauncher/PipeClient.h`, `MoqLauncher/BackendServer.cpp`, `scripts/Test-TypeDuckLauncherProtocol.ps1`, `.planning/product/protocol-fixtures/phase-04/launcher-recovery.json`
- **Verification:** Focused subagent re-review reported both blockers resolved; launcher guard and build passed.
- **Committed in:** `c8984dd`

---

**Total deviations:** 2 auto-fixed (build compatibility, review-blocked correctness)
**Impact on plan:** Both fixes were required for the planned launcher recovery behavior; no transport or product-scope expansion.

## Issues Encountered

- The first code-review subagent spawn rejected a full-history fork with an explicit role. It was respawned without the fork and completed the review.
- Existing spdlog/MSVC deprecation warnings remain during `MoqiLauncher` builds; no new build errors remain.

## Validation Commands

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckLauncherProtocol.ps1 -RepoRoot . -Strict`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckInstallerSkeleton.ps1 -RepoRoot . -Strict`
- `cmake --build build-vs32 --config Debug --target MoqiLauncher -- /m:1`
- `cmake --build build-vs32 --config Debug --target ProtoFraming_test -- /m:1`
- `build-vs32\Tests\TypeDuckProtocol\Debug\ProtoFraming_test.exe`

## User Setup Required

None.

## Next Phase Readiness

Plan 04-03 can rely on bounded launcher recovery responses and a TypeDuck-owned runtime bridge name. Plan 04-04 must execute the runtime cases listed in `launcher-recovery.json`, including timeout-before-restart and malformed/oversized frame handling.

## Self-Check

All checks passed.

---
*Phase: 04-typeduck-protocol-and-typing-mvp*
*Completed: 2026-06-24*
