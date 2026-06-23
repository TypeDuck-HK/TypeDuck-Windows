# Phase 4: TypeDuck Protocol and Typing MVP - Context

**Gathered:** 2026-06-23T18:02:17Z
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase makes TypeDuck typing work through the existing Windows TSF frontend, launcher, and TypeDuck-HK engine adapter path. It should preserve the current transport mechanics where useful: TSF DLL to launcher over a per-user Windows named pipe, launcher to backend over process stdin/stdout, and protobuf payloads wrapped in length-prefixed frames. The phase replaces or extends Moqi-shaped product semantics only where needed for TypeDuck typing and raw candidate/comment data. It does not implement the Phase 5 native dictionary parser/detail UI, visual parity, settings dialog, About dialog, or broad scaffold cleanup.

</domain>

<decisions>
## Implementation Decisions

### Protocol and Transport Boundary
- **D-01:** Keep the current transport mechanics for Phase 4 unless implementation discovers a concrete blocker: TSF DLL sends framed protobuf over the per-user named pipe to the launcher, and the launcher forwards framed protobuf over backend stdin/stdout.
- **D-02:** Do not treat the existing Moqi protobuf semantics as product truth. The current `ClientRequest`, `ServerResponse`, `Method`, `KeyEvent`, and `CandidateEntry { text, comment }` schema may be reused or extended as a compatibility bridge, but TypeDuck-specific semantics must be explicit in code/tests and must not preserve unrelated Moqi product features as required behavior.
- **D-03:** Product-facing protocol work should focus on the minimum TypeDuck typing contract: key events, composition string, candidate text/comment payloads, candidate cursor, paging, selection, commit string, errors, and health/degraded-state signaling needed for responsive typing.
- **D-04:** Moqi-only protocol surfaces such as cloud clipboard, broad UI customization, tray/menu clutter, and arbitrary backend config concepts remain scaffold residue. Phase 4 may leave them physically present if removing them is Phase 6 scope, but new TypeDuck typing behavior must not depend on them.

### Candidate Comment and Lookup Payload Handling
- **D-05:** Phase 4 should not parse `rime-dictionary-lookup-filter` raw comment payloads into structured dictionary UI fields. Treat that parsing and dictionary-detail presentation as Phase 5 UI work.
- **D-06:** Phase 4 must preserve raw candidate comment payloads end to end and display/pass them at the current comment-display position. The payload must retain control separators and CSV text exactly enough for Phase 5 to parse later.
- **D-07:** Automated/golden checks should prove raw lookup-filter payload survivability through protocol/transport: candidate text and raw comment reach the frontend, `\v`, `\f`, and `\r` semantics are not stripped or normalized, and oversized/malformed payloads are bounded.
- **D-08:** The existing `CandidateEntry { text, comment }` shape is acceptable as a Phase 4 bridge if it preserves TypeDuck raw comments faithfully. Downstream planners should not introduce a premature native dictionary view model in this phase.

### Typing MVP Behavior
- **D-09:** Phase 4 typing acceptance is ordinary frontend key capture and backend-driven candidate behavior: composition, candidate list, candidate paging, numeric selection, commit, candidate cursor, and clean reset/deactivation.
- **D-10:** Reverse lookup and Cangjie Version 3/5 behavior are handled by librime/backend. The frontend should capture and forward the relevant key events normally, then display returned composition/candidates/comments. Do not special-case reverse lookup or Cangjie in the frontend unless a concrete TSF key-capture gap is found.
- **D-11:** Candidate count per page should be respected if returned/configured through the backend path, but the frontend's Phase 4 responsibility is forwarding/using the backend result, not building the final settings UI.
- **D-12:** The minimum user-visible proof is that a user can type Cantonese in ordinary Windows text fields with returned TypeDuck candidates, raw comments, paging/selection, and commit behavior without relying on legacy Moqi fallback behavior as product truth.

### Responsiveness, Bounds, and Failure Policy
- **D-13:** Phase 4 should harden the brittle parts of the scaffold enough that TypeDuck does not inherit unbounded or hanging behavior: add frame-size caps, malformed-frame rejection, bounded buffering, and deterministic cleanup on protocol violation.
- **D-14:** Backend timeout, missing backend, backend restart, and init failure cases should return or trigger a bounded degraded state/clean reset instead of leaving TSF host applications waiting indefinitely or silently unclear.
- **D-15:** Current Moqi scaffold behavior is not assumed to be unusable, but it is not sufficient for TypeDuck guarantees: frame buffering currently has no max payload cap, timeout paths restart the backend without a clear client error response, and key handling uses synchronous RPC on the typing path.
- **D-16:** Plans should prefer automated tests for frame bounds, malformed payloads, timeout handling, restart/missing backend behavior, and raw payload preservation, plus a targeted Windows VM smoke once TSF typing is wired.

### the agent's Discretion
The planner may decide whether TypeDuck protocol semantics are represented as new protobuf fields/messages, carefully named wrapper helpers, or a compatibility adapter layer, as long as the accepted transport boundary is preserved, raw comments survive unchanged, Moqi-only product semantics are not treated as TypeDuck requirements, and responsiveness/bounds are testable.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Planning
- `.planning/PROJECT.md` - Product truth: TypeDuck Web alpha is authoritative; Moqi is scaffold only.
- `.planning/REQUIREMENTS.md` - Phase 4 requirement mapping for ENG-06, PROTO-01 through PROTO-06, and TYPE-01 through TYPE-05.
- `.planning/ROADMAP.md` - Phase 4 goal, success criteria, verification environment, and boundaries with Phase 5/6/7.
- `.planning/STATE.md` - Current project position, prior decisions, and deferred Phase 3 items affecting Phase 4.
- `.planning/phases/01-identity-and-web-parity-contract/01-CONTEXT.md` - Identity, Web alpha fixture authority, and banned scaffold-surface decisions.
- `.planning/phases/02-engine-runtime-contract-spike/02-CONTEXT.md` - TypeDuck-HK runtime proof, internal adapter boundary, and raw lookup-filter decision handoff.
- `.planning/phases/03-zh-hk-tsf-registration-and-installer-skeleton/03-CONTEXT.md` - Installed TypeDuck identity/launcher/registration decisions that Phase 4 must connect to.

### Product Contracts and Fixtures
- `.planning/product/TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md` - Phase 2 runtime staging and adapter boundary proof.
- `.planning/product/TYPEDUCK-LOOKUP-FILTER-RAW-CONTRACT.md` - Raw lookup-filter payload contract; Phase 4 preserves comments, Phase 5 parses dictionary fields.
- `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` - Candidate/settings behavior and source-backed Web alpha parity constraints.
- `.planning/product/TYPEDUCK-BANNED-SURFACES.md` - Moqi/fcitx/cloud/AI/excessive customization surfaces that must not become TypeDuck product behavior.
- `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` - TypeDuck naming, profile, runtime path, launcher, and artifact authority.

### Codebase Maps
- `.planning/codebase/INTEGRATIONS.md` - Current named-pipe, backend stdio, backend manifest, storage, and Windows integration path.
- `.planning/codebase/CONCERNS.md` - Known protocol/framing, synchronous RPC, timeout, backend restart, and scaffold leakage risks.
- `.planning/codebase/ARCHITECTURE.md` - TSF DLL, launcher, backend process bridge, and protocol data flow.
- `.planning/codebase/STACK.md` - C++/protobuf/libuv/TSF build and runtime stack.

### Protocol and Typing Code
- `proto/moqi.proto` - Current Moqi-shaped protobuf schema and compatibility fields.
- `proto/ProtoFraming.h` - Length-prefixed frame helper requiring Phase 4 bounds.
- `MoqiTextService/MoqiClient.cpp` - TSF client request/response handling, synchronous RPC path, candidate/comment application, connection handling, and launcher startup path.
- `MoqLauncher/PipeClient.cpp` - Launcher client request routing, backend timeout handling, and init failure path.
- `MoqLauncher/BackendServer.cpp` - Backend process stdin/stdout bridge, restart behavior, and backend response routing.
- `MoqLauncher/PipeServer.cpp` - Backend manifest/profile mapping and named-pipe server setup.
- `backends.json` - Current transitional backend manifest still referencing the compatibility backend shape.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `proto/ProtoFraming.h`: Existing shared framing helper used by TSF, launcher client routing, and backend stdout parsing. Reuse it, but add strict payload-size/error handling.
- `proto/moqi.proto`: Existing request/response schema already carries key events, composition, candidate list, candidate cursor, selection/page methods, commit strings, and `CandidateEntry.text/comment`.
- `MoqiTextService/MoqiClient.cpp`: Existing TSF-to-launcher RPC client builds key requests, converts responses into composition/candidates/comments, and already supports `candidate_entries`.
- `MoqLauncher/BackendServer.cpp`: Existing out-of-process backend bridge starts/restarts the backend and forwards framed protobuf messages over stdin/stdout.
- `MoqLauncher/PipeClient.cpp`: Existing per-client launcher connection object routes frontend requests to backend and has a timer-based timeout hook to harden.

### Established Patterns
- The current transport is synchronous from the TSF client perspective and length-framed across both named pipe and backend stdio boundaries.
- The launcher currently maps a language profile GUID to a backend via installed `input_methods/*/ime.json`, even though Phase 3 made the TypeDuck profile first-party. Phase 4 may need a transitional mapping that does not restore backend JSON as profile authority.
- Candidate UI state currently accepts both flat `candidate_list` and structured `candidate_entries`, with `CandidateEntry.comment` displayed as the comment line.
- Backend process isolation is an intentional safety boundary from prior phases and should remain the default unless proven blocked.

### Integration Points
- `MoqiTextService/MoqiClient.cpp` and `MoqLauncher/PipeClient.cpp` must agree on request sequencing, timeout/error behavior, frame-size limits, and cleanup semantics.
- `MoqLauncher/PipeClient.cpp` and `MoqLauncher/BackendServer.cpp` must agree on backend missing/restart/degraded responses.
- `proto/moqi.proto`, generated protobuf output, and any backend adapter changes must be regenerated/tested together.
- Phase 4 raw comment preservation feeds Phase 5 candidate/dictionary parser and native UI work.
- Phase 4 typing smoke should run against the Phase 3 TypeDuck installer/profile path once code tests pass.

</code_context>

<specifics>
## Specific Ideas

- Keep "transport" narrowly defined as named pipe plus backend stdin/stdout plus framed protobuf. Do not let "keep Moqi protobuf" mean preserving Moqi product concepts as TypeDuck behavior.
- Treat reverse lookup and Cangjie as engine/backend behavior. The frontend should forward key events and render returned candidates/comments.
- Use the existing candidate comment display position for raw lookup-filter payloads in Phase 4; a nicer parsed dictionary panel belongs to Phase 5.
- When explaining or documenting the transition, be explicit that the existing Moqi protocol schema is a compatibility bridge, not product truth.

</specifics>

<deferred>
## Deferred Ideas

- Parsing lookup-filter CSV/comment payloads into structured dictionary-detail UI fields belongs to Phase 5.
- Native dictionary panel layout, movement-triggered reveal behavior, visual styling, and settings/About surfaces belong to Phase 5.
- Broad removal of Moqi/fcitx/cloud clipboard/AI surfaces, logging path privacy cleanup, and full banned-string audits belong to Phase 6 unless a smaller removal is needed to prevent Phase 4 typing behavior from depending on them.
- Full release compatibility matrix and final installer evidence belong to Phase 7.

</deferred>

---

*Phase: 4-TypeDuck Protocol and Typing MVP*
*Context gathered: 2026-06-23T18:02:17Z*
