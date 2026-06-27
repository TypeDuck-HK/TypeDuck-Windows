# Roadmap: TypeDuck Windows IME

## Overview

TypeDuck Windows IME will move the existing Moqi Windows IME scaffold into a TypeDuck-native Cantonese IME by first locking the product identity and Web alpha parity contract, then proving the TypeDuck-HK librime and dictionary lookup runtime, installing under Chinese (Traditional, Hong Kong), replacing the protocol and typing model, building the visible candidate/settings/About experience, removing scaffold leakage, and finishing with Windows compatibility and release evidence.

## Phases

**Phase Numbering:**

- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [x] **Phase 1: Identity and Web Parity Contract** - TypeDuck identity, banned-surface rules, and dated Web alpha acceptance fixtures are locked before implementation spreads. (completed 2026-06-23)
- [x] **Phase 2: Engine Runtime Contract Spike** - The TypeDuck-HK librime fork, lookup-filter plugin, and TypeDuck runtime assets are proven on Windows. (completed 2026-06-23)
- [x] **Phase 3: zh-HK TSF Registration and Installer Skeleton** - TypeDuck installs, registers, unregisters, and appears under Chinese (Traditional, Hong Kong). (completed 2026-06-24)
- [x] **Phase 4: TypeDuck Protocol and Typing MVP** - TSF, launcher, and engine communicate with a bounded TypeDuck protocol that supports real Cantonese typing behavior. (completed 2026-06-24)
- [x] **Phase 5: Candidate, Dictionary, Settings, and About UI Parity** - Users see and configure the TypeDuck Web alpha experience in native Windows surfaces. (completed 2026-06-26 by human verification and explicit screenshot-evidence divergence)
- [ ] **Phase 6: Privacy, Security, and Scaffold Cleanup** - Off-scope Moqi/fcitx/cloud/AI behavior is removed or gated, diagnostics are safe, and visible strings are audited.
- [ ] **Phase 7: Compatibility and Release Verification** - The v1 installer and IME are verified across Windows install paths, host apps, DPI, bitness, protocol recovery, and release artifacts.

## Phase Details

### Phase 1: Identity and Web Parity Contract

**Goal**: Developer has a single source of truth for TypeDuck identity and Web alpha parity before product behavior is changed.
**Depends on**: Nothing (first phase)
**Requirements**: IDEN-02, VER-01
**Success Criteria** (what must be TRUE):

  1. Developer can inspect one TypeDuck identity contract covering executable names, DLL names, AppId/CLSID/profile GUIDs, install directories, log/data directories, pipe/mutex names, registry keys, resource strings, and release artifact names.
  2. Developer can inspect a dated TypeDuck Web alpha fixture set covering settings defaults, labels, candidate list, dictionary panel, and visual references.
  3. Developer can identify which Moqi, fcitx, WebDAV/cloud clipboard, AI, Simplified-only, and excessive customization surfaces are banned from the v1 product.

**Plans**: 2/2 plans complete
Plans:

- [x] 01-01-PLAN.md — Create the TypeDuck identity contract and banned-surface contract.
- [x] 01-02-PLAN.md — Create dated TypeDuck Web alpha fixture markdown and supporting fixture assets.

### Phase 2: Engine Runtime Contract Spike

**Goal**: Developer can prove that TypeDuck-HK librime, lookup-filter, schemas, dictionaries, and built assets produce the candidate and dictionary data TypeDuck Windows needs.
**Depends on**: Phase 1
**Requirements**: ENG-01, ENG-02, ENG-03, ENG-04, ENG-05
**Success Criteria** (what must be TRUE):

  1. Developer can build or package the pinned TypeDuck-HK librime fork and `rime-dictionary-lookup-filter` plugin for the Windows runtime target.
  2. Installed or staged runtime assets include TypeDuck schemas, dictionaries, OpenCC assets, and built schema artifacts required for Web alpha parity.
  3. Developer can run a minimal engine host or spike executable that produces Cantonese composition, candidate pages, Jyutping, and commit output from TypeDuck schema input.
  4. Developer can capture lookup-filter-backed dictionary data for candidates, including headword, Jyutping, part-of-speech, English meaning, reading notes, and enabled-language meanings when available.

**Plans:** 3/3 plans complete
Plans:
**Wave 1**

- [x] 02-01-PLAN.md — Stage the pinned TypeDuck-HK runtime, schema submodule, and runtime manifest evidence.

**Wave 2** *(blocked on Wave 1 completion)*

- [x] 02-02-PLAN.md — Prove TypeDuck Cantonese composition, candidates, Jyutping, and commits through the compatibility adapter boundary.

**Wave 3** *(blocked on Wave 2 completion)*

- [x] 02-03-PLAN.md — Validate and document raw lookup-filter CSV/comment payload evidence.

### Phase 3: zh-HK TSF Registration and Installer Skeleton

**Goal**: User can install and remove TypeDuck as a Chinese (Traditional, Hong Kong) Windows IME with deterministic first-party registration.
**Depends on**: Phase 2
**Requirements**: INST-01, INST-02, INST-03, INST-04, INST-05
**Success Criteria** (what must be TRUE):

  1. User can run a TypeDuck-branded installer worded in bilingual Traditional Hong Kong Chinese and English.
  2. User can select TypeDuck under Windows Chinese (Traditional, Hong Kong) language/input settings after installation.
  3. TypeDuck appears with deterministic TypeDuck-owned CLSID/profile GUID, `zh-HK` locale metadata, TypeDuck icon, and bilingual display text.
  4. TypeDuck works from both 32-bit and 64-bit host applications after installer registration.
  5. User can uninstall TypeDuck without leaving broken TypeDuck TSF registrations, startup entries, install files, or runtime registry/profile residue.

**Verification Environment**: Requires clean Windows 10/11 VM or equivalent disposable Windows test environment evidence. Prefer Hyper-V checkpoint before installer tests. Code review/build checks are not enough for this phase.

**Plans:** 3/3 plans complete
Plans:
**Wave 1**

- [x] 03-01-PLAN.md — Make TypeDuck TSF identity and zh-HK profile registration first-party.

**Wave 2** *(blocked on Wave 1 completion)*

- [x] 03-02-PLAN.md — Turn the installer, setup helper, and staging pipeline into a TypeDuck installer skeleton.

**Wave 3** *(blocked on Wave 2 completion)*

- [x] 03-03-PLAN.md — Verify install, registration, dual-bitness behavior, and uninstall cleanup in the launched Hyper-V VM.

**UI hint**: yes

### Phase 4: TypeDuck Protocol and Typing MVP

**Goal**: User can type Cantonese through TypeDuck with responsive key handling while TSF, launcher, and engine exchange bounded structured TypeDuck data.
**Depends on**: Phase 3
**Requirements**: ENG-06, PROTO-01, PROTO-02, PROTO-03, PROTO-04, PROTO-05, PROTO-06, TYPE-01, TYPE-02, TYPE-03, TYPE-04, TYPE-05
**Success Criteria** (what must be TRUE):

  1. User can type Cantonese in ordinary Windows text fields with composition text, candidate list, candidate paging, numeric selection, and commit behavior.
  2. Key handling stays responsive during normal typing, and engine timeout or restart cases produce a bounded degraded state or clean reset instead of hanging host applications.
  3. Developer can inspect versioned TypeDuck protocol frames carrying structured candidate, dictionary lookup, settings, capability, deploy/reconfigure, health, and error data.
  4. Developer can run golden protocol fixtures covering common Cantonese input, dictionary lookup, reverse lookup, malformed payload, timeout, and engine restart cases.
  5. User can use TypeDuck-supported candidate count, reverse lookup, and Cangjie Version 3 or Version 5 behavior where the engine confirms support.

**Verification Environment**: Mostly code-testable through protocol, parser, engine-adapter, frame-bound, timeout, and fixture tests. Once TSF typing is wired, include a targeted Windows VM smoke test for real host-process typing responsiveness.

**Plans**: 4/4 plans complete

- [x] 04-01-PLAN.md
- [x] 04-02-PLAN.md
- [x] 04-03-PLAN.md
- [x] 04-04-PLAN.md

### Phase 5: Candidate, Dictionary, Settings, and About UI Parity

**Goal**: User can see and configure the native Windows TypeDuck experience with candidate, dictionary, settings, installer-first-run, and About surfaces matching the Web alpha contract.
**Depends on**: Phase 4
**Requirements**: INST-06, CAND-01, CAND-02, CAND-03, CAND-04, CAND-05, CAND-06, CAND-07, SET-01, SET-02, SET-03, SET-04, SET-05, SET-06, SET-07, SET-08, SET-09, SET-10, LANG-03
**Success Criteria** (what must be TRUE):

  1. User sees a native candidate panel with TypeDuck Web alpha visual tone: light neutral surface, warm amber accents, compact rows, readable HK Chinese/English font stack, and restrained rounded controls.
  2. User can inspect candidate text, input code/Jyutping/comment/translation fields, and dictionary details with headword, Jyutping, part-of-speech labels, English meaning, reading notes, and More Languages rows when available.
  3. Candidate and dictionary panels stay correctly positioned near the composition/caret area without stealing focus across representative host applications, high DPI, multi-monitor, UI-less, and imperfect composition-rectangle cases.
  4. User can open the same TypeDuck settings dialog from install-time flow and after installation, with Display Languages first and Web alpha settings/order mirrored.
  5. User can configure display languages, main display language, candidate count, Jyutping visibility, Chinese typeface, Auto-completion, Auto-correction, Auto-composition, Input Memory, Reverse Lookup Settings, Show Full Input Code, and supported Cangjie behavior; settings persist locally and apply to subsequent IME sessions.
  6. User can open an About dialog showing TypeDuck Windows IME identity, version, and relevant engine/schema attribution in bilingual text.

**Verification Environment**: Closed by iterative human verification in the Windows VM plus static/build/package guards. The earlier screenshot-manifest evidence path was explicitly retired by user direction on 2026-06-26; `.planning/product/ui-fixtures` was removed and all Phase 5 screenshot tests were replaced by human verification. Formal release screenshots and broader compatibility matrix evidence are deferred to Phase 7.

**Plans**: 15/15 plans closed
Plans:
**Wave 1**

- [x] 05-01-PLAN.md — Establish the TypeDuck semantic theme/font contract from existing Web alpha evidence.

**Wave 2** *(blocked on Wave 1 completion)*

- [x] 05-02-PLAN.md — Create native CandidateInfo/CandidateEntry parser and candidate/dictionary view models.

**Wave 3** *(blocked on Wave 2 completion)*

- [x] 05-03-PLAN.md — Implement TypeDuck settings persistence and Rime customization side effects.

**Wave 4** *(blocked on Wave 3 completion)*

- [x] 05-04-PLAN.md — Render source-backed native candidate and dictionary popup.
- [x] 05-05-PLAN.md — Build native TypeDuck settings and About executable surfaces.

**Wave 5** *(blocked on Wave 4 settings/About executable completion)*

- [x] 05-07-PLAN.md — Wire installer-first-run and post-install settings launch.

**Wave 6** *(blocked on Wave 5 launch wiring completion)*

- [x] 05-08-PLAN.md — Package locked TypeDuck icons and staged About resources.

**Wave 7** *(blocked on Wave 4 candidate UI plus Wave 5/6 launch and packaging completion)*

- [x] 05-06-PLAN.md — Collect automated package checks and Windows VM evidence for Phase 5 UI parity. Closed by explicit user-approved divergence; see 05-06-SUMMARY.md and 05-VERIFICATION.md.

**Gap Closure Wave 8** *(blocked on Wave 6 packaging completion)*

- [x] 05-09-PLAN.md — Align rejected-UAT guards, remove stale evidence slots, and record runtime/Web provenance.

**Gap Closure Wave 9** *(blocked on Wave 8 guard/provenance alignment)*

- [x] 05-10-PLAN.md — Repair candidate data parity, multi-row dictionary rendering, POS pills, multilingual rows, and visual metrics.

**Gap Closure Wave 10** *(blocked on Wave 9 candidate/runtime repairs)*

- [x] 05-11-PLAN.md — Repair settings layout/copy and move About to a separate executable.

**Gap Closure Wave 11** *(blocked on Wave 10 settings/About repairs)*

- [x] 05-12-PLAN.md — Clean icon packaging/stamping and remove legacy raw icon leakage.

**Gap Closure Wave 12** *(blocked on candidate, settings/About, and icon repairs)*

- [x] 05-13-PLAN.md — Rebuild the fixed package and refresh automated/package/preview evidence.

**Gap Closure Wave 13** *(blocked on automated/package evidence refresh)*

- [x] 05-14-PLAN.md — Capture and approve required Windows VM/manual evidence. Skipped by explicit user direction after human verification replaced screenshot evidence; see 05-14-SUMMARY.md.

**Gap Closure Wave 14** *(blocked on VM/manual evidence approval)*

- [x] 05-15-PLAN.md — Write the final 05-06 evidence summary and close the UAT gap record.

**UI hint**: yes

### Phase 6: Privacy, Security, and Scaffold Cleanup

**Goal**: User sees only TypeDuck v1 behavior while local diagnostics, IPC, process cleanup, and user-facing copy meet TypeDuck privacy and security expectations.
**Depends on**: Phase 5
**Requirements**: IDEN-01, IDEN-03, IDEN-04, LANG-01, LANG-02, SEC-01, SEC-02, SEC-03, SEC-04, SEC-05, VER-02
**Success Criteria** (what must be TRUE):

  1. User sees TypeDuck, not Moqi, in installer title text, installed app names, tray/menu surfaces, binary/resource metadata, logs, data paths, and release artifact names.
  2. User-facing installer, settings, About, tray/menu, candidate UI, error, and status strings are bilingual in Traditional Hong Kong Chinese and English, with no Simplified-only wording.
  3. User-facing surfaces contain no visible Moqi, fcitx, WebDAV/cloud clipboard, AI, or other off-scope scaffold references.
  4. Runtime logs and diagnostics use TypeDuck-owned paths and avoid raw typed content by default.
  5. Named pipe access, client/server identity checks, frame-size limits, first-party settings launch, and installer cleanup avoid obvious spoofing, oversized-frame abuse, arbitrary config-tool launch, and broad legacy process kills.
  6. Developer can run automated or scripted checks that fail on visible Moqi, fcitx, WebDAV/cloud clipboard, AI, or Simplified-only strings in user-facing resources.

**Verification Environment**: Mixed. Static/code tests are expected for audits, parser bounds, logging defaults, and removed handlers. Add targeted Windows VM smoke evidence for installed-path privacy, launcher/runtime behavior, config-tool removal, pipe behavior, and absence of legacy feature surfaces after installation.

**Plans**: 2/7 plans executed
Plans:
**Wave 1**

- [x] 06-01-PLAN.md — Clean installer localization, first-page copy, Start Menu entries, process cleanup, and Simplified Inno dependency.
- [x] 06-02-PLAN.md — Prune and rename the shipped TypeDuck runtime package under `TypeDuckRuntime`.

**Wave 2** *(blocked on Wave 1 runtime package cleanup)*

- [ ] 06-03-PLAN.md — Replace manifest-driven backend discovery with a fixed TypeDuck runtime bridge.

**Wave 3** *(blocked on Wave 2 runtime discovery)*

- [ ] 06-04-PLAN.md — Move diagnostics to TypeDuck-owned paths and remove off-scope cloud/AI/fcitx frontend paths.
- [ ] 06-07-PLAN.md — Translate/remove backend diagnostics and remove or gate stale backend feature surfaces.

**Wave 4** *(blocked on Wave 3 diagnostics/off-scope cleanup)*

- [ ] 06-05-PLAN.md — Harden named-pipe namespace, ACLs, frame checks, and launcher identity sanity.

**Wave 5** *(blocked on all prior Phase 6 plans)*

- [ ] 06-06-PLAN.md — Close aggregate guard coverage and TypeDuck CI/release artifact naming.

**UI hint**: yes

### Phase 7: Compatibility and Release Verification

**Goal**: Developer can produce a TypeDuck v1 installer artifact backed by repeatable Windows installation, typing, UI, protocol, and release evidence.
**Depends on**: Phase 6
**Requirements**: VER-03, VER-04, VER-05, VER-06
**Success Criteria** (what must be TRUE):

  1. Developer can verify clean install, upgrade/reinstall, uninstall, and reboot-required registration behavior on Windows 10/11 or equivalent test VMs.
  2. Developer can verify TypeDuck typing and candidate UI in representative host apps including Notepad, browsers, Office or Office-like apps, terminal/console contexts, elevated apps, and high-DPI/multi-monitor setups.
  3. Developer can verify protocol and engine behavior with golden tests for normal input, dictionary lookup, reverse lookup, malformed frames, timeouts, and backend restart.
  4. Developer can produce a v1 installer artifact with TypeDuck naming and documented verification evidence.

**Plans**: 4 plans
Plans:

- [ ] 07-01-PLAN.md — Build repeatable non-screenshot install/reinstall/uninstall/reboot and artifact-hash evidence.
- [ ] 07-02-PLAN.md — Add protocol recovery matrix, guard, and focused C++ tests for release verification.
- [ ] 07-03-PLAN.md — Create interactive host-app and DPI human-verification notes without screenshot automation.
- [ ] 07-04-PLAN.md — Aggregate installer, protocol, host-app/DPI, cleanup, and artifact evidence into the final release manifest.

## Progress

**Execution Order:**
Phases execute in numeric order: 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Identity and Web Parity Contract | 2/2 | Complete   | 2026-06-23 |
| 2. Engine Runtime Contract Spike | 3/3 | Complete    | 2026-06-23 |
| 3. zh-HK TSF Registration and Installer Skeleton | 3/3 | Complete    | 2026-06-24 |
| 4. TypeDuck Protocol and Typing MVP | 4/4 | Complete | 2026-06-24 |
| 5. Candidate, Dictionary, Settings, and About UI Parity | 15/15 | Complete | 2026-06-26 |
| 6. Privacy, Security, and Scaffold Cleanup | 2/7 | In Progress|  |
| 7. Compatibility and Release Verification | 0/4 | Not started | - |
