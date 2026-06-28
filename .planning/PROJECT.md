# TypeDuck Windows IME

## What This Is

TypeDuck Windows IME is a Windows Text Services Framework Cantonese input method for TypeDuck. This repo starts from the Moqi Windows IME scaffold, but the product target is TypeDuck: behavior, visible UI, settings, installer language, and candidate/dictionary presentation must mirror the TypeDuck Web alpha at `http://localhost:5173/TypeDuck-Web/aap2-alpha/` and the upstream source in `I:\GitHub\TypeDuck-Web`.

The existing Moqi code is useful scaffolding for TSF registration, candidate windows, launcher/backend IPC, installer packaging, and Windows build mechanics. Moqi branding, Simplified Chinese defaults, fcitx/user-facing clutter, cloud clipboard/WebDAV/AI surfaces, and legacy backend assumptions are not product truth and may be removed or replaced.

## Core Value

Hong Kong users can install TypeDuck under Chinese (Traditional, Hong Kong) and type Cantonese with the same trusted behavior, bilingual settings, candidate details, and dictionary-like lookup experience as TypeDuck Web alpha.

## Business Context

- **Customer**: TypeDuck users who want a native Windows Cantonese IME.
- **Revenue model**: Product/project delivery, not monetization inside this repo.
- **Success metric**: A Windows installer that produces a usable TypeDuck Cantonese IME matching TypeDuck Web alpha behavior and look-and-feel.
- **Strategy notes**: TypeDuck Web alpha is the source of truth until it is promoted to `https://www.typeduck.hk/web/`.

## Requirements

### Validated

- TypeDuck Web alpha defines the target settings and candidate/dictionary UX.
- The current repo can serve as a Windows TSF scaffold with build, installer, launcher, and candidate-window foundations.
- ✓ TypeDuck identity, proposed identifiers, runtime paths, registry/profile names, and release artifact naming are centralized in the Phase 1 identity contract.
- ✓ The 2026-06-23 TypeDuck Web alpha fixture set captures settings, candidate, dictionary-panel, reverse lookup, provenance, and source-code layout authority for downstream parity work.
- ✓ Phase 2 proved the pinned TypeDuck-HK librime release, schema branch, lookup-filter provenance, dictionaries, OpenCC assets, and built schema artifacts in a Windows-shaped staged runtime.
- ✓ Phase 2 proved Cantonese composition, candidate pages, Jyutping/comment data, commit output, and raw lookup-filter dictionary payload evidence through the internal moqi-ime adapter boundary.
- ✓ Phase 4 proved the Windows typing MVP through bounded protocol/static guards, rebuilt installer packaging, and live VM user acceptance that TypeDuck typing works instead of ASCII fallback.
- ✓ Phase 4 established that schema packaging belongs in the sibling `moqi-ime` runtime build; `moqi-im-windows` should consume packaged runtime assets rather than carry a schema submodule.
- ✓ Phase 5 mapped lookup-filter raw payloads into native `CandidateInfo`/`CandidateEntry` rendering fields without moving dictionary parsing into launcher/backend transport.
- ✓ Phase 5 delivered the native candidate and dictionary popup with accepted compact TypeDuck styling, multi-row compound candidates, Jyutping/comment/translation fields, POS/register/label pills, multilingual rows, dictionary scrolling, stable hover behavior, page navigation, and dynamic sizing. The left candidate panel intentionally diverges from TypeDuck Web's exact layout; the user explicitly accepted the cleaner compact linear native design.
- ✓ Phase 5 delivered TypeDuck settings from installer and post-install entry points, with Display Languages first, grouped language controls, JSON preferences, Rime `default.custom.yaml`/`common.custom.yaml` side effects, fast redeploy, and bounded error surfacing.
- ✓ Phase 5 delivered a separate native About executable with current accepted attribution text, product/credit resources, links, version/engine/schema attribution, Start Menu/tray access, and duplicate-instance prevention.
- ✓ Phase 5 cleaned icon/resource packaging for the visible TypeDuck surfaces, including TypeDuck executable/input-picker/installer icon usage and frontend-owned About bitmap resources.
- ✓ Phase 5 visual acceptance is recorded by user human verification. All Phase 5 screenshot tests and `.planning/product/ui-fixtures` artifacts were explicitly retired by user direction on 2026-06-26.
- ✓ Phase 6 completed TypeDuck privacy/security/scaffold cleanup: installer localization, TypeDuckRuntime packaging, fixed runtime bridge, TypeDuck log/data paths, removed cloud/WebDAV/AI/fcitx surfaces, IPC hardening, backend diagnostics cleanup, CI/release naming, and aggregate guards.
- ✓ Phase 7 completed release verification with current installer artifact hash, install/reinstall/uninstall VM iterations, protocol recovery evidence, host-app/DPI human verification, and post-aggregate regression repairs.

### Active

- [ ] Prepare v1 distribution packaging and release-note handoff using the completed Phase 7 evidence packet.

### Out of Scope

- Retaining Moqi branding or Moqi product behavior — this repo is a scaffold only.
- Shipping user-visible fcitx features or references — not part of the Windows TypeDuck target.
- Shipping cloud clipboard, WebDAV, or AI controls from the scaffold — not part of the stated TypeDuck Windows scope.
- Supporting arbitrary excessive customization beyond TypeDuck Web alpha settings — product parity matters more than configurability.
- Installing as Chinese (Simplified) — explicitly rejected by product requirement.

## Context

- The local TypeDuck Web clone lives at `I:\GitHub\TypeDuck-Web`; the alpha dev server is `http://localhost:5173/TypeDuck-Web/aap2-alpha/`.
- The TypeDuck Web alpha settings observed on 2026-06-23 include: Display Languages, Candidates Jyutping, Auto-completion, Auto-correction, Auto-composition, Input Memory, Reverse Lookup Settings, Show Full Input Code, Cangjie Version, Chinese Typeface, and No. of Candidates Per Page.
- Display language options currently visible in the alpha are English, Hindi, Indonesian, Nepali, and Urdu, with a main-language marker and per-language enablement.
- Phase 5 human verification accepted the native candidate list plus dictionary panel containing Chinese terms, Jyutping, part-of-speech/register/label pills, English meanings, reading notes, canonical/written/vernacular/other-data rows, and multilingual rows when available.
- Phase 5 screenshot fixtures under `.planning/product/ui-fixtures` were removed. Historical Web alpha fixtures under `.planning/product/web-alpha-fixtures` remain source references, but Phase 5 acceptance no longer uses screenshot tests.
- The current scaffold uses C++20, Win32/COM/TSF, libIME2, libuv, protobuf, JsonCpp, spdlog, Inno Setup, and PowerShell packaging.
- Project `.ps1` scripts should be run with PowerShell 7+ (`pwsh`), not Windows PowerShell `powershell.exe`, to avoid character-encoding failures with Cantonese/Traditional Chinese literals, paths, and proof evidence.
- Documentation/evidence timestamps must use invariant UTC formatting so locale-specific time separators do not leak into docs. Use `pwsh -NoProfile -Command "([DateTime]::UtcNow.ToString('s', [Globalization.CultureInfo]::InvariantCulture) + 'Z')"`.
- The current scaffold launches a legacy backend through `backends.json`, named pipes, and protobuf frames. Phase 2 proved this boundary can host the TypeDuck-HK runtime as an internal compatibility adapter; later phases still need TypeDuck protocol, installer, and UI integration.
- Codebase map reference: `.planning/codebase/`.

## Constraints

- **Product parity**: TypeDuck Web alpha is the source of truth for behavior and UI until official release.
- **Language**: Every user-facing string must be bilingual Traditional Hong Kong Chinese and English.
- **Windows locale**: TSF profile registration must target Chinese (Traditional, Hong Kong) / `zh-HK`.
- **Engine dependency**: Use the TypeDuck-HK librime fork, despite it being older than the scaffold's current librime expectations.
- **Dictionary data**: The `rime-dictionary-lookup-filter` plugin is required to surface translations and dictionary-like data.
- **Scaffold divergence**: Divergence from Moqi is expected; preserve useful Windows architecture, not Moqi product choices.
- **Installer**: Settings must be shown during installation, with language picker prioritized.
- **UX scope**: Remove unused and excessive features; keep only TypeDuck-relevant settings and Web-alpha-accepted customization.
- **Compatibility**: Changes must respect Windows TSF/COM bitness, installer registration, and host-process behavior.

## Verification Environment Expectations

- **Phase 3 requires Windows VM evidence.** Installer, TSF registration, dual-bitness DLL deployment, language profile placement, uninstall/reinstall, and reboot-required registration behavior must be verified in a clean Windows 10/11 VM or equivalent disposable Windows test environment. Prefer Hyper-V checkpoints before install tests.
- **Phase 4 is mostly code-testable, with a VM smoke once TSF typing is wired.** Protocol, frame bounds, payload parsing, engine adapter behavior, and timeout handling should have automated code tests; real TSF typing and host-process responsiveness still need a targeted Windows VM smoke test before the phase is accepted.
- **Phase 5 is closed by human verification.** Earlier VM screenshot-manifest tests were explicitly replaced by user human verification on 2026-06-26. Do not recreate `.planning/product/ui-fixtures` as a Phase 5 acceptance gate; future screenshots belong to Phase 7 release evidence if desired.
- **Phase 6 is complete.** Static audits, code tests, backend guards, and Phase 7 live follow-up cover banned strings, removed handlers, parser bounds, logging defaults, installed-path privacy, launcher/runtime behavior, config-tool removal, pipe behavior, and absence of legacy feature surfaces.
- **Phase 7 is complete.** The final evidence packet covers clean install/reinstall/uninstall iterations, reboot-required registration path guards, host app and DPI human verification, bitness, protocol recovery, current installer hash, workflow/schema-source guards, and no-screenshot verification boundaries.

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Treat Moqi as scaffold, not product target | User explicitly wants to turn another product into TypeDuck; divergence is expected. | - Pending |
| Use TypeDuck Web alpha as behavioral and visual source of truth | The alpha will become the shipped public web product. | - Pending |
| Use Quality model profile for GSD planning | User approved Quality for model profile and defaults for other workflow settings. | - Pending |
| Structure work as scoped milestone phases | Phase work should stay tied to the roadmap goal and verification evidence without requiring MVP-mode user-story gates for technical spikes. | Accepted after Phase 2 |
| Centralize TypeDuck identity before production renames | Phase 1 locked first-party names, proposed AppId/CLSID/profile GUIDs, zh-HK bilingual profile text, paths, registry keys, and artifact names before implementation consumes them. | Accepted in Phase 1 |
| Treat dictionary layout as source-code-followable, with screenshots as evidence | The Web alpha source defines exact layout while captured dictionary-panel screenshots prove the panel can be inspected visually. | Accepted in Phase 1 |
| Use moqi-ime only as an internal TypeDuck runtime proof adapter | Phase 2 proved TypeDuck-HK librime and lookup-filter behavior through the existing process boundary while keeping Moqi/fcitx/cloud/AI surfaces banned from product truth. | Accepted in Phase 2 |
| Preserve raw lookup-filter payloads before designing final protocol/UI fields | Phase 2 captured the exact CSV columns and control-separator semantics so Phase 4/5 can map data without guessing. | Accepted in Phase 2 |
| Run project PowerShell scripts with `pwsh` | PowerShell 7+ avoids character-encoding failures observed with Cantonese/Traditional Chinese literals and proof artifacts; future plans and verification commands should prefer `pwsh -NoProfile -ExecutionPolicy Bypass -File ...`. | Accepted after Phase 2 UAT |
| Remove `ime.json` as product authority after transition | It can help bridge the Moqi scaffold, but the final TypeDuck product must not depend on backend JSON for Windows profile identity, settings entry points, or capability contracts. | Accepted before Phase 3 planning |
| Use disposable Windows VM evidence for Windows-integrated phases | TSF registration, system DLL registration, host-app candidate placement, and installed launcher behavior cannot be proven by code-only tests. | Accepted before Phase 3 planning |
| Close Phase 4 on live VM UAT plus static guards | The user installed the rebuilt installer in the VM and confirmed TypeDuck typing and schema replacement worked; deeper launcher fault-injection probes are valuable but not blocking for this MVP phase. | Accepted in Phase 4 |
| Keep TypeDuck schemas in the sibling runtime build | Schema filtering, prebuild with `rime_deployer.exe --build`, and packaged `build` artifacts belong to `moqi-ime`; this repo should not add a schema submodule. | Accepted in Phase 4 |
| Close Phase 5 on human verification plus static guards | The user iteratively verified candidate/dictionary/settings/About behavior in the VM and explicitly retired screenshot tests and `.planning/product/ui-fixtures`; release-grade screenshot matrices move to Phase 7. | Accepted in Phase 5 |
| Close Phase 6 on focused guards plus Phase 7 live follow-up | Static guards prove privacy/security cleanup, while Phase 7 VM iterations closed the remaining live installer/launcher/path behavior. | Accepted in Phase 6/7 |
| Keep screenshot-driven release verification interactive | User explicitly completed host-app/DPI judgement without screenshot capture; future regressions should preserve direct human judgement unless requirements change. | Accepted in Phase 7 |
| Keep only `resources\TypeDuck_Small.ico` as a raw installed icon | The Windows system IME picker needs a file path, but old `icons` folders and other raw icon files remain pruned. | Accepted in Phase 7 |

## Evolution

After each phase transition:
1. Move validated requirements into Validated with phase references.
2. Move rejected or no-longer-relevant requirements into Out of Scope with reasons.
3. Add newly discovered TypeDuck parity gaps to Active.
4. Log decisions that affect engine boundary, TSF registration, installer behavior, or UI parity.
5. Re-check whether TypeDuck Web alpha has changed before locking user-facing behavior.

After each milestone:
1. Review all product strings for bilingual Traditional Hong Kong Chinese and English coverage.
2. Audit installer/profile behavior against Chinese (Traditional, Hong Kong).
3. Audit visible UI for Moqi/fcitx/WebDAV/AI leakage.
4. Confirm candidate/dictionary display against TypeDuck Web alpha.

---
*Last updated: 2026-06-28 after Phase 7 closeout*
