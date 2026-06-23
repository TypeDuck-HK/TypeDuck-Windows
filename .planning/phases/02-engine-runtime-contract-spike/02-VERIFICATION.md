---
phase: 02-engine-runtime-contract-spike
verified: 2026-06-23T13:19:49Z
status: passed
score: 8/8 must-haves verified
behavior_unverified: 0
overrides_applied: 0
---

# Phase 2: Engine Runtime Contract Spike Verification Report

**Phase Goal:** Developer can prove that TypeDuck-HK librime, lookup-filter, schemas, dictionaries, and built assets produce the candidate and dictionary data TypeDuck Windows needs.  
**Verified:** 2026-06-23T13:19:49Z  
**Status:** passed  
**Re-verification:** No - previous MVP-shaped verification was removed; this is a standard goal-backward verification.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | Developer can build or package the pinned TypeDuck-HK librime fork and `rime-dictionary-lookup-filter` plugin for the Windows runtime target. | VERIFIED | `runtime-manifest.json` records the exact D-06 release URL, archive SHA-256, staged `input_methods\rime\rime.dll`, DLL SHA-256, lookup-filter source URL, and lookup-filter commit `3671814d4e4aeab8d616ceea3c7f6d88e96bba02`. `scripts\Test-TypeDuckRuntimeContract.ps1` passed. |
| 2 | Installed or staged runtime assets include TypeDuck schemas, dictionaries, OpenCC assets, and built schema artifacts required for Web alpha parity. | VERIFIED | Manifest records TypeDuck schema source `https://github.com/TypeDuck-HK/schema`, branch `aap2-alpha`, commit `b5ab44e187d010fa13c71bd320c07259b224ae5b`, plus 18 schema files, 7 dictionaries, 5 OpenCC assets, 33 built schema artifacts, and deployer exit code 0. |
| 3 | Developer can run a minimal engine host or spike executable that produces Cantonese composition, candidate pages, Jyutping, and commit output from TypeDuck schema input. | VERIFIED | `Tools/TypeduckBackendProbe/main.cpp` is substantive and wired into CMake. `typing-proof.json` records `nei` -> composition `nei`, candidates `你|尼|呢|妮|彌|妳`, commit `你`; and `housam` -> composition `hou sam`, candidates `好心|好心你|好|號|豪|毫`, commit `好心`. `scripts\Invoke-TypeDuckEngineProof.ps1 -CheckOnly` passed. |
| 4 | Developer can capture lookup-filter-backed dictionary data for candidates, including headword, Jyutping, part-of-speech, English meaning, reading notes, and enabled-language meanings when available. | VERIFIED | `lookup-filter-validation.json` status is `passed`, with 12 raw comments, 12 dictionary records, 32 CSV rows, and failed checks 0. Evidence includes part-of-speech values such as `oth`, `n`, `adj|adv`, English meanings such as `you (singular)` and `kind; come on`, and enabled-language meanings. |
| 5 | Raw lookup-filter evidence preserves exact D-10 column order and D-11/D-12 separator/CSV semantics. | VERIFIED | `lookup-filter-csv-rows.csv` begins with the exact D-10 header. `lookup-filter-raw-comments.ndjson` preserves raw comments, escaped `\f`/`\r` notation, UTF-8 hex, code points, control counts, and SHA-256. Validator self-tests cover optional `\v`, quoted fields, doubled quotes, and empty fields. |
| 6 | The Rime API compatibility investigation was performed and recorded from primary sources when adapter compatibility required it. | VERIFIED | `Tools/typeduck-moqi-ime-adapter/README.md` records primary-source checks against TypeDuck-HK and upstream librime `src/rime_api.h`, including `select_labels`, the TypeDuck-HK `RimeCandidate::quality` ABI delta, upstream absence of that field, and TypeDuck-HK commit `9315986`. Patch files add only `Quality float64` and x86 padding to the adapter mirror. |
| 7 | The proof rejects legacy Moqi fallback behavior as product evidence and keeps moqi-ime only as an internal compatibility adapter. | VERIFIED | `typing-proof.json` has `legacy_fallback_rejected: true`; `scripts\Invoke-TypeDuckEngineProof.ps1` throws on banned legacy evidence; the adapter README states the proof is not product protocol, TSF frontend rewrite, installer change, or visible UI contract. |
| 8 | Phase boundaries were respected: no final UI, protocol replacement, installer registration, or broad scaffold cleanup was introduced. | VERIFIED | Modified Phase 2 files are staging/proof scripts, probe target, adapter patches/docs, CMake probe wiring, and planning evidence. Boundary text in the runtime and lookup contracts explicitly defers final protocol/UI/installer/product naming work to later phases. Grep found no TSF registration, installer, settings dialog, native candidate UI, or new final TypeDuck protocol implementation in Phase 2-owned files. |

**Score:** 8/8 truths verified, 0 present-but-behavior-unverified.

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `third_party/typeduck-schema` | TypeDuck schema submodule on `aap2-alpha` | VERIFIED | `.gitmodules` URL is `https://github.com/TypeDuck-HK/schema`; branch is `aap2-alpha`; current submodule HEAD is `b5ab44e187d010fa13c71bd320c07259b224ae5b`. |
| `scripts/Stage-TypeDuckRuntime.ps1` | Repeatable runtime staging command | VERIFIED | 317 lines; pins the D-06 archive URL, schema submodule path, lookup-filter source, `rime_deployer.exe`, and manifest generation. |
| `scripts/Test-TypeDuckRuntimeContract.ps1` | Runtime contract validator | VERIFIED | 161 lines; command passed with `TypeDuck runtime contract passed.` |
| `.planning/product/engine-runtime-fixtures/phase-02/runtime-manifest.json` | Runtime source and asset evidence | VERIFIED | Parses as JSON and contains pinned sources, hashes, schema/dictionary/OpenCC/build lists, and deployer evidence. |
| `Tools/TypeduckBackendProbe/main.cpp` | Framed backend proof host | VERIFIED | 586 lines; uses `proto/ProtoFraming.h`, `ClientRequest`, `ServerResponse`, `METHOD_ON_KEY_DOWN`, bounded max-frame and timeout values, and records candidate entries. |
| `scripts/Invoke-TypeDuckEngineProof.ps1` | End-to-end proof runner/checker | VERIFIED | 433 lines; validates runtime, applies adapter patches, uses controlled `nei`/`housam` inputs, rejects legacy fallback evidence, and `-CheckOnly` passed. |
| `.planning/product/engine-runtime-fixtures/phase-02/typing-proof.json` | Cantonese typing proof | VERIFIED | Parses as JSON and records runtime manifest hash, adapter commit/server hash, frame log path, candidate pages, comments, and commit output. |
| `Tools/typeduck-moqi-ime-adapter/README.md` and patches | Compatibility delta handoff | VERIFIED | README records D-01 through D-04 and D-14 through D-16 boundaries; patches cover only TypeDuck-HK `RimeCandidate` ABI mirror deltas. |
| `scripts/Test-TypeDuckLookupPayload.ps1` | Raw lookup payload validator | VERIFIED | 422 lines; validates D-10 header, D-11 separators, D-12 CSV quoting/empty fields, raw/proof matching, and dictionary-backed fields. |
| `lookup-filter-raw-comments.ndjson` | Raw comment evidence | VERIFIED | 12 records sourced from `typing-proof.json`; records raw comment, escaped notation, byte/code-point metadata, control counts, payload mode, dictionary row count, and hash. |
| `lookup-filter-csv-rows.csv` | D-10 CSV row evidence | VERIFIED | Header matches the exact D-10 columns; 32 parsed dictionary rows. |
| `lookup-filter-validation.json` | Validator result | VERIFIED | Status `passed`; summary records 12 raw comments, 12 dictionary records, 32 CSV rows, 32 extracted rows, failed checks 0. |
| `.planning/product/TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md` | Runtime handoff contract | VERIFIED | Cites ENG-01 through ENG-05 and D-01 through D-16, links raw lookup contract, and preserves Phase 2 boundaries. |
| `.planning/product/TYPEDUCK-LOOKUP-FILTER-RAW-CONTRACT.md` | Raw lookup contract | VERIFIED | Cites ENG-05 and D-09 through D-13, lists exact D-10 header, separator semantics, CSV rules, evidence files, and source audit. |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `scripts/Stage-TypeDuckRuntime.ps1` | TypeDuck-HK release archive | Pinned URL constant | WIRED | Manual check found `rime-TypeDuck-v1.1.3-Windows-msvc-x86.7z` and exact GitHub URL at script lines 35-36. The helper's regex check produced a false negative because of escaped pattern handling. |
| `scripts/Stage-TypeDuckRuntime.ps1` | `third_party/typeduck-schema` | Copies schema/dictionary/OpenCC assets | WIRED | Automated key-link helper verified `typeduck-schema`; manifest confirms copied assets. |
| `.planning/product/TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md` | `.planning/product/TYPEDUCK-BANNED-SURFACES.md` | Legacy surface boundary | WIRED | Automated key-link helper verified the reference. |
| `scripts/Invoke-TypeDuckEngineProof.ps1` | `scripts/Stage-TypeDuckRuntime.ps1` | Requires staged runtime | WIRED | Automated key-link helper verified the link; script asserts staged `rime.dll` and schema files. |
| `Tools/TypeduckBackendProbe/main.cpp` | `proto/moqi.proto` | Existing protobuf proof boundary | WIRED | Automated key-link helper verified `METHOD_ON_KEY_DOWN` and `candidate_entries`; CMake links generated or checked-in protobuf. |
| `Tools/typeduck-moqi-ime-adapter/README.md` | `.planning/product/TYPEDUCK-BANNED-SURFACES.md` | Reject legacy Moqi product evidence | WIRED | Automated key-link helper verified the link. |
| `scripts/Test-TypeDuckLookupPayload.ps1` | `typing-proof.json` | Reads raw candidate comments from proof | WIRED | Automated key-link helper verified `typing-proof`; validator cross-checks raw records against proof comments exactly. |
| `TYPEDUCK-LOOKUP-FILTER-RAW-CONTRACT.md` | Web alpha fixture contract | Source-backed dictionary expectations | WIRED | Automated key-link helper verified `WEB-ALPHA-FIXTURES`. |
| `TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md` | `TYPEDUCK-LOOKUP-FILTER-RAW-CONTRACT.md` | ENG-05 handoff | WIRED | Automated key-link helper verified `LOOKUP-FILTER-RAW`. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
|----------|---------------|--------|--------------------|--------|
| `runtime-manifest.json` | Runtime/source asset lists | `Stage-TypeDuckRuntime.ps1` staging from pinned release and `third_party/typeduck-schema` | Yes | FLOWING |
| `typing-proof.json` | `inputs[].candidates`, page, composition, commit | `TypeduckBackendProbe` through moqi-ime framed stdio adapter and staged TypeDuck runtime | Yes | FLOWING |
| `lookup-filter-raw-comments.ndjson` | raw candidate comments | Extracted from `typing-proof.json` candidate comments | Yes | FLOWING |
| `lookup-filter-csv-rows.csv` | D-10 dictionary rows | Parsed from raw comments when payload mode is dictionary rows | Yes | FLOWING |
| `lookup-filter-validation.json` | validation checks and counts | `Test-TypeDuckLookupPayload.ps1` reads proof, raw NDJSON, and CSV evidence | Yes | FLOWING |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| Runtime contract validates pinned release, schema submodule, lookup-filter provenance, assets, and built schemas | `powershell -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckRuntimeContract.ps1 -RepoRoot . -StageRoot .\runtime\typeduck-phase02 -ManifestPath .planning\product\engine-runtime-fixtures\phase-02\runtime-manifest.json` | `TypeDuck runtime contract passed.` | PASS |
| Existing engine proof is valid for controlled TypeDuck inputs | `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Invoke-TypeDuckEngineProof.ps1 -RepoRoot . -StageRoot .\runtime\typeduck-phase02 -MoqiImeRoot ..\moqi-ime -OutputPath .planning\product\engine-runtime-fixtures\phase-02\typing-proof.json -CheckOnly` | `OK: existing TypeDuck typing proof is valid.` | PASS |
| Raw lookup-filter payload evidence validates D-10/D-11/D-12 and dictionary-backed fields | `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckLookupPayload.ps1 -TypingProofPath .planning\product\engine-runtime-fixtures\phase-02\typing-proof.json -RawCommentsPath .planning\product\engine-runtime-fixtures\phase-02\lookup-filter-raw-comments.ndjson -CsvRowsPath .planning\product\engine-runtime-fixtures\phase-02\lookup-filter-csv-rows.csv -ValidationPath .planning\product\engine-runtime-fixtures\phase-02\lookup-filter-validation.json` | `OK: TypeDuck lookup payload validation passed.` | PASS |
| Probe declarations | `rg -n "probe-[^\s]+\.sh|scripts/.*/tests/probe-.*\.sh" .planning/phases/02-engine-runtime-contract-spike scripts Tools` | No shell probes declared | SKIPPED |

### Probe Execution

| Probe | Command | Result | Status |
|-------|---------|--------|--------|
| Conventional `probe-*.sh` scripts | `rg -n "probe-[^\s]+\.sh|scripts/.*/tests/probe-.*\.sh" ...` | No phase-declared or conventional shell probes found | SKIPPED |

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|-------------|-------------|--------|----------|
| ENG-01 | 02-01, 02-02 | Package pinned TypeDuck-HK librime fork as engine dependency | SATISFIED | Runtime manifest pins D-06 release URL/archive hash and staged `rime.dll`; runtime validator passed; typing proof exercised the staged runtime. |
| ENG-02 | 02-01, 02-03 | Package `TypeDuck-HK/rime-dictionary-lookup-filter` plugin with engine | SATISFIED | Manifest records lookup-filter source and commit from release `version-info.txt`, tied to staged `rime.dll`; lookup-filter output validator proves emitted dictionary payloads. |
| ENG-03 | 02-01 | Include TypeDuck schemas, dictionaries, OpenCC assets, and built schema assets | SATISFIED | Manifest records schema URL/branch/commit, 18 schema files, 7 dictionaries, 5 OpenCC assets, 33 built schema artifacts, and deployer exit 0. |
| ENG-04 | 02-02 | Produce Cantonese composition, candidate pages, Jyutping, and commit output | SATISFIED | `typing-proof.json` records `nei` and `housam` compositions, page state, candidate lists, Jyutping/comment payloads, and commits; proof `-CheckOnly` passed. |
| ENG-05 | 02-03 | Produce lookup-filter-backed dictionary data for candidates | SATISFIED | Raw comments and CSV rows contain headword, Jyutping, part-of-speech, English meaning, reading/register notes, and enabled-language meanings; validator status `passed`. |

No additional Phase 2 requirement IDs were found in `.planning/REQUIREMENTS.md` traceability beyond ENG-01 through ENG-05.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| `Tools/TypeduckBackendProbe/main.cpp` | 63, 68 | `return {}` | Info | These are empty-string returns in `toUtf8()` failure/empty branches, not stubbed behavior or user-visible output. |
| Workspace | n/a | Existing dirty files | Info | `jsoncpp` is dirty as expected from build/line-ending dirt. `.planning/ROADMAP.md` and Phase 1 verification are also already modified from the user's mode-removal/human-verification edits. They are not Phase 2 implementation gaps. |
| `.planning/product/engine-runtime-fixtures/phase-02/lookup-filter-validation.json` | n/a | Validator side effect | Info | Running the requested lookup validator rewrote the file's line endings/timestamp path; timestamp content was restored, but Git still reports line-ending-only worktree dirt. No semantic diff is present. |

### Human Verification Required

None. The phase goal is a developer-facing technical proof, and the runtime/proof/lookup validation commands passed.

### Gaps Summary

No blocking gaps found. Phase 02 achieves the standard technical spike goal: the codebase contains reproducible runtime staging evidence, a framed adapter proof, raw lookup-filter evidence, validators, contracts, and adapter compatibility notes sufficient for later protocol/UI/installer phases to consume.

---

_Verified: 2026-06-23T13:19:49Z_  
_Verifier: the agent (gsd-verifier)_
