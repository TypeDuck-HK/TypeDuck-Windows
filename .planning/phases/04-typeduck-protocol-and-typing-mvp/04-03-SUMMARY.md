---
phase: 04-typeduck-protocol-and-typing-mvp
plan: 03
subsystem: tsf-client
tags: [typeduck, tsf, protobuf, candidate-ui, recovery, windows]

requires:
  - phase: 04-typeduck-protocol-and-typing-mvp
    provides: "Plan 04-01 TypeDuck protobuf candidate/page/health/error fields and bounded framing helpers"
  - phase: 04-typeduck-protocol-and-typing-mvp
    provides: "Plan 04-02 launcher degraded/error responses and TypeDuck runtime bridge routing"
provides:
  - "TypeDuck TSF client response mapping for composition, commit, candidates, raw comments, page metadata, health, and errors"
  - "Bounded TSF key-path launcher connection and degraded retry behavior"
  - "Deterministic composition/candidate reset on TypeDuck degraded, sparse failure, malformed async, and pipe failure paths"
  - "Plan 04-04 typing-client runtime proof fixture and static guard coverage"
affects: [04-04, tsf-client, candidate-window, typeduck-runtime, backend-probe]

tech-stack:
  added: []
  patterns:
    - "Prefer TypeDuck raw_lookup_comment as the current candidate comment display payload when present."
    - "Enter a short degraded retry window after launcher/backend/frame failures before attempting another key-path connection."
    - "Suppress composition-terminated backend notification before degraded reset ends the local TSF composition."
    - "Expose TypeDuck page metadata through ITfCandidateListUIElement while keeping backend paging authoritative."

key-files:
  created:
    - scripts/Test-TypeDuckTypingClient.ps1
    - .planning/product/protocol-fixtures/phase-04/typing-client.json
  modified:
    - MoqiTextService/MoqiClient.cpp
    - MoqiTextService/MoqiClient.h
    - MoqiTextService/MoqiTextService.cpp
    - MoqiTextService/MoqiTextService.h
    - MoqiTextService/MoqiCandidateWindow.cpp

key-decisions:
  - "Kept parsing out of the frontend for Phase 4; raw lookup-filter comments are passed to the existing candidate comment position."
  - "Treated reverse lookup and Cangjie as ordinary key events owned by the backend/librime path."
  - "Used a one-second degraded retry window and a 120 ms key-path launcher connect wait to replace the old repeated three-second loop."
  - "Left full runtime proof, including TSF page-string query behavior, to Plan 04-04."

patterns-established:
  - "Use jsonStringToUtf16 for TypeDuck candidate strings so embedded control separators do not rely on null-terminated conversion."
  - "Use markRpcDegraded plus resetTypeDuckDegradedState for TSF-visible recovery after failed TypeDuck responses."
  - "Use suppressNextCompositionTerminatedNotification before degraded reset calls endComposition."

requirements-completed: [ENG-06, PROTO-02, PROTO-03, PROTO-04, TYPE-01, TYPE-02, TYPE-03, TYPE-04, TYPE-05]

duration: 2h 25m
completed: 2026-06-24
status: complete
---

# Phase 4 Plan 03 Summary

**TypeDuck TSF typing client mapping with raw comment preservation, backend-owned key forwarding, and bounded degraded recovery**

## Performance

- **Duration:** ~2h 25m
- **Started:** 2026-06-24T04:50:00+08:00
- **Completed:** 2026-06-24T07:15:28+08:00
- **Tasks:** 3
- **Files modified:** 7

## Accomplishments

- Added the static typing-client guard and runtime proof fixture for Plan 04-04 ordinary key latency, missing launcher, malformed response, backend restart, raw-comment, reverse-lookup, and Cangjie cases.
- Mapped TypeDuck candidate entries, raw lookup comments, candidate page metadata, engine health, and typed errors into the TSF client response path.
- Preserved `raw_lookup_comment` at the existing comment display position without Phase 5 dictionary parsing.
- Replaced the old ten-attempt launcher connect loop with a bounded TypeDuck key-path timeout and degraded retry window.
- Reset visible composition/candidate state deterministically after failed responses, malformed async data, pipe failure, and init/connect failures.
- Wired TypeDuck page metadata into `ITfCandidateListUIElement` page/count/current-page reporting.

## Task Commits

1. **Task 1: Add RED TSF typing client guard** - `344ae5c` (test)
2. **Task 2/3: Map TypeDuck responses and bound degraded recovery** - `1dca50c` (fix)

## Files Created/Modified

- `scripts/Test-TypeDuckTypingClient.ps1` - Static TSF typing client and candidate UI contract guard.
- `.planning/product/protocol-fixtures/phase-04/typing-client.json` - Runtime proof obligations consumed by Plan 04-04.
- `MoqiTextService/MoqiClient.cpp` / `MoqiTextService/MoqiClient.h` - TypeDuck response mapping, length-aware candidate conversion, bounded key-path connect/retry, malformed async handling, and degraded reset routing.
- `MoqiTextService/MoqiTextService.cpp` / `MoqiTextService/MoqiTextService.h` - Candidate page state and degraded reset helper with composition-termination suppression.
- `MoqiTextService/MoqiCandidateWindow.cpp` - Candidate UI page/count/current-page reporting from TypeDuck page metadata.

## Decisions Made

- Kept the existing Moqi protobuf transport and launcher pipe bridge, using the TypeDuck fields added in Plan 04-01 instead of inventing a second frontend/backend channel.
- Left lookup-filter payload parsing to Phase 5; Phase 4 only preserves and displays the raw payload in the current comment position.
- Forwarded reverse lookup and Cangjie key paths through the ordinary key-event protobuf request path; the frontend contains no product-specific reverse lookup or Cangjie logic.
- Treated any failed response as a TSF degraded boundary, even if the response lacks a fully populated TypeDuck error object.

## Deviations from Plan

### Auto-fixed Issues

**1. Review-blocked degraded reset reentrancy**
- **Found during:** Subagent code review
- **Issue:** `resetTypeDuckDegradedState()` could call `endComposition()`, triggering `onCompositionTerminated()` and a nested backend RPC while the pipe was being torn down.
- **Fix:** Reused the existing `suppressNextCompositionTerminatedNotification()` hook before degraded reset ends the local composition.
- **Files modified:** `MoqiTextService/MoqiTextService.cpp`
- **Verification:** Subagent re-review PASS; `cmake --build build-vs32 --config Debug --target MoqiTextService -- /m:1`
- **Committed in:** `1dca50c`

**2. Review-flagged malformed async and raw comment hardening**
- **Found during:** Subagent code review
- **Issue:** Malformed async payloads could close/reset the pipe without entering degraded state, and candidate comment conversion still used null-terminated JSON string reads.
- **Fix:** Made `pollAsyncResponses()` return fatal status, mark malformed async data degraded, stop synchronous calls before using a closed pipe, and convert TypeDuck candidate strings with length-aware JSON string conversion.
- **Files modified:** `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiClient.h`, `scripts/Test-TypeDuckTypingClient.ps1`
- **Verification:** Typing guard, launcher guard, framing test binary, and text-service build passed; subagent re-review PASS.
- **Committed in:** `1dca50c`

**3. Review-flagged candidate page metadata dead state**
- **Found during:** Subagent code review
- **Issue:** Initial page size/count fields were stored but not consumed by TSF candidate UI reporting.
- **Fix:** Added page index state and wired page metadata into `GetCount`, `GetSelection`, `GetString`, `GetPageIndex`, and `GetCurrentPage`.
- **Files modified:** `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiTextService.h`
- **Verification:** Typing guard and Debug `MoqiTextService` build passed; subagent re-review PASS.
- **Committed in:** `1dca50c`

---

**Total deviations:** 3 auto-fixed review findings
**Impact on plan:** All fixes tighten planned TSF typing behavior and recovery semantics. No frontend lookup parsing or new transport scope was added.

## Issues Encountered

- The subagent reviewer initially blocked the implementation on degraded reset reentrancy, then passed the fixed patch.
- Residual risk: candidate-window page reporting assumes TSF queries strings for the current page's global index range because only the current backend page is resident locally. Plan 04-04 must verify this at runtime.

## Validation Commands

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckTypingClient.ps1 -RepoRoot . -Strict`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckLauncherProtocol.ps1 -RepoRoot . -Strict`
- `cmake --build build-vs32 --config Debug --target MoqiTextService -- /m:1`
- `cmake --build build-vs32 --config Debug --target ProtoFraming_test -- /m:1`
- `build-vs32\Tests\TypeDuckProtocol\Debug\ProtoFraming_test.exe`
- `git diff --check` (line-ending warnings only)

## User Setup Required

None.

## Next Phase Readiness

Plan 04-04 can now run the fixture-backed runtime proof for ordinary typing latency, raw comment preservation, malformed response reset, backend restart/reset/reconnect, and backend-owned reverse lookup/Cangjie forwarding. The candidate-page TSF query-range behavior should be part of the VM proof.

## Self-Check

All checks passed.

---
*Phase: 04-typeduck-protocol-and-typing-mvp*
*Completed: 2026-06-24*
