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

### Active

- [ ] Plug the TypeDuck-HK librime fork into the Windows IME runtime.
- [ ] Include the TypeDuck-HK `rime-dictionary-lookup-filter` plugin submodule/runtime support so translations and dictionary-like candidate data can be displayed.
- [ ] Replace Moqi product identity with TypeDuck identity across binaries, resources, installer, registry/profile metadata, data/log paths, and release artifacts.
- [ ] Install/register the IME under Chinese (Traditional, Hong Kong), not Chinese (Simplified).
- [ ] Mirror TypeDuck Web alpha settings in a native Windows settings dialog, with the language picker as the first priority.
- [ ] Show the settings dialog during installation.
- [ ] Display all user-facing text bilingually in Traditional Hong Kong Chinese and English.
- [ ] Render candidates with candidate text, Jyutping pronunciation, multilingual prompts, translations, and dictionary-like lookup details.
- [ ] Match TypeDuck Web alpha visual tone for candidate UI and settings: light neutral surface, warm amber primary accents, compact rounded controls, readable HK Chinese/English font stack, and clear two-language labels.
- [ ] Remove or hide unused scaffold features such as fcitx references, WebDAV/cloud clipboard, AI controls, and excessive customization not present in TypeDuck Web alpha.
- [ ] Add an About dialog for TypeDuck Windows IME.

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
- Candidate UI screenshots show the candidate list plus a larger dictionary panel containing Chinese terms, Jyutping, part-of-speech labels, English meanings, reading notes, and "More Languages" blocks when additional languages are enabled.
- The current scaffold uses C++20, Win32/COM/TSF, libIME2, libuv, protobuf, JsonCpp, spdlog, Inno Setup, and PowerShell packaging.
- The current scaffold launches a legacy backend through `backends.json`, named pipes, and protobuf frames. TypeDuck needs an engine boundary decision for librime and dictionary lookup data.
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

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Treat Moqi as scaffold, not product target | User explicitly wants to turn another product into TypeDuck; divergence is expected. | - Pending |
| Use TypeDuck Web alpha as behavioral and visual source of truth | The alpha will become the shipped public web product. | - Pending |
| Use Quality model profile for GSD planning | User approved Quality for model profile and defaults for other workflow settings. | - Pending |
| Structure work as vertical MVP phases | Each phase should move toward a usable Windows IME slice rather than isolated technical layers. | - Pending |
| Centralize TypeDuck identity before production renames | Phase 1 locked first-party names, proposed AppId/CLSID/profile GUIDs, zh-HK bilingual profile text, paths, registry keys, and artifact names before implementation consumes them. | Accepted in Phase 1 |
| Treat dictionary layout as source-code-followable, with screenshots as evidence | The Web alpha source defines exact layout while captured dictionary-panel screenshots prove the panel can be inspected visually. | Accepted in Phase 1 |

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
*Last updated: 2026-06-23 after Phase 1 verification*
