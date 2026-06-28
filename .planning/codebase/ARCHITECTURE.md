<!-- refreshed: 2026-06-28 -->
# Architecture

**Analysis Date:** 2026-06-28

## System Overview

```text
┌──────────────────────────────────────────────────────────────────────────────┐
│                       Windows TSF Host Processes                              │
│             Microsoft TSF loads `TypeDuckTextService.dll`                     │
├───────────────────────────────┬──────────────────────────────────────────────┤
│ TSF product layer             │ Generic TSF/COM layer                         │
│ `TypeDuck-Windows/MoqiTextService` │ `TypeDuck-Windows/libIME2/src`           │
└───────────────┬───────────────┴──────────────────────────────────────────────┘
                │ length-prefixed protobuf over per-user named pipe
                ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                            Launcher Process                                  │
│ `TypeDuckLauncher.exe` from `TypeDuck-Windows/MoqLauncher`                    │
│ Owns tray, settings bridge, named pipe clients, backend process lifetime      │
└───────────────┬──────────────────────────────────────────────────────────────┘
                │ length-prefixed protobuf over backend stdin/stdout
                ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                            Backend Runtime                                   │
│ `TypeDuck-Windows-backend/server.go` packaged as `TypeDuckRuntime/server.exe` │
│ Dispatches client sessions to Rime service                                    │
└───────────────┬──────────────────────────────────────────────────────────────┘
                │ Go wrapper calls Windows DLL exports
                ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│                     Rime Engine, Schema, Dictionary Data                      │
│ `TypeDuck-Windows-backend/input_methods/rime`                                 │
│ `TypeDuck-Windows/runtime`, `TypeDuck-Windows/third_party/typeduck-schema`     │
└──────────────────────────────────────────────────────────────────────────────┘
```

TypeDuck Windows v1 is a two-repository desktop IME product:

- `TypeDuck-Windows` owns Windows TSF integration, COM registration, in-process candidate UI, launcher IPC, settings/about windows, installer staging, and installer execution.
- `TypeDuck-Windows-backend` owns the Go backend process, input-method service dispatch, Rime/librime binding, schema deployment, settings application to Rime YAML, and candidate/dictionary payload production.
- The public repository identities are `https://github.com/TypeDuck-HK/TypeDuck-Windows` and `https://github.com/TypeDuck-HK/TypeDuck-Windows-backend`.

## Component Responsibilities

| Component | Responsibility | File |
|-----------|----------------|------|
| TSF DLL exports | Exposes `DllMain`, COM class factory, TSF registration/unregistration, and profile metadata loading. | `TypeDuck-Windows/MoqiTextService/DllEntry.cpp` |
| TypeDuck TSF identity | Defines CLSID, zh-HK profile GUID, display name, locale, installed DLL name, and program directory environment variables. | `TypeDuck-Windows/MoqiTextService/TypeDuckProfile.cpp` |
| Product IME module | Creates `Moqi::TextService`, resolves installed `TypeDuckRuntime`, launches settings, and scans `input_methods/*/ime.json` metadata. | `TypeDuck-Windows/MoqiTextService/MoqiImeModule.cpp` |
| Product text service | Connects generic TSF callbacks to TypeDuck client behavior, candidate window state, UI-less policy, composition state, and settings snapshots. | `TypeDuck-Windows/MoqiTextService/MoqiTextService.cpp`, `TypeDuck-Windows/MoqiTextService/MoqiTextService.h` |
| Frontend RPC client | Builds protobuf requests, connects to the launcher named pipe, waits for responses, applies composition/candidate/commit/UI updates, and tracks degraded backend state. | `TypeDuck-Windows/MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows/MoqiTextService/MoqiClient.h` |
| Candidate and dictionary UI | Draws the candidate panel, dictionary panel, preedit area, paging controls, and formatted lookup details from backend candidate entries. | `TypeDuck-Windows/MoqiTextService/MoqiCandidateWindow.cpp`, `TypeDuck-Windows/MoqiTextService/TypeDuckCandidateInfo.cpp` |
| Generic TSF base | Provides COM helpers, TSF activation, key sinks, edit sessions, composition mutation, compartments, language bar buttons, and window helpers. | `TypeDuck-Windows/libIME2/src/TextService.cpp`, `TypeDuck-Windows/libIME2/src/ImeModule.cpp`, `TypeDuck-Windows/libIME2/src/ComObject.h` |
| Launcher server | Enforces a single launcher instance, creates the per-user named pipe, owns tray UI, maps profile GUIDs to backend processes, and opens settings/about windows. | `TypeDuck-Windows/MoqLauncher/PipeServer.cpp`, `TypeDuck-Windows/MoqLauncher/MoqiLauncher.cpp` |
| Launcher pipe client | Parses frontend frames, initializes backend mapping, handles settings requests locally, forwards IME requests to backend, and emits bounded error responses. | `TypeDuck-Windows/MoqLauncher/PipeClient.cpp` |
| Backend process bridge | Starts `TypeDuckRuntime/server.exe`, frames protobuf over stdin/stdout, monitors stderr/stdout, restarts or terminates the process, and sends launcher-origin deploy/settings requests. | `TypeDuck-Windows/MoqLauncher/BackendServer.cpp` |
| Settings persistence | Reads, validates, writes, and applies `TypeDuckPreferences.json`; maps frontend preferences to Rime side effects. | `TypeDuck-Windows/MoqLauncher/TypeDuckPreferences.cpp`, `TypeDuck-Windows/TypeDuckSettings/TypeDuckSettingsWindow.cpp` |
| Installer helper | Copies 32-bit and 64-bit TSF DLLs to Windows system directories, runs matching `regsvr32.exe`, schedules reboot replacement/reregistration, and uninstalls TSF artifacts. | `TypeDuck-Windows/SetupHelper/SetupHelper.cpp` |
| Installer script | Stages files into `{app}`, configures x64-only setup, writes startup registration, runs helper actions, starts launcher/settings/about, and removes app data on uninstall. | `TypeDuck-Windows/installer/MoqiTsf.iss`, `TypeDuck-Windows/scripts/install.ps1` |
| Runtime staging | Packages Rime schema, `rime.dll`, engine binaries, lookup-filter evidence, and generated runtime manifests. | `TypeDuck-Windows/scripts/Stage-TypeDuckRuntime.ps1`, `TypeDuck-Windows-backend/scripts/build.ps1` |
| Backend server | Reads framed protobuf requests from stdin, creates per-client sessions, dispatches methods, and writes framed protobuf responses to stdout. | `TypeDuck-Windows-backend/server.go`, `TypeDuck-Windows-backend/protocol_io.go` |
| Backend protocol adapter | Converts generated protobuf messages to `imecore.Request`/`imecore.Response`, including key events, candidate page actions, settings updates, candidate entries, and tray notifications. | `TypeDuck-Windows-backend/imecore/protocol.go` |
| Backend service abstraction | Defines the minimal input method service interface used by Rime and other registered services. | `TypeDuck-Windows-backend/imecore/client.go`, `TypeDuck-Windows-backend/imecore/service.go` |
| Rime service | Owns input session state, key processing, activation/deactivation, candidate selection/paging, appearance updates, TypeDuck settings application, and async tray notifications. | `TypeDuck-Windows-backend/input_methods/rime/rime.go` |
| librime binding | Loads `rime.dll`, binds exported APIs, configures Rime traits/modules, creates sessions, reads composition/menu/commit state, applies schema/settings operations, and redeploys. | `TypeDuck-Windows-backend/input_methods/rime/librime.go`, `TypeDuck-Windows-backend/input_methods/rime/native_cgo.go` |
| Rime key translation | Maps Windows virtual keys and modifier state into Rime key codes and masks. | `TypeDuck-Windows-backend/input_methods/rime/rime_keyevent.go` |
| Rime config/settings | Maps TypeDuck preferences into Rime custom YAML and reload/redeploy effects. | `TypeDuck-Windows-backend/input_methods/rime/appearance_config.go`, `TypeDuck-Windows-backend/input_methods/rime/config_update.go` |
| Backend IME metadata | Declares TypeDuck profile metadata for backend packaging and optional profile scanning. | `TypeDuck-Windows-backend/input_methods/rime/ime.json` |
| Generated proto contract | Defines the cross-process request/response schema and generated language bindings. | `TypeDuck-Windows/proto/moqi.proto`, `TypeDuck-Windows-backend/proto/moqi.proto` |

## Pattern Overview

**Overall:** Out-of-process engine bridge with in-process TSF frontend.

**Key Characteristics:**
- TSF host processes load only the COM DLL and UI code from `TypeDuck-Windows/MoqiTextService`; engine work stays in `TypeDuckRuntime/server.exe`.
- All frontend-launcher-backend messages use protobuf payloads wrapped in a 32-bit little-endian length prefix.
- The TSF profile authority is the TypeDuck zh-HK profile in `TypeDuck-Windows/MoqiTextService/TypeDuckProfile.cpp`; backend `ime.json` metadata mirrors the same GUID and locale.
- The launcher is the process supervisor and settings bridge; the backend is stateless at process level except for per-client Rime sessions and shared Rime runtime initialization.
- Code identifiers retain current target/module names such as `MoqiTextService`, `MoqLauncher`, `Moqi::`, and `moqi.protocol`; treat them as current code identifiers.

## Layers

**Windows build and packaging:**
- Purpose: Build C++ targets, generate C++ protobuf, stage runtime payloads, compile installer artifacts, and package both bitnesses.
- Location: `TypeDuck-Windows/CMakeLists.txt`, `TypeDuck-Windows/scripts`, `TypeDuck-Windows/installer`
- Contains: `MoqiTextService`, `MoqLauncher`, `SetupHelper`, `TypeDuckSettings`, `Preview`, `Tools/TypeduckBackendProbe`, test targets, generated protobuf.
- Depends on: MSVC, Windows SDK, CMake, protobuf, jsoncpp, libuv, spdlog, Inno Setup, Go runtime package output.
- Used by: Local builds and GitHub Actions workflows in `TypeDuck-Windows/.github/workflows`.

**TSF/COM base:**
- Purpose: Provide reusable TSF plumbing, COM lifetime management, edit sessions, candidate window base behavior, and language bar support.
- Location: `TypeDuck-Windows/libIME2/src`
- Contains: `TextService`, `ImeModule`, `ComObject`, `ComPtr`, `EditSession`, `CandidateWindow`, `LangBarButton`, `DisplayAttributeProvider`.
- Depends on: Microsoft TSF, COM, Win32, shlwapi.
- Used by: `TypeDuck-Windows/MoqiTextService`.

**Product TSF layer:**
- Purpose: Own TypeDuck profile identity, frontend RPC, candidate rendering, settings interop, UI-less behavior, and TypeDuck-specific TSF callbacks.
- Location: `TypeDuck-Windows/MoqiTextService`
- Contains: DLL exports, `Moqi::TextService`, `Moqi::Client`, candidate UI, TypeDuck profile constants, candidate information parsing.
- Depends on: `TypeDuck-Windows/libIME2/src`, generated protobuf, `TypeDuck-Windows/proto/ProtoFraming.h`, JsonCpp, Win32/GDI/User32.
- Used by: Windows TSF host processes through `TypeDuckTextService.dll`.

**Launcher and IPC layer:**
- Purpose: Decouple TSF host processes from backend process lifetime and provide tray/settings/redeploy controls.
- Location: `TypeDuck-Windows/MoqLauncher`
- Contains: Named pipe server/client, backend server process bridge, TypeDuck preferences, pipe security, tray window.
- Depends on: libuv, generated protobuf, JsonCpp, spdlog, Win32 shell/pipe/process APIs.
- Used by: TSF DLL clients and installer startup registration.

**Settings UI layer:**
- Purpose: Present and persist TypeDuck preferences, open about dialog, and request backend-side settings application through the launcher.
- Location: `TypeDuck-Windows/TypeDuckSettings`
- Contains: Settings executable entry point, settings window, about dialog, resources, icons.
- Depends on: Win32 UI APIs, `TypeDuck-Windows/MoqLauncher/TypeDuckPreferences.cpp`.
- Used by: Installer postinstall actions, launcher tray menu, TSF configure action.

**Setup and registration layer:**
- Purpose: Install app files, copy TSF DLLs into system locations, register COM/TSF profiles, configure autostart, and remove installed artifacts.
- Location: `TypeDuck-Windows/SetupHelper`, `TypeDuck-Windows/installer`, `TypeDuck-Windows/scripts/install.ps1`
- Contains: Elevated helper, Inno Setup script, staging script, runtime copy filters.
- Depends on: Admin elevation, `regsvr32.exe`, Task Scheduler, Program Files, Windows system directories.
- Used by: Local and release packaging flows.

**Backend server layer:**
- Purpose: Own framed stdin/stdout server loop, request dispatch, per-client sessions, and service registration.
- Location: `TypeDuck-Windows-backend/server.go`, `TypeDuck-Windows-backend/protocol_io.go`, `TypeDuck-Windows-backend/imecore`
- Contains: `Server`, `Client`, `TextService`, protobuf conversion, frame IO.
- Depends on: Go runtime, generated Go protobuf, stdin/stdout supplied by launcher.
- Used by: `TypeDuckRuntime/server.exe` launched from `TypeDuck-Windows/MoqLauncher/BackendServer.cpp`.

**Rime engine layer:**
- Purpose: Convert Windows IME events into Rime operations and convert Rime state into frontend composition/candidate/dictionary responses.
- Location: `TypeDuck-Windows-backend/input_methods/rime`
- Contains: `IME`, `rimeBackend`, native backend, librime bindings, key translation, appearance/settings, custom phrase, super abbreviation, sync/deploy utilities.
- Depends on: `rime.dll`, Rime shared data, Rime user data, dictionary lookup module, TypeDuck settings JSON.
- Used by: Backend server session factories.

**Shared protocol layer:**
- Purpose: Keep frontend C++ and backend Go request/response contracts aligned.
- Location: `TypeDuck-Windows/proto`, `TypeDuck-Windows-backend/proto`
- Contains: `moqi.proto`, generated C++ files, generated Go files, C++ framing helper.
- Depends on: Protocol Buffers.
- Used by: `Moqi::Client`, `PipeClient`, `BackendServer`, backend `server.go`, and `imecore/protocol.go`.

## Data Flow

### Primary Keystroke Path

1. Windows TSF calls the product text service hooks in `TypeDuck-Windows/libIME2/src/TextService.cpp` and `TypeDuck-Windows/MoqiTextService/MoqiTextService.cpp`.
2. `Moqi::TextService` delegates key filtering and handling to `Moqi::Client` in `TypeDuck-Windows/MoqiTextService/MoqiClient.cpp`.
3. `Moqi::Client` serializes a `moqi.protocol.ClientRequest` from `TypeDuck-Windows/proto/moqi.proto` and frames it with `TypeDuck-Windows/proto/ProtoFraming.h`.
4. The request travels over the launcher named pipe to `TypeDuck-Windows/MoqLauncher/PipeClient.cpp`.
5. `PipeClient` maps the TypeDuck profile GUID to the fixed backend bridge configured in `TypeDuck-Windows/MoqLauncher/PipeServer.cpp`.
6. `TypeDuck-Windows/MoqLauncher/BackendServer.cpp` starts or reuses `TypeDuckRuntime/server.exe` and writes the framed request to backend stdin.
7. `TypeDuck-Windows-backend/server.go` reads the frame with `TypeDuck-Windows-backend/protocol_io.go`, converts it through `TypeDuck-Windows-backend/imecore/protocol.go`, and dispatches to `TypeDuck-Windows-backend/input_methods/rime/rime.go`.
8. `rime.IME` translates keys through `TypeDuck-Windows-backend/input_methods/rime/rime_keyevent.go` and calls the native backend in `TypeDuck-Windows-backend/input_methods/rime/native_cgo.go`.
9. The native backend calls librime exports bound in `TypeDuck-Windows-backend/input_methods/rime/librime.go`, then reads commit, composition, menu, candidate, and option state.
10. The backend response returns through the same framed protobuf path, and `Moqi::Client` applies commit strings, composition, candidate entries, candidate page data, UI customization, errors, and tray notifications.

### Startup and Registration Flow

1. `TypeDuck-Windows/installer/MoqiTsf.iss` installs staged files into `{app}`, writes the `TypeDuckLauncher` HKCU startup value, runs `TypeDuckSetupHelper.exe`, and starts `TypeDuckLauncher.exe /apply-settings`.
2. `TypeDuck-Windows/SetupHelper/SetupHelper.cpp` copies `TypeDuckTextService.dll` from `{app}` and `{app}\x64` to the matching Windows system directories.
3. `SetupHelper` runs 32-bit and 64-bit `regsvr32.exe` with `TYPEDUCK_PROGRAM_DIR` set to the installed app directory.
4. `TypeDuck-Windows/MoqiTextService/DllEntry.cpp` handles `DllRegisterServer`, builds the TypeDuck zh-HK language profile, scans installed `TypeDuckRuntime/input_methods/*/ime.json`, and registers TSF categories/profiles through `libIME2`.
5. On user logon or installer finish, `TypeDuck-Windows/MoqLauncher/MoqiLauncher.cpp` starts the launcher and `TypeDuck-Windows/MoqLauncher/PipeServer.cpp` initializes pipe, tray, preferences, backend mapping, and optional settings application.
6. When a TSF host loads the IME, `DllGetClassObject` creates `Moqi::ImeModule`, then `Moqi::ImeModule::createTextService` returns a `Moqi::TextService` instance.

### Backend Runtime and Rime Data Flow

1. `TypeDuck-Windows-backend/scripts/build.ps1` builds `server.exe`, packages `input_methods/rime`, copies icons and Rime data into `scripts/build/TypeDuckRuntime`.
2. `TypeDuck-Windows/scripts/install.ps1` copies the backend runtime tree into staged `{app}\TypeDuckRuntime` while filtering source files and unused input method folders.
3. `TypeDuck-Windows/scripts/Stage-TypeDuckRuntime.ps1` stages pinned librime release files, schema files, `rime.dll`, engine binaries, and lookup-filter evidence into `TypeDuck-Windows/runtime`.
4. `TypeDuck-Windows-backend/input_methods/rime/librime.go` initializes Rime with shared data under `TypeDuckRuntime/input_methods/rime/data`, user data under `%APPDATA%\TypeDuckIME\Rime`, and modules `default`, `levers`, and `dictionary_lookup`.
5. Rime candidate comments carry lookup/dictionary payloads. The backend exposes them as `CandidateEntry.Comment`, and the frontend parses/formats them through `TypeDuck-Windows/MoqiTextService/TypeDuckCandidateInfo.cpp`.

### Settings Flow

1. `TypeDuck-Windows/TypeDuckSettings/main.cpp` launches `Moqi::TypeDuckSettings::RunSettingsWindow`.
2. `TypeDuck-Windows/MoqLauncher/TypeDuckPreferences.cpp` validates and writes `TypeDuckPreferences.json`, with Rime-impacting preferences represented as `RimeSideEffects`.
3. `TypeDuck-Windows/MoqLauncher/PipeClient.cpp` handles settings snapshot/update requests from the TSF side and uses `BackendServer::applyTypeDuckPreferences` for Rime side effects.
4. `TypeDuck-Windows-backend/server.go` dispatches `typeduckSettingsUpdate` to `TypeDuck-Windows-backend/input_methods/rime/rime.go`.
5. `TypeDuck-Windows-backend/input_methods/rime/appearance_config.go` converts settings into `default.custom.yaml` and `common.custom.yaml` updates and triggers reload/redeploy behavior.

**State Management:**
- TSF composition/range state lives in `Ime::TextService` under `TypeDuck-Windows/libIME2/src/TextService.cpp`.
- Product frontend state lives in `Moqi::TextService` and `Moqi::Client` under `TypeDuck-Windows/MoqiTextService`.
- Launcher state lives in the `PipeServer` singleton and per-connection `PipeClient` objects under `TypeDuck-Windows/MoqLauncher`.
- Backend session state lives in `Server.clients` in `TypeDuck-Windows-backend/server.go` and per-client `rime.IME` instances in `TypeDuck-Windows-backend/input_methods/rime/rime.go`.
- Native Rime runtime state is process-global through `rimeInitOnce`, `rimeInitOK`, and `rimeRuntime` in `TypeDuck-Windows-backend/input_methods/rime/native_cgo.go`.

## Key Abstractions

**`Ime::TextService`:**
- Purpose: Base class for TSF activation, key sinks, composition ranges, compartments, language bars, and display attributes.
- Examples: `TypeDuck-Windows/libIME2/src/TextService.h`, `TypeDuck-Windows/libIME2/src/TextService.cpp`.
- Pattern: Template-method base class; product code overrides callbacks in `TypeDuck-Windows/MoqiTextService/MoqiTextService.cpp`.

**`Moqi::TextService`:**
- Purpose: Product controller inside each TSF host process.
- Examples: `TypeDuck-Windows/MoqiTextService/MoqiTextService.h`, `TypeDuck-Windows/MoqiTextService/MoqiTextService.cpp`.
- Pattern: Owns a `Moqi::Client`, candidate window, message window, UI customization, and active profile state.

**`Moqi::Client`:**
- Purpose: Converts TSF events to protobuf requests and applies protobuf responses to the text service.
- Examples: `TypeDuck-Windows/MoqiTextService/MoqiClient.h`, `TypeDuck-Windows/MoqiTextService/MoqiClient.cpp`.
- Pattern: Synchronous request/response adapter with guarded pipe connection, sequence numbers, async polling, and degraded backend handling.

**`PipeServer` / `PipeClient` / `BackendServer`:**
- Purpose: Provide launcher-level multiplexing, process supervision, timeout handling, and tray/settings controls.
- Examples: `TypeDuck-Windows/MoqLauncher/PipeServer.h`, `TypeDuck-Windows/MoqLauncher/PipeClient.h`, `TypeDuck-Windows/MoqLauncher/BackendServer.h`.
- Pattern: libuv event loop objects with Win32 GUI thread and bounded framed protobuf IO.

**`imecore.TextService`:**
- Purpose: Backend interface implemented by input method engines.
- Examples: `TypeDuck-Windows-backend/imecore/client.go`, `TypeDuck-Windows-backend/imecore/service.go`.
- Pattern: `Init`, `HandleRequest`, and `Close` define backend session lifecycle.

**`rime.IME`:**
- Purpose: Backend session implementation for Rime and TypeDuck settings/candidate behavior.
- Examples: `TypeDuck-Windows-backend/input_methods/rime/rime.go`.
- Pattern: Mutex-protected session object that wraps Rime state, frontend UI customization, candidate overlay features, settings, and async notifications.

**`rimeBackend`:**
- Purpose: Boundary around native Rime operations.
- Examples: `TypeDuck-Windows-backend/input_methods/rime/rime.go`, `TypeDuck-Windows-backend/input_methods/rime/native_cgo.go`.
- Pattern: Interface allows Rime operations to be tested and isolates DLL calls behind native backend methods.

**`Proto::FrameBuffer`:**
- Purpose: Reassembles stream bytes into bounded protobuf payloads.
- Examples: `TypeDuck-Windows/proto/ProtoFraming.h`, `TypeDuck-Windows/MoqLauncher/PipeClient.cpp`, `TypeDuck-Windows/MoqLauncher/BackendServer.cpp`.
- Pattern: Shared 32-bit little-endian length prefix with a 1 MiB payload ceiling for client/backend paths.

## Entry Points

**TSF COM DLL:**
- Location: `TypeDuck-Windows/MoqiTextService/DllEntry.cpp`
- Triggers: Windows TSF host process loads `TypeDuckTextService.dll` or `regsvr32.exe` calls registration exports.
- Responsibilities: DLL lifecycle logging, COM class factory, TSF register/unregister, profile metadata loading.

**Text service creation:**
- Location: `TypeDuck-Windows/MoqiTextService/MoqiImeModule.cpp`
- Triggers: TSF requests the text service CLSID from the COM class factory.
- Responsibilities: Resolve program directory, create `Moqi::TextService`, open settings UI from TSF configure.

**Launcher:**
- Location: `TypeDuck-Windows/MoqLauncher/MoqiLauncher.cpp`, `TypeDuck-Windows/MoqLauncher/PipeServer.cpp`
- Triggers: Installer `[Run]`, HKCU startup key, manual start, restart recovery.
- Responsibilities: Initialize pipe server, tray menu, backend mapping, preferences, backend process lifecycle, and settings/about launch.

**Settings app:**
- Location: `TypeDuck-Windows/TypeDuckSettings/main.cpp`
- Triggers: Installer postinstall, tray menu, TSF configure command.
- Responsibilities: Run settings window and persist TypeDuck preferences.

**Setup helper:**
- Location: `TypeDuck-Windows/SetupHelper/SetupHelper.cpp`
- Triggers: Inno Setup install/uninstall actions or scheduled reregistration task.
- Responsibilities: Elevated copy/register/unregister of TSF DLLs and reboot fallback.

**Backend server:**
- Location: `TypeDuck-Windows-backend/server.go`
- Triggers: `TypeDuck-Windows/MoqLauncher/BackendServer.cpp` starts `TypeDuckRuntime/server.exe`.
- Responsibilities: Read protobuf frames from stdin, dispatch requests, write protobuf frames to stdout.

**Backend Rime service:**
- Location: `TypeDuck-Windows-backend/input_methods/rime/rime.go`
- Triggers: Backend `METHOD_INIT` for the TypeDuck profile GUID.
- Responsibilities: Initialize Rime, handle key/mode/candidate/settings methods, produce frontend response state.

**Generated contract:**
- Location: `TypeDuck-Windows/proto/moqi.proto`, `TypeDuck-Windows-backend/proto/moqi.proto`
- Triggers: CMake/protoc in frontend, Go generation in backend.
- Responsibilities: Define methods, key events, settings payloads, candidate entries, candidate page metadata, health/errors, and UI customization.

## Architectural Constraints

- **Threading:** TSF callbacks run inside arbitrary host processes through `TypeDuck-Windows/MoqiTextService`; launcher pipe/backend work runs on `uv_default_loop()` in `TypeDuck-Windows/MoqLauncher`, while the tray window uses a Win32 message thread; backend request handling is serialized by `Server.mu` in `TypeDuck-Windows-backend/server.go`.
- **Global state:** Frontend DLL uses `g_imeModule`, `g_dllModule`, and `g_imeModuleMutex` in `TypeDuck-Windows/MoqiTextService/DllEntry.cpp`; launcher uses `PipeServer::singleton_` in `TypeDuck-Windows/MoqLauncher/PipeServer.cpp`; backend native Rime uses `rimeInitOnce`, `rimeInitOK`, and `rimeRuntime` in `TypeDuck-Windows-backend/input_methods/rime/native_cgo.go`.
- **Bitness:** Installer deploys both 32-bit and 64-bit `TypeDuckTextService.dll`; `TypeDuckLauncher.exe` and `TypeDuckSetupHelper.exe` are built from the Win32 configuration in `TypeDuck-Windows/CMakeLists.txt`.
- **COM lifetime:** Objects exposed through TSF/COM use `Ime::ComObject` and `Ime::ComPtr` from `TypeDuck-Windows/libIME2/src`; keep COM reference counting intact.
- **Protocol compatibility:** Edit `.proto` schemas first, regenerate both C++ and Go bindings, and keep `TypeDuck-Windows/proto/moqi.proto` aligned with `TypeDuck-Windows-backend/proto/moqi.proto`.
- **Frame limits:** Use `TypeDuck-Windows/proto/ProtoFraming.h` and `TypeDuck-Windows-backend/protocol_io.go` for the length-prefixed binary stream; do not introduce ad hoc JSON or newline-delimited messages on the launcher/backend path.
- **Locale/profile:** The primary TypeDuck profile is `zh-HK` with GUID `{C6E8F5DF-6504-44F9-B7CF-17A195373A83}` in both `TypeDuck-Windows/MoqiTextService/TypeDuckProfile.cpp` and `TypeDuck-Windows-backend/input_methods/rime/ime.json`.
- **Installed runtime layout:** The launcher expects `TypeDuckRuntime/server.exe` under the installed app directory and Rime metadata/data below `TypeDuckRuntime/input_methods/rime`.
- **User data:** TypeDuck settings and logs use `%APPDATA%\TypeDuckIME` and `%LOCALAPPDATA%\TypeDuckIME` paths through `TypeDuck-Windows/MoqLauncher/TypeDuckPreferences.cpp`, `TypeDuck-Windows-backend/input_methods/rime/appearance_config.go`, and `TypeDuck-Windows-backend/server.go`.

## Anti-Patterns

### Bypassing the Launcher

**What happens:** TSF code directly starting or talking to backend engine code from `TypeDuck-Windows/MoqiTextService`.
**Why it's wrong:** TSF DLLs load into many host processes; engine crashes, blocking IO, or DLL path issues would affect host applications.
**Do this instead:** Route all engine calls through `Moqi::Client` in `TypeDuck-Windows/MoqiTextService/MoqiClient.cpp`, `PipeClient` in `TypeDuck-Windows/MoqLauncher/PipeClient.cpp`, and `BackendServer` in `TypeDuck-Windows/MoqLauncher/BackendServer.cpp`.

### Editing Generated Protobuf Files

**What happens:** Changing `TypeDuck-Windows/proto/moqi.pb.h`, `TypeDuck-Windows/proto/moqi.pb.cc`, or `TypeDuck-Windows-backend/proto/moqi.pb.go` directly.
**Why it's wrong:** Generated files are overwritten and can diverge across repositories.
**Do this instead:** Edit `TypeDuck-Windows/proto/moqi.proto` and `TypeDuck-Windows-backend/proto/moqi.proto`, then regenerate C++/Go bindings through the repository build commands.

### Treating Backend Menus as Product UI

**What happens:** Adding user-facing settings or large menu trees from `TypeDuck-Windows-backend/input_methods/rime/rime.go` and surfacing them through language-bar menu responses.
**Why it's wrong:** TypeDuck Windows v1 owns user-facing settings in `TypeDuck-Windows/TypeDuckSettings` and launcher tray actions; duplicating menu UI creates multiple settings surfaces.
**Do this instead:** Put user-facing settings in `TypeDuck-Windows/TypeDuckSettings` and pass engine side effects through `METHOD_TYPEDUCK_SETTINGS_UPDATE`.

### Parsing Candidate Lookup Text in the Wrong Layer

**What happens:** Backend-formatted lookup comments are rendered directly as opaque candidate comment text in frontend drawing code.
**Why it's wrong:** The candidate panel needs structured dictionary fields, language preferences, romanization visibility, and typeface preferences.
**Do this instead:** Keep backend candidate data in `CandidateEntry` responses and format dictionary details through `TypeDuck-Windows/MoqiTextService/TypeDuckCandidateInfo.cpp` before drawing in `TypeDuck-Windows/MoqiTextService/MoqiCandidateWindow.cpp`.

## Error Handling

**Strategy:** Return explicit HRESULT/Win32/protobuf/typeduck health errors at process boundaries and keep host processes responsive.

**Patterns:**
- TSF/COM methods return `HRESULT` and use `FAILED`/`SUCCEEDED` checks in `TypeDuck-Windows/libIME2/src` and `TypeDuck-Windows/MoqiTextService`.
- Setup failures return typed exit codes and bilingual message boxes from `TypeDuck-Windows/SetupHelper/SetupHelper.cpp`.
- Frontend pipe failures mark backend degraded and retry later in `TypeDuck-Windows/MoqiTextService/MoqiClient.cpp`.
- Launcher malformed frames and backend timeouts produce bounded `TypeDuckError` responses in `TypeDuck-Windows/MoqLauncher/PipeClient.cpp` and `TypeDuck-Windows/MoqLauncher/BackendServer.cpp`.
- Backend request failures return `imecore.Response{Success:false, Error:...}` through `TypeDuck-Windows-backend/server.go`.

## Cross-Cutting Concerns

**Logging:** Frontend logs under `%LOCALAPPDATA%\TypeDuckIME\Log` from `TypeDuck-Windows/MoqiTextService` and `TypeDuck-Windows/libIME2/src`; launcher uses spdlog daily rotating files in `TypeDuck-Windows/MoqLauncher/PipeServer.cpp`; backend logs to `TypeDuckBackend-YYYY-MM-DD.log` from `TypeDuck-Windows-backend/server.go`.

**Validation:** Preferences validate in `TypeDuck-Windows/MoqLauncher/TypeDuckPreferences.cpp`; backend settings normalize in `TypeDuck-Windows-backend/input_methods/rime/appearance_config.go`; protocol frame payloads are bounded by `TypeDuck-Windows/proto/ProtoFraming.h`.

**Authentication:** Not applicable for local TSF/launcher/backend IPC. Pipe access is controlled by local Windows security attributes in `TypeDuck-Windows/MoqLauncher/PipeSecurity.cpp`.

**Installer safety:** `TypeDuck-Windows/SetupHelper/SetupHelper.cpp` uses elevation checks, system-directory detection, `regsvr32.exe`, and reboot fallback for locked DLL replacement.

---

*Architecture analysis: 2026-06-28*
