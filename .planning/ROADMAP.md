# Roadmap: TypeDuck Windows IME

## Overview

TypeDuck Windows IME will move the existing Moqi Windows IME scaffold into a TypeDuck-native Cantonese IME by first locking the product identity and Web alpha parity contract, then proving the TypeDuck-HK librime and dictionary lookup runtime, installing under Chinese (Traditional, Hong Kong), replacing the protocol and typing model, building the visible candidate/settings/About experience, removing scaffold leakage, and finishing with Windows compatibility and release evidence.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [ ] **Phase 1: Identity and Web Parity Contract** - TypeDuck identity, banned-surface rules, and dated Web alpha acceptance fixtures are locked before implementation spreads.
- [ ] **Phase 2: Engine Runtime Contract Spike** - The TypeDuck-HK librime fork, lookup-filter plugin, and TypeDuck runtime assets are proven on Windows.
- [ ] **Phase 3: zh-HK TSF Registration and Installer Skeleton** - TypeDuck installs, registers, unregisters, and appears under Chinese (Traditional, Hong Kong).
- [ ] **Phase 4: TypeDuck Protocol and Typing MVP** - TSF, launcher, and engine communicate with a bounded TypeDuck protocol that supports real Cantonese typing behavior.
- [ ] **Phase 5: Candidate, Dictionary, Settings, and About UI Parity** - Users see and configure the TypeDuck Web alpha experience in native Windows surfaces.
- [ ] **Phase 6: Privacy, Security, and Scaffold Cleanup** - Off-scope Moqi/fcitx/cloud/AI behavior is removed or gated, diagnostics are safe, and visible strings are audited.
- [ ] **Phase 7: Compatibility and Release Verification** - The v1 installer and IME are verified across Windows install paths, host apps, DPI, bitness, protocol recovery, and release artifacts.

## Phase Details

### Phase 1: Identity and Web Parity Contract
**Goal**: Developer has a single source of truth for TypeDuck identity and Web alpha parity before product behavior is changed.
**Mode:** mvp
**Depends on**: Nothing (first phase)
**Requirements**: IDEN-02, VER-01
**Success Criteria** (what must be TRUE):
  1. Developer can inspect one TypeDuck identity contract covering executable names, DLL names, AppId/CLSID/profile GUIDs, install directories, log/data directories, pipe/mutex names, registry keys, resource strings, and release artifact names.
  2. Developer can inspect a dated TypeDuck Web alpha fixture set covering settings defaults, labels, candidate list, dictionary panel, and visual references.
  3. Developer can identify which Moqi, fcitx, WebDAV/cloud clipboard, AI, Simplified-only, and excessive customization surfaces are banned from the v1 product.
**Plans**: 2 plans
Plans:
- [ ] 01-01-PLAN.md — Create the TypeDuck identity contract and banned-surface contract.
- [ ] 01-02-PLAN.md — Create dated TypeDuck Web alpha fixture markdown and supporting fixture assets.

### Phase 2: Engine Runtime Contract Spike
**Goal**: Developer can prove that TypeDuck-HK librime, lookup-filter, schemas, dictionaries, and built assets produce the candidate and dictionary data TypeDuck Windows needs.
**Mode:** mvp
**Depends on**: Phase 1
**Requirements**: ENG-01, ENG-02, ENG-03, ENG-04, ENG-05
**Success Criteria** (what must be TRUE):
  1. Developer can build or package the pinned TypeDuck-HK librime fork and `rime-dictionary-lookup-filter` plugin for the Windows runtime target.
  2. Installed or staged runtime assets include TypeDuck schemas, dictionaries, OpenCC assets, and built schema artifacts required for Web alpha parity.
  3. Developer can run a minimal engine host or spike executable that produces Cantonese composition, candidate pages, Jyutping, and commit output from TypeDuck schema input.
  4. Developer can capture lookup-filter-backed dictionary data for candidates, including headword, Jyutping, part-of-speech, English meaning, reading notes, and enabled-language meanings when available.
**Plans**: TBD

### Phase 3: zh-HK TSF Registration and Installer Skeleton
**Goal**: User can install and remove TypeDuck as a Chinese (Traditional, Hong Kong) Windows IME with deterministic first-party registration.
**Mode:** mvp
**Depends on**: Phase 2
**Requirements**: INST-01, INST-02, INST-03, INST-04, INST-05
**Success Criteria** (what must be TRUE):
  1. User can run a TypeDuck-branded installer worded in bilingual Traditional Hong Kong Chinese and English.
  2. User can select TypeDuck under Windows Chinese (Traditional, Hong Kong) language/input settings after installation.
  3. TypeDuck appears with deterministic TypeDuck-owned CLSID/profile GUID, `zh-HK` locale metadata, TypeDuck icon, and bilingual display text.
  4. TypeDuck works from both 32-bit and 64-bit host applications after installer registration.
  5. User can uninstall TypeDuck without leaving broken TypeDuck TSF registrations, startup entries, install files, or runtime registry/profile residue.
**Plans**: TBD
**UI hint**: yes

### Phase 4: TypeDuck Protocol and Typing MVP
**Goal**: User can type Cantonese through TypeDuck with responsive key handling while TSF, launcher, and engine exchange bounded structured TypeDuck data.
**Mode:** mvp
**Depends on**: Phase 3
**Requirements**: ENG-06, PROTO-01, PROTO-02, PROTO-03, PROTO-04, PROTO-05, PROTO-06, TYPE-01, TYPE-02, TYPE-03, TYPE-04, TYPE-05
**Success Criteria** (what must be TRUE):
  1. User can type Cantonese in ordinary Windows text fields with composition text, candidate list, candidate paging, numeric selection, and commit behavior.
  2. Key handling stays responsive during normal typing, and engine timeout or restart cases produce a bounded degraded state or clean reset instead of hanging host applications.
  3. Developer can inspect versioned TypeDuck protocol frames carrying structured candidate, dictionary lookup, settings, capability, deploy/reconfigure, health, and error data.
  4. Developer can run golden protocol fixtures covering common Cantonese input, dictionary lookup, reverse lookup, malformed payload, timeout, and engine restart cases.
  5. User can use TypeDuck-supported candidate count, reverse lookup, and Cangjie Version 3 or Version 5 behavior where the engine confirms support.
**Plans**: TBD

### Phase 5: Candidate, Dictionary, Settings, and About UI Parity
**Goal**: User can see and configure the native Windows TypeDuck experience with candidate, dictionary, settings, installer-first-run, and About surfaces matching the Web alpha contract.
**Mode:** mvp
**Depends on**: Phase 4
**Requirements**: INST-06, CAND-01, CAND-02, CAND-03, CAND-04, CAND-05, CAND-06, CAND-07, SET-01, SET-02, SET-03, SET-04, SET-05, SET-06, SET-07, SET-08, SET-09, SET-10, LANG-03
**Success Criteria** (what must be TRUE):
  1. User sees a native candidate panel with TypeDuck Web alpha visual tone: light neutral surface, warm amber accents, compact rows, readable HK Chinese/English font stack, and restrained rounded controls.
  2. User can inspect candidate text, input code/Jyutping/comment/translation fields, and dictionary details with headword, Jyutping, part-of-speech labels, English meaning, reading notes, and More Languages rows when available.
  3. Candidate and dictionary panels stay correctly positioned near the composition/caret area without stealing focus across representative host applications, high DPI, multi-monitor, UI-less, and imperfect composition-rectangle cases.
  4. User can open the same TypeDuck settings dialog from install-time flow and after installation, with Display Languages first and Web alpha settings/order mirrored.
  5. User can configure display languages, main display language, candidate count, Jyutping visibility, Chinese typeface, Auto-completion, Auto-correction, Auto-composition, Input Memory, Reverse Lookup Settings, Show Full Input Code, and supported Cangjie behavior; settings persist locally and apply to subsequent IME sessions.
  6. User can open an About dialog showing TypeDuck Windows IME identity, version, and relevant engine/schema attribution in bilingual text.
**Plans**: TBD
**UI hint**: yes

### Phase 6: Privacy, Security, and Scaffold Cleanup
**Goal**: User sees only TypeDuck v1 behavior while local diagnostics, IPC, process cleanup, and user-facing copy meet TypeDuck privacy and security expectations.
**Mode:** mvp
**Depends on**: Phase 5
**Requirements**: IDEN-01, IDEN-03, IDEN-04, LANG-01, LANG-02, SEC-01, SEC-02, SEC-03, SEC-04, SEC-05, VER-02
**Success Criteria** (what must be TRUE):
  1. User sees TypeDuck, not Moqi, in installer title text, installed app names, tray/menu surfaces, binary/resource metadata, logs, data paths, and release artifact names.
  2. User-facing installer, settings, About, tray/menu, candidate UI, error, and status strings are bilingual in Traditional Hong Kong Chinese and English, with no Simplified-only wording.
  3. User-facing surfaces contain no visible Moqi, fcitx, WebDAV/cloud clipboard, AI, or other off-scope scaffold references.
  4. Runtime logs and diagnostics use TypeDuck-owned paths and avoid raw typed content by default.
  5. Named pipe access, client/server identity checks, frame-size limits, first-party settings launch, and installer cleanup avoid obvious spoofing, oversized-frame abuse, arbitrary config-tool launch, and broad legacy process kills.
  6. Developer can run automated or scripted checks that fail on visible Moqi, fcitx, WebDAV/cloud clipboard, AI, or Simplified-only strings in user-facing resources.
**Plans**: TBD
**UI hint**: yes

### Phase 7: Compatibility and Release Verification
**Goal**: Developer can produce a TypeDuck v1 installer artifact backed by repeatable Windows installation, typing, UI, protocol, and release evidence.
**Mode:** mvp
**Depends on**: Phase 6
**Requirements**: VER-03, VER-04, VER-05, VER-06
**Success Criteria** (what must be TRUE):
  1. Developer can verify clean install, upgrade/reinstall, uninstall, and reboot-required registration behavior on Windows 10/11 or equivalent test VMs.
  2. Developer can verify TypeDuck typing and candidate UI in representative host apps including Notepad, browsers, Office or Office-like apps, terminal/console contexts, elevated apps, and high-DPI/multi-monitor setups.
  3. Developer can verify protocol and engine behavior with golden tests for normal input, dictionary lookup, reverse lookup, malformed frames, timeouts, and backend restart.
  4. Developer can produce a v1 installer artifact with TypeDuck naming and documented verification evidence.
**Plans**: TBD

## Progress

**Execution Order:**
Phases execute in numeric order: 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Identity and Web Parity Contract | 0/TBD | Not started | - |
| 2. Engine Runtime Contract Spike | 0/TBD | Not started | - |
| 3. zh-HK TSF Registration and Installer Skeleton | 0/TBD | Not started | - |
| 4. TypeDuck Protocol and Typing MVP | 0/TBD | Not started | - |
| 5. Candidate, Dictionary, Settings, and About UI Parity | 0/TBD | Not started | - |
| 6. Privacy, Security, and Scaffold Cleanup | 0/TBD | Not started | - |
| 7. Compatibility and Release Verification | 0/TBD | Not started | - |
