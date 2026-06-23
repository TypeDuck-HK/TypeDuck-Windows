---
phase: 04-typeduck-protocol-and-typing-mvp
plan: 01
subsystem: protocol
tags: [protobuf, framing, typeduck, cplusplus, cmake, tests]

requires:
  - phase: 04-typeduck-protocol-and-typing-mvp
    provides: "Phase 4 discussion decisions D-01 through D-16 and TypeDuck raw lookup boundary"
provides:
  - "Explicit TypeDuck protobuf protocol semantics over the existing moqi.protocol compatibility package"
  - "Bounded protobuf frame parser and bounded serializer helpers"
  - "Protocol contract guard and golden fixture matrix for PROTO-01 through PROTO-06"
  - "Generated-protobuf CMake path that can use fetched protobuf::protoc or an explicit local protoc"
affects: [04-02, 04-03, 04-04, protocol, launcher, tsf-client, backend-probe]

tech-stack:
  added: []
  patterns:
    - "Keep generated protobuf includes before repo-root includes when compiling generated moqi.pb.cc."
    - "Preserve CandidateEntry.text/comment while adding TypeDuck-specific candidate metadata fields."
    - "Clear protobuf messages on parse failure so malformed payloads do not leak partial state."

key-files:
  created: []
  modified:
    - CMakeLists.txt
    - proto/moqi.proto
    - proto/ProtoFraming.h
    - Tests/TypeDuckProtocol/CMakeLists.txt
    - Tests/TypeDuckProtocol/ProtoFraming_test.cpp
    - .planning/product/protocol-fixtures/phase-04/protocol-contract.json

key-decisions:
  - "Kept package moqi.protocol as the compatibility bridge while adding first-class TypeDuck messages and methods."
  - "Used 1 MiB payload caps for both client and backend frames in Phase 4."
  - "Left lookup-filter payload parsing deferred to Phase 5; Phase 4 preserves raw comments byte-exactly."

patterns-established:
  - "FrameBuffer tracks lastError, violation state, buffered byte count, and explicit max payload size."
  - "CMake-generated protobuf headers must take precedence over checked-in fallback generated headers."
  - "Fetched protobuf 33.5 requires libupb and dynamic MSVC runtime alignment for Debug test targets."

requirements-completed: [PROTO-01, PROTO-02, PROTO-03, PROTO-04, PROTO-05, PROTO-06]

duration: 2h
completed: 2026-06-23
status: complete
---

# Phase 4 Plan 01 Summary

**TypeDuck protobuf semantics and bounded frame parsing over the existing TSF-launcher-backend transport**

## Performance

- **Duration:** ~2h
- **Started:** 2026-06-23T17:20:00Z
- **Completed:** 2026-06-23T19:20:06Z
- **Tasks:** 3
- **Files modified:** 9

## Accomplishments

- Added explicit TypeDuck protocol, candidate page, settings, capability, deploy/reconfigure, health, and error messages without renaming the existing protobuf package.
- Hardened `FrameBuffer` with payload caps, violation/error state, buffered byte introspection, bounded serialization, and parse-failure clearing.
- Verified raw lookup comments preserve vertical-tab, form-feed, and carriage-return separators without Phase 4 dictionary UI parsing.
- Made the protocol test target compile against generated protobuf headers instead of stale checked-in fallback headers.

## Task Commits

1. **Task 1: Add RED protocol and frame contract checks** - `c1580ed` (test)
2. **Task 2: Extend protobuf semantics and bounded framing** - `86887c8` (feat)
3. **Task 3: Regenerate protocol outputs through the normal build path** - `c192ad6` (docs)

## Files Created/Modified

- `proto/moqi.proto` - Adds TypeDuck protocol fields/messages while preserving Moqi compatibility fields.
- `proto/ProtoFraming.h` - Adds bounded frame parsing, bounded serialization, parser cleanup, and frame limits.
- `Tests/TypeDuckProtocol/ProtoFraming_test.cpp` - Verifies oversize rejection, truncated-frame pending behavior, malformed payload cleanup, and raw comment preservation.
- `Tests/TypeDuckProtocol/CMakeLists.txt` - Builds the TypeDuck protocol test against generated protobuf output.
- `CMakeLists.txt` - Allows fetched protobuf to provide `protoc`, enables required libupb, aligns MSVC runtime settings, and mitigates protobuf tool PDB contention.
- `MoqiTextService/CMakeLists.txt`, `MoqLauncher/CMakeLists.txt`, `Tools/TypeduckBackendProbe/CMakeLists.txt` - Prefer generated proto headers over checked-in fallback headers.
- `.planning/product/protocol-fixtures/phase-04/protocol-contract.json` - Records generated fields and 1 MiB frame caps.

## Decisions Made

- Kept `moqi.protocol` and the existing frame transport as the compatibility boundary per D-01.
- Added TypeDuck semantics beside existing Moqi-shaped fields rather than treating old Moqi meanings as product truth per D-02.
- Preserved raw lookup comments in `CandidateEntry.comment` and `CandidateEntry.raw_lookup_comment`, with parsing deferred per D-05/D-06/D-08.
- Chose a conservative 1 MiB cap for both TSF-client and backend frame payloads.

## Deviations from Plan

### Auto-fixed Issues

**1. Generated header precedence**
- **Found during:** Task 2 build verification
- **Issue:** The test and product targets compiled generated `moqi.pb.cc` while including stale checked-in `proto/moqi.pb.h`.
- **Fix:** Moved `${MOQI_GENERATED_PROTO_ROOT}` before `${PROJECT_SOURCE_DIR}` in generated-proto consumers.
- **Files modified:** `Tests/TypeDuckProtocol/CMakeLists.txt`, `MoqiTextService/CMakeLists.txt`, `MoqLauncher/CMakeLists.txt`, `Tools/TypeduckBackendProbe/CMakeLists.txt`
- **Verification:** `ProtoFraming_test` compiled against generated accessors.
- **Committed in:** `86887c8`

**2. Fetched protobuf toolchain support**
- **Found during:** Task 2 build verification
- **Issue:** A clean build could fetch protobuf but could not generate schema output when no system `protoc` existed; enabling fetched `protoc` also required libupb and MSVC runtime/PDB adjustments.
- **Fix:** Enabled fetched `protobuf::protoc` when no explicit/system `protoc` exists, enabled libupb, aligned Debug runtime settings, added `/FS`, and constrained protobuf tool target compile parallelism.
- **Files modified:** `CMakeLists.txt`
- **Verification:** `cmake --build build-vs32 --config Debug --target ProtoFraming_test -- /m:1`
- **Committed in:** `86887c8`

---

**Total deviations:** 2 auto-fixed (build correctness)
**Impact on plan:** Both fixes were required for the planned generated protobuf verification path; no product scope was added.

## Issues Encountered

- `protoc` was not on PATH. Verification used the fetched `build-vs32/_deps/protobuf-build/Debug/protoc.exe` via `-DMOQI_PROTOC_EXECUTABLE=...` after CMake produced it.
- Protobuf `proto3` parse failures can leave unknown bytes on a message object. `parsePayload` now clears the message on parse failure.

## Validation Commands

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckProtocolContract.ps1 -RepoRoot . -Strict`
- `cmake -S . -B build-vs32 -G "Visual Studio 17 2022" -A Win32 -DMOQI_PROTOC_EXECUTABLE="D:\VSProjects\moqi-im-windows\build-vs32\_deps\protobuf-build\Debug\protoc.exe"`
- `cmake --build build-vs32 --config Debug --target ProtoFraming_test -- /m:1`
- `build-vs32\Tests\TypeDuckProtocol\Debug\ProtoFraming_test.exe`

## User Setup Required

None.

## Next Phase Readiness

Plans 04-02 and 04-03 can consume explicit TypeDuck fields and bounded framing helpers. The launcher should instantiate `FrameBuffer` with the named client/backend caps and map malformed/oversized parse failures to TypeDuck health/error responses.

## Self-Check

All checks passed.

---
*Phase: 04-typeduck-protocol-and-typing-mvp*
*Completed: 2026-06-23*
