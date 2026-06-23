# Feature Research

**Domain:** Windows Cantonese IME for TypeDuck
**Researched:** 2026-06-23
**Confidence:** HIGH

## Feature Landscape

### Table Stakes (Users Expect These)

Features users assume exist. Missing these means TypeDuck Windows IME does not meet the v1 promise.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Native Windows TSF IME registration under Chinese (Traditional, Hong Kong) | Users must be able to select TypeDuck from the Windows language/input picker in the correct Hong Kong Traditional Chinese locale. | HIGH | Requires deterministic TypeDuck profile metadata, GUIDs, installer registration, and removal of Simplified Chinese defaults. Do not depend only on backend `ime.json` discovery. |
| TypeDuck-HK librime engine integration | The product target is TypeDuck Cantonese behavior, not Moqi backend behavior. | HIGH | Engine boundary must be settled early. Use the TypeDuck-HK librime fork and package TypeDuck schemas/dictionaries as first-party runtime assets. |
| Cantonese composition, candidate selection, paging, and commit | A Cantonese IME is unusable without reliable preedit text, candidate navigation, numeric selection, paging, and commit behavior. | HIGH | Keep TSF key handling responsive; avoid blocking host apps on long backend calls. Candidate count must honor TypeDuck settings. |
| Candidate list with Chinese candidate, input code, Jyutping, comments, prompts, and translations | TypeDuck Web alpha exposes richer candidate data than a plain IME list; Windows v1 must carry the same information. | HIGH | Requires protocol/view-model fields beyond generic candidate text/comment. Candidate window must be redesigned around structured candidate metadata. |
| Dictionary lookup filter support | The known target explicitly requires `TypeDuck-HK/rime-dictionary-lookup-filter` so dictionary-like data can appear in the UI. | HIGH | Backend must emit lookup entries with term, Jyutping, part of speech, meanings, reading notes, and more-language rows. |
| Dictionary/detail panel for selected candidate | Web alpha screenshots show a candidate list plus a dictionary panel; users will expect the same lookup experience. | HIGH | Build as a compact native panel tied to candidate selection. Do not squeeze all dictionary data into a one-line comment. |
| Settings dialog mirroring TypeDuck Web alpha | Users should see the same concepts and defaults across Web and Windows. | MEDIUM | Include Display Languages, Candidates Jyutping, candidate count, Chinese typeface, auto-completion, auto-correction, auto-composition, input memory, reverse lookup, full input code, and Cangjie version. |
| Display language picker | Multilingual prompt/translation behavior depends on user-selected languages, and the user called this the first priority. | MEDIUM | Must include English, Hindi, Indonesian, Nepali, and Urdu with enablement checkboxes plus a main-language marker, matching Web alpha semantics. |
| Bilingual Traditional Hong Kong Chinese and English UI copy | Every user-facing surface must be understandable to Hong Kong users and English readers. | MEDIUM | Applies to installer, settings, About, tray/menu, error messages, candidate controls, and setup prompts. Prefer paired labels such as `顯示語言 / Display Languages`. |
| Installer shown settings flow | Users need to configure language/display behavior during installation, not hunt for a separate tool after install. | MEDIUM | Installer should launch the TypeDuck settings dialog or an install-time equivalent, with Display Languages first. |
| TypeDuck visual identity and Web alpha look-and-feel | v1 is a TypeDuck product, not a renamed Moqi binary. | MEDIUM | Light neutral surfaces, warm amber accents, compact rounded controls, readable HK Chinese/English fonts, and Web-alpha-like candidate spacing. |
| About dialog | Users need product identity, version, project attribution, and support/update context. | LOW | Keep concise and bilingual. Include TypeDuck Windows IME version and relevant engine/schema version where available. |
| Clean TypeDuck product identity across binaries, installer, paths, tray, logs, and artifacts | A native installer that says Moqi, writes Moqi paths, or exposes Moqi tray text breaks trust. | MEDIUM | Rename coherently across CMake, resources, installer, startup registry, log paths, pipe names, and release artifact names. |
| Uninstall and reinstall cleanup | IMEs are system-integrated; stale TSF profiles or old DLLs cause user-visible breakage. | MEDIUM | Must clean TypeDuck-owned registry/profile entries and avoid leaving Moqi leftovers once renamed. |
| Basic diagnostics without typed-content leakage | Support needs logs, but input methods handle sensitive text. | MEDIUM | Logs should be opt-in, bounded, TypeDuck-namespaced, and avoid recording candidate/content details unless explicitly enabled for debugging. |

### Differentiators (TypeDuck-Specific Value)

Features that make this more than a generic Cantonese IME. These should be protected in roadmap scope.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Web alpha parity across Windows and Web | Users can move between TypeDuck Web and native Windows without relearning settings or candidate interpretation. | HIGH | Treat Web alpha source files and screenshots as acceptance references for settings, candidate data, and visual behavior. |
| Rich dictionary-like candidate context | TypeDuck can teach and disambiguate, not just input characters. | HIGH | Show large Chinese terms, Jyutping, part-of-speech tags, English meanings, colloquial/literary notes, and more-language meanings. |
| Multilingual prompts and translations | TypeDuck serves multilingual learners and users beyond English-only Cantonese explanations. | HIGH | Depends on display-language settings and lookup-filter payloads. v1 should support the Web alpha language set rather than an open-ended language marketplace. |
| User-controlled Jyutping visibility | Users can choose between learning support and compact candidate UI. | MEDIUM | Mirror Web alpha modes: Always Show, Only in Reverse Lookup, Hide. This setting should affect both list and detail presentation consistently. |
| Cantonese-focused reverse lookup settings | Users can discover Chinese terms from codes/readings and inspect dictionary data in the same flow. | MEDIUM | Requires clear UI state and engine/schema support. Avoid generic Moqi/fcitx menu concepts. |
| Cangjie version choice for hybrid workflows | Hong Kong users may expect Cangjie 3/5 behavior when TypeDuck exposes it in Web alpha. | MEDIUM | Include only the Web alpha-supported choices. Do not expand into arbitrary shape-code schemes for v1. |
| Chinese typeface choice, Sung or Hei | Candidate readability matters for Chinese text and is visible in Web alpha. | LOW | Implement as a simple display preference, not a full font picker. |
| Input memory aligned with TypeDuck behavior | Personalized candidate ranking can make the IME feel native and efficient. | HIGH | Keep user data local by default. Needs explicit storage location, reset behavior, and privacy language. |
| Auto-completion, auto-correction, and auto-composition toggles | Power users can tune typing assistance without leaving the TypeDuck model. | MEDIUM | Implement only if engine behavior exists and can be tested. Defaults should match Web alpha. |
| Full input code visibility | Advanced users and learners can inspect how candidates map to codes. | LOW | Should be a direct setting and reflected in candidate rows/detail panel. |

### Anti-Features (Deliberately Do Not Build or Show)

These are not v1 features. Keeping them out is part of delivering TypeDuck cleanly.

| Anti-Feature | Why It Might Be Requested | Why Problematic | Alternative |
|--------------|---------------------------|-----------------|-------------|
| Visible Moqi branding, Moqi paths, Moqi tray labels, or Moqi release artifact names | The scaffold already contains them and renaming takes cross-layer work. | Users would receive the wrong product, and support/debugging would be confused by mixed identities. | Replace with a single TypeDuck identity contract before user-facing release. |
| User-visible fcitx references, docs, menus, or feature labels | The inherited project mentions fcitx concepts and may expose backend-driven controls. | TypeDuck Windows IME is a TSF product; fcitx leakage feels irrelevant and unfinished on Windows. | Remove from UI/docs and keep only internal technical references if absolutely needed for dependency attribution. |
| WebDAV or cloud clipboard | Existing launcher has cloud clipboard plumbing, and sync can sound valuable. | It is outside scope, creates privacy/security risk, and distracts from Cantonese IME parity. | Do not bundle or expose it in v1. If ever reconsidered, require explicit consent, privacy design, and separate roadmap research. |
| AI controls or AI config actions | Modern IMEs sometimes market AI writing help. | User explicitly called these scaffold leftovers; they dilute TypeDuck's core input/dictionary experience and add unclear data handling. | Remove/hide entirely. Consider future AI only as a separate product decision after v1 stability. |
| Excessive customization beyond Web alpha settings | Power users may ask for every font, color, key, backend, or UI option. | More settings slow the rewrite, complicate testing, and break cross-platform parity. | Ship the Web alpha settings set only; add new settings only when TypeDuck Web also adopts them. |
| Generic backend-provided config tool launcher | The scaffold can launch tools declared in backend metadata. | Trusting arbitrary backend config tools is a security and UX risk. | Use one signed/native TypeDuck settings dialog with a fixed settings model. |
| Installing as Chinese (Simplified) or exposing Simplified installer chrome | The inherited installer already uses Simplified Chinese assets. | Explicitly conflicts with the Hong Kong Traditional Chinese requirement. | Install/register under Chinese (Traditional, Hong Kong) with Traditional Hong Kong Chinese and English strings. |
| Multiple Moqi/fcitx input schemes in one Windows package | The scaffold can discover multiple backend input methods. | v1 success depends on one coherent TypeDuck Cantonese experience, not a bundle of unrelated schemes. | Ship the TypeDuck Cantonese profile and only the schema choices required by Web alpha. |
| Remote/account sync in v1 | Input memory and language preferences might seem useful across devices. | Requires account, security, privacy, conflict handling, and support work outside current requirements. | Keep settings and input memory local. Export/import can be researched for v2. |
| Full theme marketplace or custom CSS-like styling | Web alpha has a recognizable visual style, and users may want tweaks. | Theme systems create visual regression risk and undermine TypeDuck parity. | Provide only the Web alpha-supported display controls, such as typeface and candidate count. |
| Shipping Moqi legacy backend as a fallback | It may make early builds appear functional. | It would validate the wrong behavior and hide librime integration gaps. | Build a thin TypeDuck backend/runtime path early, even if feature-limited. |

## Deferred v2 Candidates

| Feature | Why Defer | Dependency Before Reconsidering |
|---------|-----------|----------------------------------|
| Cloud sync for settings and input memory | Requires accounts, consent, privacy model, storage security, conflict resolution, and support processes. | Stable local settings/input memory plus product decision on account infrastructure. |
| Advanced keyboard remapping and custom selection keys | Useful for power users but expands QA across host apps and TSF edge cases. | v1 key handling stability, telemetry/support evidence, and a bounded settings design. |
| Additional display languages beyond Web alpha | Valuable for broader learning use cases but translation quality and UI fit need validation. | Web alpha language expansion and lookup-filter payload support. |
| Import/export settings | Helpful for migration and support, but not necessary for first usable IME. | Stable settings schema and versioned config storage. |
| Visual theme packs | Cosmetic value only after the candidate and settings surfaces are stable. | Visual parity baseline and screenshot regression process. |
| In-app update checker | Useful for distribution but adds networking, signing, and update policy concerns. | Signed release pipeline and update-channel strategy. |
| Accessibility deepening beyond baseline keyboard/screen-reader correctness | Important but should be researched against actual TSF/candidate window behavior after v1 UI exists. | Candidate/settings UI implementation and manual accessibility audit. |
| Enterprise deployment controls | Some Windows IMEs need MSI/GPO-style deployment. | v1 installer stability and evidence of managed deployment demand. |

## Feature Dependencies

```text
TypeDuck product identity
    ├──requires──> deterministic TSF profile identity
    ├──requires──> installer/resource/string rename
    └──enables──> clean About dialog, logs, tray, release artifacts

Chinese (Traditional, Hong Kong) registration
    ├──requires──> TypeDuck profile GUID and locale contract
    └──requires──> installer/setup-helper registration changes

TypeDuck-HK librime integration
    ├──requires──> engine boundary decision
    ├──requires──> packaged schemas/dictionaries
    ├──enables──> Cantonese composition and candidates
    └──enables──> Cangjie version setting if supported by schema/runtime

Dictionary lookup filter support
    ├──requires──> TypeDuck-HK librime runtime
    ├──requires──> protocol/view-model fields for dictionary metadata
    ├──enables──> candidate translations
    ├──enables──> dictionary/detail panel
    └──enables──> multilingual prompts and More Languages rows

Settings dialog
    ├──requires──> bilingual string system
    ├──requires──> settings persistence schema
    ├──controls──> display languages
    ├──controls──> candidates per page
    ├──controls──> Jyutping visibility
    ├──controls──> Chinese typeface
    ├──controls──> auto-completion/correction/composition
    ├──controls──> input memory
    ├──controls──> reverse lookup
    ├──controls──> show full input code
    └──controls──> Cangjie version

Candidate window Web alpha parity
    ├──requires──> structured candidate metadata
    ├──requires──> display-language settings
    ├──requires──> Jyutping visibility setting
    ├──requires──> candidates-per-page setting
    ├──requires──> typeface setting
    └──requires──> dictionary/detail panel rendering

Installer settings flow
    ├──requires──> settings dialog
    ├──requires──> bilingual installer strings
    └──requires──> TypeDuck identity and zh-HK registration path

Anti-feature cleanup
    ├──conflicts──> WebDAV/cloud clipboard
    ├──conflicts──> AI controls
    ├──conflicts──> visible fcitx references
    ├──conflicts──> Moqi branding
    └──conflicts──> excessive customization
```

### Dependency Notes

- **Engine before candidate parity:** Rich candidate/dictionary UI depends on structured data from TypeDuck-HK librime and the lookup filter. Build the data path before polishing the candidate window.
- **Profile identity before installer polish:** The installer cannot be considered correct until the IME registers under Chinese (Traditional, Hong Kong) with TypeDuck-owned GUIDs and names.
- **Settings schema before install-time settings:** Showing settings during installation requires a stable settings model; otherwise installer choices will not map cleanly to runtime behavior.
- **Bilingual copy cuts across everything:** Do not treat bilingual text as a late translation pass. Every new user-facing surface should start with Traditional Hong Kong Chinese and English labels.
- **Anti-feature cleanup is a product feature:** Removing Moqi/fcitx/WebDAV/AI surfaces should be scheduled explicitly because these surfaces already exist in the scaffold.

## MVP Definition

### Launch With (v1)

Minimum viable product for a credible TypeDuck Windows IME.

- [ ] TypeDuck product identity across installer, binaries, resources, tray/menu, logs, and artifacts.
- [ ] Chinese (Traditional, Hong Kong) TSF registration and clean install/uninstall behavior.
- [ ] TypeDuck-HK librime runtime integration with packaged TypeDuck schemas and dictionaries.
- [ ] Cantonese composition, candidate paging/selection, commit, and responsive TSF behavior.
- [ ] Lookup-filter-backed candidate metadata and dictionary/detail panel.
- [ ] Native candidate window showing candidate, code/Jyutping/comment, prompts, translations, and dictionary-like details.
- [ ] Native settings dialog mirroring TypeDuck Web alpha, with Display Languages first.
- [ ] Installer path that shows settings during installation.
- [ ] Bilingual Traditional Hong Kong Chinese and English strings on all user-facing surfaces.
- [ ] About dialog with TypeDuck identity and version information.
- [ ] Explicit removal/hiding of Moqi, fcitx, WebDAV/cloud clipboard, AI, Simplified Chinese installer, and excessive customization surfaces.

### Add After Validation (v1.x)

Features that can follow once core typing, settings, and installer behavior are stable.

- [ ] Refined input memory controls and reset UI once storage semantics are proven.
- [ ] Better diagnostics UI for support, with typed-content-safe logging defaults.
- [ ] Expanded manual compatibility matrix for Office, browsers, terminals, elevated apps, UWP/immersive apps, multi-monitor, and high-DPI setups.
- [ ] Candidate/dictionary visual refinements based on side-by-side Web alpha screenshot review.
- [ ] Settings import/export if support cases show repeated setup friction.

### Future Consideration (v2+)

Do not let these expand the v1 scope.

- [ ] Cloud sync for settings/input memory.
- [ ] Additional display languages beyond Web alpha.
- [ ] Advanced key remapping and custom selection-key layouts.
- [ ] Theme packs or broad candidate-window customization.
- [ ] In-app update checker and release-channel UI.
- [ ] Enterprise deployment controls.
- [ ] AI-assisted writing, translation, or explanation features.

## Feature Prioritization Matrix

| Feature | User Value | Implementation Cost | Priority |
|---------|------------|---------------------|----------|
| Chinese (Traditional, Hong Kong) registration | HIGH | HIGH | P1 |
| TypeDuck-HK librime integration | HIGH | HIGH | P1 |
| Cantonese composition/candidate commit path | HIGH | HIGH | P1 |
| Dictionary lookup filter support | HIGH | HIGH | P1 |
| Candidate list with structured metadata | HIGH | HIGH | P1 |
| Dictionary/detail panel | HIGH | HIGH | P1 |
| Settings dialog matching Web alpha | HIGH | MEDIUM | P1 |
| Display language picker | HIGH | MEDIUM | P1 |
| Bilingual Traditional Hong Kong Chinese and English copy | HIGH | MEDIUM | P1 |
| Installer settings flow | HIGH | MEDIUM | P1 |
| Product identity replacement | HIGH | MEDIUM | P1 |
| Anti-feature cleanup for Moqi/fcitx/WebDAV/AI | HIGH | MEDIUM | P1 |
| About dialog | MEDIUM | LOW | P1 |
| Auto-completion/correction/composition toggles | MEDIUM | MEDIUM | P2 |
| Input memory | MEDIUM | HIGH | P2 |
| Reverse lookup settings | MEDIUM | MEDIUM | P2 |
| Chinese typeface setting | MEDIUM | LOW | P2 |
| Full input code visibility | MEDIUM | LOW | P2 |
| Cangjie version setting | MEDIUM | MEDIUM | P2 |
| Diagnostics without typed-content leakage | MEDIUM | MEDIUM | P2 |
| Settings import/export | LOW | MEDIUM | P3 |
| Cloud sync | LOW | HIGH | P3 |
| Advanced key remapping | LOW | HIGH | P3 |
| Theme packs | LOW | HIGH | P3 |
| AI writing controls | LOW | HIGH | Do not build |

**Priority key:**
- P1: Must have for v1 launch
- P2: Should have for v1 if engine/UI support lands cleanly; otherwise v1.x
- P3: Future consideration only
- Do not build: Conflicts with TypeDuck Windows v1 scope

## V1 Feature Phasing Recommendation

1. **Identity, Locale, and Scaffold Cleanup**
   - Deliver TypeDuck naming, zh-HK profile contract, installer/resource string direction, and visible removal of Moqi/fcitx/WebDAV/AI surfaces.
   - Rationale: Prevents every later feature from being built on incorrect product identity.

2. **Engine and Data Path**
   - Deliver TypeDuck-HK librime integration, schema/dictionary packaging, lookup-filter support, and structured candidate/dictionary payloads.
   - Rationale: UI parity cannot be real until the runtime emits the right Cantonese and dictionary data.

3. **Candidate and Dictionary UI**
   - Deliver Web-alpha-like candidate list and dictionary/detail panel with Jyutping, code, comments, translations, and more-language rows.
   - Rationale: This is the main daily product surface and the primary TypeDuck differentiator.

4. **Settings and Installer Flow**
   - Deliver native settings dialog, install-time settings display, persistence, bilingual labels, and About dialog.
   - Rationale: Settings need stable engine/UI flags to control; installer flow needs the settings dialog to exist.

5. **Stabilization and Acceptance**
   - Deliver uninstall/reinstall checks, host-app compatibility testing, typed-content-safe diagnostics, and visual parity review.
   - Rationale: Windows IMEs fail in edge-case host processes; v1 needs reliability proof, not more scope.

## Competitor / Reference Feature Analysis

| Reference | Feature Signal | TypeDuck Windows v1 Approach |
|-----------|----------------|------------------------------|
| TypeDuck Web alpha | Source of truth for settings, candidate data shape, dictionary panel, visual tone, and language picker. | Mirror visible behavior and look-and-feel unless Windows TSF constraints require a documented adaptation. |
| Native Windows IME expectations | Users expect language profile installation, candidate selection, paging, commit, settings, uninstall, and host-app compatibility. | Implement as table stakes; no credit for novelty here. |
| Existing Moqi scaffold | Provides TSF/launcher/installer/candidate-window mechanics but includes wrong product surfaces. | Preserve useful Windows infrastructure; remove or replace product behavior, branding, and non-TypeDuck features. |

## Sources

- `.planning/PROJECT.md` - Product target, active requirements, out-of-scope decisions, TypeDuck Web alpha setting list, candidate/dictionary context.
- `.planning/codebase/ARCHITECTURE.md` - TSF/frontend/backend boundaries, installer/registration flow, and scaffold anti-patterns.
- `.planning/codebase/STRUCTURE.md` - Locations for TypeDuck TSF behavior, backend integration, protocol, installer, and candidate UI work.
- `.planning/codebase/CONCERNS.md` - Legacy Moqi/fcitx/WebDAV/AI leakage, missing TypeDuck features, security/performance risks, and registration fragility.
- `.planning/codebase/TESTING.md` - Test gaps and TypeDuck-specific verification guidance.
- Known target context supplied with this research task - Web alpha settings, candidate/dictionary screenshots, required TypeDuck-HK librime and lookup-filter integration.

---
*Feature research for: TypeDuck Windows IME*
*Researched: 2026-06-23*
