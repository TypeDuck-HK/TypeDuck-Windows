# Codebase Structure

**Analysis Date:** 2026-06-23

## Directory Layout

```text
moqi-im-windows/
├── .github/               # GitHub Actions release/nightly packaging workflows
├── .planning/codebase/    # GSD-generated codebase maps
├── .vscode/               # Local editor settings
├── cmake/                 # MSVC runtime flag override files
├── installer/             # Inno Setup script, installer compiler wrapper, translation assets
├── jsoncpp/               # Vendored jsoncpp dependency
├── libIME2/               # Generic TSF/COM support library plus small unit tests
├── libuv/                 # Vendored libuv dependency used by launcher
├── MoqiTextService/       # In-process TSF DLL and product-specific Windows IME UI/client logic
├── MoqLauncher/           # Win32 launcher, named pipe server, backend process bridge, tray UI
├── others/imgs/           # README/demo images for legacy Moqi scaffold
├── Preview/               # Standalone candidate window preview executable
├── proto/                 # Protobuf schema, generated C++ protobuf files, framing helper
├── scripts/               # Build, package, install-stage, and icon generation PowerShell scripts
├── SetupHelper/           # Admin helper for copying/registering TSF DLLs
├── backends.json          # Installed backend process manifest
├── CMakeLists.txt         # Top-level native build graph
├── README.md              # Legacy Moqi product documentation
├── TODO.md                # Small project note file
└── version.txt            # Semantic version input for resources/build
```

## Directory Purposes

**`MoqiTextService`:**
- Purpose: Product-specific Windows TSF text service DLL; this is the main frontend surface for the TypeDuck rewrite.
- Contains: COM DLL exports, TSF subclass, backend RPC client, candidate window, lang-bar button, resources, logging helpers.
- Key files: `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `MoqiTextService/MoqiTextService.rc.in`.

**`MoqLauncher`:**
- Purpose: Per-user background process that isolates backend lifetime from TSF host processes and forwards messages between named pipe clients and backend stdio.
- Contains: Pipe server/client, backend process manager, libuv pipe wrapper, pipe security, JSON helpers, tray menu and notifications.
- Key files: `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeClient.cpp`, `MoqLauncher/BackendServer.cpp`, `MoqLauncher/UvPipe.h`, `MoqLauncher/PipeSecurity.cpp`, `MoqLauncher/MoqiLauncher.cpp`.

**`libIME2`:**
- Purpose: Generic TSF/COM foundation reused by the product text service.
- Contains: Static library source under `libIME2/src`, GoogleTest under `libIME2/lib/googletest-release-1.10.0`, and unit tests under `libIME2/test`.
- Key files: `libIME2/src/TextService.cpp`, `libIME2/src/ImeModule.cpp`, `libIME2/src/EditSession.cpp`, `libIME2/src/ComObject.h`, `libIME2/src/ComPtr.h`, `libIME2/test/ComObject_test.cpp`, `libIME2/test/ComPtr_test.cpp`.

**`proto`:**
- Purpose: Shared protocol between TSF DLL, launcher, and backend.
- Contains: Authoritative schema, checked-in generated C++ protobuf files, and frame assembly/parsing helper.
- Key files: `proto/moqi.proto`, `proto/ProtoFraming.h`, `proto/moqi.pb.cc`, `proto/moqi.pb.h`.

**`SetupHelper`:**
- Purpose: Elevated install-time helper for TSF DLL file operations and COM/TSF registration.
- Contains: Single Win32 executable target and source file.
- Key files: `SetupHelper/SetupHelper.cpp`, `SetupHelper/CMakeLists.txt`.

**`installer`:**
- Purpose: Inno Setup package definition and package compilation helper.
- Contains: `.iss` installer script, Chinese Simplified translation asset, README, build script, generated `dist` output, and ignored `stage` tree.
- Key files: `installer/MoqiTsf.iss`, `installer/build-installer.ps1`, `installer/README.txt`, `installer/Inno-Setup-Chinese-Simplified-Translation/ChineseSimplified.isl`.

**`scripts`:**
- Purpose: Local packaging workflow orchestration.
- Contains: Build script for Win32/x64, installer staging script, one-click backend+frontend+installer script, and icon generation script.
- Key files: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`, `scripts/generate-glyph-icon.ps1`.

**`Preview`:**
- Purpose: Standalone Win32 candidate UI preview.
- Contains: `MoqiCandidatePreview` executable source and CMake target.
- Key files: `Preview/main.cpp`, `Preview/CMakeLists.txt`.

**`jsoncpp`:**
- Purpose: Vendored JSON parser used by product service and launcher.
- Contains: Upstream jsoncpp source, tests, docs, and CMake files.
- Key files: `jsoncpp/CMakeLists.txt`, `jsoncpp/include/json/json.h`, `jsoncpp/src/lib_json/json_reader.cpp`, `jsoncpp/src/lib_json/json_value.cpp`, `jsoncpp/src/lib_json/json_writer.cpp`.

**`libuv`:**
- Purpose: Vendored async/process/pipe library used by `MoqLauncher`.
- Contains: Upstream libuv source, include files, docs, samples, and tests.
- Key files: `libuv/CMakeLists.txt`, `libuv/include/uv.h`, `libuv/src/win/pipe.c`, `libuv/src/win/process.c`.

**`cmake`:**
- Purpose: MSVC runtime and release flag overrides used before top-level `project()`.
- Contains: C and C++ override files.
- Key files: `cmake/c_flag_overrides.cmake`, `cmake/cxx_flag_overrides.cmake`.

**`.github`:**
- Purpose: CI packaging pipelines for nightly and release installers.
- Contains: GitHub Actions workflows that checkout this repo and sibling backend repo, install tools, build package, and upload artifacts.
- Key files: `.github/workflows/release.yml`, `.github/workflows/nightly.yml`.

## Key File Locations

**Entry Points:**
- `MoqiTextService/DllEntry.cpp`: TSF DLL load, class factory, register/unregister exports, backend `ime.json` scan.
- `MoqiTextService/MoqiImeModule.cpp`: Text service CLSID, program directory discovery, backend input-method metadata loading.
- `MoqiTextService/MoqiTextService.cpp`: Product text service lifecycle and TSF callback overrides.
- `MoqLauncher/MoqiLauncher.cpp`: Launcher executable entry point.
- `MoqLauncher/PipeServer.cpp`: Launcher single-instance setup, named pipe listening, backend initialization, tray GUI thread.
- `SetupHelper/SetupHelper.cpp`: Installer helper action dispatch.
- `Preview/main.cpp`: Candidate preview app entry point.

**Configuration:**
- `CMakeLists.txt`: Top-level project, dependency fetch, protobuf generation, target inclusion rules.
- `MoqiTextService/CMakeLists.txt`: TSF DLL source list and link dependencies.
- `MoqLauncher/CMakeLists.txt`: Launcher source list, libuv/spdlog definitions, link dependencies.
- `libIME2/src/CMakeLists.txt`: `libIME2_static` source list.
- `backends.json`: Backend process name, command, working directory, and params.
- `version.txt`: Version read by CMake and resource templates.
- `.vscode/settings.json`: Workspace editor settings.

**Core Logic:**
- `libIME2/src/TextService.cpp`: Generic TSF activation, key handling, composition, compartments, and display attributes.
- `libIME2/src/ImeModule.cpp`: COM class factory and TSF registration implementation.
- `MoqiTextService/MoqiClient.cpp`: Backend RPC request/response handling and UI/composition application.
- `MoqiTextService/MoqiCandidateWindow.cpp`: Product candidate window rendering and `ITfCandidateListUIElement` methods.
- `MoqLauncher/BackendServer.cpp`: Backend process start/restart, stdin/stdout/stderr forwarding, backend response dispatch.
- `MoqLauncher/PipeClient.cpp`: Per-client pipe parsing, backend selection, timeout handling.
- `proto/moqi.proto`: Method and message contract.
- `proto/ProtoFraming.h`: Length-prefixed protobuf framing helper.

**Testing:**
- `libIME2/test/ComObject_test.cpp`: COM object helper unit tests.
- `libIME2/test/ComPtr_test.cpp`: COM smart pointer unit tests.
- `libIME2/test/CMakeLists.txt`: Test target definitions.

**Packaging:**
- `scripts/build.ps1`: CMake configure/build for Win32 and x64.
- `scripts/install.ps1`: Installer staging, artifact resolution, backend runtime copy.
- `scripts/_all_in_package.ps1`: One-click backend, Windows binaries, and installer build.
- `installer/build-installer.ps1`: Inno Setup compiler wrapper and stage validation.
- `installer/MoqiTsf.iss`: Install/uninstall script, startup registry, SetupHelper invocation, Moqi registry cleanup.

## Naming Conventions

**Files:**
- Product TSF files use `Moqi*` PascalCase prefixes: `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiCandidateWindow.h`, `MoqiTextService/MoqiLangBarButton.cpp`.
- Launcher files use role nouns in PascalCase: `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeClient.h`, `MoqLauncher/BackendServer.cpp`.
- Generic library files use PascalCase class names under `libIME2/src`: `libIME2/src/TextService.cpp`, `libIME2/src/ComPtr.h`, `libIME2/src/DisplayAttributeProvider.cpp`.
- Build scripts use lowercase verbs or descriptive names: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`, `installer/build-installer.ps1`.
- Generated protobuf files live beside the schema: `proto/moqi.pb.cc`, `proto/moqi.pb.h`.

**Directories:**
- Product executable/library directories are PascalCase: `MoqiTextService`, `MoqLauncher`, `SetupHelper`, `Preview`.
- Vendored dependencies keep upstream names: `jsoncpp`, `libuv`, `libIME2`.
- Support directories are lowercase: `cmake`, `installer`, `proto`, `scripts`, `others`.

## Where to Add New Code

**New TypeDuck TSF Behavior:**
- Primary code: `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiTextService.h`.
- Backend request/response behavior: `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiClient.h`.
- Candidate visual feel matching TypeDuck Web alpha: `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/MoqiCandidateWindow.h`.
- Lang-bar and preserved-key controls: `MoqiTextService/MoqiLangBarButton.cpp`, `MoqiTextService/MoqiLangBarButton.h`, `MoqiTextService/MoqiClient.cpp`.

**New Backend Protocol Capability:**
- Schema: `proto/moqi.proto`.
- Framing: reuse `proto/ProtoFraming.h`.
- TSF client handling: `MoqiTextService/MoqiClient.cpp`.
- Launcher pipe handling: `MoqLauncher/PipeClient.cpp`.
- Backend process handling: `MoqLauncher/BackendServer.cpp`.
- Regenerate generated sources through the top-level protobuf custom command in `CMakeLists.txt`.

**New Backend Runtime Integration:**
- Backend manifest: `backends.json`.
- Launcher backend discovery and GUID routing: `MoqLauncher/PipeServer.cpp`.
- Backend process launch details: `MoqLauncher/BackendServer.cpp`.
- Installer staging of runtime payload: `scripts/install.ps1`, `scripts/_all_in_package.ps1`.
- TypeDuck librime fork and dictionary lookup filter plugin belong in the staged backend runtime tree copied by `scripts/install.ps1`, not in `libIME2/src`.

**New Registration/Installer Behavior:**
- TSF CLSID and product module identity: `MoqiTextService/MoqiImeModule.cpp`.
- Language profile scanning and `locale`/`fallbackLocale` mapping: `MoqiTextService/DllEntry.cpp`.
- Registration mechanics: `libIME2/src/ImeModule.cpp`.
- Installer UI, install directory, startup registry, and cleanup: `installer/MoqiTsf.iss`.
- TSF DLL copy/register/unregister actions: `SetupHelper/SetupHelper.cpp`.
- Chinese (Traditional, Hong Kong) installation target requires coordinated changes in `installer/MoqiTsf.iss`, backend `ime.json`, `MoqiTextService/DllEntry.cpp`, and `libIME2/src/ImeModule.cpp`.

**New Build or Packaging Step:**
- Local build orchestration: `scripts/build.ps1`.
- Installer staging: `scripts/install.ps1`.
- Full package flow: `scripts/_all_in_package.ps1`.
- Inno compiler invocation: `installer/build-installer.ps1`.
- CI packaging: `.github/workflows/release.yml`, `.github/workflows/nightly.yml`.

**New Generic TSF Utility:**
- Shared TSF/COM implementation: `libIME2/src`.
- Tests: `libIME2/test`.
- Use `libIME2/src` only for generic TSF behavior that is not TypeDuck-specific.

**Utilities:**
- Launcher filesystem/JSON helpers: `MoqLauncher/Utils.cpp`, `MoqLauncher/Utils.h`.
- TSF logging helpers: `MoqiTextService/TsfLog.cpp`, `MoqiTextService/TsfLog.h`, `libIME2/src/DebugLogFile.h`.
- Shared protobuf framing: `proto/ProtoFraming.h`.

## Special Directories

**`jsoncpp`:**
- Purpose: Vendored third-party JSON library.
- Generated: No.
- Committed: Yes.

**`libuv`:**
- Purpose: Vendored third-party async/process/pipe library.
- Generated: No.
- Committed: Yes.

**`libIME2/lib/googletest-release-1.10.0`:**
- Purpose: Vendored GoogleTest dependency for `libIME2/test`.
- Generated: No.
- Committed: Yes.

**`proto/moqi.pb.cc` and `proto/moqi.pb.h`:**
- Purpose: Checked-in protobuf-generated C++ sources.
- Generated: Yes.
- Committed: Yes.

**`build-vs32`, `build-vs64`, `build-*`, `cmake-build-ninja`:**
- Purpose: Local CMake build outputs.
- Generated: Yes.
- Committed: No; ignored by `.gitignore`.

**`installer/stage`:**
- Purpose: Staged installer payload built by `scripts/install.ps1`.
- Generated: Yes.
- Committed: No; ignored by `.gitignore`.

**`installer/dist`:**
- Purpose: Inno Setup output directory for setup executables.
- Generated: Yes.
- Committed: No for normal development; output name is controlled by `installer/MoqiTsf.iss`.

**`others/imgs`:**
- Purpose: Legacy Moqi README/demo images.
- Generated: No.
- Committed: Yes.

---

*Structure analysis: 2026-06-23*
