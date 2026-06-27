---
phase: 07-compatibility-and-release-verification
plan: 02
subsystem: protocol-recovery-verification
tags: [cpp, protobuf, powershell, release-verification, protocol-recovery]

requires:
  - phase: 04-typeduck-protocol-and-typing-mvp
    provides: TypeDuck protobuf schema, frame bounds, launcher recovery semantics, and lookup payload preservation.
  - phase: 06-privacy-security-and-scaffold-cleanup
    provides: Redacted diagnostics policy and bounded recovery error/health enums.
provides:
  - ProtocolRecovery_test GoogleTest target for frame/protobuf/lookup/sequence recovery cases.
  - TypeduckBackendProbe protocol-recovery mode with executed timeout, restart/crash, redeploy-failure, and degraded-state evidence.
  - Phase 7 protocol recovery matrix and strict guard bound to executed evidence.
affects: [phase-07-release-verification, final-release-manifest, VER-05]

tech-stack:
  added: [GoogleTest target, PowerShell protocol recovery guard, JSON release evidence]
  patterns: [non-visual recovery probe, redacted evidence rows, matrix-as-index-not-evidence]

key-files:
  created:
    - Tests/TypeDuckProtocol/ProtocolRecovery_test.cpp
    - scripts/Invoke-TypeDuckProtocolRecoveryProbe.ps1
    - scripts/Test-TypeDuckProtocolRecovery.ps1
    - .planning/product/release-fixtures/phase-07/protocol-recovery-cases.json
    - .planning/product/release-fixtures/phase-07/protocol-recovery-results.json
  modified:
    - Tools/TypeduckBackendProbe/main.cpp
    - Tests/TypeDuckProtocol/CMakeLists.txt

key-decisions:
  - "Protocol recovery evidence is non-visual and redacted; probe logs are generated under build-vs32 while committed evidence stays in protocol-recovery-results.json."
  - "The recovery matrix is an index only; live timeout, restart/crash, redeploy failure, and degraded-state proof must come from executed probe results."
  - "Reverse lookup remains conditional and explicitly skipped-with-reason when no live reverse-lookup backend response is available, rather than fabricating unsupported output."

patterns-established:
  - "Use TypeduckBackendProbe --mode protocol-recovery for repeatable live recovery evidence without TSF host apps or screenshot automation."
  - "Use scripts/Test-TypeDuckProtocolRecovery.ps1 -Strict to reject matrix-only coverage, screenshot/raw typed content fields, and routine user-facing technical detail."

requirements-completed: ["VER-05"]

duration: 13 min
completed: 2026-06-27
status: complete
---

# Phase 07 Plan 02: Protocol Recovery Verification Summary

**Repeatable TypeDuck protocol recovery tests and redacted live probe evidence for VER-05 release readiness.**

## Performance

- **Duration:** 13 min
- **Started:** 2026-06-27T12:03:13Z
- **Completed:** 2026-06-27T12:16:46Z
- **Tasks:** 3
- **Files modified:** 7

## Accomplishments

- Added `ProtocolRecovery_test` for valid Cantonese request/response frames, lookup-filter separator preservation, invalid protobuf clearing, bounded frame errors, and stale sequence detection.
- Added `TypeduckBackendProbe --mode protocol-recovery` plus a PowerShell runner that executes backend-timeout, backend-restart-crash, settings-update-redeploy-failure, and bounded-degraded-state cases.
- Added a Phase 7 protocol recovery matrix and strict guard requiring executed live evidence for recovery cases and rejecting screenshot/raw typed content fields.

## Task Commits

1. **Task 1 RED: Add failing protocol recovery probe runner** - `27528cb` (test)
2. **Task 1 GREEN: Add protocol recovery probe mode** - `3ee2d95` (feat)
3. **Task 2 RED: Add failing protocol recovery unit tests** - `bbd9da8` (test)
4. **Task 2 GREEN: Implement sequence helper** - `129f3dc` (feat)
5. **Task 3 RED: Add failing protocol recovery guard** - `bcfc8ef` (test)
6. **Task 3 GREEN: Bind matrix to evidence** - `ef68064` (feat)

**Plan metadata:** pending final docs commit

## Files Created/Modified

- `Tools/TypeduckBackendProbe/main.cpp` - Adds embedded deterministic protocol-recovery mode and redacted JSON evidence writer.
- `Tests/TypeDuckProtocol/ProtocolRecovery_test.cpp` - Adds focused GoogleTest coverage for D-11 frame/protobuf/lookup/sequence cases.
- `Tests/TypeDuckProtocol/CMakeLists.txt` - Wires `ProtocolRecovery_test` with generated protobuf sources.
- `scripts/Invoke-TypeDuckProtocolRecoveryProbe.ps1` - Builds/runs the probe and validates required live recovery result rows.
- `scripts/Test-TypeDuckProtocolRecovery.ps1` - Validates the D-11 matrix, live executed evidence, failure policy, and no forbidden evidence fields.
- `.planning/product/release-fixtures/phase-07/protocol-recovery-cases.json` - Lists all required D-11 recovery cases as an index.
- `.planning/product/release-fixtures/phase-07/protocol-recovery-results.json` - Stores executed recovery evidence produced by the probe.

## Decisions Made

- Probe logs are generated in `build-vs32\TypeDuckProtocolRecovery` so no extra `.planning` artifact outside the plan ownership list is introduced.
- Reverse lookup is explicitly conditional with a skipped-with-reason matrix entry until a live backend response exists.
- Routine technical recovery detail remains in redacted evidence/logs, not routine user-facing UI.

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

None.

## Threat Flags

None.

## Issues Encountered

- Concurrent Phase 7 planning artifacts were already present in the working tree (`07-01-SUMMARY.md` and state/roadmap/requirements updates). They were left unstaged unless required by GSD closeout.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckProtocolRecovery.ps1 -RepoRoot . -Strict` - passed.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Invoke-TypeDuckProtocolRecoveryProbe.ps1 -RepoRoot . -BuildDir build-vs32 -EvidencePath .planning\product\release-fixtures\phase-07\protocol-recovery-results.json -Strict` - passed.
- `cmake --build build-vs32 --config Debug --target ProtocolRecovery_test -- /m:1` - passed.
- `cmake --build build-vs32 --config Debug --target TypeduckBackendProbe -- /m:1` - passed.
- `build-vs32\Tests\TypeDuckProtocol\Debug\ProtocolRecovery_test.exe` - passed, 5 tests.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckProtocolContract.ps1 -RepoRoot . -Strict` - passed.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Protocol recovery evidence is ready for Phase 7 aggregate release verification. Remaining Phase 7 work should consume `protocol-recovery-cases.json` and `protocol-recovery-results.json` without reintroducing screenshot automation.

## Self-Check: PASSED

- Created/modified files exist.
- Task commits are present in git history.
- Required verification commands passed.

---
*Phase: 07-compatibility-and-release-verification*
*Completed: 2026-06-27*
