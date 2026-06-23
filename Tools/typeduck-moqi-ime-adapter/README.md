# TypeDuck moqi-ime Adapter Handoff

This directory records the internal compatibility adapter work for Phase 02 Plan 02. It uses `moqi-ime` only as a backend process boundary to prove the staged TypeDuck runtime from Plan 02-01; it is not a product protocol, TSF frontend rewrite, installer change, or visible UI contract.

## Boundary

- D-01: the proof keeps the existing `moqi-ime` framed protobuf stdio process shape so the Windows repo can exercise a backend without changing TSF host-process behavior.
- D-02: `scripts/Invoke-TypeDuckEngineProof.ps1` prepares an isolated adapter work directory and overlays the staged TypeDuck runtime into the adapter's expected `input_methods/rime` layout.
- D-03: compatibility deltas are minimal patch files under `Tools/typeduck-moqi-ime-adapter/patches`.
- D-04: the proof does not move librime into the Windows TSF DLL or replace the frontend with direct in-process librime calls.
- D-14 and D-15: proof evidence must come from TypeDuck Cantonese schema output and rejects legacy Moqi fallback evidence.
- D-16: final TypeDuck protocol replacement, product naming, installer/runtime cleanup, candidate UI, and settings UI remain outside this Phase 2 proof unless they block engine evidence.

## Adapter Setup

The proof runner expects a sibling checkout of `https://github.com/gaboolic/moqi-ime` at `..\moqi-ime`, or an equivalent `-MoqiImeRoot`.

The verified adapter source was:

- repository: `https://github.com/gaboolic/moqi-ime`
- commit: `88f532714e91390994d021deb3db8c11a566d0fb`
- build mode: Windows x86 backend server, because the staged TypeDuck runtime for this phase provides an x86 `rime.dll`

Run the proof with:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Invoke-TypeDuckEngineProof.ps1 -RepoRoot . -StageRoot .\runtime\typeduck-phase02 -MoqiImeRoot ..\moqi-ime -OutputPath .planning\product\engine-runtime-fixtures\phase-02\typing-proof.json
```

The runner applies patch files in lexical order and treats known post-patch markers as already applied, so repeated runs can reuse the same sibling checkout.

## Patch Files

- `0001-typeduck-librime-1.1.3-candidate-abi.patch` adds `Quality float64` to the adapter's Go mirror of `RimeCandidate`.
- `0002-typeduck-librime-1.1.3-candidate-abi-x86-padding.patch` adds the x86 padding needed for the Windows x86 proof build.

These patches are intentionally adapter-local. They do not modify TypeDuck-HK librime, do not change the committed staged runtime manifest, and do not define a final Windows IME backend protocol.

## Rime API Compatibility Findings

TypeDuck librime uses its own version numbering, so the investigation compared API shape and commit history rather than assuming the TypeDuck `v1.1.3` label matches an official librime release.

Primary sources checked:

- upstream librime: `https://github.com/rime/librime`
- TypeDuck-HK librime fork: `https://github.com/TypeDuck-HK/librime`

Findings:

- The TypeDuck-HK fork's `src/rime_api.h` at its `v1.1.3` tag / release commit includes `RimeContext::select_labels`, matching the official upstream API family after upstream commit `b992f35`.
- The TypeDuck-HK fork also adds `double quality` to `RimeCandidate`. That field is not present in official upstream `src/rime_api.h` at current upstream HEAD.
- The relevant TypeDuck-HK fork commit for candidate weights is `9315986` (`Print Candidate Weights in API Console for Easy Debugging`).
- The current `moqi-ime` Go adapter mirror lacked the TypeDuck-HK `RimeCandidate::quality` field, so candidate materialization could stall or misread memory once real TypeDuck candidates were available.

## Evidence

The proof output is `.planning/product/engine-runtime-fixtures/phase-02/typing-proof.json`, with raw framed request/response evidence in `.planning/product/engine-runtime-fixtures/phase-02/backend-stdio-frames.ndjson`.

The controlled inputs are `nei` and `housam`. Passing evidence includes:

- composition state;
- candidate page state;
- TypeDuck Cantonese candidate strings such as `你`, `尼`, `呢`, `好心`, and `好心你`;
- Jyutping/comment payloads such as `nei5`, `nei4`, `hou2sam1`, and `hou2sam1nei5`;
- commit output after selecting the first candidate.

If these files contain only legacy fallback output, the proof is invalid for ENG-04.
