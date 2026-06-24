# Phase 5: Candidate, Dictionary, Settings, and About UI Parity - Context

**Gathered:** 2026-06-24T00:46:20Z
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase makes the visible TypeDuck Windows experience native and Web-alpha-aligned: candidate panel, movement-triggered dictionary detail panel, settings dialog, installer-first-run settings entry, persisted TypeDuck settings, and About dialog. It consumes the Phase 4 TypeDuck protocol and raw lookup-filter payload work, maps candidate/dictionary data into native UI fields, and verifies focus-safe Windows behavior in real host applications. It does not broaden customization beyond TypeDuck Web alpha, perform the full Phase 6 scaffold/privacy audit, or replace the backend/runtime architecture unless a UI parity blocker is discovered.

</domain>

<decisions>
## Implementation Decisions

### Native Candidate and Dictionary Rendering
- **D-01:** Prefer a native Win32/custom-drawn candidate and dictionary surface over Qt for the IME popup. The dictionary layout has table-like rows, captions, badges, and multi-language text, but this is still a measured owner-drawn panel problem, not enough to justify bringing Qt into the TSF popup path.
- **D-02:** Qt should not be integrated into the in-process TSF candidate/dictionary window by default. It would add deployment weight, event-loop/focus complexity, host-process risk, DPI/focus/accessibility uncertainty, and extra 32-bit/64-bit packaging concerns exactly where Phase 5 requires boring focus behavior.
- **D-03:** If a heavier UI toolkit is considered later, limit that discussion to a separate first-party settings/About executable or launcher-hosted dialog, not the candidate popup injected into arbitrary TSF host processes. Even there, use it only if native Win32 controls become a proven blocker.
- **D-04:** Preserve the Web alpha interaction rule that dictionary detail reveal is movement-triggered, not passive pointer-rest hover. The native candidate panel should avoid flicker when a stationary mouse happens to sit over candidates while the user keeps typing.
- **D-05:** The candidate/dictionary layout should follow the TypeDuck Web source structure: input buffer, page nav, compact candidate rows, selection label, Jyutping according to setting, Chinese text with Sung/Hei choice, note/input code, definition/canonical label columns, dictionary indicator, and side dictionary panel with entry head/body/tables/More Languages.

### Visual Theme and Semantic Tokens
- **D-06:** Name colors semantically for the Windows/native theme contract. Do not preserve Tailwind/DaisyUI names such as `primary`, `base-100`, `base-300`, or `base-content-400` as product-facing token names merely because they appear in `tailwind.config.ts`.
- **D-07:** The TypeDuck theme contract should expose semantic roles such as `panel_background`, `dictionary_background`, `input_buffer_background`, `panel_border`, `selection_background`, `accent`, `text_primary`, `text_secondary`, `pronunciation_text`, `definition_text`, `disabled_text`, and `link_text`.
- **D-08:** Keep the Phase 5 bundled palette set small: Web-alpha-derived `light` and `dark` palettes only. Additional named novelty themes from the Moqi scaffold are not TypeDuck v1 product scope.
- **D-09:** The Windows palette values should be source-backed by the refreshed TypeDuck Web alpha, but token names may intentionally differ from the Web/Tailwind names when semantic Windows names are clearer.
- **D-10:** The settings UI should not expose arbitrary color editing in Phase 5. Theme data is a product contract for rendering parity, not a broad customization surface.

### `appearance_themes.json` Ownership and Shape
- **D-11:** Keep one authoritative bundled `appearance_themes.json` source for the TypeDuck runtime/package. The current duplicated installed/staged files under `input_methods/rime/appearance_themes.json` and `input_methods/rime/data/appearance_themes.json` are byte-identical scaffold artifacts copied by the sibling backend build for legacy loader compatibility.
- **D-12:** Prefer `input_methods/rime/appearance_themes.json` as the canonical packaged runtime location because the current backend loader searches that path first and the source file in `D:\VSProjects\moqi-ime\input_methods\rime\appearance_themes.json` is embedded by the Go backend. Keep `input_methods/rime/data/appearance_themes.json` only as a temporary compatibility copy if needed during transition, and remove it once loader/package tests prove the canonical path is sufficient.
- **D-13:** Replace the current Moqi/Simplified novelty theme list with a TypeDuck-owned schema. It should include `version`, a top-level font contract, and `themes`; fonts must stay outside the `themes` key so light/dark palettes do not duplicate font-stack data.
- **D-14:** The top-level font contract should describe HK Chinese/English and display-language stacks, including Sung and Hei presentation choices, dictionary/comment text, and multilingual fallbacks for English, Hindi, Indonesian, Nepali, and Urdu where Windows has suitable fonts.
- **D-15:** The `themes` key should contain only palette definitions. A planner may choose exact JSON field names, but the shape should make the separation obvious, for example top-level `fonts` plus `themes: [{ id: "light", palette: {...}}, { id: "dark", palette: {...}}]`.

### Settings, Persistence, and Runtime Configuration
- **D-16:** The same TypeDuck settings dialog must open during installation and after installation, with Display Languages first and Web alpha order mirrored.
- **D-17:** Settings persistence should be TypeDuck-owned and should ultimately generate/update the runtime configuration needed by the backend, including `common.custom.yaml` or equivalent schema customization, instead of treating candidate count and schema settings as static packaged data.
- **D-18:** The Web alpha settings set is the Phase 5 scope: Display Languages, main display language, No. of Candidates Per Page 4-10, Chinese Typeface Sung/Hei, Candidates Jyutping modes, Auto-completion, Auto-correction, Auto-composition, Input Memory, Reverse Lookup Settings, Show Full Input Code, and Cangjie Version 3/5 where supported.
- **D-19:** Capability-gated settings should remain visible only when TypeDuck can explain support cleanly. Engine-backed settings may be disabled with bilingual explanatory text if the engine reports unsupported or unavailable behavior.
- **D-20:** About dialog content should be bilingual and concise: TypeDuck Windows IME identity, version, TypeDuck-HK librime/schema attribution, lookup-filter attribution, and relevant local/offline privacy wording if needed. Broad privacy copy cleanup remains Phase 6 unless necessary for About correctness.

### Dictionary Data Mapping
- **D-21:** Phase 5 should parse the Phase 2/4 raw lookup-filter payload into structured native UI fields. The parser must preserve the D-10 CSV header order and D-11 separator semantics while treating backend text as untrusted display data.
- **D-22:** Dictionary rows should support multiple matched entries per candidate, including compound candidates such as `好心你`, and should render entry-level headword, Jyutping, reading notes, part-of-speech badges, register, labels, main definition, written/vernacular/collocation rows, canonical references, and More Languages rows.
- **D-23:** Candidate-row definitions should obey the enabled display languages and main display language from settings, matching the Web alpha behavior rather than hardcoding English-only display.

### Verification and Evidence
- **D-24:** Refresh or re-check the TypeDuck Web alpha fixtures before implementation planning because Phase 5 is directly visual and Web alpha can drift.
- **D-25:** Preview harness screenshots are useful for layout iteration, but acceptance requires VM or equivalent Windows evidence for candidate placement, focus behavior, settings entry points, installer-first-run settings, high DPI, multi-monitor, UI-less/imperfect composition rectangles, and representative host apps.
- **D-26:** Add focused automated tests where possible: theme JSON schema/loader, duplicate theme-file package behavior, settings persistence, lookup payload parser, candidate/dictionary view-model mapping, and banned Moqi theme/string leakage in Phase 5 surfaces.

### the agent's Discretion
The planner may choose the exact implementation split, drawing API, internal C++ view-model names, and JSON schema field names, provided the result stays native/focus-safe for the candidate popup, uses semantic theme names, keeps fonts outside `themes`, keeps only light/dark palettes, treats `input_methods/rime/appearance_themes.json` as canonical after compatibility is proven, and mirrors the Web alpha settings/candidate/dictionary behavior.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Planning
- `.planning/PROJECT.md` - Product truth: TypeDuck Web alpha is authoritative; Moqi is scaffold only.
- `.planning/REQUIREMENTS.md` - Phase 5 requirement mapping for INST-06, CAND-01 through CAND-07, SET-01 through SET-10, and LANG-03.
- `.planning/ROADMAP.md` - Phase 5 goal, success criteria, dependencies, and VM verification expectations.
- `.planning/STATE.md` - Current project position, Phase 5 focus, and deferred settings/schema customization work.
- `.planning/phases/02-engine-runtime-contract-spike/02-CONTEXT.md` - Raw lookup-filter CSV/comment contract handoff.
- `.planning/phases/03-zh-hk-tsf-registration-and-installer-skeleton/03-CONTEXT.md` - Installer-first-run settings boundary and TypeDuck installed identity.
- `.planning/phases/04-typeduck-protocol-and-typing-mvp/04-CONTEXT.md` - Phase 4 protocol/candidate payload boundary and deferred Phase 5 native UI work.

### Product Contracts and Fixtures
- `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` - Source-backed settings order, candidate/dictionary layout, visual references, interaction rules, and Web token values.
- `.planning/product/TYPEDUCK-LOOKUP-FILTER-RAW-CONTRACT.md` - Required raw lookup-filter CSV header, separator semantics, and evidence files.
- `.planning/product/TYPEDUCK-BANNED-SURFACES.md` - Banned Moqi/fcitx/cloud/AI/excessive-customization surfaces relevant to theme/settings cleanup.
- `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` - TypeDuck names, installed paths, version/resource identity, and About/installer references.

### TypeDuck Web Alpha Source
- `I:\GitHub\TypeDuck-Web\src\Preferences.tsx` - Settings labels, order, controls, and Display Languages behavior.
- `I:\GitHub\TypeDuck-Web\src\consts.ts` - Language labels/codes, defaults, Jyutping modes, definition layout, dictionary labels, register, and part-of-speech maps.
- `I:\GitHub\TypeDuck-Web\src\CandidatePanel.tsx` - Candidate panel structure, dictionary reveal state, page nav, and movement-triggered hide/show behavior.
- `I:\GitHub\TypeDuck-Web\src\Candidate.tsx` - Candidate row table structure, Jyutping visibility, main-language definitions, canonical references, and dictionary indicator.
- `I:\GitHub\TypeDuck-Web\src\DictionaryPanel.tsx` - Dictionary entry head/body/table/More Languages rendering.
- `I:\GitHub\TypeDuck-Web\src\index.css` - Candidate/dictionary panel CSS, font stacks, sizing, border, highlight, and table styling.
- `I:\GitHub\TypeDuck-Web\tailwind.config.ts` - Palette values and font token source only; do not copy Tailwind token names blindly into Windows theme JSON.

### Windows Frontend Code
- `MoqiTextService/MoqiCandidateWindow.cpp` - Current native candidate window drawing, sizing, mouse handling, focus-safe popup style, and legacy colors to replace.
- `MoqiTextService/MoqiCandidateWindow.h` - Candidate UI item model and window methods available for extension.
- `MoqiTextService/MoqiTextService.cpp` - Candidate window ownership, font setup, UI-less policy, candidate state, placement, and update hooks.
- `MoqiTextService/MoqiTextService.h` - Product UI state fields for candidate list/page/preedit/settings integration.
- `MoqiTextService/MoqiClient.cpp` - Backend response handling, `CandidateEntry` conversion, `CustomizeUi` application, and settings protocol bridge.
- `proto/moqi.proto` - Current TypeDuck protocol fields for candidate entries, candidate page, settings snapshot/update, capabilities, health, and error data.
- `Preview/main.cpp` - Candidate preview harness useful for layout screenshots but not sufficient for final TSF placement/focus proof.

### Backend Runtime and Theme Code
- `D:\VSProjects\moqi-ime\input_methods\rime\appearance_themes.json` - Current embedded builtin theme source file to replace with TypeDuck light/dark semantic theme contract.
- `D:\VSProjects\moqi-ime\input_methods\rime\appearance_themes.go` - Current loader search order, embedded fallback, theme registry, and theme application hooks.
- `D:\VSProjects\moqi-ime\input_methods\rime\appearance_config.go` - Current appearance/settings persistence, customize UI map, candidate count config write, and scaffold customization surface.
- `D:\VSProjects\moqi-ime\scripts\build.ps1` - Current package step that copies `appearance_themes.json` into both `input_methods/rime` and `input_methods/rime/data`.
- `installer/stage/win32/TypeDuckIME/moqi-ime/input_methods/rime/appearance_themes.json` - Current staged canonical-path copy, byte-identical to the data copy at context time.
- `installer/stage/win32/TypeDuckIME/moqi-ime/input_methods/rime/data/appearance_themes.json` - Current staged compatibility copy, byte-identical to the canonical-path copy at context time.

### Codebase Maps
- `.planning/codebase/CONVENTIONS.md` - New TypeDuck-owned code style, bilingual string rule, and legacy scaffold treatment.
- `.planning/codebase/STRUCTURE.md` - File locations for candidate UI, settings/protocol integration, preview, installer, and backend staging.
- `.planning/codebase/STACK.md` - Win32/TSF/C++/protobuf/backend/runtime stack and packaging constraints.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `MoqiTextService/MoqiCandidateWindow.cpp`: Already has a non-activating topmost popup, owner normalization, double-buffered GDI painting, rounded clipping, row measurement, mouse hit testing, wheel paging, and selection click handling. This is the right starting point for native TypeDuck candidate/dictionary rendering.
- `MoqiTextService/MoqiTextService.cpp`: Already creates fonts, owns candidate page state, updates the candidate window, and applies backend UI customization values.
- `proto/moqi.proto`: Already carries structured `CandidateEntry` fields, `TypeDuckCandidatePage`, settings snapshot/update, capabilities, health, and error messages from Phase 4.
- `D:\VSProjects\moqi-ime\input_methods\rime\appearance_themes.go`: Already supports embedded builtin themes, filesystem override loading, user theme merge, and theme application into backend style fields.
- `D:\VSProjects\moqi-ime\input_methods\rime\appearance_config.go`: Already writes local appearance config and exposes `customizeUIMap()` to the Windows frontend, though much of its scaffold customization is broader than TypeDuck v1.
- `Preview/main.cpp`: Useful for rapid candidate-panel visual iteration before VM/host-app verification.

### Established Patterns
- Candidate popup rendering is native Win32/GDI today and uses `WS_EX_NOACTIVATE`; preserving this focus model is more important than adopting a richer toolkit.
- Backend theme data currently lives in the sibling `moqi-ime` runtime and is copied into this repo's installer stage; Phase 5 may need coordinated changes across both repos.
- The current appearance theme JSON is scaffold/Moqi-oriented, Simplified-labeled, and overly broad. It conflicts with the TypeDuck scope of Web-alpha parity and limited customization.
- TypeDuck Web uses table markup for candidates and dictionary details, but the native implementation can represent the same structure with measured rows/columns and semantic view models instead of an actual toolkit table widget.

### Integration Points
- Candidate/dictionary view-model mapping connects `proto::CandidateEntry.raw_lookup_comment` and structured candidate fields to native row/dictionary rendering.
- Settings UI connects installer `[Run]` or setup flow, post-install entry point, TypeDuck-owned local storage, backend settings update protocol, and generated Rime custom config.
- Theme JSON changes connect the sibling backend source, backend loader search order, installer staging, and frontend `CustomizeUi`/native rendering tokens.
- Verification must connect preview screenshots, automated parser/settings/theme tests, and VM evidence from real TSF host apps.

</code_context>

<specifics>
## Specific Ideas

- User direction: color names should be semantic. Do not preserve the exact color names from TypeDuck Web `tailwind.config.ts` when creating Windows/runtime theme schema.
- User direction: `appearance_themes.json` should include the two light/dark color palettes and font information, but font information should live outside the `themes` key.
- Advice captured: the table layout is not complicated enough to justify Qt for the candidate/dictionary popup. Treat it as native measured layout unless implementation proves a concrete blocker.
- Advice captured: keep `input_methods/rime/appearance_themes.json` as the canonical bundled theme file and treat the `input_methods/rime/data/appearance_themes.json` copy as temporary compatibility baggage.
- Current evidence: the two staged copies are byte-identical at SHA-256 `C0F877B9FF6F9FFC8802FD717560EB13B05A47987A0ECC4C56CD789A6DE01F4C`, and `D:\VSProjects\moqi-ime\scripts\build.ps1` currently copies the same source file to both paths.
- Suggested JSON direction: top-level `version`, top-level `fonts`, and `themes` containing only `light` and `dark` palette entries with semantic role keys.

</specifics>

<deferred>
## Deferred Ideas

- Additional novelty/custom themes beyond Web-alpha-derived light/dark belong to v2 advanced customization, not Phase 5.
- Full removal of Moqi/fcitx/cloud/AI surfaces and automated whole-product banned-string audits remain Phase 6 unless a smaller removal is necessary for Phase 5 UI parity.
- Full release compatibility matrix and final installer artifact evidence remain Phase 7.
- Qt or another heavy UI toolkit can be revisited only if native settings/About implementation hits a concrete blocker; it should remain out of the TSF candidate popup path.

</deferred>

---

*Phase: 5-Candidate, Dictionary, Settings, and About UI Parity*
*Context gathered: 2026-06-24T00:46:20Z*
