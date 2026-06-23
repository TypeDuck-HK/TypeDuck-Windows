# Phase 2: Engine Runtime Contract Spike - Context

**Gathered:** 2026-06-23
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase proves the TypeDuck engine/runtime path on Windows: TypeDuck-HK `rime.dll`, the dictionary lookup filter, TypeDuck schema assets, dictionaries, OpenCC assets, and built schema artifacts must produce Cantonese composition, candidates, commits, and lookup-filter CSV comment data. The fastest accepted path is to use the existing `moqi-ime` backend shape as an internal compatibility adapter while changing the least possible code. Native UI parity, final TypeDuck protocol design, installer registration, and broad scaffold cleanup remain later phases.

</domain>

<decisions>
## Implementation Decisions

### Engine Boundary and Fastest Shipping Path
- **D-01:** Use the current `moqi-ime` backend shape as an internal compatibility adapter for Phase 2, not as product truth.
- **D-02:** Modify the least amount necessary to load TypeDuck-HK `rime.dll` and TypeDuck schema assets through the existing Rime DLL loading path.
- **D-03:** Focus Phase 2 code work on compatibility issues caused by `rime_api` differences between the current `moqi-ime` Rime expectations and the TypeDuck-HK librime fork.
- **D-04:** Do not rewrite the Windows TSF frontend around direct in-process librime calls in Phase 2. Direct standalone librime probing may be used only as fallback/debug evidence if the `moqi-ime` compatibility route blocks proof.

### Runtime Asset Packaging
- **D-05:** Produce a Windows-shaped staged TypeDuck runtime tree that later packaging can consume.
- **D-06:** Download/use the replacement DLL from `https://github.com/TypeDuck-HK/librime/releases/download/v1.1.3/rime-TypeDuck-v1.1.3-Windows-msvc-x86.7z`.
- **D-07:** Do not document the out-of-scope process used to create the replacement `rime.dll`; downstream docs should treat the release artifact as already available.
- **D-08:** Add `https://github.com/TypeDuck-HK/schema` as a submodule on branch `aap2-alpha`; use it as the source for TypeDuck schemas, dictionaries, and OpenCC assets.

### Dictionary Output Contract
- **D-09:** Phase 2 should prove raw lookup-filter CSV/comment output first, not invent a new structured TypeDuck candidate/dictionary schema yet.
- **D-10:** The expected CSV/comment columns are: `match_input_buffer,honzi,jyutping,canonical_honzi,canonical_jyutping,components_honzi,components_jyutping,pron_label,lit_col_reading,pos,register,label,written_form,vernacular_form,collocation,eng,hin,urd,nep,ind`.
- **D-11:** The candidate comment format intentionally uses control-character separators. In escaped notation: optional leading `\v` marks reverse lookup; text before the first `\f` is the candidate note; dictionary entries follow only when the remaining payload starts with `\r`; each dictionary entry is one CSV row and entries are separated by `\r`. If the payload after the note does not start with `\r`, it is treated as `\f`-separated Jyutping-only pronunciation data instead.
- **D-12:** CSV parsing follows standard comma separation with double-quoted fields and doubled quotes (`""`) inside quoted values. Empty fields are allowed. Do not strip or normalize `\v`, `\f`, or `\r` during Phase 2 evidence capture; they are part of the Web alpha-compatible payload format, not corrupt text.
- **D-13:** Later Phase 4/5 work maps those raw columns into protocol and UI fields. Phase 2 only needs to capture proof that the engine emits them for representative TypeDuck inputs.

### Legacy Backend Limits
- **D-14:** Using `moqi-ime` as a bridge does not allow shipping unchanged Moqi product behavior. Moqi/fcitx/cloud/AI surfaces remain banned from TypeDuck v1 user-visible behavior.
- **D-15:** Do not treat legacy `moqi-ime` fallback behavior as valid product behavior. The bridge is acceptable only insofar as it loads TypeDuck-HK librime/schema assets and exposes the required TypeDuck engine evidence.
- **D-16:** Broad protocol replacement, TypeDuck product naming, installer/runtime packaging cleanup, and visible UI removal belong to later mapped phases unless a minimal change is required to complete the engine proof.

### the agent's Discretion
The planner may choose the exact staging directory name and verification command shape, provided the staged tree clearly separates TypeDuck runtime assets from legacy Moqi product truth and records reproducible proof outputs.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Planning
- `.planning/PROJECT.md` - Product truth: TypeDuck Web alpha is authoritative; Moqi is scaffold only.
- `.planning/REQUIREMENTS.md` - ENG-01 through ENG-05 plus banned legacy backend fallback and Web alpha parity constraints.
- `.planning/ROADMAP.md` - Phase 2 goal, requirements, success criteria, dependencies, and later phase boundaries.
- `.planning/STATE.md` - Current project position and known Phase 2 concerns.
- `.planning/phases/01-identity-and-web-parity-contract/01-CONTEXT.md` - Prior decisions: TypeDuck identity authority, banned legacy surfaces, and Web alpha fixture authority.

### Product Contracts and Fixtures
- `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` - TypeDuck-owned runtime/package naming authority; engine runtime name was deferred to Phase 2.
- `.planning/product/TYPEDUCK-BANNED-SURFACES.md` - Legacy `moqi-ime` fallback, visible Moqi/fcitx/WebDAV/cloud/AI surfaces, and excessive customization are banned from v1 product surfaces.
- `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` - Candidate/dictionary field expectations, Web alpha evidence, reverse lookup evidence, and refresh guidance.

### Codebase Maps
- `.planning/codebase/STACK.md` - Current Windows scaffold stack, backend manifest, and packaging assumptions.
- `.planning/codebase/INTEGRATIONS.md` - Current launcher/backend process boundary, backend runtime staging, and `backends.json` relationship.
- `.planning/codebase/CONCERNS.md` - Engine boundary risk, legacy backend fallback warning, unbounded protocol/security caveats, and missing dictionary lookup model.

### External Runtime Sources
- `https://github.com/gaboolic/moqi-ime` - Reference backend/adapter shape that dynamically loads `input_methods\rime\rime.dll` and speaks framed protobuf to the Windows launcher.
- `https://github.com/rime/librime` - Upstream librime source lineage.
- `https://github.com/TypeDuck-HK/librime` - TypeDuck-HK librime fork to use for the replacement runtime.
- `https://github.com/TypeDuck-HK/rime-dictionary-lookup-filter` - Required lookup-filter plugin that emits dictionary/comment data for candidates.
- `https://github.com/TypeDuck-HK/schema/tree/aap2-alpha` - TypeDuck schema branch to add as submodule for schemas, dictionaries, and OpenCC assets.
- `https://github.com/TypeDuck-HK/librime/releases/download/v1.1.3/rime-TypeDuck-v1.1.3-Windows-msvc-x86.7z` - Replacement Windows `rime.dll` release artifact for Phase 2 runtime proof.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `backends.json`: Current manifest points to `moqi-ime\server.exe`; useful for understanding launcher expectations.
- `scripts/install.ps1`: Copies a provided `moqi-ime` runtime tree into installer staging; this repo stages backend output but does not own `rime.dll` today.
- `scripts/_all_in_package.ps1`: Builds the sibling `moqi-ime` runtime before Windows packaging; useful for locating the existing backend build handoff.
- `MoqLauncher/BackendServer.cpp`: Starts the backend process and forwards framed protobuf requests over stdio.
- `MoqLauncher/PipeServer.cpp` and `MoqLauncher/PipeClient.cpp`: Map language profile GUIDs to backend servers and route TSF client traffic.
- `proto/ProtoFraming.h` and `proto/moqi.proto`: Existing frame/protobuf contract that the adapter currently speaks.

### Established Patterns
- The Windows repo does not build `rime.dll` directly; it stages a backend runtime package that contains `server.exe`, `input_methods\rime\rime.dll`, and Rime data.
- `moqi-ime` dynamically loads `rime.dll` from `input_methods\rime\rime.dll`, so the TypeDuck-HK DLL can be tested through the existing adapter shape if exported APIs match or are shimmed.
- Current TSF/launcher process isolation protects arbitrary Windows host processes from engine crashes or hangs better than immediate in-process librime integration.
- Existing `ime.json` metadata is legacy Moqi/Simplified-oriented; do not treat it as final TypeDuck profile truth.

### Integration Points
- Phase 2 runtime proof feeds Phase 4 protocol and typing MVP work.
- The TypeDuck schema submodule and staged runtime tree feed Phase 7 release packaging.
- Raw lookup-filter CSV/comment evidence feeds Phase 5 candidate/dictionary UI parity.
- Any minimal `moqi-ime` adapter changes must preserve the later ability to remove or hide Moqi/fcitx/cloud/AI user-visible surfaces.

</code_context>

<specifics>
## Specific Ideas

- Prefer speed to architectural purity for Phase 2: use `moqi-ime` as a temporary internal adapter if it gets TypeDuck-HK librime running faster.
- The middle-end is not required by TypeDuck-Web, but it is acceptable on Windows because it preserves process isolation and avoids immediate dual-bitness in-process DLL loading concerns.
- Phase 2 documentation should say the replacement `rime.dll` is available from the TypeDuck-HK librime v1.1.3 release URL, not document how that release was produced.
- The expected lookup-filter output contract is the explicit CSV column list from the TypeDuck schema/plugin path.
- Preserve and test escaped control-character semantics in candidate comments: `\v` reverse-lookup marker, `\f` note/pronunciation separator, and `\r` dictionary CSV entry separator.

</specifics>

<deferred>
## Deferred Ideas

- Native candidate/dictionary UI implementation belongs to Phase 5.
- Final TypeDuck protocol/candidate dictionary data model belongs to Phase 4.
- Full TypeDuck installer/runtime cleanup and release artifact naming belong to Phase 3, Phase 6, and Phase 7.
- Direct in-process librime integration can be reconsidered later if the adapter path fails or becomes the larger long-term maintenance risk.

</deferred>

---

*Phase: 2-Engine Runtime Contract Spike*
*Context gathered: 2026-06-23*
