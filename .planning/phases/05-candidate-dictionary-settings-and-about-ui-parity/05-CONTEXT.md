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
- **D-05:** Exact candidate/dictionary layout behavior must be source-code-backed by TypeDuck Web, not approximated from screenshot feel. The Phase 1 verification explicitly accepted screenshots as sufficient visual references only because exact layout is followable from TypeDuck Web source. If layout details are uncertain, downstream agents must check `.planning/product` and `I:\GitHub\TypeDuck-Web` before guessing.
- **D-06:** The candidate/dictionary layout should follow the TypeDuck Web source structure: input buffer, page nav, compact candidate rows, selection label, Jyutping according to setting, Chinese text with Sung/Hei choice, note/input code, definition/canonical label columns, dictionary indicator, and side dictionary panel with entry head/body/tables/More Languages.

### Visual Theme and Semantic Tokens
- **D-07:** Name colors semantically for the Windows/native theme contract. Do not preserve Tailwind/DaisyUI names such as `primary`, `base-100`, `base-300`, or `base-content-400` as product-facing token names merely because they appear in `tailwind.config.ts`.
- **D-08:** The TypeDuck theme contract should expose semantic roles such as `panel_background`, `dictionary_background`, `input_buffer_background`, `panel_border`, `selection_background`, `accent`, `text_primary`, `text_secondary`, `pronunciation_text`, `definition_text`, `disabled_text`, and `link_text`.
- **D-09:** Keep the Phase 5 bundled palette set small: Web-alpha-derived `light` and `dark` palettes only. Additional named novelty themes from the Moqi scaffold are not TypeDuck v1 product scope.
- **D-10:** The Windows palette values should be source-backed by the existing TypeDuck Web alpha fixture and local source, but token names may intentionally differ from the Web/Tailwind names when semantic Windows names are clearer.
- **D-11:** The settings UI should not expose arbitrary color editing in Phase 5. Theme data is a product contract for rendering parity, not a broad customization surface.

### `appearance_themes.json` Ownership and Shape
- **D-12:** Keep one authoritative bundled `appearance_themes.json` source for the TypeDuck runtime/package. The current duplicated installed/staged files under `input_methods/rime/appearance_themes.json` and `input_methods/rime/data/appearance_themes.json` are byte-identical scaffold artifacts copied by the sibling backend build for legacy loader compatibility.
- **D-13:** Prefer `input_methods/rime/appearance_themes.json` as the canonical packaged runtime location because the current backend loader searches that path first and the source file in `D:\VSProjects\moqi-ime\input_methods\rime\appearance_themes.json` is embedded by the Go backend. Keep `input_methods/rime/data/appearance_themes.json` only as a temporary compatibility copy if needed during transition, and remove it once loader/package tests prove the canonical path is sufficient.
- **D-14:** Replace the current Moqi/Simplified novelty theme list with a TypeDuck-owned schema. It should include `version`, a top-level font contract, and `themes`; fonts must stay outside the `themes` key so light/dark palettes do not duplicate font-stack data.
- **D-15:** The top-level font contract should describe HK Chinese/English and display-language stacks, including Sung and Hei presentation choices, dictionary/comment text, and multilingual fallbacks for English, Hindi, Indonesian, Nepali, and Urdu where Windows has suitable fonts.
- **D-16:** The `themes` key should contain only palette definitions. A planner may choose exact JSON field names, but the shape should make the separation obvious, for example top-level `fonts` plus `themes: [{ id: "light", palette: {...}}, { id: "dark", palette: {...}}]`.

### Settings, Persistence, and Runtime Configuration
- **D-17:** The same TypeDuck settings dialog must open during installation and after installation, with Display Languages first and Web alpha order mirrored.
- **D-18:** Follow exactly the same two-column settings layout as TypeDuck Web. This is a layout parity requirement, not a loose visual suggestion.
- **D-19:** Whether settings save immediately or require a Confirm button is left to the planner/implementer. The chosen behavior must be clear, bilingual where user-facing, and must not create mismatched install-time versus post-install settings behavior.
- **D-20:** Settings persistence should be TypeDuck-owned and should ultimately generate/update the runtime configuration needed by the backend, including generated `common.custom.yaml` / `default.custom.yaml` side effects or equivalent schema customization, instead of treating candidate count and schema settings as static packaged data.
- **D-21:** The Web alpha settings set is the Phase 5 scope: Display Languages, main display language, No. of Candidates Per Page 4-10, Chinese Typeface Sung/Hei, Candidates Jyutping modes, Auto-completion, Auto-correction, Auto-composition, Input Memory, Reverse Lookup Settings, Show Full Input Code, and Cangjie Version 3/5 where supported.
- **D-22:** Capability-gated settings should remain visible only when TypeDuck can explain support cleanly. Engine-backed settings may be disabled with bilingual explanatory text if the engine reports unsupported or unavailable behavior.

### About Dialog and Branding Assets
- **D-23:** The About dialog must include `D:\VSProjects\moqi-ime\icons\About_Banner.bmp` at the top, the exact bilingual text block below, then `D:\VSProjects\moqi-ime\icons\Credit_Logos.bmp`.
- **D-24:** The About dialog must include these links with these exact labels and URLs: `TypeDuck 網站 Website` -> `https://typeduck.hk`; `LearnDuck 粵拼打字入門 Introduction to Jyutping Typing` -> `https://learn.typeduck.hk`; `粵拼方案 Jyutping Scheme` -> `https://lshk.org/jyutping-scheme/`; `TypeDuck 原始碼 Source Code` -> `https://github.com/TypeDuck-HK/TypeDuck-Windows`.
- **D-25:** About and branding bitmap resources should be moved into a proper TypeDuck product resource/assets location during implementation, not left semantically buried under a generic `icons` directory. Keep `D:\VSProjects\moqi-ime\icons` as the current source location for planning.
- **D-26:** `D:\VSProjects\moqi-ime\icons\Installer.bmp` is installer-specific and should be left for later installer-focused phases unless Phase 5 needs to reference installer-first-run settings only.
- **D-27:** The About dialog exact text is:

```cpp
L"歡迎使用 TypeDuck 打得 —— 設有少數族裔語言提示粵拼輸入法！有字想打？一裝即用，毋須再等，即刻打得！\n"
L"Welcome to TypeDuck: a Cantonese input keyboard with minority language prompts! Got something you want to type? Have your fingers ready, get, set, TYPE DUCK!\n"
L"\n"
L"如有任何查詢，歡迎電郵至 info@typeduck.hk 或 lchaakming@eduhk.hk。\n"
L"Should you have any enquiries, please email info@typeduck.hk or lchaakming@eduhk.hk.\n"
L"\n"
L"本輸入法由香港教育大學語言學及現代語言系開發。特別鳴謝「語文教育及研究常務委員會」資助本計劃。\n"
L"This input method is developed by the Department of Linguistics and Modern Language Studies, the Education University of Hong Kong. Special thanks to the Standing Committee on Language Education and Research for funding this project."
```

### Icon Usage
- **D-28:** Use `D:\VSProjects\moqi-ime\icons\TypeDuck_Transparent.ico` for all executable icons except the installer/uninstaller, including `%PROGRAMFILES(x86)%\TypeDuckIME\TypeDuckLauncher.exe`, `%PROGRAMFILES(x86)%\TypeDuckIME\TypeDuckSetupHelper.exe`, and `%PROGRAMFILES(x86)%\TypeDuckIME\moqi-ime\server.exe`.
- **D-29:** Use `D:\VSProjects\moqi-ime\icons\TypeDuck_Small.ico` for the system input method picker menu.
- **D-30:** Use `D:\VSProjects\moqi-ime\icons\TypeDuck.ico` for the installer, uninstaller (`%PROGRAMFILES(x86)%\TypeDuckIME\unins000.exe`), and any other broad product branding surface where the full TypeDuck icon is appropriate.
- **D-31:** Do not use these legacy Moqi images anywhere: `D:\VSProjects\moqi-ime\icons\moqi.png`, `D:\VSProjects\moqi-ime\icons\mo.ico`, `D:\VSProjects\moqi-ime\icons\mo.png`, `D:\VSProjects\moqi-ime\icons\moqi.ico`.

### Dictionary Data Mapping
- **D-32:** Phase 5 should parse the Phase 2/4 raw lookup-filter payload into structured native UI fields. The parser must preserve the D-10 CSV header order and D-11 separator semantics while treating backend text as untrusted display data.
- **D-33:** Dictionary rows should support multiple matched entries per candidate, including compound candidates such as `好心你`, and should render entry-level headword, Jyutping, reading notes, part-of-speech badges, register, labels, main definition, written/vernacular/collocation rows, canonical references, and More Languages rows.
- **D-34:** Candidate-row definitions should obey the enabled display languages and main display language from settings, matching the Web alpha behavior rather than hardcoding English-only display.

### Verification and Evidence
- **D-36:** Superseded on 2026-06-26 by explicit user direction: Phase 5 screenshot tests, screenshot manifests, and `.planning/product/ui-fixtures` were retired and replaced by iterative user human verification plus focused static/build/package guards. Preview harness screenshots remain useful for debugging only, not Phase 5 acceptance. Fresh release screenshots or broader host-app/DPI evidence belong to Phase 7 if desired.
- **D-37:** Add focused automated tests where possible: theme JSON schema/loader, duplicate theme-file package behavior, settings persistence, lookup payload parser, candidate/dictionary view-model mapping, About resource/text/link presence, icon assignment, and banned Moqi theme/string leakage in Phase 5 surfaces.

### Candidate Data Parsing Style
- **D-38:** Parse and store lookup-filter display data as native equivalents of TypeDuck Web's `CandidateInfo` and `CandidateEntry`. These names are intentionally product/domain terms and should guide the C++ view-model shape.
- **D-39:** Parse raw lookup comments near rendering/view-model creation, not far upstream in the protocol, launcher, or backend transport path. Do not pass fully structured dictionary data through unrelated layers when the raw comment plus basic candidate fields are enough to preserve the boundary.
- **D-40:** Use a small helper like TypeDuck Web's `ConsumedString` for initial control-character parsing (`\v`, first `\f`, optional leading `\r`) so the parser remains readable and testable.
- **D-41:** Hard-coded mappings equivalent to TypeDuck Web `consts.ts` are expected and acceptable for Phase 5: language codes/names/labels, Jyutping visibility labels, definition layout, other-data row labels, pronunciation labels, literary/colloquial readings, registers, parts of speech, labels, and check columns.

### Settings State and Rime Side Effects
- **D-42:** Persist settings in a TypeDuck-owned JSON file shaped like TypeDuck Web `DEFAULT_PREFERENCES`, not in `common.custom.yaml`. The JSON preferences file is the Windows source of truth; Rime custom YAML files are generated side effects for the engine.
- **D-43:** Follow the current TypeDuck Web settings side-effect split: interface-only settings do not update Rime YAML. These are `displayLanguages`, `mainLanguage`, `isHeiTypeface`, `showRomanization`, and `showReverseCode`.
- **D-44:** Rime engine settings must be applied through the same conceptual path as TypeDuck Web `wasm/api.cpp` where possible: use the levers/custom-settings API instead of hand-writing YAML when available in the Windows runtime.
- **D-45:** `pageSize` updates `default.custom.yaml` by customizing `menu/page_size`; it is the one Web-alpha preference that maps to `default.custom.yaml`.
- **D-46:** `enableCompletion`, `enableCorrection`, `enableSentence`, `enableLearning`, and `isCangjie5` update `common.custom.yaml` through the `__patch` list. The current Web mapping is: always include `common:/show_cangjie_roots`; include `common:/disable_completion` when `enableCompletion` is false; include `common:/enable_correction` when `enableCorrection` is true; include `common:/disable_sentence` when `enableSentence` is false; include `common:/disable_learning` when `enableLearning` is false; include `common:/use_cangjie3` when `isCangjie5` is false.
- **D-47:** After any customization that changes Rime custom settings, redeploy/reconfigure the engine before expecting behavior to change. TypeDuck Web calls `Rime.customize(...)` and then `Rime.deploy()`; Windows should preserve the same semantic order and expose a bounded failure state if deploy fails.

### the agent's Discretion
The planner may choose the exact implementation split, drawing API, internal C++ view-model names, JSON schema field names, and settings save-vs-confirm behavior, provided the result stays native/focus-safe for the candidate popup, uses semantic theme names, keeps fonts outside `themes`, keeps only light/dark palettes, treats `input_methods/rime/appearance_themes.json` as canonical after compatibility is proven, mirrors the Web alpha settings/candidate/dictionary behavior from source code, keeps parsing near rendering, persists JSON preferences as source of truth, applies Rime YAML only as generated side effects, redeploys after customization, and preserves the exact About/icon decisions above.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Planning
- `.planning/PROJECT.md` - Product truth: TypeDuck Web alpha is authoritative; Moqi is scaffold only.
- `.planning/REQUIREMENTS.md` - Phase 5 requirement mapping for INST-06, CAND-01 through CAND-07, SET-01 through SET-10, and LANG-03.
- `.planning/ROADMAP.md` - Phase 5 goal, success criteria, dependencies, and VM verification expectations.
- `.planning/STATE.md` - Current project position, Phase 5 closeout, and Phase 6 handoff.
- `.planning/phases/01-identity-and-web-parity-contract/01-VERIFICATION.md` - Human verification that exact UI layout should be followed from TypeDuck Web source code, with screenshots serving as visual references.
- `.planning/phases/02-engine-runtime-contract-spike/02-CONTEXT.md` - Raw lookup-filter CSV/comment contract handoff.
- `.planning/phases/03-zh-hk-tsf-registration-and-installer-skeleton/03-CONTEXT.md` - Installer-first-run settings boundary and TypeDuck installed identity.
- `.planning/phases/04-typeduck-protocol-and-typing-mvp/04-CONTEXT.md` - Phase 4 protocol/candidate payload boundary and deferred Phase 5 native UI work.

### Product Contracts and Fixtures
- `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` - Source-backed settings order, candidate/dictionary layout, visual references, interaction rules, and Web token values.
- `.planning/product/web-alpha-fixtures/2026-06-23/source-metadata.json` - Machine-readable provenance, layout authority list, and screenshot capture metadata.
- `.planning/product/web-alpha-fixtures/2026-06-23/settings-order.json` - Source-backed settings order/defaults/options; pair with `Preferences.tsx` for exact two-column layout behavior.
- `.planning/product/web-alpha-fixtures/2026-06-23/candidate-list-sample.json` - Source/runtime-backed candidate row expectations.
- `.planning/product/web-alpha-fixtures/2026-06-23/dictionary-detail-sample.json` - Source/runtime-backed dictionary detail fields, compound entry evidence, and layout authority.
- `.planning/product/TYPEDUCK-LOOKUP-FILTER-RAW-CONTRACT.md` - Required raw lookup-filter CSV header, separator semantics, and evidence files.
- `.planning/product/TYPEDUCK-BANNED-SURFACES.md` - Banned Moqi/fcitx/cloud/AI/excessive-customization surfaces relevant to theme/settings cleanup.
- `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` - TypeDuck names, installed paths, version/resource identity, and About/installer references.

### TypeDuck Web Alpha Source
- `I:\GitHub\TypeDuck-Web\src\Preferences.tsx` - Settings labels, order, controls, Display Languages behavior, and exact two-column layout source.
- `I:\GitHub\TypeDuck-Web\src\App.tsx` - Surrounding Web alpha page structure around the settings component.
- `I:\GitHub\TypeDuck-Web\src\consts.ts` - Language labels/codes, defaults, Jyutping modes, definition layout, dictionary labels, register, and part-of-speech maps.
- `I:\GitHub\TypeDuck-Web\src\CandidateInfo.ts` - Source model for `CandidateInfo`, `CandidateEntry`, near-rendering comment parsing, dictionary-field mapping, and row helper methods.
- `I:\GitHub\TypeDuck-Web\src\utils.ts` - `ConsumedString` and CSV parsing helpers that should inspire the native parser shape.
- `I:\GitHub\TypeDuck-Web\src\hooks.ts` - `usePreferences()` persistence behavior following `DEFAULT_PREFERENCES`.
- `I:\GitHub\TypeDuck-Web\src\worker.ts` - Current settings-to-customize bitfield mapping and deploy call path.
- `I:\GitHub\TypeDuck-Web\src\types.ts` - Split between `RimePreferences` and `InterfacePreferences`.
- `I:\GitHub\TypeDuck-Web\src\CandidatePanel.tsx` - Candidate panel structure, dictionary reveal state, page nav, and movement-triggered hide/show behavior.
- `I:\GitHub\TypeDuck-Web\src\Candidate.tsx` - Candidate row table structure, Jyutping visibility, main-language definitions, canonical references, and dictionary indicator.
- `I:\GitHub\TypeDuck-Web\src\DictionaryPanel.tsx` - Dictionary entry head/body/table/More Languages rendering.
- `I:\GitHub\TypeDuck-Web\src\index.css` - Candidate/dictionary panel CSS, font stacks, sizing, border, highlight, and table styling.
- `I:\GitHub\TypeDuck-Web\wasm\api.cpp` - `customize()` and `deploy()` source for `default.custom.yaml`, `common.custom.yaml`, levers API use, and redeploy semantics.
- `I:\GitHub\TypeDuck-Web\schema\common.yaml` - Shows `__patch: common.custom:/patch?`, confirming `common.custom.yaml` patch consumption.
- `I:\GitHub\TypeDuck-Web\tailwind.config.ts` - Palette values and font token source only; do not copy Tailwind token names blindly into Windows theme JSON.

### Windows Frontend Code
- `MoqiTextService/MoqiCandidateWindow.cpp` - Current native candidate window drawing, sizing, mouse handling, focus-safe popup style, and legacy colors to replace.
- `MoqiTextService/MoqiCandidateWindow.h` - Candidate UI item model and window methods available for extension.
- `MoqiTextService/MoqiTextService.cpp` - Candidate window ownership, font setup, UI-less policy, candidate state, placement, and update hooks.
- `MoqiTextService/MoqiTextService.h` - Product UI state fields for candidate list/page/preedit/settings integration.
- `MoqiTextService/MoqiClient.cpp` - Backend response handling, `CandidateEntry` conversion, `CustomizeUi` application, and settings protocol bridge.
- `proto/moqi.proto` - Current TypeDuck protocol fields for candidate entries, candidate page, settings snapshot/update, capabilities, health, and error data.
- `Preview/main.cpp` - Candidate preview harness useful for local layout debugging; screenshot output is no longer a Phase 5 acceptance gate.

### Backend Runtime and Theme Code
- `D:\VSProjects\moqi-ime\input_methods\rime\appearance_themes.json` - Current embedded builtin theme source file to replace with TypeDuck light/dark semantic theme contract.
- `D:\VSProjects\moqi-ime\input_methods\rime\appearance_themes.go` - Current loader search order, embedded fallback, theme registry, and theme application hooks.
- `D:\VSProjects\moqi-ime\input_methods\rime\appearance_config.go` - Current appearance/settings persistence, customize UI map, candidate count config write, and scaffold customization surface.
- `D:\VSProjects\moqi-ime\input_methods\rime\rime.go` - Existing backend style/defaults and engine option handling that may need alignment with TypeDuck preferences.
- `D:\VSProjects\moqi-ime\scripts\build.ps1` - Current package step that copies `appearance_themes.json` into both `input_methods/rime` and `input_methods/rime/data`.
- `D:\VSProjects\moqi-ime\icons\About_Banner.bmp` - Required top About dialog banner source asset.
- `D:\VSProjects\moqi-ime\icons\Credit_Logos.bmp` - Required About dialog credit/logo source asset.
- `D:\VSProjects\moqi-ime\icons\TypeDuck_Transparent.ico` - Required executable icon source for launcher, setup helper, and backend server.
- `D:\VSProjects\moqi-ime\icons\TypeDuck_Small.ico` - Required system input method picker icon source.
- `D:\VSProjects\moqi-ime\icons\TypeDuck.ico` - Required installer/uninstaller and broad branding icon source.
- `D:\VSProjects\moqi-ime\icons\Installer.bmp` - Installer bitmap source; explicitly deferred to later installer phases.
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
- `I:\GitHub\TypeDuck-Web\src\CandidateInfo.ts`: Provides the intended data model shape and proves parsing can live next to rendering-facing view models rather than deep in transport.
- `I:\GitHub\TypeDuck-Web\src\utils.ts`: Provides a compact `ConsumedString` pattern for readable control-character parsing.
- `I:\GitHub\TypeDuck-Web\wasm\api.cpp`: Provides the current TypeDuck Web levers/custom-settings behavior for settings that affect Rime files and deploy.
- `D:\VSProjects\moqi-ime\input_methods\rime\appearance_themes.go`: Already supports embedded builtin themes, filesystem override loading, user theme merge, and theme application into backend style fields.
- `D:\VSProjects\moqi-ime\input_methods\rime\appearance_config.go`: Already writes local appearance config and exposes `customizeUIMap()` to the Windows frontend, though much of its scaffold customization is broader than TypeDuck v1.
- `D:\VSProjects\moqi-ime\icons\About_Banner.bmp` and `D:\VSProjects\moqi-ime\icons\Credit_Logos.bmp`: Existing source assets for the About dialog, but implementation should move/copy them into a semantically appropriate TypeDuck resource/assets location.
- `D:\VSProjects\moqi-ime\icons\TypeDuck_Transparent.ico`, `TypeDuck_Small.ico`, and `TypeDuck.ico`: Existing source icon assets with distinct usage rules for executables, input picker, and installer/uninstaller/branding.
- `Preview/main.cpp`: Useful for rapid candidate-panel visual iteration before VM/host-app verification.

### Established Patterns
- Candidate popup rendering is native Win32/GDI today and uses `WS_EX_NOACTIVATE`; preserving this focus model is more important than adopting a richer toolkit.
- Backend theme data currently lives in the sibling `moqi-ime` runtime and is copied into this repo's installer stage; Phase 5 may need coordinated changes across both repos.
- The current appearance theme JSON is scaffold/Moqi-oriented, Simplified-labeled, and overly broad. It conflicts with the TypeDuck scope of Web-alpha parity and limited customization.
- TypeDuck Web uses table markup for candidates and dictionary details, but the native implementation can represent the same structure with measured rows/columns and semantic view models instead of an actual toolkit table widget.
- TypeDuck Web splits preferences into Rime-affecting preferences and interface-only preferences. Windows should keep the same conceptual split even if it uses a native JSON file instead of browser local storage.
- `common.custom.yaml` is consumed as a Rime patch side effect, not as the settings state database. Treating it as the source of truth would lose UI-only preferences and make Web parity brittle.
- Phase 1 verification already locks source-code-backed layout authority: screenshots prove visual sufficiency, while implementation details must be checked against TypeDuck Web source and `.planning/product` fixtures.
- The current TypeDuck icon/bitmap assets live in the sibling backend `icons` directory; Phase 5 should treat that as a source location, not the final semantic resource organization.

### Integration Points
- Candidate/dictionary view-model mapping connects `proto::CandidateEntry.raw_lookup_comment` and structured candidate fields to native row/dictionary rendering.
- Settings UI connects installer `[Run]` or setup flow, post-install entry point, TypeDuck-owned JSON preferences, backend settings update protocol, generated Rime custom config, and redeploy/reconfigure.
- Rime-affecting settings connect `pageSize` to `default.custom.yaml` and the option patch list to `common.custom.yaml`; interface-only settings connect only to JSON persistence and native rendering.
- About UI connects resource packaging, exact bilingual content, external link opening, executable/icon resources, and product attribution.
- Icon assignment connects CMake/resources for `TypeDuckLauncher.exe`, `TypeDuckSetupHelper.exe`, backend `server.exe`, TSF profile/input-picker icon metadata, and installer/uninstaller branding.
- Theme JSON changes connect the sibling backend source, backend loader search order, installer staging, and frontend `CustomizeUi`/native rendering tokens.
- Verification now connects user human verification with automated parser/settings/theme/window/package guards. Screenshot fixtures under `.planning/product/ui-fixtures` are retired for Phase 5.

</code_context>

<specifics>
## Specific Ideas

- User direction: color names should be semantic. Do not preserve the exact color names from TypeDuck Web `tailwind.config.ts` when creating Windows/runtime theme schema.
- User direction: `appearance_themes.json` should include the two light/dark color palettes and font information, but font information should live outside the `themes` key.
- User direction: settings dialog should follow exactly the same two-column layout as TypeDuck Web. Confirm button versus immediate save is implementer discretion.
- User direction: About dialog must use `About_Banner.bmp`, exact provided bilingual text, `Credit_Logos.bmp`, and the four specified links.
- User direction: TypeDuck icon assignments are fixed: `TypeDuck_Transparent.ico` for executables except installer/uninstaller, `TypeDuck_Small.ico` for system input method picker, and `TypeDuck.ico` for installer/uninstaller/branding.
- User direction: legacy Moqi images `moqi.png`, `mo.ico`, `mo.png`, and `moqi.ico` must not be used anywhere.
- User direction: data should be parsed and stored as `CandidateInfo` and `CandidateEntry`; parsing should happen near rendering; use a helper like TypeDuck Web `ConsumedString`; hard-coded mapping tables like `consts.ts` are expected.
- User direction: save settings as JSON following TypeDuck Web `DEFAULT_PREFERENCES`; do not treat `common.custom.yaml` as persistence state; use Rime custom YAML only as generated side effects and remember to redeploy after customization.
- Current TypeDuck Web setting evidence: `pageSize` writes `menu/page_size` via `default.custom.yaml`; `enableCompletion`, `enableCorrection`, `enableSentence`, `enableLearning`, `isCangjie5`, and always-show Cangjie roots write `common.custom.yaml` patch items; `displayLanguages`, `mainLanguage`, `isHeiTypeface`, `showRomanization`, and `showReverseCode` are interface-only preferences.
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
- `Installer.bmp` belongs with later installer branding/polish work, not Phase 5 unless installer-first-run settings specifically need to reference it.

</deferred>

---

*Phase: 5-Candidate, Dictionary, Settings, and About UI Parity*
*Context gathered: 2026-06-24T00:46:20Z*
