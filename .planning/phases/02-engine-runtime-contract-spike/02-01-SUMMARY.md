---
phase: 02-engine-runtime-contract-spike
plan: 01
subsystem: engine-runtime
tags: [typeduck, librime, rime, powershell, runtime-staging, schema]

requires:
  - phase: 01-identity-and-web-parity-contract
    provides: TypeDuck identity, banned-surface contract, and Web alpha fixture authority
provides:
  - Pinned TypeDuck-HK runtime staging script
  - TypeDuck-HK schema submodule on aap2-alpha
  - Runtime manifest evidence for rime.dll, lookup-filter provenance, schemas, dictionaries, OpenCC assets, and built schemas
  - Phase 2 engine runtime handoff contract
affects: [phase-02, phase-04, phase-05, phase-07, engine-runtime, packaging]

tech-stack:
  added: [PowerShell runtime staging/check scripts, TypeDuck-HK schema submodule]
  patterns: [manifest-backed runtime evidence, generated runtime/cache ignored from git]

key-files:
  created:
    - scripts/Test-TypeDuckRuntimeContract.ps1
    - scripts/Stage-TypeDuckRuntime.ps1
    - .planning/product/engine-runtime-fixtures/phase-02/runtime-manifest.json
    - .planning/product/TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md
  modified:
    - .gitmodules
    - .gitignore
    - third_party/typeduck-schema

key-decisions:
  - "Record lookup-filter evidence as static runtime inclusion: version-info.txt names rime-dictionary-lookup-filter commit 3671814d4e4aeab8d616ceea3c7f6d88e96bba02, and the manifest ties that provenance to the staged rime.dll SHA-256."
  - "Keep generated runtime and archive cache trees ignored; commit only reproducible scripts and manifest evidence."

patterns-established:
  - "PowerShell staging scripts validate external runtime inputs through exact URLs, submodule branch/commit, and SHA-256 evidence."
  - "Phase 2 runtime evidence lives in .planning/product/engine-runtime-fixtures/phase-02/runtime-manifest.json."

requirements-completed: [ENG-01, ENG-02, ENG-03]

duration: 10min
completed: 2026-06-23
status: complete
---

# Phase 02 Plan 01: Stage TypeDuck Runtime Evidence Summary

**Pinned TypeDuck-HK librime and aap2-alpha schema assets now stage into a Windows-shaped Rime runtime with manifest-backed DLL, lookup-filter, dictionary, OpenCC, and built-schema evidence.**

## Performance

- **Duration:** 10 min
- **Started:** 2026-06-23T11:30:25Z
- **Completed:** 2026-06-23T11:40:32Z
- **Tasks:** 3
- **Files modified:** 7

## Accomplishments

- Added `scripts/Test-TypeDuckRuntimeContract.ps1`, which fails on missing schema submodule, missing exact D-06 release URL evidence, or missing staged rime/plugin/schema/dictionary/OpenCC/built-schema evidence.
- Added `third_party/typeduck-schema` as a submodule pinned to `https://github.com/TypeDuck-HK/schema`, branch `aap2-alpha`, commit `b5ab44e187d010fa13c71bd320c07259b224ae5b`.
- Added `scripts/Stage-TypeDuckRuntime.ps1`, which downloads/reuses the exact TypeDuck-HK librime v1.1.3 archive, stages `input_methods\rime\rime.dll`, copies TypeDuck schema assets, runs `rime_deployer.exe`, and writes `runtime-manifest.json`.
- Recorded manifest evidence: 18 schema files, 7 dictionary files, 5 OpenCC assets, 33 built schema artifacts, staged `rime.dll` SHA-256 `4758c34daceb548a2aef54bb2e5a228baca1511efc07c43fbe84c5d35e7a6d4e`, and lookup-filter commit `3671814d4e4aeab8d616ceea3c7f6d88e96bba02`.
- Created `.planning/product/TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md` as the Phase 2 handoff contract and carried forward the later `rime_api` compatibility investigation note.

## Task Commits

1. **Task 1: Add the failing runtime staging contract check** - `2deee7b` (test)
2. **Task 2: Stage the pinned TypeDuck runtime tree** - `62dcd0f` (feat)
3. **Task 3: Record the runtime contract handoff** - `171669c` (docs)

## Files Created/Modified

- `scripts/Test-TypeDuckRuntimeContract.ps1` - Contract validator for schema submodule, release URL, lookup-filter, and staged runtime evidence.
- `scripts/Stage-TypeDuckRuntime.ps1` - Repeatable staging command for the pinned TypeDuck-HK runtime tree and manifest.
- `.planning/product/engine-runtime-fixtures/phase-02/runtime-manifest.json` - Machine-readable evidence for runtime sources, hashes, deployer output, and staged assets.
- `.planning/product/TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md` - Handoff contract for later adapter, typing, lookup-filter, packaging, and UI phases.
- `.gitmodules` and `third_party/typeduck-schema` - TypeDuck-HK schema submodule on `aap2-alpha`.
- `.gitignore` - Ignores generated `runtime/typeduck-phase02/` and `.cache/typeduck-phase02/` outputs.

## Decisions Made

- The TypeDuck-HK release archive contains `version-info.txt` naming `rime-dictionary-lookup-filter` at commit `3671814d4e4aeab8d616ceea3c7f6d88e96bba02`, but no standalone plugin DLL. The manifest records lookup-filter provenance against the staged `rime.dll` artifact and its SHA-256.
- The staged runtime remains developer evidence only. No installer, TSF registration, protocol, or UI changes were made.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Ignored generated staging/cache outputs**
- **Found during:** Task 2
- **Issue:** Running the staging script creates local cache and runtime trees that should not remain as untracked generated files.
- **Fix:** Added `.cache/typeduck-phase02/` and `runtime/typeduck-phase02/` to `.gitignore`.
- **Files modified:** `.gitignore`
- **Verification:** `git status --short --ignored .cache runtime` shows both paths ignored.
- **Committed in:** `62dcd0f`

**Total deviations:** 1 auto-fixed (Rule 3)
**Impact on plan:** Generated outputs stay reproducible from committed scripts and manifest evidence without polluting git status.

## Issues Encountered

- The release archive layout differs from a separate-plugin assumption. It includes `dist\lib\rime.dll` and `version-info.txt`, with lookup-filter source commit evidence in `version-info.txt`. The staging script records that actual layout instead of inventing a standalone plugin artifact.
- PowerShell timestamp formatting initially followed local culture; `Stage-TypeDuckRuntime.ps1` now writes invariant UTC ISO-8601 timestamps.

## Known Stubs

None. Stub-pattern scan found only internal empty-string initializers in PowerShell scripts, not UI placeholders or data-source stubs.

## Threat Flags

None. The GitHub release download, schema submodule, and local staging writes are the trust boundaries already covered by the plan threat model and are mitigated through exact URL/branch pins plus SHA-256 manifest evidence.

## Verification

- `powershell -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckRuntimeContract.ps1 -RepoRoot . -StageRoot .\runtime\typeduck-phase02 -ManifestPath .planning\product\engine-runtime-fixtures\phase-02\runtime-manifest.json` - passed.
- `powershell -NoProfile -Command '$p=".planning/product/TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md"; ...'` - passed required token check for D-01 through D-08, D-14 through D-16, ENG-01 through ENG-03, the release archive, `aap2-alpha`, and `TYPEDUCK-BANNED-SURFACES`.

## User Setup Required

None for the committed repository state. Re-running staging requires either `7z.exe` or a Windows `tar.exe` capable of extracting `.7z`; this environment used `C:\Program Files\7-Zip\7z.exe`.

## Next Phase Readiness

Phase 2 Plan 02 can consume `runtime\typeduck-phase02` after running `scripts\Stage-TypeDuckRuntime.ps1`, then prove Cantonese composition, candidates, Jyutping, and commits across the compatibility adapter boundary. If the adapter hits `rime_api` incompatibility, use the compatibility investigation note in `.planning/product/TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md` before patching.

## Self-Check: PASSED

- Verified created/modified files exist.
- Verified task commits exist: `2deee7b`, `62dcd0f`, `171669c`.
- Re-ran the runtime contract validator successfully.

---
*Phase: 02-engine-runtime-contract-spike*
*Completed: 2026-06-23*
