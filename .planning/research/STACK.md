# Project Research: Stack

**Research Date:** 2026-06-23
**Project:** TypeDuck Windows IME
**Confidence:** Medium-high for Windows scaffold choices; medium for librime integration details until the TypeDuck-HK fork is built inside this repo.

## Executive Summary

Keep the repo's Windows-native foundation: C++20, Win32/COM/TSF, CMake, Visual Studio, Inno Setup, and the existing dual-bitness installer flow. Replace the Moqi product stack above that foundation with a TypeDuck-specific identity, TypeDuck-HK librime runtime, TypeDuck schema assets, and `rime-dictionary-lookup-filter` data path.

The safest engine strategy for early phases is to preserve the existing launcher/backend boundary while replacing the legacy Moqi backend with a TypeDuck engine backend. That limits TSF host-process risk and lets librime/plugin crashes stay outside host apps. A later in-process engine can be considered only after protocol, settings, and candidate/dictionary data are stable.

## Recommended Stack

### Windows Frontend

- **C++20 + Win32/COM/TSF**: Keep for `MoqiTextService/`, `libIME2/src/`, `SetupHelper/`, and candidate UI. This is already the operational Windows IME surface.
- **libIME2 TSF layer**: Keep as scaffold for activation, composition, edit sessions, language profile registration, and candidate-window plumbing. Avoid putting TypeDuck product logic inside `libIME2/src/`.
- **GDI/User32 candidate UI initially**: Keep current candidate-window technology for v1, but refactor its data model and rendering to match TypeDuck Web alpha. Do not introduce a webview for the candidate popup unless GDI parity proves infeasible.
- **Native Win32 settings/about dialogs**: Preferred for installer/runtime settings because the current stack already ships native binaries and installer helpers. Match TypeDuck Web alpha controls and copy, not its React implementation.

### Engine Runtime

- **TypeDuck-HK/librime fork**: Required engine dependency. Add it as the canonical librime source for this product, even if older than any existing upstream assumptions.
- **TypeDuck-HK/rime-dictionary-lookup-filter**: Required plugin dependency. Ensure it is built, packaged, and configured with schemas that reference `dictionary_lookup_filter`.
- **TypeDuck Web schema assets**: Use `schema/template.yaml`, `schema/jyut6ping3.schema.yaml`, `schema/jyut6ping3_scolar.dict.yaml`, Cangjie schemas, OpenCC data, and built schema assets as the functional source of truth.
- **OpenCC/schema resources**: Package only the TypeDuck-required resources. Avoid carrying unrelated Moqi/fcitx assets.

### IPC and Backend Boundary

- **Keep launcher boundary for v1**: Current `MoqLauncher` + named pipe + backend process model already isolates engine lifetime from TSF host processes.
- **Evolve protocol deliberately**: Extend or replace `proto/moqi.proto` to carry TypeDuck candidate entries, Jyutping, translations, part-of-speech, reading notes, "More Languages" rows, settings, deploy/customize state, and errors.
- **Rename protocol/product surfaces**: `moqi.proto`, `MoqiClient`, pipe names, mutexes, log paths, and backend manifests should move toward TypeDuck names in a coordinated phase.

### Build and Packaging

- **CMake + Visual Studio 2022**: Keep as the Windows build system.
- **PowerShell packaging scripts**: Keep but rename/re-scope for TypeDuck. Remove sibling `moqi-ime` assumptions from `scripts/install.ps1` and `scripts/_all_in_package.ps1`.
- **Inno Setup 6**: Keep for installer output, but replace Simplified Chinese/Moqi script content with Traditional Hong Kong Chinese and English TypeDuck text.
- **Dual-bitness TSF DLLs**: Preserve Win32 and x64 TSF DLL registration because 32-bit and 64-bit Windows host processes both need the IME.

## Decisions With Rationale

| Area | Recommendation | Rationale | Confidence |
|------|----------------|-----------|------------|
| TSF foundation | Keep `libIME2/src` and existing TSF DLL shape | It already handles COM activation, edit sessions, display attributes, and bitness constraints. | High |
| Engine boundary | Start out-of-process via launcher/backend | Reduces host-process crash risk and fits current `MoqLauncher` architecture. | Medium-high |
| Candidate UI | Keep native window, rebuild model/rendering | Existing TSF candidate ownership is fragile; changing rendering is safer than changing host integration immediately. | Medium |
| Settings UI | Native dialog mirroring Web alpha | Installer/runtime settings need Windows integration and bilingual copy; React stack is not needed in-process. | Medium-high |
| Protocol | Version a TypeDuck protocol | Current protobuf is Moqi-shaped and lacks dictionary lookup structure. | High |
| Installer | Keep Inno + `SetupHelper`, rewrite product identity | Existing admin/system-DLL flow is valuable but all user-facing language/profile data must change. | High |

## What Not To Use

- **Do not keep the legacy Moqi Go backend as product architecture**. It is scaffold only and does not satisfy TypeDuck librime/plugin requirements.
- **Do not expose fcitx settings, docs, or feature references**. They are not part of the Windows product target.
- **Do not preserve WebDAV/cloud clipboard/AI scaffold features** unless the TypeDuck product explicitly adds them later.
- **Do not make backend `ime.json` the only source of zh-HK registration truth**. TypeDuck profile identity should be deterministic and first-party.
- **Do not use a WebView candidate popup by default**. It adds focus, latency, accessibility, and host-window complexity before native rendering has been tested.
- **Do not edit checked-in generated protobuf files as source of truth**. Change `.proto` and regenerate; consider removing stale generated sources from source control later.

## Integration Notes

### TypeDuck Web Parity Inputs

Observed TypeDuck Web alpha files and behavior should drive implementation:

- `I:\GitHub\TypeDuck-Web\src\Preferences.tsx`: language picker, page size, typeface, Jyutping visibility, auto-completion/correction/composition, input memory, reverse lookup, show full input code, Cangjie version.
- `I:\GitHub\TypeDuck-Web\src\CandidatePanel.tsx`, `Candidate.tsx`, `DictionaryPanel.tsx`, `CandidateInfo.ts`: candidate list and dictionary panel behavior.
- `I:\GitHub\TypeDuck-Web\src\worker.ts`: `customize({ pageSize, enableCompletion, enableCorrection, enableSentence, enableLearning, isCangjie5 })`.
- `I:\GitHub\TypeDuck-Web\schema\template.yaml`: includes `dictionary_lookup_filter`.
- `I:\GitHub\TypeDuck-Web\schema\jyut6ping3_scolar.dict.yaml`: dictionary rows with multilingual meanings.

### Existing Windows Scaffold Anchors

- `MoqiTextService/DllEntry.cpp` and `MoqiTextService/MoqiImeModule.cpp`: profile registration and product identity.
- `MoqiTextService/MoqiClient.cpp`: protocol request/response adapter.
- `MoqiTextService/MoqiCandidateWindow.cpp`: candidate rendering surface.
- `MoqLauncher/PipeServer.cpp` and `MoqLauncher/BackendServer.cpp`: backend lifecycle and IPC.
- `proto/moqi.proto`: current contract to replace or version.
- `installer/MoqiTsf.iss`, `SetupHelper/SetupHelper.cpp`, `scripts/install.ps1`: installer and registration flow.

## Open Questions

- Whether TypeDuck-HK/librime should be built as an out-of-process backend executable for v1 or linked into `MoqLauncher`.
- Whether `rime-dictionary-lookup-filter` exposes all dictionary metadata needed by the Web alpha UI or whether additional parsing is needed.
- Whether TypeDuck Web alpha settings are final enough to freeze native settings storage names now.
- Whether TypeDuck Windows should support stable/beta channel side-by-side installs.
- How much candidate visual parity is required before the first internal installer.

## Roadmap Implications

1. Start with product identity and deterministic zh-HK profile registration before deep engine work, because all later install/runtime validation depends on correct identity.
2. Establish the TypeDuck engine backend and schema/plugin packaging before candidate UI richness, because the UI needs real structured data.
3. Extend the protocol and candidate view model before polishing rendering, so dictionary and multilingual data are not squeezed through comment strings.
4. Build settings/about dialogs after core settings keys are wired to engine customization, with installer launch integration as its own acceptance path.
5. Add tests/smoke scripts around installer/profile registration, protocol framing, and candidate view model before broader UI polish.

## Sources

- `.planning/PROJECT.md`
- `.planning/codebase/STACK.md`
- `.planning/codebase/ARCHITECTURE.md`
- `.planning/codebase/INTEGRATIONS.md`
- `.planning/codebase/CONCERNS.md`
- TypeDuck Web alpha inspection at `http://localhost:5173/TypeDuck-Web/aap2-alpha/`
- TypeDuck Web local source summary from `I:\GitHub\TypeDuck-Web`

---
*Stack research completed: 2026-06-23*
