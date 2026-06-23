# Architecture Research

**Domain:** Windows TSF Cantonese IME rewrite from Moqi scaffold to TypeDuck Windows IME
**Researched:** 2026-06-23
**Confidence:** HIGH for current scaffold architecture, MEDIUM for TypeDuck Web parity details because Web source was provided as target context rather than read from this workspace.

## Recommendation

Keep the current in-process TSF DLL plus per-user launcher boundary, but replace the legacy Moqi backend process with a TypeDuck engine host that embeds the TypeDuck-HK librime fork, TypeDuck schemas, and `rime-dictionary-lookup-filter`.

Do not put librime directly inside `MoqiTextService.dll` for the first milestone. The TSF DLL is loaded into arbitrary 32-bit and 64-bit host processes, and current code already has a working isolation pattern through `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/PipeServer.cpp`, `MoqLauncher/BackendServer.cpp`, and `proto/moqi.proto`. Preserve that boundary while replacing the payload and protocol semantics. The TypeDuck engine host can be a dedicated native executable, for example `TypeDuckEngine.exe`, launched by the per-user launcher and spoken to through framed protobuf.

Make TSF registration and visible product identity first-party Windows code, not backend JSON. Current registration loads profile name, GUID, locale, fallback locale, and icon from backend `input_methods/*/ime.json` in `MoqiTextService/DllEntry.cpp:174` and `MoqiTextService/DllEntry.cpp:211`; the launcher maps the same JSON GUID to a backend in `MoqLauncher/PipeServer.cpp:157` and `MoqLauncher/PipeServer.cpp:177`. For TypeDuck, profile identity must be deterministic: Chinese (Traditional, Hong Kong), `zh-HK`, bilingual profile/display text, fixed TypeDuck profile GUID, fixed TypeDuck TSF CLSID, and TypeDuck icon resources. Backend metadata should describe engine assets and schemas, not decide whether the Windows profile is Hong Kong Traditional Chinese.

## Target System Overview

```text
Windows TSF host processes
  |
  | load 32-bit or 64-bit TSF COM DLL
  v
TypeDuckTextService.dll
  - TSF activation, key sinks, composition ranges
  - thin TypeDuck controller and UI adapter
  - candidate window, dictionary panel, settings/about entry points
  |
  | per-user named pipe, bounded framed protobuf
  v
TypeDuckLauncher.exe
  - one instance per user/session
  - named pipe server and backend supervision
  - tray/status only if product-required
  |
  | stdio or local IPC, framed protobuf
  v
TypeDuckEngine.exe
  - embeds TypeDuck-HK librime fork
  - loads TypeDuck schemas and user data
  - loads rime-dictionary-lookup-filter
  - returns candidates, lookup details, composition, and settings state
```

## Component Boundaries

| Component | Responsibility | Keep From Scaffold | Replace or Add |
|-----------|----------------|--------------------|----------------|
| `libIME2/src` TSF foundation | Generic COM/TSF activation, key event dispatch, edit sessions, composition ranges, candidate UI element interfaces. | Keep as generic platform layer. Current registration implementation is in `libIME2/src/ImeModule.cpp:217`, `libIME2/src/ImeModule.cpp:241`, and user-profile registry writes start around `libIME2/src/ImeModule.cpp:281`. | Only change for generic TSF bugs or zh-HK registration mechanics that cannot live above it. Do not put TypeDuck product behavior here. |
| TypeDuck TSF service | Product controller loaded in host processes. Converts TSF callbacks into engine requests and applies composition/candidate responses. | Start from `MoqiTextService/MoqiTextService.cpp` and `MoqiTextService/MoqiClient.cpp`. Candidate UI lifecycle currently sits around `MoqiTextService/MoqiTextService.cpp:811` and response handling around `MoqiTextService/MoqiClient.cpp:386`. | Rename/re-scope to TypeDuck, remove Moqi/fcitx/WebDAV/AI behavior, add TypeDuck candidate/dictionary view model and bilingual copy. |
| TypeDuck engine client protocol | Stable boundary between TSF host and engine. Must be narrow, bounded, and versioned. | Reuse framed protobuf and named-pipe connection pattern in `proto/ProtoFraming.h`, `MoqiTextService/MoqiClient.cpp:1505`, and `MoqLauncher/BackendServer.cpp:292`. | Replace `moqi.protocol` with `typeduck.protocol.v1`; add dictionary lookup, candidate metadata, settings, capabilities, and protocol version fields. Remove cloud clipboard and generic Moqi UI customization. |
| Launcher | Per-user process that isolates engine lifetime and avoids loading engine dependencies into every host app. | Keep `MoqLauncher/PipeServer.cpp` named pipe server, backend mapping, single-user pipe naming, and process supervision shape. | Rename namespaces, pipe path, mutex, data/log directories. Remove cloud clipboard listener in `MoqLauncher/PipeServer.cpp:678` and tray notifications unless product-required. |
| TypeDuck engine host | Owns librime, schema deployment, dictionary lookup plugin, and user data. | Keep the backend process launch model in `MoqLauncher/BackendServer.cpp:171` and staged runtime payload shape in `scripts/install.ps1:165`. | New executable/package that embeds TypeDuck-HK librime and `rime-dictionary-lookup-filter`, exposes TypeDuck protocol, and persists settings in TypeDuck paths. |
| Candidate and dictionary UI | Presents TypeDuck Web alpha candidate list and dictionary panel in native Win32/TSF-compatible UI. | Use `MoqiTextService/MoqiCandidateWindow.cpp` only as window ownership/TSF UI element scaffold. It currently has flat `text/comment` candidates and a 10-item TSF count cap in `MoqiTextService/MoqiCandidateWindow.cpp:319`. | Introduce `CandidateViewModel`, `DictionaryLookupViewModel`, and renderer code that supports Chinese text, Jyutping, part of speech, multilingual meanings, and "More Languages" sections. |
| Settings and about UI | Native dialog matching Web alpha settings and bilingual text. Must be callable during install and from IME configuration. | Current `MoqiTextService/MoqiImeModule.cpp:145` launches arbitrary backend `configTool` from `ime.json`. | Replace with first-party `TypeDuckSettings.exe` or dialog DLL entry, backed by a stable settings file and engine apply/redeploy command. Add About dialog and installer launch step. |
| Installer and registration | Installs both TSF bitnesses, registers COM/TSF profile, stages runtime assets, launches settings. | Keep `SetupHelper/SetupHelper.cpp` dual-bitness copy/register mechanics and Inno + PowerShell staging. | Replace `installer/MoqiTsf.iss` Moqi identity, Simplified Chinese language file, `{autopf32}\MoqiIM`, `MoqiLauncher.exe`, and `MoqiTextService.dll` paths. Ensure zh-HK profile registration is deterministic. |

## Engine Boundary Decision

### Recommended: Out-of-Process Librime Behind TypeDuck Engine Host

**Use because:** TSF DLLs are loaded into many host processes, in both bitnesses. Loading librime, schema assets, dictionary plugin code, and user-data mutation into every host process raises crash risk, versioning risk, dependency conflicts, and performance surprises. The scaffold already isolates backend lifetime through the launcher and backend process. Use that hard-won boundary.

**Implementation shape:**
- Add a TypeDuck backend entry to a renamed manifest, for example `typeduck-engine.json`, or replace manifest discovery with first-party constants if only one engine exists.
- Package `TypeDuckEngine.exe`, TypeDuck-HK librime DLLs, schemas, dictionaries, and `rime-dictionary-lookup-filter` under the TypeDuck install directory.
- Keep `TypeDuckTextService.dll` synchronous enough for TSF key decisions, but bound all per-key IPC with strict timeouts and a degraded-mode path.
- Add protocol size caps before accepting frames. `Proto::FrameBuffer` currently trusts a 32-bit length field, which is a high-risk boundary for both launcher and engine.

**Trade-offs:** More IPC work and protocol design now, but fewer host-process crashes and easier engine updates. This is the right trade for a Windows IME that must run inside Office, browsers, terminals, elevated apps, and 32-bit legacy apps.

### Alternative: In-Process Librime Inside TSF DLL

**Reject for first milestone.** It offers lower per-key latency and a simpler mental model, but it forces a 32-bit and 64-bit librime/plugin runtime, loads third-party engine code into arbitrary host processes, and makes engine crashes take down the target application. It also makes settings/schema deployment harder because every host process can become an engine owner.

Use in-process librime only if later measurement proves pipe/backend latency cannot meet typing requirements after protocol trimming and bounded fast paths. If revisited, put librime behind a `TypeduckEngine` C++ interface and build both bitnesses with identical plugin availability before moving it into the TSF DLL.

### Alternative: Keep Legacy Moqi Backend Compatibility

**Reject.** The Moqi backend contract preserves product decisions that TypeDuck explicitly does not want: cloud clipboard, generic UI customization, Moqi/fcitx surfaces, and backend-owned profile metadata. The Windows IPC scaffolding is useful; the Moqi product protocol is not the target.

## Protocol and Schema Changes

Replace `proto/moqi.proto` with a TypeDuck protocol or introduce `proto/typeduck.proto` while both client and engine migrate. The current schema contains useful concepts but is too flat for dictionary parity.

| Area | Current Shape | Required TypeDuck Shape |
|------|---------------|-------------------------|
| Candidate list | `candidate_list` strings plus `CandidateEntry{text, comment}` in `proto/moqi.proto:73` and `proto/moqi.proto:158`. | `Candidate` with stable id, display text, input code, Jyutping, prompt/comment, source schema, quality flags, selection key, and optional lookup reference. |
| Dictionary panel | Not modeled. Data can only be squeezed into `comment`. | `DictionaryLookup` payload with headword, jyutping readings, part-of-speech labels, English meaning, enabled-language meanings, notes, examples if available, and source/collation order from `rime-dictionary-lookup-filter`. |
| Settings | Generic `CustomizeUi` colors/fonts plus legacy behavior toggles in `proto/moqi.proto:81`. | `SettingsSnapshot` matching Web alpha: display languages, candidate Jyutping, completion, correction, sentence/composition, learning, reverse lookup, full input code, Cangjie 5, Chinese typeface, page size. |
| Settings mutation | Backend config tool launched from `ime.json` in `MoqiTextService/MoqiImeModule.cpp:145`. | `GetSettings`, `SetSettings`, `ApplySettings`, `DeploySchema`, and `RestartEngine` messages. Settings UI writes through TypeDuck settings service or engine RPC, not arbitrary JSON command launch. |
| Feature discovery | Implicit by backend behavior. | `Capabilities` response on init: dictionary lookup available, plugin version, schema list, enabled languages, maximum page size, reverse lookup support. |
| Legacy features | `METHOD_CLOUD_CLIPBOARD_UPLOAD` in `proto/moqi.proto:27`, tray notifications, generic buttons/menus. | Remove from MVP unless TypeDuck explicitly needs them. Do not expose WebDAV, AI, or fcitx surfaces. |
| Reliability | No explicit protocol version or frame size limit. | Add protocol version, client build, engine build, max frame size, request deadline, error category, and reset/recover messages. |

Recommended message flow:

```text
TSF activate
  -> Init(profile_guid, app_context, protocol_version)
  <- InitResult(capabilities, settings_snapshot, engine_status)

Key down/filter
  -> ProcessKey(key_event, composition_state, deadline_ms)
  <- InputState(preedit, commit, candidates, dictionary_lookup, cursor, page_info)

Candidate hover/select/page
  -> CandidateAction(candidate_id, action)
  <- InputState(...)

Settings change
  -> SetSettings(settings_delta, apply_mode)
  <- SettingsResult(settings_snapshot, deploy_required, engine_status)
```

## Dictionary Lookup Data Flow

The dictionary lookup filter should be an engine-side enrichment step, not UI-side parsing of candidate comments.

1. `TypeDuckEngine.exe` receives a key event and passes it to the TypeDuck-HK librime session.
2. Librime returns composition, page candidates, and candidate metadata.
3. `rime-dictionary-lookup-filter` adds lookup records for the active candidate or page candidates.
4. The engine converts Rime/plugin data into `Candidate` and `DictionaryLookup` protobuf messages.
5. `TypeDuckTextService.dll` converts protobuf to a native `CandidatePanelViewModel`.
6. `TypeDuckCandidateWindow` renders compact candidates and an optional dictionary panel with Web-alpha-equivalent fields.

Do not let the Win32 renderer parse display strings such as `候選 詞性 meaning`. That makes localization and Web parity fragile. The renderer should receive already-separated fields and focus on layout, keyboard interaction, accessibility/TSF UI element reporting, and host-window positioning.

## Candidate UI Architecture

Current `MoqiCandidateWindow.cpp` is a usable HWND and `ITfCandidateListUIElement` scaffold, but not enough for TypeDuck Web alpha parity. It measures and paints flat rows with candidate text and a single comment field, uses a white/blue palette in `MoqiTextService/MoqiCandidateWindow.cpp:13`, and caps TSF-reported count at 10 in `GetCount`.

Recommended UI split:

| Module | Responsibility |
|--------|----------------|
| `CandidatePanelViewModel` | Pure data from engine: preedit, candidates, selected id, page info, selection keys, lookup data, display language state. |
| `CandidateWindowController` | TSF session coordination: create/show/hide/update window, owner synchronization, UI-less handling, `ITfUIElementMgr` notifications. Current lifecycle code in `MoqiTextService/MoqiTextService.cpp:777` through `MoqiTextService/MoqiTextService.cpp:923` belongs here. |
| `CandidatePanelRenderer` | Paint compact TypeDuck candidate list with warm amber accent, neutral surface, rounded but compact controls, HK Chinese/English font stack, and no oversized marketing styling. |
| `DictionaryPanelRenderer` | Paint selected-candidate lookup details: Chinese term, Jyutping, labels, English meaning, enabled "More Languages" blocks. It should have a measured max width/height and scroll or truncate safely. |
| `CandidateAccessibilityAdapter` | Maps the visible candidate page to TSF candidate UI element methods. Keep candidate count aligned with page size and selection keys. |

Implementation should preserve the current host-window hardening work: owner resolution from TSF active view, `GetFocus`, and `GetForegroundWindow`; topmost/no-activate candidate window style; immersive-app handling; and candidate-window trace logging. These are technically useful scaffold pieces even though the visual rendering must change.

## Settings and About Architecture

Build settings as a first-party TypeDuck surface, not as a backend-provided config tool.

Recommended shape:

```text
TypeDuckSettings.exe or native dialog module
  |
  +-- Settings UI: bilingual Traditional Hong Kong Chinese and English
  +-- Settings store: %LOCALAPPDATA%\TypeDuck\settings.json
  +-- Engine RPC: validate/apply/deploy settings
  +-- Installer mode: first-run settings with language picker first
```

Settings parity targets from provided Web alpha context:
- Display Languages with main language and enabled languages.
- Candidates Jyutping.
- Auto-completion, auto-correction, auto-composition/sentence, input memory/learning.
- Reverse lookup settings.
- Show full input code.
- Cangjie version.
- Chinese typeface.
- Number of candidates per page.

Architecture constraints:
- Settings labels and messages must be bilingual Traditional Hong Kong Chinese and English.
- The installer must launch settings during installation after files are staged and before final launch, using Inno `[Run]` or a custom wizard page in `installer/MoqiTsf.iss`.
- Engine settings should be validated by the engine because toggles such as completion, correction, sentence, learning, and Cangjie schema affect librime configuration.
- The TSF configuration entry should open the same settings surface. Replace `MoqiTextService/MoqiImeModule.cpp:145` arbitrary `configTool` lookup with a fixed TypeDuck settings command.
- About dialog should read product version from the same CMake/version resource path as the TSF DLL and launcher, not hard-coded strings.

## Installer and zh-HK Registration Architecture

Current installer architecture is useful but Moqi-specific. `installer/MoqiTsf.iss:23` installs to `{autopf32}\MoqiIM`; `installer/MoqiTsf.iss:36` uses only Simplified Chinese; `installer/MoqiTsf.iss:50` launches `MoqiLauncher.exe`; and `scripts/install.ps1:165` stages `win32\MoqiIM` and `x64\MoqiIM`. These must become TypeDuck-specific.

Recommended registration contract:

| Field | TypeDuck Target |
|-------|-----------------|
| TSF CLSID | New stable TypeDuck CLSID, synchronized across `TypeDuckImeModule.cpp`, Inno cleanup, resources, and setup helper. |
| Profile GUID | New stable TypeDuck Cantonese profile GUID. Do not derive from backend `ime.json`. |
| Locale | `zh-HK`. |
| Fallback locale | `zh-Hant-HK` or an explicitly verified fallback only if Windows conversion requires it. The primary profile must still appear under Chinese (Traditional, Hong Kong). |
| Profile name | Bilingual Traditional HK Chinese and English, for example `TypeDuck 粵語輸入法 / TypeDuck Cantonese`. |
| Install directory | `%ProgramFiles(x86)%\TypeDuckIME` or `%ProgramFiles(x86)%\TypeDuck`. |
| User data | `%LOCALAPPDATA%\TypeDuck` for logs/settings/user data unless roaming is intentionally required. |
| Pipe path | `\\.\pipe\<username>\TypeDuckIME\Launcher`, not `MoqiIM`. |
| Startup key | `TypeDuckLauncher`, not `MoqiLauncher`. |

Keep `SetupHelper` as the elevated system-DLL/register helper because TSF bitness deployment is fragile. Refactor constants first: environment variable, scheduled task name, DLL names, source/target paths, process names, and log location. Then update registration logic. The current `MOQI_PROGRAM_DIR` path in `SetupHelper/SetupHelper.cpp:17` and `MoqiTextService/MoqiImeModule.cpp:45` should become a TypeDuck constant.

## Proposed Project Structure

Use the current directories during early migration to avoid destabilizing CMake, then rename once the TypeDuck slice works. Add boundaries before wholesale file renames.

```text
MoqiTextService/                  # temporary scaffold name until rename phase
  TypeDuckProfile.*               # first-party CLSID/profile/locale/icon constants
  TypeDuckSettingsClient.*        # opens settings, sends settings RPC
  CandidateViewModel.*            # renderer-neutral candidate and dictionary data
  TypeDuckCandidateWindow.*       # eventual replacement for MoqiCandidateWindow
  TypeDuckProtocolAdapter.*       # protobuf-to-viewmodel and TSF request mapping

MoqLauncher/
  TypeDuckLauncherConfig.*        # pipe names, app paths, logs, engine manifest
  TypeDuckEngineSupervisor.*      # renamed/refined BackendServer

TypeDuckEngine/                   # new runtime host
  main.cpp
  RimeEngine.*
  DictionaryLookupMapper.*
  SettingsStore.*
  ProtocolServer.*

proto/
  typeduck.proto

installer/
  TypeDuckTsf.iss
  TraditionalChineseHongKong.isl

scripts/
  build.ps1
  install.ps1                    # parameterized product/stage constants
```

## Phaseable Migration Path

1. **Product identity and zh-HK registration contract**
   - Centralize TypeDuck product constants, new CLSID/profile GUID, `zh-HK` locale, install paths, pipe paths, log paths, resource strings, and bilingual profile text.
   - Update `DllEntry.cpp`, `MoqiImeModule.cpp`, `libIME2/src/ImeModule.cpp` usage, `SetupHelper.cpp`, `installer/MoqiTsf.iss`, and `scripts/install.ps1`.
   - Risk avoided: registering no profile or registering under Simplified Chinese because backend JSON is missing or wrong.

2. **TypeDuck engine host behind existing launcher**
   - Add `TypeDuckEngine.exe` with minimal librime init, schema load, key processing, and plain candidates.
   - Keep `MoqLauncher/BackendServer.cpp` supervision but rename paths and remove Moqi backend assumptions.
   - Risk avoided: direct in-process librime destabilizing host applications before parity is proven.

3. **Protocol v1 and dictionary lookup payload**
   - Add `typeduck.proto`, version handshake, frame caps, candidate metadata, dictionary lookup messages, and settings messages.
   - Migrate `MoqiTextService/MoqiClient.cpp` response handling into `TypeDuckProtocolAdapter`.
   - Risk avoided: flattening dictionary records into candidate comments and blocking Web parity.

4. **Candidate and dictionary UI parity**
   - Build native view model and renderer around Web alpha structures: candidate, Jyutping, translations, part-of-speech, "More Languages".
   - Use `Preview/main.cpp` as a standalone renderer harness before wiring into TSF.
   - Risk avoided: debugging visual layout only inside live TSF host processes.

5. **Settings, installer first-run, and about**
   - Replace backend `configTool` lookup with first-party settings UI.
   - Launch settings during install, language picker first.
   - Persist/apply settings through engine RPC and settings store.
   - Risk avoided: settings UI diverging from Web alpha or writing engine config the running backend does not understand.

6. **Remove scaffold features and finish rename**
   - Remove cloud clipboard, AI/WebDAV/fcitx references, Moqi tray/actions, generated stale protobuf artifacts if no longer needed, and legacy docs.
   - Rename binaries/directories after constants and tests are stable.
   - Risk avoided: accidental product leakage from Moqi into TypeDuck release artifacts.

## High-Risk Boundaries and Sequencing Constraints

| Boundary | Risk | Required Sequencing |
|----------|------|---------------------|
| TSF registration | Broken install, wrong language, orphaned TIP registry keys. | Define TypeDuck CLSID/profile/locale before engine work. Verify clean install, upgrade, uninstall, reboot-required replacement on Windows 10/11. |
| Bitness | 64-bit hosts need x64 TSF DLL while launcher and setup are currently Win32-oriented. | Preserve dual TSF builds and system-directory registration while renaming. Do not require x64 launcher unless tested. |
| Key handling IPC | `filterKeyDown` and `onKeyDown` synchronously call backend paths; latency or dead engine can block typing. | Add deadlines, fast-fail circuit breaker, and local consumed-key policy before increasing engine complexity. |
| Protocol frames | Current frame buffer has no documented max payload. Dictionary data can be larger than flat candidates. | Add size caps and bounded dictionary payloads in the protocol phase before enabling rich lookup. |
| Candidate window host ownership | Office, browsers, terminal, UWP/immersive, elevated apps, and multi-monitor DPI behave differently. | Preserve scaffold owner-resolution logic, add preview harness, then manually smoke-test representative hosts after each UI change. |
| Settings deploy | Rime settings often require schema reload/deploy. | Route settings through engine validation and explicit deploy/restart status; do not let UI directly edit arbitrary Rime files. |

## Anti-Patterns to Avoid

### Backend-Owned Windows Identity

**What people do:** Keep `ime.json` as the source for TypeDuck profile name, GUID, locale, and icon.
**Why it is wrong:** TypeDuck must always install under Chinese (Traditional, Hong Kong). A missing engine payload should not silently unregister or misregister the Windows profile.
**Do this instead:** Put TypeDuck profile constants in first-party Windows code and use engine metadata only for schemas/runtime capabilities.

### Dictionary Data in Candidate Comments

**What people do:** Store Jyutping, translation, part-of-speech, and multilingual meanings in one `comment` string.
**Why it is wrong:** It cannot support the Web alpha dictionary panel cleanly, makes bilingual UI brittle, and blocks accessibility/testing.
**Do this instead:** Use structured `DictionaryLookup` messages and renderer-neutral view models.

### In-Process Engine First

**What people do:** Load librime and plugins directly inside the TSF DLL to avoid IPC.
**Why it is wrong:** It spreads engine dependencies into every host app and doubles plugin/bitness problems.
**Do this instead:** Ship an out-of-process engine host first, measure latency, then reconsider only with data.

### One-Off Product Rename

**What people do:** Rename installer text but leave pipe paths, log directories, startup registry keys, CLSID cleanup, and resource metadata as Moqi.
**Why it is wrong:** Side-by-side conflicts and user-visible leakage are almost guaranteed.
**Do this instead:** Create a product identity contract and apply it across code, installer, scripts, resources, logs, pipes, and CI artifacts.

### Arbitrary Config Tool Launch

**What people do:** Preserve `configTool` from backend `ime.json`.
**Why it is wrong:** It delegates product settings UX and execution trust to installed JSON.
**Do this instead:** Launch a fixed signed TypeDuck settings surface and communicate with the engine through typed settings RPC.

## Integration Points

| Integration | Pattern | Notes |
|-------------|---------|-------|
| Windows TSF | COM DLL registration, `ITfInputProcessorProfiles`, profile manager, language profile enablement. | Current implementation in `libIME2/src/ImeModule.cpp` is useful but must be verified for `zh-HK`. |
| Launcher IPC | Per-user named pipe with framed protobuf. | Rename and harden. Keep local-session boundary. |
| Engine process | Launcher-supervised hidden process with stdin/stdout framed protobuf. | Replace `moqi-ime\server.exe` in `backends.json:4` with TypeDuck engine package or first-party engine config. |
| librime | Embedded in `TypeDuckEngine.exe`. | Engine owns Rime sessions, schema deploy, user data, plugin load, and dictionary lookup mapping. |
| Installer | Inno Setup plus `SetupHelper`. | Replace Simplified Chinese translation with Traditional HK Chinese and English. Launch settings during install. |
| Settings store | Local user JSON or engine-owned Rime config. | Use TypeDuck setting names mirroring Web alpha, validate through engine, and apply via deploy/restart when required. |

## Roadmap Implications

- Start with registration/identity because engine and UI work are not useful if Windows installs the TIP under the wrong language or with Moqi identity.
- Keep the launcher boundary through MVP. It is the least risky way to integrate the older TypeDuck-HK librime fork and plugin without destabilizing host apps.
- Protocol work must precede dictionary UI. The UI cannot be made Web-alpha-equivalent if the engine only sends text/comment pairs.
- Build a standalone candidate preview early. Native TSF UI debugging is slow, and the existing `Preview/main.cpp` target is a good place to lock layout, bilingual text, and dictionary panel behavior before live TSF integration.
- Settings should follow engine/protocol basics but precede installer polish, because installer first-run must show the real settings UI and language picker.

## Sources

- `.planning/PROJECT.md` for TypeDuck product requirements, Web alpha parity scope, bilingual requirement, and zh-HK install target.
- `.planning/codebase/ARCHITECTURE.md` for current TSF DLL, launcher, backend process, protocol, and installer flow.
- `.planning/codebase/STRUCTURE.md` for file ownership and recommended extension points.
- `.planning/codebase/INTEGRATIONS.md` for named pipe, local backend, TSF, installer, and logging integration points.
- `.planning/codebase/CONCERNS.md` for high-risk areas: backend boundary, profile metadata, protocol gaps, installer, candidate UI, and legacy feature leakage.
- `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`.
- `MoqLauncher/PipeServer.cpp`, `MoqLauncher/BackendServer.cpp`, `proto/moqi.proto`, `SetupHelper/SetupHelper.cpp`, `installer/MoqiTsf.iss`, `scripts/install.ps1`, `backends.json`.
- Provided TypeDuck target context for Web alpha files: `CandidatePanel.tsx`, `Candidate.tsx`, `DictionaryPanel.tsx`, `CandidateInfo.ts`, `Preferences.tsx`, and `worker.ts customize()`.

---
*Architecture research for: TypeDuck Windows IME*
*Researched: 2026-06-23*
