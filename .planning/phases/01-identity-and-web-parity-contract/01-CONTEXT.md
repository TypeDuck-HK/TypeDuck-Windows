# Phase 1: Identity and Web Parity Contract - Context

**Gathered:** 2026-06-23
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase creates the source-of-truth planning artifacts that downstream TypeDuck Windows IME work must follow: a complete TypeDuck identity contract, a dated TypeDuck Web alpha parity fixture set, and a banned-surface rule set for legacy scaffold leakage. It does not apply broad production renames or implement runtime behavior yet.

</domain>

<decisions>
## Implementation Decisions

### Identity Contract Shape
- **D-01:** Create a durable product contract at `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md`.
- **D-02:** The contract must cover executable names, DLL names, AppId, CLSID, profile GUIDs, install directories, log/data directories, pipe and mutex names, registry keys, resource strings, installer/release artifact names, and user-visible display names.
- **D-03:** Every identity entry must include current Moqi scaffold value when discoverable, target TypeDuck value, ownership/status (`locked`, `proposed`, or `deferred`), affected files, and verification notes.
- **D-04:** The contract is the authority for later implementation phases; downstream agents should not invent new product names, paths, GUIDs, or registry locations outside this contract without updating it.

### TypeDuck-Owned Identifier Strategy
- **D-05:** Prefer new deterministic TypeDuck-owned CLSID/profile GUID/AppId values instead of preserving Moqi identifiers. Moqi identifiers are scaffold residue unless explicitly retained for migration compatibility.
- **D-06:** Keep TypeDuck identity names channel-ready where cheap, but do not expand phase scope into beta/stable side-by-side support. Future channel support can extend the contract later.
- **D-07:** The TSF profile target must be Chinese (Traditional, Hong Kong) / `zh-HK`, with bilingual Traditional Hong Kong Chinese and English display text.

### Web Alpha Parity Fixtures
- **D-08:** Create a dated fixture set at `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` plus any small supporting assets under `.planning/product/web-alpha-fixtures/2026-06-23/`.
- **D-09:** Fixtures must be source-backed, not memory-backed. Capture the alpha URL, local TypeDuck Web source path, source commit/hash when available, capture date, browser/viewport details for screenshots, and any access limitations.
- **D-10:** The fixture set must cover settings order, settings labels, defaults/options, display-language behavior, candidate list presentation, dictionary/detail panel data, visual references, and known Web alpha gaps or unsupported Windows mappings.
- **D-11:** If `http://localhost:5173/TypeDuck-Web/aap2-alpha/` or `I:\GitHub\TypeDuck-Web` is unavailable during execution, the fixture artifact must record that as blocked/partial instead of fabricating parity data.

### Banned Scaffold Surfaces
- **D-12:** Create a negative contract at `.planning/product/TYPEDUCK-BANNED-SURFACES.md`.
- **D-13:** The banned list must include visible Moqi branding, fcitx references, WebDAV/cloud clipboard, AI controls, Simplified-only Chinese copy, arbitrary backend-declared config tools, excessive customization beyond Web alpha, and legacy Moqi backend fallback behavior.
- **D-14:** The banned-surface artifact should include audit patterns and likely files for later verification, but Phase 1 should not attempt the full cleanup. Cleanup belongs mainly to Phase 6.

### Phase Boundary and Handoff
- **D-15:** Phase 1 should be documentation and fixture capture only, with small helper scripts allowed only if they make the contract or fixture capture reproducible.
- **D-16:** Do not rename binaries, registry keys, protocol packages, installer files, or runtime paths in production code during Phase 1 unless the plan explicitly proves it is only creating documentation or fixture support. Implementation happens in later mapped phases.
- **D-17:** Downstream planning must treat IDEN-02 and VER-01 as the only requirements this phase completes.

### the agent's Discretion
Auto mode selected the recommended defaults without user prompts. The agent may choose exact markdown table layouts, screenshot filenames, and fixture JSON shapes, provided the artifacts remain human-readable, source-backed, and easy for later phases to reference.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Planning
- `.planning/PROJECT.md` - Product source of truth: TypeDuck Web alpha is authoritative; Moqi is scaffold only.
- `.planning/REQUIREMENTS.md` - Locked requirement mapping for IDEN-02 and VER-01 plus banned v1/out-of-scope product behavior.
- `.planning/ROADMAP.md` - Phase 1 goal, requirements, success criteria, and downstream phase boundaries.
- `.planning/STATE.md` - Current project position and known concerns affecting early phases.

### Codebase Maps
- `.planning/codebase/STRUCTURE.md` - Product, installer, launcher, protocol, script, and packaging file locations where identity surfaces live.
- `.planning/codebase/CONCERNS.md` - Known Moqi identity debt, banned feature leakage, TSF registration risks, and missing TypeDuck fixture/visual contracts.
- `.planning/codebase/INTEGRATIONS.md` - Current Windows API, backend, storage, COM/TSF identity, and release integration points.

### External Source-of-Truth References
- `http://localhost:5173/TypeDuck-Web/aap2-alpha/` - TypeDuck Web alpha runtime to capture dated settings, candidate, dictionary, and visual fixtures from.
- `I:\GitHub\TypeDuck-Web` - Local upstream TypeDuck Web source; record commit/hash and relevant files during fixture capture if accessible.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `.planning/codebase/*.md`: Existing codebase maps already identify most identity, installer, protocol, and banned-feature surfaces.
- `Preview/main.cpp` and `MoqiTextService/MoqiCandidateWindow.cpp`: Useful later for candidate visual parity, but Phase 1 should only reference them as future fixture consumers.
- `installer/MoqiTsf.iss`, `SetupHelper/SetupHelper.cpp`, `MoqiTextService/MoqiImeModule.cpp`, and `MoqiTextService/DllEntry.cpp`: Key future consumers of the identity contract.

### Established Patterns
- Planning artifacts live under `.planning/`; phase-specific context lives under `.planning/phases/NN-slug/`.
- Product identity is currently scattered, so the Phase 1 contract must centralize target values before implementation spreads.
- The current TSF language profile is discovered from backend metadata, but TypeDuck needs first-party deterministic `zh-HK` profile identity.

### Integration Points
- Identity contract feeds Phase 3 installer/registration, Phase 4 protocol naming, Phase 5 UI copy, Phase 6 cleanup/audit, and Phase 7 release artifact verification.
- Web alpha fixtures feed Phase 5 candidate/settings/About UI work and Phase 7 release verification.
- Banned-surface rules feed Phase 6 automated or scripted leak checks.

</code_context>

<specifics>
## Specific Ideas

- Preserve useful Windows architecture from the Moqi scaffold while treating Moqi product names and feature assumptions as non-authoritative.
- Use the TypeDuck Web alpha observed on 2026-06-23 as the initial dated fixture source, but refresh directly from the local alpha/source during execution.
- Keep the first phase crisp: lock contracts and fixtures, then let later phases apply them.

</specifics>

<deferred>
## Deferred Ideas

- Applying TypeDuck identity across binaries, registry keys, installer strings, paths, CI artifact names, and visible UI belongs to later implementation phases.
- Implementing Web alpha candidate/settings/About UI parity belongs to Phase 5.
- Automated banned-string verification belongs mainly to Phase 6, though Phase 1 may document the audit patterns.

</deferred>

---

*Phase: 1-Identity and Web Parity Contract*
*Context gathered: 2026-06-23*
