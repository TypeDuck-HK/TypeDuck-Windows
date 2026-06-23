# Requirements: TypeDuck Windows IME

**Defined:** 2026-06-23
**Core Value:** Hong Kong users can install TypeDuck under Chinese (Traditional, Hong Kong) and type Cantonese with the same trusted behavior, bilingual settings, candidate details, and dictionary-like lookup experience as TypeDuck Web alpha.

## v1 Requirements

### Product Identity

- [ ] **IDEN-01**: User sees TypeDuck, not Moqi, in installer title text, installed app names, tray/menu surfaces, binary/resource metadata, logs, data paths, and release artifact names.
- [x] **IDEN-02**: Developer has a single TypeDuck identity contract covering executable names, DLL names, AppId/CLSID/profile GUIDs, install directories, log/data directories, pipe/mutex names, registry keys, resource strings, and release artifact names.
- [ ] **IDEN-03**: User-facing surfaces contain no visible Moqi product strings after the TypeDuck identity phase.
- [ ] **IDEN-04**: User-facing surfaces contain no visible fcitx, WebDAV/cloud clipboard, or AI feature references in the v1 product.

### Installation and TSF Registration

- [ ] **INST-01**: User can install TypeDuck Windows IME with an installer branded and worded in bilingual Traditional Hong Kong Chinese and English.
- [ ] **INST-02**: User can select TypeDuck under Windows Chinese (Traditional, Hong Kong) language/input settings after installation.
- [ ] **INST-03**: The installed TSF profile uses deterministic TypeDuck-owned CLSID/profile GUID, `zh-HK` locale metadata, TypeDuck icon, and bilingual display text.
- [ ] **INST-04**: Installer registers both 32-bit and 64-bit TSF DLLs so TypeDuck works in 32-bit and 64-bit host applications.
- [ ] **INST-05**: User can uninstall TypeDuck and remove TypeDuck-owned TSF registration, startup entries, install files, and runtime registry/profile residue without leaving broken profile entries.
- [ ] **INST-06**: Installer launches the TypeDuck settings dialog or install-time settings flow during installation with Display Languages shown first.

### Engine and Runtime Assets

- [x] **ENG-01**: Developer can build or package the pinned TypeDuck-HK librime fork as the TypeDuck Windows IME engine dependency.
- [x] **ENG-02**: Developer can build or package the `TypeDuck-HK/rime-dictionary-lookup-filter` plugin with the TypeDuck engine.
- [x] **ENG-03**: Installed runtime includes the TypeDuck schemas, dictionaries, OpenCC assets, and built schema assets required to mirror TypeDuck Web alpha.
- [x] **ENG-04**: TypeDuck engine runtime can produce Cantonese composition, candidate pages, Jyutping, and commit output from TypeDuck schema input.
- [ ] **ENG-05**: TypeDuck engine runtime can produce lookup-filter-backed dictionary data for candidates, including headword, Jyutping, part-of-speech, English meaning, reading notes, and enabled-language meanings when available.
- [ ] **ENG-06**: Engine/runtime failure does not crash arbitrary TSF host applications; user receives a bounded degraded state or clean reset.

### Protocol and Data Model

- [ ] **PROTO-01**: TypeDuck TSF frontend and engine communicate through a versioned TypeDuck protocol rather than Moqi-shaped product messages.
- [ ] **PROTO-02**: Protocol frames enforce maximum payload sizes and reject malformed or oversized frames without unbounded buffering.
- [ ] **PROTO-03**: Protocol responses carry structured candidate data with candidate id, display text, input code, Jyutping, comments/prompts, translations, page info, and selection metadata.
- [ ] **PROTO-04**: Protocol responses carry structured dictionary lookup data instead of flattening dictionary details into one comment string.
- [ ] **PROTO-05**: Protocol supports settings snapshot, settings update, engine capability, deploy/reconfigure, and engine health/error messages required by the settings UI.
- [ ] **PROTO-06**: Golden protocol fixtures cover common Cantonese input, dictionary lookup, reverse lookup, malformed payload, timeout, and engine restart cases.

### Typing Behavior

- [ ] **TYPE-01**: User can type Cantonese through TypeDuck in ordinary Windows text fields with composition text, candidate list, candidate paging, numeric selection, and commit behavior.
- [ ] **TYPE-02**: User can use candidate count per page according to the TypeDuck setting.
- [ ] **TYPE-03**: User can use reverse lookup behavior consistent with TypeDuck Web alpha where supported by the engine.
- [ ] **TYPE-04**: User can choose Cangjie Version 3 or Version 5 where TypeDuck Web alpha exposes that setting and the engine confirms support.
- [ ] **TYPE-05**: Key handling remains responsive under normal typing; engine timeouts or restarts do not hang host applications.

### Candidate and Dictionary UI

- [ ] **CAND-01**: User sees a native candidate panel styled to match TypeDuck Web alpha visual tone: light neutral surface, warm amber highlight/accent, compact rows, readable HK Chinese/English font stack, and restrained rounded controls.
- [ ] **CAND-02**: User sees each candidate's Chinese text and relevant input code/Jyutping/comment/translation fields according to settings and available engine data.
- [ ] **CAND-03**: User can inspect a selected or highlighted candidate's dictionary-like detail panel with headword, Jyutping, part-of-speech labels, English meaning, reading notes, and More Languages rows when available.
- [ ] **CAND-04**: User can configure Jyutping visibility with the Web alpha modes: Always Show, Only in Reverse Lookup, and Hide.
- [ ] **CAND-05**: User can configure Chinese typeface between Sung and Hei for candidate/dictionary presentation.
- [ ] **CAND-06**: Candidate and dictionary panels position reliably near the composition/caret area without stealing focus in representative Windows host applications.
- [ ] **CAND-07**: Candidate UI remains coherent under high DPI, multi-monitor placement, UI-less TSF hosts, and host applications with imperfect composition rectangles.

### Settings and About

- [ ] **SET-01**: User can open a TypeDuck settings dialog from the installer flow.
- [ ] **SET-02**: User can open the same TypeDuck settings dialog after installation from a TypeDuck configuration entry point.
- [ ] **SET-03**: Settings dialog mirrors TypeDuck Web alpha settings and order, with Display Languages first.
- [ ] **SET-04**: User can enable display languages from the Web alpha language set: English, Hindi, Indonesian, Nepali, and Urdu.
- [ ] **SET-05**: User can choose one main display language from the enabled display languages.
- [ ] **SET-06**: User can configure number of candidates per page using the Web alpha-supported range.
- [ ] **SET-07**: User can configure Auto-completion, Auto-correction, Auto-composition, and Input Memory where engine support is available.
- [ ] **SET-08**: User can configure Reverse Lookup Settings and Show Full Input Code where engine support is available.
- [ ] **SET-09**: Settings persist locally under TypeDuck-owned storage and apply to subsequent TypeDuck IME sessions.
- [ ] **SET-10**: User can open an About dialog showing TypeDuck Windows IME identity, version, and relevant engine/schema attribution in bilingual text.

### Language and Copy

- [ ] **LANG-01**: Every user-facing installer, settings, About, tray/menu, candidate UI, error, and status string is bilingual in Traditional Hong Kong Chinese and English.
- [ ] **LANG-02**: User-facing Chinese text uses Traditional Hong Kong wording, not Simplified Chinese wording or Simplified-only installer chrome.
- [ ] **LANG-03**: TypeDuck display-language labels and dictionary language names match TypeDuck Web alpha unless the Web alpha changes and the project context is updated.

### Privacy, Security, and Diagnostics

- [ ] **SEC-01**: Runtime logs and diagnostics use TypeDuck-owned paths and avoid raw typed content by default.
- [ ] **SEC-02**: Cloud clipboard/WebDAV behavior from the scaffold is removed or compile-gated out of v1.
- [ ] **SEC-03**: Arbitrary backend-provided config tool launching is removed or replaced by a fixed first-party TypeDuck settings entry point.
- [ ] **SEC-04**: Named pipe access and client/server identity checks are hardened enough to prevent obvious same-user spoofing and oversized-frame abuse.
- [ ] **SEC-05**: Installer avoids broad legacy process kills and uses TypeDuck-specific process names and cleanup operations.

### Verification and Release Readiness

- [x] **VER-01**: Developer has a dated TypeDuck Web alpha parity fixture set covering settings defaults, labels, candidate list, dictionary panel, and visual references.
- [ ] **VER-02**: Developer can run automated or scripted checks that fail when visible Moqi, fcitx, WebDAV/cloud clipboard, AI, or Simplified-only strings remain in user-facing resources.
- [ ] **VER-03**: Developer can verify clean install, upgrade/reinstall, uninstall, and reboot-required registration behavior on Windows 10/11 or equivalent test VMs.
- [ ] **VER-04**: Developer can verify TypeDuck typing and candidate UI in representative host apps including Notepad, browsers, Office or Office-like apps, terminal/console contexts, elevated apps, and high-DPI/multi-monitor setups.
- [ ] **VER-05**: Developer can verify protocol and engine behavior with golden tests for normal input, dictionary lookup, reverse lookup, malformed frames, timeouts, and backend restart.
- [ ] **VER-06**: Developer can produce a v1 installer artifact with TypeDuck naming and documented verification evidence.

## v2 Requirements

Deferred to future release. Tracked but not in current roadmap.

### Sync and Portability

- **SYNC-01**: User can sync settings across devices with explicit consent and privacy controls.
- **SYNC-02**: User can export and import TypeDuck settings as a local file.
- **SYNC-03**: User can sync or migrate input memory with clear reset and privacy behavior.

### Advanced Customization

- **ADV-01**: User can configure advanced key mappings or custom selection keys.
- **ADV-02**: User can choose additional visual themes beyond the Web alpha visual baseline.
- **ADV-03**: User can enable additional display languages beyond the TypeDuck Web alpha language set.

### Distribution

- **DIST-01**: User can receive in-app update notifications or use release channels.
- **DIST-02**: Administrator can deploy TypeDuck through enterprise-oriented installer controls.
- **DIST-03**: User can install beta and stable channels side by side if product direction requires it.

## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Feature | Reason |
|---------|--------|
| Moqi product behavior or branding | The repo is scaffold only; user explicitly wants TypeDuck. |
| Visible fcitx features or references | Not part of the Windows TypeDuck target and would look like unfinished scaffold leakage. |
| WebDAV/cloud clipboard | Outside stated scope and privacy-sensitive. |
| AI writing, translation, or explanation controls | Scaffold leftover and not part of TypeDuck Windows v1. |
| Excessive customization beyond Web alpha settings | Parity and testability matter more than broad configurability. |
| Installing as Chinese (Simplified) | Explicitly rejected; target is Chinese (Traditional, Hong Kong). |
| Generic backend-declared configuration tools | Security and UX risk; TypeDuck needs one first-party settings surface. |
| Shipping the legacy Moqi backend as fallback | Validates the wrong behavior and can hide TypeDuck engine gaps. |
| Cloud/account features | Require privacy, account, conflict, and support design outside v1. |
| WebView candidate popup as default architecture | Adds host-process/focus/accessibility risk before native rendering is proven insufficient. |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| IDEN-01 | Phase 6 | Pending |
| IDEN-02 | Phase 1 | Complete |
| IDEN-03 | Phase 6 | Pending |
| IDEN-04 | Phase 6 | Pending |
| INST-01 | Phase 3 | Pending |
| INST-02 | Phase 3 | Pending |
| INST-03 | Phase 3 | Pending |
| INST-04 | Phase 3 | Pending |
| INST-05 | Phase 3 | Pending |
| INST-06 | Phase 5 | Pending |
| ENG-01 | Phase 2 | Complete |
| ENG-02 | Phase 2 | Complete |
| ENG-03 | Phase 2 | Complete |
| ENG-04 | Phase 2 | Complete |
| ENG-05 | Phase 2 | Pending |
| ENG-06 | Phase 4 | Pending |
| PROTO-01 | Phase 4 | Pending |
| PROTO-02 | Phase 4 | Pending |
| PROTO-03 | Phase 4 | Pending |
| PROTO-04 | Phase 4 | Pending |
| PROTO-05 | Phase 4 | Pending |
| PROTO-06 | Phase 4 | Pending |
| TYPE-01 | Phase 4 | Pending |
| TYPE-02 | Phase 4 | Pending |
| TYPE-03 | Phase 4 | Pending |
| TYPE-04 | Phase 4 | Pending |
| TYPE-05 | Phase 4 | Pending |
| CAND-01 | Phase 5 | Pending |
| CAND-02 | Phase 5 | Pending |
| CAND-03 | Phase 5 | Pending |
| CAND-04 | Phase 5 | Pending |
| CAND-05 | Phase 5 | Pending |
| CAND-06 | Phase 5 | Pending |
| CAND-07 | Phase 5 | Pending |
| SET-01 | Phase 5 | Pending |
| SET-02 | Phase 5 | Pending |
| SET-03 | Phase 5 | Pending |
| SET-04 | Phase 5 | Pending |
| SET-05 | Phase 5 | Pending |
| SET-06 | Phase 5 | Pending |
| SET-07 | Phase 5 | Pending |
| SET-08 | Phase 5 | Pending |
| SET-09 | Phase 5 | Pending |
| SET-10 | Phase 5 | Pending |
| LANG-01 | Phase 6 | Pending |
| LANG-02 | Phase 6 | Pending |
| LANG-03 | Phase 5 | Pending |
| SEC-01 | Phase 6 | Pending |
| SEC-02 | Phase 6 | Pending |
| SEC-03 | Phase 6 | Pending |
| SEC-04 | Phase 6 | Pending |
| SEC-05 | Phase 6 | Pending |
| VER-01 | Phase 1 | Complete |
| VER-02 | Phase 6 | Pending |
| VER-03 | Phase 7 | Pending |
| VER-04 | Phase 7 | Pending |
| VER-05 | Phase 7 | Pending |
| VER-06 | Phase 7 | Pending |

**Coverage:**

- v1 requirements: 58 total
- Mapped to phases: 58
- Unmapped: 0

---
*Requirements defined: 2026-06-23*
*Last updated: 2026-06-23 after roadmap creation*
