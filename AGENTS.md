<!-- GSD:project-start source:PROJECT.md -->

## Project

**TypeDuck Windows IME**

TypeDuck Windows IME is a Windows Text Services Framework Cantonese input method for TypeDuck. This repo starts from the Moqi Windows IME scaffold, but the product target is TypeDuck: behavior, visible UI, settings, installer language, and candidate/dictionary presentation must mirror the TypeDuck Web alpha at `http://localhost:5173/TypeDuck-Web/aap2-alpha/` and the upstream source in `I:\GitHub\TypeDuck-Web`.

The existing Moqi code is useful scaffolding for TSF registration, candidate windows, launcher/backend IPC, installer packaging, and Windows build mechanics. Moqi branding, Simplified Chinese defaults, fcitx/user-facing clutter, cloud clipboard/WebDAV/AI surfaces, and legacy backend assumptions are not product truth and may be removed or replaced.

**Core Value:** Hong Kong users can install TypeDuck under Chinese (Traditional, Hong Kong) and type Cantonese with the same trusted behavior, bilingual settings, candidate details, and dictionary-like lookup experience as TypeDuck Web alpha.

### Constraints

- **Product parity**: TypeDuck Web alpha is the source of truth for behavior and UI until official release.
- **Language**: Every user-facing string must be bilingual Traditional Hong Kong Chinese and English.
- **Windows locale**: TSF profile registration must target Chinese (Traditional, Hong Kong) / `zh-HK`.
- **Engine dependency**: Use the TypeDuck-HK librime fork, despite it being older than the scaffold's current librime expectations.
- **Dictionary data**: The `rime-dictionary-lookup-filter` plugin is required to surface translations and dictionary-like data.
- **Scaffold divergence**: Divergence from Moqi is expected; preserve useful Windows architecture, not Moqi product choices.
- **Installer**: Settings must be shown during installation, with language picker prioritized.
- **UX scope**: Remove unused and excessive features; keep only TypeDuck-relevant settings and Web-alpha-accepted customization.
- **Compatibility**: Changes must respect Windows TSF/COM bitness, installer registration, and host-process behavior.

<!-- GSD:project-end -->

<!-- GSD:stack-start source:codebase/STACK.md -->

## Technology Stack

## Languages

- C++20 - Windows TSF text service, launcher, setup helper, preview utility, and libIME2 integration in `MoqiTextService/`, `MoqLauncher/`, `SetupHelper/`, `Preview/`, and `libIME2/src/`.
- C - vendored libuv Windows event-loop/process/pipe implementation in `libuv/src/`.
- PowerShell 5.1 - build, staging, installer packaging, icon generation, and install scripts in `scripts/*.ps1` and `installer/build-installer.ps1`.
- Inno Setup Pascal Script - graphical installer and registry/process orchestration in `installer/MoqiTsf.iss`.
- Protocol Buffers schema - frontend/backend IPC contract in `proto/moqi.proto`.
- Go - external legacy backend language for the sibling `moqi-ime` runtime referenced by `README.md`, `.github/workflows/nightly.yml`, `.github/workflows/release.yml`, and `scripts/_all_in_package.ps1`; no Go source lives in this repo.

## Runtime

- Windows desktop IME runtime using Microsoft Text Services Framework (TSF), COM DLL registration, Win32 windows/messages, shell tray APIs, named pipes, and child processes.
- Minimum Windows target is Windows 8+ for the top-level build (`_WIN32_WINNT=0x0602` in `CMakeLists.txt`); libuv itself is compiled with a Vista-level target (`_WIN32_WINNT=0x0600` in `libuv/CMakeLists.txt`).
- Installer is x64-only (`ArchitecturesAllowed=x64` in `installer/MoqiTsf.iss`) but deploys both Win32 and x64 TSF DLLs.
- No language package manager manifest is present for the Windows frontend.
- CMake FetchContent downloads `spdlog` and `protobuf` when local protobuf paths are not supplied (`CMakeLists.txt`).
- Git submodules provide `libuv`, `libIME2`, `jsoncpp`, and the Inno Simplified Chinese translation (`.gitmodules`).
- Lockfile: Not detected.

## Frameworks

- Microsoft TSF / COM / Win32 API - IME registration, key event handling, candidate UI, language bar buttons, shell tray, clipboard listener, registry integration, and setup helper operations across `MoqiTextService/`, `MoqLauncher/`, `SetupHelper/`, and `libIME2/src/`.
- libIME2 - static TSF abstraction layer built from `libIME2/src/CMakeLists.txt` and linked by `MoqiTextService/CMakeLists.txt`.
- libuv - launcher event loop, named pipe server, subprocess management, timers, async handoff, and stdio pipes built by `libuv/CMakeLists.txt` and linked by `MoqLauncher/CMakeLists.txt`.
- Protocol Buffers - framed binary frontend/backend IPC generated from `proto/moqi.proto`; framing helpers live in `proto/ProtoFraming.h`.
- JsonCpp - JSON config/metadata parsing for `backends.json`, `input_methods/*/ime.json`, launcher config, and UI metadata in `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `MoqLauncher/PipeServer.cpp`, and `MoqLauncher/Utils.cpp`.
- GoogleTest 1.10.0 - vendored under `libIME2/lib/googletest-release-1.10.0` and enabled by `libIME2/CMakeLists.txt`.
- No top-level automated tests for `MoqiTextService/`, `MoqLauncher/`, `SetupHelper/`, or installer scripts are detected.
- CMake 3.21+ - top-level generator and dependency orchestration in `CMakeLists.txt`; README lists CMake 3.21+ as prerequisite.
- Visual Studio 2022 / MSBuild - default generator `Visual Studio 17 2022` in `scripts/build.ps1` and CI workflows.
- Inno Setup 6 - installer compiler required by `installer/build-installer.ps1` and `installer/README.txt`.
- GitHub Actions on `windows-2022` - nightly and release packaging pipelines in `.github/workflows/nightly.yml` and `.github/workflows/release.yml`.

## Key Dependencies

- `libIME2` submodule - TSF/IME base classes, COM helpers, candidate windows, message windows, and language bar support (`libIME2/src/`, `.gitmodules`).
- `libuv` submodule, version macros `1.11.1-dev` - launcher event loop, named pipes, async callbacks, timers, process spawning, and stdio streams (`libuv/include/uv-version.h`, `libuv/CMakeLists.txt`).
- `protobuf` 33.5 - schema compiler/runtime for `proto/moqi.proto`; downloaded from GitHub by `CMakeLists.txt` or provided through local CMake cache vars.
- `jsoncpp` 1.8.4 - static JSON parser/writer used for backend manifests, IME metadata, config, and UI bridge objects (`jsoncpp/include/json/version.h`).
- `spdlog` v1.2.1 - launcher rotating file logging (`CMakeLists.txt`, `MoqLauncher/PipeServer.cpp`).
- Windows SDK libraries - TSF/COM/Win32 dependencies including `Rpcrt4`, `Advapi32`, `Shell32`, `shlwapi.lib`, `gdi32`, `user32`, `ws2_32`, `psapi`, `iphlpapi`, and `userenv` across component `CMakeLists.txt` files.
- `MoqiLauncher.exe` - 32-bit launcher target built only when `CMAKE_SIZEOF_VOID_P EQUAL 4` (`CMakeLists.txt`, `MoqLauncher/CMakeLists.txt`).
- `MoqiTextService.dll` - TSF COM DLL built for Win32 and x64 (`MoqiTextService/CMakeLists.txt`, `scripts/build.ps1`).
- `SetupHelper.exe` - elevated installer helper for system-directory TSF DLL copy, regsvr32 registration, scheduled re-registration, and uninstall cleanup (`SetupHelper/CMakeLists.txt`, `SetupHelper/SetupHelper.cpp`).
- `MoqiCandidatePreview.exe` - standalone candidate UI preview using GDI/User32 (`Preview/CMakeLists.txt`).

## Configuration

- Build-time CMake cache variables: `MOQI_PROTOBUF_ROOT`, `MOQI_PROTOBUF_SOURCE_DIR`, and `MOQI_PROTOC_EXECUTABLE` in `CMakeLists.txt` and `scripts/build.ps1`.
- Setup-time environment variable: `MOQI_PROGRAM_DIR` tells the TSF DLL where the installed payload lives during registration and runtime lookup (`SetupHelper/SetupHelper.cpp`, `MoqiTextService/MoqiImeModule.cpp`).
- Runtime environment paths: `%LOCALAPPDATA%\MoqiIM` stores launcher config and logs (`MoqLauncher/PipeServer.cpp`, `libIME2/src/DebugLogConfig.cpp`); `%APPDATA%\Moqi` stores legacy cloud clipboard config (`MoqLauncher/Utils.cpp`, `MoqLauncher/PipeServer.cpp`).
- Backend subprocess environment adds `PYTHONIOENCODING=utf-8:ignore` for Unicode-safe backend stdio (`MoqLauncher/BackendServer.cpp`).
- Top-level build orchestration: `CMakeLists.txt`.
- MSVC runtime override: `CMAKE_MSVC_RUNTIME_LIBRARY` in `CMakeLists.txt`, plus `cmake/c_flag_overrides.cmake` and `cmake/cxx_flag_overrides.cmake`.
- Version source: `version.txt`, parsed by `CMakeLists.txt` and applied to resource templates such as `MoqiTextService/MoqiTextService.rc.in` and `MoqLauncher/version.rc.in`.
- Backend manifest: `backends.json` currently points to legacy `moqi-ime\server.exe`.
- IME language profile metadata: installed backend folders under `input_methods/*/ime.json`, scanned by `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiImeModule.cpp`, and `MoqLauncher/PipeServer.cpp`.
- Installer script: `installer/MoqiTsf.iss`; staging script: `scripts/install.ps1`; full packaging script: `scripts/_all_in_package.ps1`.

## Platform Requirements

- Windows with Visual Studio 2022, Windows SDK, CMake 3.21+, PowerShell 5.1, git submodules, and Inno Setup 6 for installer output (`README.md`, `scripts/build.ps1`, `installer/README.txt`).
- A `protoc.exe` path must be discoverable through `MOQI_PROTOC_EXECUTABLE`, `MOQI_PROTOBUF_ROOT`, `find_program(protoc)`, or CMake FetchContent protobuf build (`CMakeLists.txt`).
- Full package builds expect a sibling legacy backend checkout at `..\moqi-ime` unless `-MoqiImeRoot` or `-MoqiImeSource` is provided (`scripts/_all_in_package.ps1`, `scripts/install.ps1`).
- Installed payload currently targets `%ProgramFiles(x86)%\MoqiIM` via Inno Setup (`installer/MoqiTsf.iss`).
- TSF DLLs are copied into `SysWOW64\MoqiTextService.dll` and `System32\MoqiTextService.dll`, then registered with `regsvr32.exe` by `SetupHelper/SetupHelper.cpp`.
- Launcher autostart is configured at `HKCU\Software\Microsoft\Windows\CurrentVersion\Run\MoqiLauncher` (`installer/MoqiTsf.iss`).
- TypeDuck rewrite note: Moqi product names, paths, icons, Simplified Chinese installer language, legacy `moqi-ime` backend, Rime/fcitx references, and `%APPDATA%\Moqi` / `%LOCALAPPDATA%\MoqiIM` locations are current scaffold details, not target product behavior. Future TypeDuck work should keep the Windows TSF/COM/build architecture where useful while replacing legacy branding/config and installing under Chinese (Traditional, Hong Kong).

<!-- GSD:stack-end -->

<!-- GSD:conventions-start source:CONVENTIONS.md -->

## Conventions

## Naming Patterns

- Use PascalCase for first-party C++ class modules: `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiCandidateWindow.h`, `MoqLauncher/PipeServer.cpp`, `libIME2/src/TextService.h`.
- Keep implementation/header pairs side by side in the owning module directory: `MoqiTextService/MoqiClient.cpp` with `MoqiTextService/MoqiClient.h`, `MoqLauncher/PipeClient.cpp` with `MoqLauncher/PipeClient.h`.
- Use lowercase for protocol/source schema files: `proto/moqi.proto`, `proto/ProtoFraming.h` is the small hand-written framing helper around generated protobuf code.
- Use PowerShell verb-noun script names for build/package automation: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/generate-glyph-icon.ps1`.
- Treat Moqi-branded filenames and user-facing text as legacy scaffold for replacement, not product naming to preserve: `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/PipeServer.cpp`, `installer/MoqiTsf.iss`, `backends.json`.
- Existing C++ uses lower camelCase for methods and free functions: `waitForRpcConnection()` in `MoqiTextService/MoqiClient.cpp`, `formatCodePoints()` in `MoqiTextService/MoqiClient.cpp`, `getPipeName()` in `MoqLauncher/PipeServer.cpp`.
- Windows entry points and callbacks keep Win32 naming/signatures: `wWinMain()` in `SetupHelper/SetupHelper.cpp`, `CALLBACK Client::onAsyncPollTimer()` in `MoqiTextService/MoqiClient.cpp`, `DllRegisterServer()` in `MoqiTextService/DllEntry.cpp`.
- Test names use GoogleTest `TEST(SuiteName, CaseName)` with PascalCase case names: `TEST(TestComPtr, DefaultsToNull)` in `libIME2/test/ComPtr_test.cpp`, `TEST(TestIUnknownImpl, QueryInterfaceTSF)` in `libIME2/test/ComObject_test.cpp`.
- PowerShell helper functions use approved verb-style names or local action names with PascalCase: `Invoke-Step` and `Resolve-ProtobufRoot` in `scripts/build.ps1`, `Copy-MoqiImeRuntime` in `scripts/install.ps1`.
- C++ class members use trailing underscores: `pipe_`, `rpcInProgress_`, `pendingAsyncResponses_` in `MoqiTextService/MoqiClient.h`; `singleInstanceMutex_` and `logger_` in `MoqLauncher/PipeServer.h`.
- Local variables use lower camelCase in newer first-party C++: `serializedReply` in `MoqiTextService/MoqiClient.cpp`, `targetWindow` in `MoqiTextService/MoqiClient.cpp`, `reboot_required` is also present in newer helper code in `SetupHelper/SetupHelper.cpp`.
- Constants are mixed. Legacy code uses all caps `MAX_LOG_FILE_SIZE` in `MoqLauncher/PipeServer.cpp`; newer helper and preview code use `k`-prefixed constants such as `kExitSuccess` in `SetupHelper/SetupHelper.cpp` and `kWindowClassName` in `Preview/main.cpp`. Prefer `k`-prefixed constants for new TypeDuck code.
- CMake variables are uppercase and project-prefixed: `MOQI_PROTOBUF_ROOT`, `MOQI_GENERATED_PROTO_DIR`, `MOQI_PROTO_CPP` in `CMakeLists.txt`. Rename or introduce TypeDuck-specific variables consistently when replacing scaffold-owned build config.
- Classes and structs use PascalCase: `Ime::ComPtr` in `libIME2/src/ComPtr.h`, `Moqi::Client` in `MoqiTextService/MoqiClient.h`, `PipeSecurityAttributes` in `MoqLauncher/PipeSecurity.h`.
- Namespaces separate framework and product layers: `Ime` for the reusable TSF/libIME layer in `libIME2/src/ComObject.h`, `Moqi` for product scaffold code in `MoqiTextService/MoqiClient.h` and `MoqLauncher/PipeServer.cpp`, `Moqi::Proto` for framing in `proto/ProtoFraming.h`.
- Generated protobuf types live under `moqi::protocol` in `proto/moqi.pb.h`; do not hand-edit `proto/moqi.pb.h` or `proto/moqi.pb.cc`.

## Code Style

- Formatting tool: Not detected. There is no `.clang-format`, `.editorconfig`, `.clang-tidy`, or lint config at repo root.
- C++ formatting is inconsistent across legacy and newer code. Legacy `libIME2/src/ImeEngine.cpp` uses 4-space indentation and compact `if(...)` style; newer `SetupHelper/SetupHelper.cpp`, `Preview/main.cpp`, and `proto/ProtoFraming.h` use 2-space indentation, braces on the same line, and spaces inside control statements.
- For new TypeDuck-owned files, use the newer 2-space C++ style from `SetupHelper/SetupHelper.cpp` and `proto/ProtoFraming.h`. Do not reformat untouched legacy files in `libIME2/src` or large scaffold files such as `MoqiTextService/MoqiClient.cpp` as part of feature changes.
- Keep C++ standard at C++20 to match `CMakeLists.txt`, `libIME2/CMakeLists.txt`, and `MoqiTextService/CMakeLists.txt`.
- Use Unicode-aware Windows APIs and wide strings for Windows UI/paths: `CreateWindowExW()` in `Preview/main.cpp`, `MessageBoxW()` and `std::wstring` in `SetupHelper/SetupHelper.cpp`, `StringFromCLSID()` conversion helpers in `MoqiTextService/TsfLog.cpp`.
- Tool used: Not detected.
- Key rules: Not enforced by config. Compile-time safety comes from MSVC flags in `CMakeLists.txt`, including `/utf-8`, disabled RTTI via `/GR-`, `_UNICODE`, `UNICODE`, and C++20.
- Manual review should check COM reference-counting, HRESULT branches, handle closure, Win32 resource cleanup, generated-file boundaries, and legacy Moqi user-facing text.

## Import Organization

- CMake include directories provide root-relative includes from `${CMAKE_SOURCE_DIR}` and generated protobuf root in `MoqiTextService/CMakeLists.txt` and `MoqLauncher/CMakeLists.txt`.
- Include generated protobuf as `proto/moqi.pb.h`, as in `MoqiTextService/MoqiClient.cpp` and `MoqiTextService/MoqiClient.h`.
- Include libIME headers either through `libIME2/src/...` for cross-module usage (`MoqiTextService/MoqiClient.h`) or direct local names inside `libIME2/src` (`libIME2/src/ImeEngine.h`).

## Error Handling

- COM and TSF functions return `HRESULT` and compare against `S_OK`, `FAILED(...)`, or `SUCCEEDED(...)`: `libIME2/src/ComObject.h`, `libIME2/src/ImeModule.cpp`, `MoqiTextService/MoqiTextService.cpp`.
- Win32 helper executables return integer exit codes and build user-visible failure strings: `kExitFailure`, `FormatWindowsErrorMessage()`, and `ShowFailureAndReturn()` in `SetupHelper/SetupHelper.cpp`.
- RPC and pipe failures generally return `false`, close the pipe, and reset text service state: `Client::callRpcMethod()` and `Client::closeRpcConnection()` in `MoqiTextService/MoqiClient.cpp`.
- Exceptions are used sparingly around operations that can fail during setup/logging: `MoqLauncher/PipeSecurity.cpp`, `MoqLauncher/PipeServer.cpp`, and PowerShell scripts in `scripts/build.ps1` and `scripts/install.ps1`.
- PowerShell scripts set `$ErrorActionPreference = "Stop"` and throw on failed external commands: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`.

## Logging

- Launcher logging uses `spdlog::rotating_logger_mt` with fallback to stderr in `MoqLauncher/PipeServer.cpp`.
- TSF/debug logging writes to `%LOCALAPPDATA%\MoqiIM\Log\tsf-debug.log` and `OutputDebugStringW()` in `MoqiTextService/TsfLog.cpp`, `MoqiTextService/DllEntry.cpp`, and `MoqiTextService/MoqiClient.cpp`.
- Debug logging gates are controlled through `Ime::isDebugLoggingEnabled()` and `Ime::isTraceLoggingEnabled()` in `libIME2/src/DebugLogConfig.cpp` and `MoqiTextService/MoqiClient.cpp`.
- PowerShell scripts report progress with `Write-Host` and warnings with `Write-Warning`: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`.
- For TypeDuck replacement work, rename log directories and logger names when product-owned paths are changed; do not leave new user-facing TypeDuck behavior writing to Moqi-branded locations unless explicitly preserving migration compatibility.

## Comments

- Use comments for Win32/TSF constraints, COM ownership, and non-obvious Windows behavior. Good examples are TSF DLL placement comments in `SetupHelper/SetupHelper.cpp`, app-container pipe security notes in `MoqLauncher/PipeServer.cpp`, and COM `IUnknown` pointer comments in `libIME2/src/ComObject.h`.
- Keep FIXME/TODO comments specific and actionable. Existing debt examples are in `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/BackendServer.cpp`, `libIME2/src/CandidateWindow.cpp`, and `libIME2/test/ComObject_test.cpp`.
- Do not add comments that only restate simple code; existing useful comments explain Windows API limitations, TSF lifecycle, or migration-sensitive behavior.
- Not applicable. This is a C++/PowerShell codebase.
- C++ has no Doxygen convention in first-party files; comments are plain `//` and block license headers.

## Function Design

## Module Design

- TSF COM exports live in `MoqiTextService/DllEntry.cpp` and `MoqiTextService/MoqiTextService.def`.
- Reusable IME/TSF primitives are built as `libIME2_static` from `libIME2/src/CMakeLists.txt`.
- Product TSF DLL code is built as `MoqiTextService` in `MoqiTextService/CMakeLists.txt`; launcher code is built as `MoqiLauncher` in `MoqLauncher/CMakeLists.txt`; installer helper code is built from `SetupHelper/CMakeLists.txt`.
- Moqi branding, Simplified Chinese UI strings, backend names, and fcitx/Moqi-specific configuration are scaffold details in paths such as `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/PipeServer.cpp`, `installer/MoqiTsf.iss`, and `backends.json`.
- New product-owned UI strings should be bilingual Traditional Hong Kong Chinese and English where user-facing; do not copy unused Moqi/fcitx language-bar or menu clutter from `MoqLauncher/PipeServer.cpp` or `MoqiTextService/MoqiClient.cpp`.
- TypeDuck librime fork and dictionary lookup filter plugin work should use new TypeDuck names in new code/config while preserving required Windows TSF conventions from `libIME2/src` and installer registration patterns from `SetupHelper/SetupHelper.cpp`.

<!-- GSD:conventions-end -->

<!-- GSD:architecture-start source:ARCHITECTURE.md -->

## Architecture

## System Overview

```text

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

- Keep TSF/COM mechanics in `libIME2/src`; put TypeDuck product behavior in the product text service layer represented by `MoqiTextService/*`.
- Communicate between the in-process TSF DLL and the launcher through a per-user Windows named pipe created by `MoqLauncher/PipeServer.cpp`.
- Communicate between launcher and backend through backend process stdin/stdout using protobuf frames defined by `proto/moqi.proto` and `proto/ProtoFraming.h`.
- Treat Moqi-specific branding, launcher names, install directory names, cloud clipboard, AI, and fcitx-facing concepts in `README.md`, `installer/MoqiTsf.iss`, `backends.json`, `MoqLauncher/*`, and `MoqiTextService/*` as legacy scaffold.
- Preserve the architectural boundary for the TypeDuck rewrite: Windows TSF frontend in this repo, TypeDuck librime fork and dictionary lookup filter plugin in the backend/runtime payload, and installation metadata under Chinese (Traditional, Hong Kong).

## Layers

- Purpose: Defines targets, compiler settings, generated protobuf output, third-party dependencies, and 32-bit/x64 target inclusion.
- Location: `CMakeLists.txt`
- Contains: `MoqiTextService`, `MoqLauncher`, `SetupHelper`, `Preview`, `libIME2`, `libuv`, `jsoncpp`, generated `proto/moqi.pb.*`.
- Depends on: Visual Studio/MSVC, CMake, protobuf, jsoncpp, libuv, spdlog, Windows SDK.
- Used by: `scripts/build.ps1`, `.github/workflows/release.yml`, `.github/workflows/nightly.yml`.
- Purpose: Provides generic Microsoft TSF text service plumbing and Win32 window abstractions.
- Location: `libIME2/src`
- Contains: `TextService`, `ImeModule`, `EditSession`, `KeyEvent`, `CandidateWindow`, `LangBarButton`, `DisplayAttributeProvider`, `ComObject`, `ComPtr`.
- Depends on: Windows TSF COM APIs, Win32, `shlwapi.lib`.
- Used by: `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`.
- Purpose: Owns product-specific TSF behavior, backend RPC, candidate rendering, lang-bar controls, user-facing messages, and process-specific UI policy.
- Location: `MoqiTextService`
- Contains: `MoqiTextService.cpp`, `MoqiClient.cpp`, `MoqiCandidateWindow.cpp`, `MoqiLangBarButton.cpp`, `MoqiImeModule.cpp`, `DllEntry.cpp`.
- Depends on: `libIME2/src`, `jsoncpp`, generated protobuf includes, `proto/ProtoFraming.h`.
- Used by: Windows TSF hosts through `MoqiTextService.dll`.
- Purpose: Keeps backend process lifetime outside TSF host processes and multiplexes multiple TSF clients over one backend bridge.
- Location: `MoqLauncher`
- Contains: `PipeServer`, `PipeClient`, `BackendServer`, `PipeSecurityAttributes`, `UvPipe`, JSON helpers, tray UI.
- Depends on: `libuv`, protobuf, jsoncpp, spdlog, Win32 shell/pipe/process APIs.
- Used by: `MoqiTextService/MoqiClient.cpp` through the launcher named pipe.
- Purpose: Defines the request/response contract for lifecycle, key events, preserved keys, composition, candidate lists, UI customization, tray notification, and cloud clipboard upload.
- Location: `proto/moqi.proto`, `proto/ProtoFraming.h`
- Contains: `ClientRequest`, `ServerResponse`, `KeyEvent`, `CandidateEntry`, `CustomizeUi`, `PreservedKey`, `TrayNotification`.
- Depends on: protobuf C++ generation in `CMakeLists.txt`.
- Used by: `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/PipeClient.cpp`, `MoqLauncher/BackendServer.cpp`.
- Purpose: Builds both bitnesses, stages runtime payload, installs application files, copies TSF DLLs to system directories, and registers COM/TSF language profiles.
- Location: `scripts`, `installer`, `SetupHelper`
- Contains: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`, `installer/MoqiTsf.iss`, `SetupHelper/SetupHelper.cpp`.
- Depends on: Visual Studio 2022, CMake, protoc/protobuf source, Inno Setup 6, Windows admin privileges.
- Used by: local release builds and `.github/workflows/release.yml`.

## Data Flow

### Primary Keystroke Path

### Registration and Install Path

### Backend and Language Profile Discovery

- TSF composition state lives in `Ime::TextService` and is manipulated via `startComposition`, `setCompositionString`, `setCompositionCursor`, and `endComposition` (`libIME2/src/TextService.cpp:385`, `libIME2/src/TextService.cpp:521`, `libIME2/src/TextService.cpp:638`, `libIME2/src/TextService.cpp:452`).
- Product UI state lives in `Moqi::TextService` fields for candidate list, selection, preedit, appearance, UI-less policy, message window, and current language profile (`MoqiTextService/MoqiTextService.h`).
- Backend client state lives in `Moqi::Client`, including sequence numbers, request/response buffers, lang-bar button maps, preserved keys, async responses, and the launcher pipe (`MoqiTextService/MoqiClient.h`).
- Launcher global state lives in `PipeServer` as a singleton with clients, backends, GUID map, tray notification queue, and clipboard async handle (`MoqLauncher/PipeServer.h`).

## Key Abstractions

- Purpose: Base class for TSF activation, key sink handling, composition range mutation, compartments, and display attributes.
- Examples: `libIME2/src/TextService.h`, `libIME2/src/TextService.cpp`.
- Pattern: Template-method base class; product behavior overrides hooks such as `onActivate`, `onDeactivate`, `onKeyDown`, `onPreservedKey`, `onCompartmentChanged`, and language-profile callbacks.
- Purpose: Product-level IME controller for the current TSF host process.
- Examples: `MoqiTextService/MoqiTextService.h`, `MoqiTextService/MoqiTextService.cpp`.
- Pattern: Subclass of `Ime::TextService` that owns a `Moqi::Client` and Win32 candidate/message UI.
- Purpose: RPC adapter between TSF events and backend protocol responses.
- Examples: `MoqiTextService/MoqiClient.h`, `MoqiTextService/MoqiClient.cpp`.
- Pattern: Stateful request builder and response applier; future TypeDuck backend behavior should flow through this protocol boundary.
- Purpose: Per-user IPC server, per-TSF-client pipe connection, and managed backend process.
- Examples: `MoqLauncher/PipeServer.h`, `MoqLauncher/PipeClient.h`, `MoqLauncher/BackendServer.h`.
- Pattern: libuv event-loop objects with callbacks and process supervision.
- Purpose: Reassembles stream data into complete protobuf payloads with a 32-bit little-endian length prefix.
- Examples: `proto/ProtoFraming.h`, `MoqLauncher/PipeClient.cpp`, `MoqLauncher/BackendServer.cpp`, `MoqiTextService/MoqiClient.cpp`.
- Pattern: Shared framing utility; use it for all launcher/backend protocol messages.
- Purpose: Data structure used during TSF language-profile registration.
- Examples: `libIME2/src/ImeModule.h`, `MoqiTextService/DllEntry.cpp`.
- Pattern: Metadata loaded from backend `ime.json`; TypeDuck Hong Kong Traditional Chinese registration data belongs in the installed backend metadata plus registration code paths.

## Entry Points

- Location: `MoqiTextService/DllEntry.cpp`
- Triggers: Windows loads `MoqiTextService.dll` into TSF host processes.
- Responsibilities: `DllMain`, `DllGetClassObject`, `DllCanUnloadNow`, `DllRegisterServer`, `DllUnregisterServer`.
- Location: `libIME2/src/ImeModule.cpp`, `MoqiTextService/MoqiImeModule.cpp`
- Triggers: TSF requests a class object for the text service CLSID.
- Responsibilities: Return a `Moqi::TextService` instance and register TSF categories/language profiles.
- Location: `MoqLauncher/MoqiLauncher.cpp`, `MoqLauncher/PipeServer.cpp`
- Triggers: Inno `[Run]`, startup registry key, or manual process launch.
- Responsibilities: Enforce single instance, initialize backends, listen on named pipe, run tray GUI thread, and run the libuv loop.
- Location: `SetupHelper/SetupHelper.cpp`
- Triggers: `installer/MoqiTsf.iss` runs `SetupHelper.exe /i`, `/r`, or `/u`.
- Responsibilities: Copy, register, unregister, and schedule reboot-time TSF registration.
- Location: `Preview/main.cpp`
- Triggers: Manual build/run of `MoqiCandidatePreview`.
- Responsibilities: Standalone Win32 preview for candidate rendering experiments.
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

### Single-File Brand Changes

### Protocol Bypass Around Launcher

### User-Facing Scaffold Leakage

## Error Handling

- Return `HRESULT` from COM entry points and registration functions in `MoqiTextService/DllEntry.cpp` and `libIME2/src/ImeModule.cpp`.
- Guard backend callback invocation from TSF event handlers in `MoqiTextService/MoqiTextService.cpp`.
- Restart backend processes after read errors and timeouts in `MoqLauncher/BackendServer.cpp` and `MoqLauncher/PipeClient.cpp`.
- Use `SetupHelper` exit codes for installer success/restart-required/failure in `SetupHelper/SetupHelper.cpp` and `installer/MoqiTsf.iss`.
- Write TSF/candidate-window diagnostics to local app data log files from `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`, and `MoqiTextService/TsfLog.cpp`.

## Cross-Cutting Concerns

<!-- GSD:architecture-end -->

<!-- GSD:skills-start source:skills/ -->

## Project Skills

No project skills found. Add skills to any of: `.claude/skills/`, `.agents/skills/`, `.cursor/skills/`, `.github/skills/`, or `.codex/skills/` with a `SKILL.md` index file.
<!-- GSD:skills-end -->

<!-- GSD:workflow-start source:GSD defaults -->

## GSD Workflow Enforcement

Before using Edit, Write, or other file-changing tools, start work through a GSD command so planning artifacts and execution context stay in sync.

Use these entry points:

- `/gsd-quick` for small fixes, doc updates, and ad-hoc tasks
- `/gsd-debug` for investigation and bug fixing
- `/gsd-execute-phase` for planned phase work

Do not make direct repo edits outside a GSD workflow unless the user explicitly asks to bypass it.
<!-- GSD:workflow-end -->

<!-- GSD:profile-start -->

## Developer Profile

> Profile not yet configured. Run `/gsd-profile-user` to generate your developer profile.
> This section is managed by `generate-claude-profile` -- do not edit manually.
<!-- GSD:profile-end -->
