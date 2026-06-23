---
phase: 02-engine-runtime-contract-spike
plan: 02
subsystem: engine-runtime
tags: [typeduck, librime, rime, moqi-ime, backend-proof, protobuf]

requires:
  - phase: 02-engine-runtime-contract-spike
    plan: 01
    provides: staged TypeDuck-HK runtime and runtime-manifest evidence
provides:
  - Win32 framed protobuf backend proof probe
  - TypeDuck Cantonese typing proof through the moqi-ime adapter boundary
  - Repo-local TypeDuck-HK librime API compatibility patches
  - Adapter patch handoff notes
affects: [phase-02, engine-runtime, adapter-proof, protocol-planning]

tech-stack:
  added: [C++20 probe target, PowerShell proof runner, Go adapter build invocation]
  patterns: [framed protobuf stdio proof, repo-local adapter patch stack, manifest-backed runtime evidence]

key-files:
  created:
    - Tools/TypeduckBackendProbe/CMakeLists.txt
    - Tools/TypeduckBackendProbe/main.cpp
    - Tools/typeduck-moqi-ime-adapter/README.md
    - Tools/typeduck-moqi-ime-adapter/patches/0001-typeduck-librime-1.1.3-candidate-abi.patch
    - Tools/typeduck-moqi-ime-adapter/patches/0002-typeduck-librime-1.1.3-candidate-abi-x86-padding.patch
    - .planning/product/engine-runtime-fixtures/phase-02/typing-proof.json
    - .planning/product/engine-runtime-fixtures/phase-02/backend-stdio-frames.ndjson
  modified:
    - CMakeLists.txt
    - scripts/Invoke-TypeDuckEngineProof.ps1

key-decisions:
  - "Prove ENG-04 through the existing moqi-ime framed protobuf process boundary instead of direct in-process librime."
  - "Treat TypeDuck-HK librime version labels as fork-specific; compare Rime API shape and commit history before patching."
  - "Patch only the adapter mirror of RimeCandidate for the TypeDuck-HK ABI delta, keeping the patch stack repo-local."

requirements-completed: [ENG-04]

duration: 55min
completed: 2026-06-23
status: complete
---

# Phase 02 Plan 02: Adapter Runtime Proof Summary

**TypeDuck Cantonese composition, candidates, Jyutping/comment payloads, page state, and commit output now pass through the minimal moqi-ime backend adapter boundary using the staged Plan 02-01 runtime.**

## Performance

- **Duration:** 55 min
- **Completed:** 2026-06-23T12:36:41Z
- **Tasks:** 3
- **Files modified:** 9

## Accomplishments

- Added `Tools/TypeduckBackendProbe`, a Win32 console target that launches the backend adapter, speaks the existing `proto/moqi.proto` framed stdio contract, and records request/response frames.
- Extended `scripts/Invoke-TypeDuckEngineProof.ps1` from RED checks into the full proof runner: it validates the staged runtime manifest, applies adapter patches, builds the x86 adapter server, prepares an isolated TypeDuck runtime workdir, runs controlled inputs, and validates output.
- Generated `.planning/product/engine-runtime-fixtures/phase-02/typing-proof.json` and `backend-stdio-frames.ndjson`.
- Recorded proof evidence for `nei` and `housam`, including candidates `你`, `尼`, `呢`, `好心`, and `好心你`, Jyutping/comment tokens including `nei5`, `nei4`, `hou2sam1`, and `hou2sam1nei5`, candidate page state, and commit output.
- Added the adapter handoff README and repo-local patch stack under `Tools/typeduck-moqi-ime-adapter/`.

## Task Commits

1. **Task 1: Add failing adapter behavior proof checks** - `1f48fab` (test)
2. **Task 2: Implement the framed backend probe and proof runner** - `bda7a59` (feat)
3. **Task 3: Document adapter deltas and boundary compliance** - `54e3a90` (docs)

## Adapter Setup Used

- Sibling adapter checkout: `D:\VSProjects\moqi-ime`
- Canonical source: `https://github.com/gaboolic/moqi-ime`
- Adapter commit: `88f532714e91390994d021deb3db8c11a566d0fb`
- Adapter server build: Windows x86 Go executable, run from the isolated ignored workdir under `runtime\typeduck-phase02\adapter-work`
- Runtime source: Plan 02-01 staged TypeDuck runtime manifest SHA-256 `71ea765798dd204afc6e6b690325c8d2879ae0acfb24ab5bc02571fa06d63c01`

## Rime API Compatibility Investigation

Triggered by TypeDuck-HK librime candidate ABI incompatibility.

Primary sources used:

- Upstream librime repository: `https://github.com/rime/librime`
- TypeDuck-HK fork repository: `https://github.com/TypeDuck-HK/librime`

Findings:

- TypeDuck-HK librime uses fork-specific version numbering, so the patch was based on API shape and commit history, not an assumed official librime version match.
- TypeDuck-HK `src/rime_api.h` at its `v1.1.3` tag/release commit contains `RimeContext::select_labels`, aligning with the official upstream API family after upstream commit `b992f35`.
- TypeDuck-HK also adds `double quality` to `RimeCandidate`; official upstream `src/rime_api.h` does not contain that field at current upstream HEAD.
- TypeDuck-HK commit `9315986` adds candidate weights to the API console path and is the relevant fork-side API change.
- The `moqi-ime` Go adapter mirror lacked that `RimeCandidate` field, and the x86 Windows proof also needed trailing struct padding. The patch stack adapts only the adapter mirror.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Added checked-in protobuf fallback for local proof builds**
- **Found during:** Task 2
- **Issue:** The local environment had no `protoc` available, while checked-in `proto/moqi.pb.*` already existed.
- **Fix:** `CMakeLists.txt` now falls back to checked-in generated protobuf files when `protoc` is absent.
- **Files modified:** `CMakeLists.txt`
- **Commit:** `bda7a59`

**2. [Rule 3 - Blocking] Patched the adapter's TypeDuck-HK RimeCandidate ABI mirror**
- **Found during:** Task 2
- **Issue:** Candidate materialization stalled once real TypeDuck candidates were available because the adapter mirrored upstream `RimeCandidate` without the TypeDuck-HK `quality` field.
- **Fix:** Added repo-local adapter patches for `Quality float64` and x86 padding.
- **Files modified:** `Tools/typeduck-moqi-ime-adapter/patches/*`
- **Commit:** `bda7a59`

**3. [Rule 3 - Blocking] Used the current desktop TypeDuck Web schema build for proof input**
- **Found during:** Task 2
- **Issue:** The staged release runtime lacked the desktop `jyut6ping3.prism.bin` in the adapter user build location, producing composition without candidates.
- **Fix:** The proof runner overlays the current desktop schema checkout from `I:\GitHub\TypeDuck-Web\schema` and copies its `build` artifacts into the isolated adapter user data path. Per user clarification, dirty removed mobile/platform files are not proof inputs, and `common.custom.yaml` is treated as a generated frontend template.
- **Files modified:** `scripts/Invoke-TypeDuckEngineProof.ps1`
- **Commit:** `bda7a59`

**4. [User-directed adjustment] Restored literal CJK expected candidates and used pwsh**
- **Found during:** Task 2
- **Issue:** An earlier runner edit had changed expected candidates into UTF-8/codepoint workaround values.
- **Fix:** Restored literal expected candidate strings in `scripts/Invoke-TypeDuckEngineProof.ps1`; subsequent commands used `pwsh`.
- **Files modified:** `scripts/Invoke-TypeDuckEngineProof.ps1`
- **Commit:** `bda7a59`

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Invoke-TypeDuckEngineProof.ps1 -RepoRoot . -StageRoot .\runtime\typeduck-phase02 -MoqiImeRoot ..\moqi-ime -OutputPath .planning\product\engine-runtime-fixtures\phase-02\typing-proof.json -CheckOnly` - passed after proof generation.
- `pwsh -NoProfile -ExecutionPolicy Bypass -Command '& { ... scripts\build.ps1 ... scripts\Invoke-TypeDuckEngineProof.ps1 ... }'` - passed. Win32 Release full solution built, x64 Release `MoqiTextService` built, and the proof runner regenerated valid evidence.
- `pwsh -NoProfile -Command '& { $doc="tools/typeduck-moqi-ime-adapter/README.md"; ... }'` - passed required adapter README token and proof JSON checks.
- `pwsh -NoProfile -Command '& { $p=".planning/product/engine-runtime-fixtures/phase-02/typing-proof.json"; ... }'` - passed proof structure check.

Warnings observed during build were existing CMake deprecation/dev warnings plus the intentional fallback warning for missing local `protoc`.

## Known Stubs

None. Stub scan across the created/modified proof runner, probe, and README found no TODO/FIXME/placeholder strings or UI/data stubs.

## Threat Flags

None. The new process execution, runtime file access, and external adapter trust boundary are the planned proof surfaces, constrained to an ignored local workdir and manifest-backed runtime inputs.

## Blockers

None. ENG-04 is complete for the Phase 2 spike scope.

## Self-Check: PASSED

- Verified created files exist: probe target, proof runner updates, adapter README/patches, `typing-proof.json`, and `backend-stdio-frames.ndjson`.
- Verified task commits exist: `1f48fab`, `bda7a59`, `54e3a90`.
- Verified final proof JSON parses and contains runtime manifest plus controlled inputs.

---
*Phase: 02-engine-runtime-contract-spike*
*Completed: 2026-06-23*
