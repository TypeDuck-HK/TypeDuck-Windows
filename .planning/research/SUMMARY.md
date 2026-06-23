# Project Research Summary

**Project:** TypeDuck Windows IME
**Domain:** Windows Text Services Framework Cantonese IME rewrite from Moqi scaffold to TypeDuck
**Researched:** 2026-06-23
**Confidence:** HIGH for product direction and feature scope; MEDIUM-HIGH overall because TypeDuck-HK librime/plugin integration still needs Windows build proof.

## Executive Summary

TypeDuck Windows IME is a native Windows TSF Cantonese input method, not a continuation of the Moqi product. The existing Moqi repository should be treated as a Windows integration scaffold: keep the COM/TSF DLL shape, launcher/backend process isolation, candidate-window ownership patterns, SetupHelper, Inno installer, CMake/Visual Studio build, and dual-bitness registration mechanics; replace the product identity, engine, protocol, user-facing UI, installer language, and feature set with TypeDuck-specific behavior.

The recommended approach is to preserve the out-of-process launcher boundary for v1 and put a new TypeDuck engine host behind it. That engine host should embed the TypeDuck-HK librime fork, package TypeDuck schemas/dictionaries, load `rime-dictionary-lookup-filter`, and emit structured candidate plus dictionary lookup payloads. TypeDuck Web alpha is the acceptance source for settings, candidate/dictionary UX, bilingual copy, visual tone, and display-language behavior.

The major risks are building on the wrong identity, registering under the wrong Windows language, flattening dictionary data into old candidate comments, and polishing UI before the real engine/plugin data path exists. Mitigate these by locking a TypeDuck identity and Web parity contract first, proving librime/plugin output early, making zh-HK TSF registration deterministic in first-party Windows code, and versioning a TypeDuck protocol before rich candidate UI work.

## Key Findings

### Recommended Stack

Keep the Windows-native stack and replace the Moqi product layer. The scaffold's C++20, Win32/COM/TSF, libIME2 foundation, launcher/backend IPC, CMake, Visual Studio 2022, PowerShell packaging, Inno Setup 6, and SetupHelper are valuable. The product runtime above that foundation should move to TypeDuck-HK librime, TypeDuck schema assets, `rime-dictionary-lookup-filter`, TypeDuck protocol messages, and native bilingual TypeDuck settings/candidate/about UI.

**Core technologies:**
- C++20 + Win32/COM/TSF: native IME host integration, composition, key handling, and profile registration.
- libIME2 TSF layer: scaffold for activation, edit sessions, language profiles, display attributes, and candidate UI plumbing.
- Out-of-process launcher/backend boundary: isolates librime/plugin crashes from arbitrary host applications.
- TypeDuck-HK librime fork: required Cantonese engine behavior and schema compatibility.
- `rime-dictionary-lookup-filter`: required source for translations and dictionary-like candidate details.
- Protobuf framed IPC: keep the pattern, but replace Moqi-shaped messages with `typeduck.protocol.v1` and frame limits.
- Native Win32 candidate/settings/about UI: best fit for installer integration, TSF constraints, and bilingual Windows UX.
- Inno Setup + SetupHelper: retain for dual-bitness TSF install, COM registration, startup integration, and uninstall cleanup.

**Critical version and dependency requirements:**
- Pin exact TypeDuck-HK librime and lookup-filter commits before UI work depends on them.
- Preserve x86 and x64 TSF DLL registration; Windows host processes can be either bitness.
- Register the profile under Chinese (Traditional, Hong Kong) / `zh-HK`, not Simplified Chinese.
- Use TypeDuck Web alpha settings and candidate/dictionary behavior as dated acceptance fixtures.

### Expected Features

**Must have for v1:**
- TypeDuck product identity across binaries, resources, installer, registry/profile metadata, data/log paths, pipe/mutex names, tray/menu surfaces, and release artifacts.
- Deterministic TSF registration under Chinese (Traditional, Hong Kong) with TypeDuck-owned CLSID/profile GUID and bilingual profile text.
- TypeDuck-HK librime runtime with TypeDuck schemas/dictionaries and lookup-filter support.
- Reliable Cantonese composition, candidate selection, paging, numeric selection, commit, and responsive key handling.
- Structured candidate list containing Chinese candidate, input code, Jyutping, comments/prompts, translations, and lookup references.
- Dictionary/detail panel for selected candidates with headword, Jyutping, part-of-speech, English meanings, reading notes, and enabled-language rows.
- Native settings dialog matching TypeDuck Web alpha, with Display Languages first.
- Bilingual Traditional Hong Kong Chinese and English text on every user-facing surface.
- Installer flow that shows settings during install.
- About dialog with TypeDuck identity, version, engine/schema attribution where available.
- Explicit removal or hiding of Moqi, fcitx, WebDAV/cloud clipboard, AI controls, Simplified-only installer text, and excessive customization.

**Should have or protect as differentiators:**
- Web alpha parity across settings, candidate data shape, dictionary panel behavior, and visual tone.
- Rich dictionary-like candidate context as the primary TypeDuck differentiator over generic Cantonese IMEs.
- Multilingual prompts/translations for the Web alpha language set: English, Hindi, Indonesian, Nepali, and Urdu.
- User-controlled Jyutping visibility, candidate count, Chinese typeface, full input code visibility, reverse lookup, Cangjie version, input memory, auto-completion, auto-correction, and auto-composition where engine support is verified.
- Typed-content-safe diagnostics and support logs.

**Defer to v2+:**
- Cloud sync for settings/input memory.
- Additional display languages beyond Web alpha.
- Advanced keyboard remapping and custom selection layouts.
- Theme packs or broad candidate-window customization.
- In-app updater/release-channel UI.
- Enterprise deployment controls.
- AI writing, translation, or explanation features.

### Architecture Approach

Use a layered architecture: keep a thin TSF DLL in host processes, route engine work through a per-user launcher, and run TypeDuck-HK librime plus lookup filter in a dedicated TypeDuck engine host. Make profile registration first-party and deterministic instead of backend-JSON-owned. Replace the old Moqi protocol with a versioned TypeDuck protocol that carries structured candidate, dictionary, settings, capability, error, and health data.

**Major components:**
1. TypeDuck TSF service DLL: handles COM/TSF activation, key sinks, composition, profile hooks, candidate UI lifecycle, and settings/about entry points.
2. TypeDuck launcher: maintains one per-user/session IPC endpoint, supervises the engine process, owns pipe/mutex/log path constants, and avoids legacy cloud/tray behavior unless explicitly needed.
3. TypeDuck engine host: embeds TypeDuck-HK librime, loads schemas and lookup filter, owns engine sessions, maps plugin output to protocol messages, and validates/applies settings.
4. TypeDuck protocol adapter: performs version handshake, bounded frame handling, deadlines, candidate/dictionary mapping, settings RPC, and degraded-mode behavior.
5. Candidate and dictionary UI: uses renderer-neutral view models, native drawing, Web-alpha-like compact layout, warm amber accents, HK Chinese/English fonts, and TSF candidate UI element consistency.
6. Settings/about UI: fixed first-party native dialog or executable with bilingual labels, Display Languages first, settings persistence, engine validation, install-time mode, and version attribution.
7. Installer and registration flow: Inno Setup plus SetupHelper, renamed and scoped to TypeDuck, with dual-bitness registration, zh-HK profile verification, installer settings launch, upgrade/uninstall cleanup, and fresh-VM evidence.

### Critical Pitfalls

1. **Deferring the engine boundary** — prove TypeDuck-HK librime plus lookup-filter output before candidate UI or settings depend on it.
2. **Treating Moqi-to-TypeDuck as cosmetic rename** — create and audit a TypeDuck identity contract covering binaries, CLSIDs, GUIDs, paths, pipes, logs, registry, installer, resources, protocol, and artifacts.
3. **Letting backend `ime.json` own zh-HK registration** — compile or stage TypeDuck profile identity in first-party Windows code so missing backend payloads cannot change the Windows language profile.
4. **Squeezing dictionary lookup into candidate comments** — version the protocol and use typed `Candidate` and `DictionaryLookup` data structures before rendering the new UI.
5. **Building candidate UI without TSF host compatibility evidence** — use a preview harness for layout, but accept UI only after testing real host apps, DPI, focus, placement, and `ITfCandidateListUIElement` behavior.
6. **Remembering Web alpha parity instead of encoding it** — capture dated settings inventories, screenshots, candidate fixtures, defaults, labels, and visual tokens early.
7. **Shipping hidden scaffold features** — remove or compile-gate fcitx, WebDAV/cloud clipboard, AI, Moqi menus, Simplified-only strings, and arbitrary backend config-tool launch paths.

## Implications for Roadmap

Based on research, use seven vertical phases. The order intentionally puts identity, engine proof, zh-HK installability, and protocol shape before rich UI polish.

### Phase 1: Identity and Web Parity Contract

**Rationale:** Every later phase depends on knowing what TypeDuck is, what Moqi surfaces are banned, and what Web alpha behavior must be matched.
**Delivers:** TypeDuck identity contract, banned-surface list, Web alpha settings inventory, candidate/dictionary fixture plan, bilingual copy rules, visual parity references, and initial static audits.
**Addresses:** Product identity replacement, bilingual UI requirement, anti-feature cleanup policy, Web alpha source-of-truth requirement.
**Avoids:** Cosmetic Moqi rename; Web alpha parity remembered but not encoded; hidden scaffold features.

### Phase 2: Engine Runtime Contract Spike

**Rationale:** Candidate/dictionary UI cannot be real until TypeDuck-HK librime and lookup filter produce the required structured data on Windows.
**Delivers:** Pinned TypeDuck-HK librime and lookup-filter commits, Windows build proof, minimal TypeDuck engine host or spike executable, schema/plugin deployment proof, golden output fixtures for Cantonese inputs, and written boundary decision.
**Uses:** TypeDuck-HK librime, `rime-dictionary-lookup-filter`, TypeDuck schemas/dictionaries, current launcher/backend isolation pattern.
**Addresses:** TypeDuck engine integration, lookup-filter support, Cantonese candidate data source.
**Avoids:** Deferred engine boundary; underestimated older librime fork; mocked dictionary UI.
**Research flag:** Needs deeper research during phase planning because fork build behavior, plugin ABI, deployment paths, and lookup-filter payload shape are still the least proven areas.

### Phase 3: zh-HK TSF Registration and Installer Skeleton

**Rationale:** A Windows IME is not viable until it installs, registers, unregisters, and appears under the correct Hong Kong Traditional Chinese profile.
**Delivers:** TypeDuck CLSID/profile GUID constants, `zh-HK` locale contract, renamed installer skeleton, TypeDuck paths, SetupHelper updates, dual-bitness registration, clean install/uninstall smoke evidence, and settings-launch placeholder if the real dialog is not ready.
**Uses:** Existing libIME2 registration scaffolding, SetupHelper, Inno Setup, PowerShell packaging flow.
**Addresses:** Native Windows TSF registration, TypeDuck profile identity, installer direction, uninstall/reinstall cleanup.
**Avoids:** Backend JSON-owned profile identity; installer treated as end-of-project packaging; wrong Simplified Chinese registration.

### Phase 4: TypeDuck Protocol and Candidate/Dictionary Model

**Rationale:** The old text/comment protocol cannot support TypeDuck dictionary parity, multilingual rows, settings, capabilities, or reliable IPC.
**Delivers:** `typeduck.proto`, protocol version handshake, max frame sizes, request deadlines, candidate metadata, dictionary lookup messages, settings/capabilities messages, mapper tests, malformed payload tests, and golden serialized fixtures from real engine output.
**Uses:** Existing framed protobuf pattern, renamed pipe/client/server adapters, engine spike output.
**Addresses:** Structured candidate metadata, dictionary/detail data, settings transport, page state, reverse lookup state.
**Avoids:** Dictionary data flattened into comments; unbounded IPC framing; key-path blocking without deadlines.
**Research flag:** Needs focused protocol/security research during phase planning for frame caps, named-pipe ACLs, malformed input behavior, and TSF key-path latency.

### Phase 5: Candidate, Dictionary, Settings, and About UI Parity

**Rationale:** Once real data and settings contracts exist, the visible TypeDuck UX can be built against fixtures instead of guesses.
**Delivers:** Native candidate list, dictionary panel, view models, renderer harness, Web-alpha-like visual styling, bilingual settings dialog with Display Languages first, runtime settings persistence/apply flow, installer first-run settings mode, About dialog, and host-app screenshot matrix.
**Uses:** Candidate/dictionary protocol model, settings RPC, Web alpha fixture contract, native Win32 UI.
**Addresses:** Candidate UI parity, dictionary panel, display-language picker, settings parity, bilingual copy, TypeDuck visual tone, About dialog.
**Avoids:** UI-only preview acceptance; settings divergence; excessive customization beyond Web alpha.
**Research flag:** Needs phase-level UI research against live TypeDuck Web alpha before implementation because the alpha can change and screenshots/defaults should be refreshed.

### Phase 6: Privacy, Security, and Scaffold Cleanup

**Rationale:** The inherited scaffold contains privacy-sensitive and off-scope behavior that must be removed at source boundaries, not just hidden.
**Delivers:** Removal or hard compile-gating of cloud clipboard/WebDAV/AI/fcitx/Moqi product paths, arbitrary config-tool launch removal, log scrubbing, TypeDuck-namespaced diagnostics, named-pipe hardening, banned-string audits, and runtime proof that legacy listeners are absent.
**Addresses:** Anti-feature cleanup, typed-content-safe diagnostics, local-only input memory privacy, user trust.
**Avoids:** Hidden scaffold features shipping; logs leaking typed content; backend-provided executable launch.

### Phase 7: Compatibility and Release Verification

**Rationale:** Windows IMEs fail in host-specific edge cases; release readiness requires evidence across install, host apps, DPI, bitness, protocol recovery, and cleanup paths.
**Delivers:** Fresh VM clean install/upgrade/uninstall/reinstall transcripts, reboot-required replacement test, x86/x64 registration evidence, host app matrix for Notepad, Office, Chrome/Edge, terminal, UWP/immersive, elevated app, multi-monitor/high-DPI screenshots, typing latency measurements, backend crash/restart tests, CTest/CI output, and final Web alpha parity audit.
**Addresses:** Installer reliability, TSF host compatibility, protocol resilience, performance, release confidence.
**Avoids:** Existing COM helper tests being mistaken for product coverage; developer-machine-only validation.

### Phase Ordering Rationale

- Identity and parity contract come first because product naming, banned surfaces, zh-HK locale, bilingual copy, and Web alpha fixtures affect every later file touched.
- Engine proof comes before rich UI because the UI needs real TypeDuck candidate and dictionary fields from librime plus lookup filter, not mocks or old Moqi comments.
- TSF registration and installer skeleton happen early because correct installability is core product behavior, not packaging polish.
- Protocol/model work precedes UI rendering so dictionary details, multilingual rows, Jyutping modes, settings, and paging are structured and testable.
- UI follows protocol and engine because settings and candidate controls must map to real engine behavior.
- Privacy/security cleanup and compatibility verification are explicit phases because inherited scaffold features and Windows host-process edge cases are high-risk release blockers.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 2:** TypeDuck-HK librime fork build, lookup-filter ABI/loading, schema deployment, and actual dictionary payload extraction.
- **Phase 4:** Protocol security, frame caps, named-pipe ACLs, key-path latency, degraded-mode behavior, and malformed payload handling.
- **Phase 5:** Live Web alpha parity refresh, native candidate panel layout constraints, accessibility/TSF UI element behavior, and installer settings UX.
- **Phase 7:** Host-app compatibility matrix, Windows 10/11 install/uninstall mechanics, reboot replacement, and high-DPI/multi-monitor verification.

Phases with standard patterns where planning can usually skip a separate research phase:
- **Phase 1:** Identity inventory and parity contract are project-specific synthesis work already well covered by current research.
- **Phase 3:** Inno Setup, SetupHelper, TSF registration scaffolding, and dual-bitness packaging patterns are documented in the repo; planning should inspect code rather than conduct broad external research.
- **Phase 6:** Static banned-string audits, log scrubbing, and removal of unused feature paths are standard engineering cleanup once scope is fixed.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | MEDIUM-HIGH | Windows scaffold choices are clear and repo-backed; TypeDuck-HK librime plus lookup-filter integration needs a clean Windows build and runtime proof. |
| Features | HIGH | Requirements, Web alpha parity targets, anti-features, and MVP scope are explicit and consistent across research. |
| Architecture | HIGH | Current TSF/launcher/backend/installer boundaries are well understood; recommended out-of-process engine boundary aligns with Windows IME risk. |
| Pitfalls | MEDIUM-HIGH | Pitfalls are concrete and repo-specific, but several depend on validating actual Windows host behavior and older fork/plugin behavior. |

**Overall confidence:** MEDIUM-HIGH

### Gaps to Address

- TypeDuck-HK librime fork build details: pin commits and prove Windows x86/x64 compatible runtime packaging before broad implementation.
- Lookup-filter payload shape: confirm whether the plugin exposes all dictionary metadata needed by Web alpha or whether extra dictionary parsing is required.
- Exact zh-HK registration mapping: verify Windows language/profile APIs, fallback locale behavior, and display placement on clean Windows 10/11 installs.
- Web alpha drift: refresh settings/defaults/screenshots immediately before phase planning for UI and settings.
- Settings storage schema: decide stable local settings path, names, migration behavior, and engine deploy/restart semantics.
- Candidate panel host behavior: validate placement, focus, UI-less mode, high DPI, and accessibility in representative real apps.
- Privacy/logging policy: define exactly what default diagnostics may include and prove no raw typed content is written.
- Release/deployment policy: signing, update channel, stable/beta side-by-side behavior, and enterprise deployment remain out of current v1 scope unless product direction changes.

## Sources

### Primary
- `.planning/PROJECT.md` — TypeDuck product target, validated requirements, active scope, constraints, Web alpha context, and out-of-scope decisions.
- `.planning/research/STACK.md` — recommended Windows/native stack, engine boundary, build/packaging choices, and TypeDuck dependency notes.
- `.planning/research/FEATURES.md` — table-stakes features, differentiators, anti-features, MVP definition, and feature dependencies.
- `.planning/research/ARCHITECTURE.md` — TSF/launcher/engine architecture, component boundaries, protocol needs, installer/registration contract, and migration path.
- `.planning/research/PITFALLS.md` — critical failure modes, phase mapping, verification evidence, and release-risk checklist.

### Secondary
- `.planning/codebase/ARCHITECTURE.md` — current scaffold architecture and extension points referenced by researchers.
- `.planning/codebase/STRUCTURE.md` — file ownership and likely implementation locations referenced by researchers.
- `.planning/codebase/INTEGRATIONS.md` — launcher/backend, installer, TSF, logging, and legacy integration surfaces referenced by researchers.
- `.planning/codebase/CONCERNS.md` — known scaffold risks, TypeDuck gaps, security/performance concerns, and cleanup needs referenced by researchers.
- `.planning/codebase/TESTING.md` — current test gaps and TypeDuck verification guidance referenced by researchers.

### External and Product References
- TypeDuck Web alpha at `http://localhost:5173/TypeDuck-Web/aap2-alpha/` — source of truth for settings, candidate/dictionary UX, and visual tone.
- Local TypeDuck Web source at `I:\GitHub\TypeDuck-Web` — referenced files include `Preferences.tsx`, `CandidatePanel.tsx`, `Candidate.tsx`, `DictionaryPanel.tsx`, `CandidateInfo.ts`, `worker.ts`, TypeDuck schema YAML, dictionaries, and OpenCC assets.
- Microsoft Learn TSF and COM registration documentation — referenced by pitfalls research for profile registration, `ITfCandidateListUIElement`, and `regsvr32` behavior.
- Rime/librime upstream and TypeDuck-HK dependency context — useful background, but TypeDuck-HK fork behavior must be verified directly.

---
*Research completed: 2026-06-23*
*Ready for roadmap: yes*
