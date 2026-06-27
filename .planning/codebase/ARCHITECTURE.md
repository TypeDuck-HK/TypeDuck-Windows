<!-- refreshed: 2026-06-23 -->
# Architecture

**Analysis Date:** 2026-06-23

## System Overview

```text
┌─────────────────────────────────────────────────────────────┐
│                  Windows TSF Host Processes                  │
│       apps load `MoqiTextService/MoqiTextService.dll`        │
├──────────────────┬──────────────────┬───────────────────────┤
│  TSF COM Module  │  Text Service UI │  Backend RPC Client   │
│ `DllEntry.cpp`   │ `MoqiTextService`│ `MoqiClient.cpp`      │
│ `MoqiImeModule`  │ `MoqiCandidate*` │ `proto/moqi.proto`    │
└────────┬─────────┴────────┬─────────┴──────────┬────────────┘
         │                  │                     │
         ▼                  ▼                     ▼
┌─────────────────────────────────────────────────────────────┐
│              Generic TSF/COM Support Library                │
│              `libIME2/src`                                  │
└─────────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────────┐
│              Per-User Launcher and Backend Bridge           │
│ `MoqLauncher/PipeServer.cpp` listens on named pipe          │
│ `MoqLauncher/BackendServer.cpp` owns backend process stdio  │
└─────────────────────────────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────────────────────────────┐
│       Fixed TypeDuck runtime bridge                         │
│       `TypeDuckRuntime/server.exe`                          │
└─────────────────────────────────────────────────────────────┘
```

## Component Responsibilities

| Component | Responsibility | File |
|-----------|----------------|------|
| TSF DLL exports | Exposes COM DLL entry points, creates the module singleton, scans installed backend input-method metadata, and registers language profiles. | `MoqiTextService/DllEntry.cpp` |
| Product IME module | Defines the text service CLSID, locates the installed program directory, loads backend `input_methods/*/ime.json`, and opens a backend config tool. | `MoqiTextService/MoqiImeModule.cpp` |
| Product text service | Bridges `libIME2` callbacks to product behavior, owns client lifetime, candidate window state, message window state, UI-less handling, and activation/deactivation behavior. | `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiTextService.h` |
| Backend RPC client | Converts TSF lifecycle/key/menu/candidate events to protobuf requests, synchronously waits for backend responses, and applies composition/candidate/commit/UI updates. | `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiClient.h` |
| Candidate UI | Implements the product candidate window as a Win32 window and `ITfCandidateListUIElement`. | `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/MoqiCandidateWindow.h` |
| Generic TSF base | Implements reusable TSF activation, keystroke sinks, edit sessions, composition ranges, compartments, display attributes, windows, and COM helpers. | `libIME2/src/TextService.cpp`, `libIME2/src/ImeModule.cpp`, `libIME2/src/ComObject.h`, `libIME2/src/ComPtr.h` |
| Launcher pipe server | Runs one launcher instance per user, creates the named pipe, maps language-profile GUIDs to backend servers, owns tray UI, and hosts a GUI message thread. | `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeServer.h` |
| Backend process bridge | Starts/restarts the configured backend process with libuv, forwards framed protobuf requests to stdin, and routes framed responses from stdout to pipe clients. | `MoqLauncher/BackendServer.cpp`, `MoqLauncher/BackendServer.h` |
| Installer helper | Copies 32-bit and 64-bit TSF DLLs into system directories, registers/unregisters them, and schedules delayed registration when files are locked. | `SetupHelper/SetupHelper.cpp` |
| Installer script | Stages files, writes startup registry entries, runs `SetupHelper.exe`, and purges Moqi registry residue on uninstall. | `installer/MoqiTsf.iss`, `scripts/install.ps1` |

## Pattern Overview

**Overall:** Out-of-process IME backend with an in-process Windows TSF frontend.

**Key Characteristics:**
- Keep TSF/COM mechanics in `libIME2/src`; put TypeDuck product behavior in the product text service layer represented by `MoqiTextService/*`.
- Communicate between the in-process TSF DLL and the launcher through a per-user Windows named pipe created by `MoqLauncher/PipeServer.cpp`.
- Communicate between launcher and backend through backend process stdin/stdout using protobuf frames defined by `proto/moqi.proto` and `proto/ProtoFraming.h`.
- Treat Moqi-specific branding, launcher names, install directory names, cloud clipboard, AI, fcitx-facing concepts, and deleted manifest-driven backend discovery as legacy scaffold.
- Preserve the architectural boundary for the TypeDuck rewrite: Windows TSF frontend in this repo, TypeDuck librime fork and dictionary lookup filter plugin in the backend/runtime payload, and installation metadata under Chinese (Traditional, Hong Kong).

## Layers

**Build and Target Graph:**
- Purpose: Defines targets, compiler settings, generated protobuf output, third-party dependencies, and 32-bit/x64 target inclusion.
- Location: `CMakeLists.txt`
- Contains: `MoqiTextService`, `MoqLauncher`, `SetupHelper`, `Preview`, `libIME2`, `libuv`, `jsoncpp`, generated `proto/moqi.pb.*`.
- Depends on: Visual Studio/MSVC, CMake, protobuf, jsoncpp, libuv, spdlog, Windows SDK.
- Used by: `scripts/build.ps1`, `.github/workflows/release.yml`, `.github/workflows/nightly.yml`.

**TSF/COM Foundation:**
- Purpose: Provides generic Microsoft TSF text service plumbing and Win32 window abstractions.
- Location: `libIME2/src`
- Contains: `TextService`, `ImeModule`, `EditSession`, `KeyEvent`, `CandidateWindow`, `LangBarButton`, `DisplayAttributeProvider`, `ComObject`, `ComPtr`.
- Depends on: Windows TSF COM APIs, Win32, `shlwapi.lib`.
- Used by: `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`.

**Product TSF Service:**
- Purpose: Owns product-specific TSF behavior, backend RPC, candidate rendering, lang-bar controls, user-facing messages, and process-specific UI policy.
- Location: `MoqiTextService`
- Contains: `MoqiTextService.cpp`, `MoqiClient.cpp`, `MoqiCandidateWindow.cpp`, `MoqiLangBarButton.cpp`, `MoqiImeModule.cpp`, `DllEntry.cpp`.
- Depends on: `libIME2/src`, `jsoncpp`, generated protobuf includes, `proto/ProtoFraming.h`.
- Used by: Windows TSF hosts through `MoqiTextService.dll`.

**IPC and Backend Lifecycle:**
- Purpose: Keeps backend process lifetime outside TSF host processes and multiplexes multiple TSF clients over one backend bridge.
- Location: `MoqLauncher`
- Contains: `PipeServer`, `PipeClient`, `BackendServer`, `PipeSecurityAttributes`, `UvPipe`, JSON helpers, tray UI.
- Depends on: `libuv`, protobuf, jsoncpp, spdlog, Win32 shell/pipe/process APIs.
- Used by: `MoqiTextService/MoqiClient.cpp` through the launcher named pipe.

**Protocol:**
- Purpose: Defines the request/response contract for lifecycle, key events, preserved keys, composition, candidate lists, UI customization, tray notification, and cloud clipboard upload.
- Location: `proto/moqi.proto`, `proto/ProtoFraming.h`
- Contains: `ClientRequest`, `ServerResponse`, `KeyEvent`, `CandidateEntry`, `CustomizeUi`, `PreservedKey`, `TrayNotification`.
- Depends on: protobuf C++ generation in `CMakeLists.txt`.
- Used by: `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/PipeClient.cpp`, `MoqLauncher/BackendServer.cpp`.

**Packaging and Registration:**
- Purpose: Builds both bitnesses, stages runtime payload, installs application files, copies TSF DLLs to system directories, and registers COM/TSF language profiles.
- Location: `scripts`, `installer`, `SetupHelper`
- Contains: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`, `installer/MoqiTsf.iss`, `SetupHelper/SetupHelper.cpp`.
- Depends on: Visual Studio 2022, CMake, protoc/protobuf source, Inno Setup 6, Windows admin privileges.
- Used by: local release builds and `.github/workflows/release.yml`.

## Data Flow

### Primary Keystroke Path

1. Windows TSF calls `Ime::TextService::OnTestKeyDown` or `Ime::TextService::OnKeyDown` (`libIME2/src/TextService.cpp:1108`, `libIME2/src/TextService.cpp:1138`).
2. `libIME2` wraps the key in `Ime::KeyEvent` and dispatches to the product override `Moqi::TextService::onKeyDown` (`MoqiTextService/MoqiTextService.cpp:507`).
3. `Moqi::TextService` delegates to `Moqi::Client::onKeyDown`, which creates a protobuf request and adds key state (`MoqiTextService/MoqiClient.cpp:1090`, `MoqiTextService/MoqiClient.cpp:540`).
4. `Moqi::Client::callRpcMethod` sends a length-prefixed protobuf frame to the launcher pipe and waits for a response frame (`MoqiTextService/MoqiClient.cpp:1534`, `proto/ProtoFraming.h:31`).
5. `PipeClient::handleClientMessage` parses the request frame and forwards it to the selected backend (`MoqLauncher/PipeClient.cpp:95`, `MoqLauncher/PipeClient.cpp:112`).
6. `BackendServer::handleClientMessage` starts the backend if needed and writes the framed request to backend stdin (`MoqLauncher/BackendServer.cpp:98`, `MoqLauncher/BackendServer.cpp:171`).
7. `BackendServer::onStdoutRead` buffers backend stdout frames and writes responses back to the matching pipe client (`MoqLauncher/BackendServer.cpp:258`, `MoqLauncher/BackendServer.cpp:292`).
8. `Moqi::Client` converts `ServerResponse` to JSON-like state and applies commit, composition, candidate, button, preserved-key, and UI updates (`MoqiTextService/MoqiClient.cpp:386`, `MoqiTextService/MoqiClient.cpp:904`).

### Registration and Install Path

1. `scripts/build.ps1` builds Win32 full solution and x64 `MoqiTextService.dll` (`scripts/build.ps1`).
2. `scripts/install.ps1` stages `MoqiLauncher.exe`, `SetupHelper.exe`, Win32/x64 `MoqiTextService.dll`, and the `TypeDuckRuntime` backend runtime under `installer/stage` (`scripts/install.ps1`).
3. `installer/build-installer.ps1` validates staged files and compiles `installer/MoqiTsf.iss` with Inno Setup (`installer/build-installer.ps1`).
4. `installer/MoqiTsf.iss` installs to `{autopf32}\MoqiIM`, writes the `MoqiLauncher` startup registry value, and invokes `SetupHelper.exe /i` (`installer/MoqiTsf.iss`).
5. `SetupHelper.exe` copies TSF DLLs into SysWOW64/System32 and registers them; locked DLLs trigger a scheduled post-reboot registration task (`SetupHelper/SetupHelper.cpp:649`, `SetupHelper/SetupHelper.cpp:714`).
6. `DllRegisterServer` scans `input_methods/*/ime.json` from installed backend directories and calls `ImeModule::registerServer` (`MoqiTextService/DllEntry.cpp:211`, `libIME2/src/ImeModule.cpp:356`).

### Backend and Language Profile Discovery

1. `Moqi::ImeModule` reads `TYPEDUCK_PROGRAM_DIR`, falls back through the legacy registration alias, or resolves `%ProgramFiles(x86)%\TypeDuckIME` (`MoqiTextService/MoqiImeModule.cpp`).
2. `Moqi::ImeModule` uses a fixed runtime directory list containing `TypeDuckRuntime`; it does not parse `backends.json`.
3. `DllRegisterServer` seeds the first-party TypeDuck zh-HK profile, then optionally scans fixed runtime metadata under `TypeDuckRuntime\input_methods\<name>\ime.json` for compatibility (`MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiImeModule.cpp`).
4. `MoqLauncher/PipeServer.cpp` constructs one in-code `typeduck-runtime-bridge` for `TypeDuckRuntime\server.exe`, seeds the TypeDuck profile GUID mapping, then optionally scans fixed runtime metadata without overriding the seeded mapping.

**State Management:**
- TSF composition state lives in `Ime::TextService` and is manipulated via `startComposition`, `setCompositionString`, `setCompositionCursor`, and `endComposition` (`libIME2/src/TextService.cpp:385`, `libIME2/src/TextService.cpp:521`, `libIME2/src/TextService.cpp:638`, `libIME2/src/TextService.cpp:452`).
- Product UI state lives in `Moqi::TextService` fields for candidate list, selection, preedit, appearance, UI-less policy, message window, and current language profile (`MoqiTextService/MoqiTextService.h`).
- Backend client state lives in `Moqi::Client`, including sequence numbers, request/response buffers, lang-bar button maps, preserved keys, async responses, and the launcher pipe (`MoqiTextService/MoqiClient.h`).
- Launcher global state lives in `PipeServer` as a singleton with clients, backends, GUID map, tray notification queue, and clipboard async handle (`MoqLauncher/PipeServer.h`).

## Key Abstractions

**`Ime::TextService`:**
- Purpose: Base class for TSF activation, key sink handling, composition range mutation, compartments, and display attributes.
- Examples: `libIME2/src/TextService.h`, `libIME2/src/TextService.cpp`.
- Pattern: Template-method base class; product behavior overrides hooks such as `onActivate`, `onDeactivate`, `onKeyDown`, `onPreservedKey`, `onCompartmentChanged`, and language-profile callbacks.

**`Moqi::TextService`:**
- Purpose: Product-level IME controller for the current TSF host process.
- Examples: `MoqiTextService/MoqiTextService.h`, `MoqiTextService/MoqiTextService.cpp`.
- Pattern: Subclass of `Ime::TextService` that owns a `Moqi::Client` and Win32 candidate/message UI.

**`Moqi::Client`:**
- Purpose: RPC adapter between TSF events and backend protocol responses.
- Examples: `MoqiTextService/MoqiClient.h`, `MoqiTextService/MoqiClient.cpp`.
- Pattern: Stateful request builder and response applier; future TypeDuck backend behavior should flow through this protocol boundary.

**`PipeServer`, `PipeClient`, and `BackendServer`:**
- Purpose: Per-user IPC server, per-TSF-client pipe connection, and managed backend process.
- Examples: `MoqLauncher/PipeServer.h`, `MoqLauncher/PipeClient.h`, `MoqLauncher/BackendServer.h`.
- Pattern: libuv event-loop objects with callbacks and process supervision.

**`Proto::FrameBuffer`:**
- Purpose: Reassembles stream data into complete protobuf payloads with a 32-bit little-endian length prefix.
- Examples: `proto/ProtoFraming.h`, `MoqLauncher/PipeClient.cpp`, `MoqLauncher/BackendServer.cpp`, `MoqiTextService/MoqiClient.cpp`.
- Pattern: Shared framing utility; use it for all launcher/backend protocol messages.

**`Ime::LangProfileInfo`:**
- Purpose: Data structure used during TSF language-profile registration.
- Examples: `libIME2/src/ImeModule.h`, `MoqiTextService/DllEntry.cpp`.
- Pattern: Metadata loaded from backend `ime.json`; TypeDuck Hong Kong Traditional Chinese registration data belongs in the installed backend metadata plus registration code paths.

## Entry Points

**TSF DLL Load:**
- Location: `MoqiTextService/DllEntry.cpp`
- Triggers: Windows loads `MoqiTextService.dll` into TSF host processes.
- Responsibilities: `DllMain`, `DllGetClassObject`, `DllCanUnloadNow`, `DllRegisterServer`, `DllUnregisterServer`.

**Text Service COM Factory:**
- Location: `libIME2/src/ImeModule.cpp`, `MoqiTextService/MoqiImeModule.cpp`
- Triggers: TSF requests a class object for the text service CLSID.
- Responsibilities: Return a `Moqi::TextService` instance and register TSF categories/language profiles.

**Launcher Process:**
- Location: `MoqLauncher/MoqiLauncher.cpp`, `MoqLauncher/PipeServer.cpp`
- Triggers: Inno `[Run]`, startup registry key, or manual process launch.
- Responsibilities: Enforce single instance, initialize backends, listen on named pipe, run tray GUI thread, and run the libuv loop.

**Setup Helper:**
- Location: `SetupHelper/SetupHelper.cpp`
- Triggers: `installer/MoqiTsf.iss` runs `SetupHelper.exe /i`, `/r`, or `/u`.
- Responsibilities: Copy, register, unregister, and schedule reboot-time TSF registration.

**Preview Utility:**
- Location: `Preview/main.cpp`
- Triggers: Manual build/run of `MoqiCandidatePreview`.
- Responsibilities: Standalone Win32 preview for candidate rendering experiments.

**CI Builds:**
- Location: `.github/workflows/release.yml`, `.github/workflows/nightly.yml`
- Triggers: Release publication, push to `main`, or workflow dispatch.
- Responsibilities: Checkout this repo and sibling backend, install Inno Setup/protoc, build installer, publish artifacts.

## Architectural Constraints

- **Threading:** TSF callbacks execute in host process contexts through `MoqiTextService.dll`; launcher IPC/process work runs on `uv_default_loop()` in `MoqLauncher/PipeServer.cpp`, while tray/window messages run on a separate GUI thread created by `uv_thread_create` in `MoqLauncher/PipeServer.cpp:485`.
- **Global state:** `MoqiTextService/DllEntry.cpp` uses global `g_imeModule`, `g_dllModule`, and `g_imeModuleMutex`; `MoqLauncher/PipeServer.h` uses static `PipeServer* singleton_`.
- **COM lifetime:** Product COM objects use `Ime::ComObject` and `Ime::ComPtr`; do not replace TSF object ownership with `std::unique_ptr` where the object is exposed through COM interfaces (`libIME2/src/ComObject.h`, `libIME2/src/ComPtr.h`, `MoqiTextService/MoqiTextService.h`).
- **Bitness:** Win32 build includes `MoqLauncher` and `SetupHelper`; x64 build only needs `MoqiTextService.dll` for 64-bit host processes (`CMakeLists.txt`, `scripts/build.ps1`, `scripts/install.ps1`).
- **Protocol generation:** `proto/moqi.pb.cc` and `proto/moqi.pb.h` are generated artifacts; edit `proto/moqi.proto` and regenerate through CMake/protoc (`CMakeLists.txt`, `proto/moqi.proto`).
- **Registration source of truth:** Text service CLSID appears in `MoqiTextService/MoqiImeModule.cpp` and installer cleanup logic in `installer/MoqiTsf.iss`; keep these synchronized when TypeDuck identity replaces Moqi identity.
- **Installer language/locale target:** `installer/MoqiTsf.iss` uses Simplified Chinese installer language and Moqi install paths; TypeDuck target installation under Chinese (Traditional, Hong Kong) requires coordinated changes in `installer/MoqiTsf.iss`, backend `ime.json`, `MoqiTextService/DllEntry.cpp`, and `libIME2/src/ImeModule.cpp`.
- **Legacy product features:** Cloud clipboard, AI/config-tool assumptions, fcitx/readme claims, and Moqi-specific paths are scaffold behavior in `README.md`, `proto/moqi.proto`, `MoqLauncher/PipeServer.cpp`, `MoqLauncher/BackendServer.cpp`, and `MoqiTextService/MoqiClient.cpp`.

## Anti-Patterns

### Product Logic in Generic TSF Foundation

**What happens:** `libIME2/src` is a reusable TSF/COM layer, while `MoqiTextService/*` contains product-specific UI and backend behavior.
**Why it's wrong:** TypeDuck behavior placed in `libIME2/src` couples product semantics to the generic TSF wrapper and makes upstream-style TSF maintenance harder.
**Do this instead:** Add TypeDuck-specific candidate visual feel, bilingual Traditional Hong Kong Chinese/English labels, and backend protocol behavior in `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`, and `MoqiTextService/MoqiLangBarButton.cpp`.

### Single-File Brand Changes

**What happens:** Moqi identity is distributed across `README.md`, `CMakeLists.txt`, `backends.json`, `MoqiTextService/MoqiImeModule.cpp`, `MoqiTextService/MoqiTextService.rc.in`, `MoqLauncher/PipeServer.cpp`, `installer/MoqiTsf.iss`, `scripts/*.ps1`, and `.github/workflows/*.yml`.
**Why it's wrong:** Updating only installer text or only executable names leaves COM registration, startup registry, pipe names, app data paths, and backend routing inconsistent.
**Do this instead:** Treat TypeDuck branding as a cross-layer rename touching registration, installer, launcher pipe/app-data paths, resource files, scripts, CI artifact names, and backend metadata together.

### Protocol Bypass Around Launcher

**What happens:** The architecture expects `MoqiTextService/MoqiClient.cpp` to call the backend through `MoqLauncher` and `proto/ProtoFraming.h`.
**Why it's wrong:** Direct backend calls from TSF host processes would reintroduce process-lifetime, crash-isolation, and bitness problems the launcher boundary solves.
**Do this instead:** Extend `proto/moqi.proto`, regenerate protobuf sources through `CMakeLists.txt`, and route new TypeDuck backend capabilities through `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/PipeClient.cpp`, and `MoqLauncher/BackendServer.cpp`.

### User-Facing Scaffold Leakage

**What happens:** Moqi cloud clipboard, AI actions, fcitx references, Simplified Chinese installer copy, and Moqi-specific tray labels are visible in `README.md`, `proto/moqi.proto`, `MoqLauncher/PipeServer.cpp`, `installer/MoqiTsf.iss`, and backend-driven UI fields consumed by `MoqiTextService/MoqiClient.cpp`.
**Why it's wrong:** TypeDuck Windows must mirror TypeDuck Web alpha behavior and visual feel, with bilingual Traditional Hong Kong Chinese and English text and no unused fcitx/user-facing clutter.
**Do this instead:** Remove or hide unused scaffold features at the source boundary: installer/resources in `installer/MoqiTsf.iss` and `MoqiTextService/*.rc.in`, launcher menus in `MoqLauncher/PipeServer.cpp`, backend-exposed protocol/UI fields in `proto/moqi.proto`, and response handling in `MoqiTextService/MoqiClient.cpp`.

## Error Handling

**Strategy:** HRESULT/Win32 return codes at TSF and installer boundaries, SEH/exception guards around product client callbacks, backend restart on pipe/stdout errors, and log files for diagnostics.

**Patterns:**
- Return `HRESULT` from COM entry points and registration functions in `MoqiTextService/DllEntry.cpp` and `libIME2/src/ImeModule.cpp`.
- Guard backend callback invocation from TSF event handlers in `MoqiTextService/MoqiTextService.cpp`.
- Restart backend processes after read errors and timeouts in `MoqLauncher/BackendServer.cpp` and `MoqLauncher/PipeClient.cpp`.
- Use `SetupHelper` exit codes for installer success/restart-required/failure in `SetupHelper/SetupHelper.cpp` and `installer/MoqiTsf.iss`.
- Write TSF/candidate-window diagnostics to local app data log files from `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`, and `MoqiTextService/TsfLog.cpp`.

## Cross-Cutting Concerns

**Logging:** `MoqLauncher` uses `spdlog` configured by `MoqLauncher/PipeServer.cpp`; TSF DLL and candidate window logging use custom file helpers in `MoqiTextService/TsfLog.cpp`, `MoqiTextService/DllEntry.cpp`, and `libIME2/src/DebugLogFile.h`.
**Validation:** JSON config is parsed with jsoncpp in `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `MoqLauncher/Utils.cpp`, and `MoqLauncher/PipeServer.cpp`; protocol payloads are validated by protobuf parse/serialize success in `proto/ProtoFraming.h`.
**Authentication:** Not applicable for local TSF/launcher IPC; named pipe security attributes are configured for app-container access in `MoqLauncher/PipeSecurity.cpp` and `MoqLauncher/PipeServer.cpp`.

---

*Architecture analysis: 2026-06-23*
