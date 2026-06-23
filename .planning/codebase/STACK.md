# Technology Stack

**Analysis Date:** 2026-06-23

## Languages

**Primary:**
- C++20 - Windows TSF text service, launcher, setup helper, preview utility, and libIME2 integration in `MoqiTextService/`, `MoqLauncher/`, `SetupHelper/`, `Preview/`, and `libIME2/src/`.
- C - vendored libuv Windows event-loop/process/pipe implementation in `libuv/src/`.

**Secondary:**
- PowerShell 5.1 - build, staging, installer packaging, icon generation, and install scripts in `scripts/*.ps1` and `installer/build-installer.ps1`.
- Inno Setup Pascal Script - graphical installer and registry/process orchestration in `installer/MoqiTsf.iss`.
- Protocol Buffers schema - frontend/backend IPC contract in `proto/moqi.proto`.
- Go - external legacy backend language for the sibling `moqi-ime` runtime referenced by `README.md`, `.github/workflows/nightly.yml`, `.github/workflows/release.yml`, and `scripts/_all_in_package.ps1`; no Go source lives in this repo.

## Runtime

**Environment:**
- Windows desktop IME runtime using Microsoft Text Services Framework (TSF), COM DLL registration, Win32 windows/messages, shell tray APIs, named pipes, and child processes.
- Minimum Windows target is Windows 8+ for the top-level build (`_WIN32_WINNT=0x0602` in `CMakeLists.txt`); libuv itself is compiled with a Vista-level target (`_WIN32_WINNT=0x0600` in `libuv/CMakeLists.txt`).
- Installer is x64-only (`ArchitecturesAllowed=x64` in `installer/MoqiTsf.iss`) but deploys both Win32 and x64 TSF DLLs.

**Package Manager:**
- No language package manager manifest is present for the Windows frontend.
- CMake FetchContent downloads `spdlog` and `protobuf` when local protobuf paths are not supplied (`CMakeLists.txt`).
- Git submodules provide `libuv`, `libIME2`, `jsoncpp`, and the Inno Simplified Chinese translation (`.gitmodules`).
- Lockfile: Not detected.

## Frameworks

**Core:**
- Microsoft TSF / COM / Win32 API - IME registration, key event handling, candidate UI, language bar buttons, shell tray, clipboard listener, registry integration, and setup helper operations across `MoqiTextService/`, `MoqLauncher/`, `SetupHelper/`, and `libIME2/src/`.
- libIME2 - static TSF abstraction layer built from `libIME2/src/CMakeLists.txt` and linked by `MoqiTextService/CMakeLists.txt`.
- libuv - launcher event loop, named pipe server, subprocess management, timers, async handoff, and stdio pipes built by `libuv/CMakeLists.txt` and linked by `MoqLauncher/CMakeLists.txt`.
- Protocol Buffers - framed binary frontend/backend IPC generated from `proto/moqi.proto`; framing helpers live in `proto/ProtoFraming.h`.
- JsonCpp - JSON config/metadata parsing for `backends.json`, `input_methods/*/ime.json`, launcher config, and UI metadata in `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `MoqLauncher/PipeServer.cpp`, and `MoqLauncher/Utils.cpp`.

**Testing:**
- GoogleTest 1.10.0 - vendored under `libIME2/lib/googletest-release-1.10.0` and enabled by `libIME2/CMakeLists.txt`.
- No top-level automated tests for `MoqiTextService/`, `MoqLauncher/`, `SetupHelper/`, or installer scripts are detected.

**Build/Dev:**
- CMake 3.21+ - top-level generator and dependency orchestration in `CMakeLists.txt`; README lists CMake 3.21+ as prerequisite.
- Visual Studio 2022 / MSBuild - default generator `Visual Studio 17 2022` in `scripts/build.ps1` and CI workflows.
- Inno Setup 6 - installer compiler required by `installer/build-installer.ps1` and `installer/README.txt`.
- GitHub Actions on `windows-2022` - nightly and release packaging pipelines in `.github/workflows/nightly.yml` and `.github/workflows/release.yml`.

## Key Dependencies

**Critical:**
- `libIME2` submodule - TSF/IME base classes, COM helpers, candidate windows, message windows, and language bar support (`libIME2/src/`, `.gitmodules`).
- `libuv` submodule, version macros `1.11.1-dev` - launcher event loop, named pipes, async callbacks, timers, process spawning, and stdio streams (`libuv/include/uv-version.h`, `libuv/CMakeLists.txt`).
- `protobuf` 33.5 - schema compiler/runtime for `proto/moqi.proto`; downloaded from GitHub by `CMakeLists.txt` or provided through local CMake cache vars.
- `jsoncpp` 1.8.4 - static JSON parser/writer used for backend manifests, IME metadata, config, and UI bridge objects (`jsoncpp/include/json/version.h`).
- `spdlog` v1.2.1 - launcher rotating file logging (`CMakeLists.txt`, `MoqLauncher/PipeServer.cpp`).
- Windows SDK libraries - TSF/COM/Win32 dependencies including `Rpcrt4`, `Advapi32`, `Shell32`, `shlwapi.lib`, `gdi32`, `user32`, `ws2_32`, `psapi`, `iphlpapi`, and `userenv` across component `CMakeLists.txt` files.

**Infrastructure:**
- `MoqiLauncher.exe` - 32-bit launcher target built only when `CMAKE_SIZEOF_VOID_P EQUAL 4` (`CMakeLists.txt`, `MoqLauncher/CMakeLists.txt`).
- `MoqiTextService.dll` - TSF COM DLL built for Win32 and x64 (`MoqiTextService/CMakeLists.txt`, `scripts/build.ps1`).
- `SetupHelper.exe` - elevated installer helper for system-directory TSF DLL copy, regsvr32 registration, scheduled re-registration, and uninstall cleanup (`SetupHelper/CMakeLists.txt`, `SetupHelper/SetupHelper.cpp`).
- `MoqiCandidatePreview.exe` - standalone candidate UI preview using GDI/User32 (`Preview/CMakeLists.txt`).

## Configuration

**Environment:**
- Build-time CMake cache variables: `MOQI_PROTOBUF_ROOT`, `MOQI_PROTOBUF_SOURCE_DIR`, and `MOQI_PROTOC_EXECUTABLE` in `CMakeLists.txt` and `scripts/build.ps1`.
- Setup-time environment variable: `MOQI_PROGRAM_DIR` tells the TSF DLL where the installed payload lives during registration and runtime lookup (`SetupHelper/SetupHelper.cpp`, `MoqiTextService/MoqiImeModule.cpp`).
- Runtime environment paths: `%LOCALAPPDATA%\MoqiIM` stores launcher config and logs (`MoqLauncher/PipeServer.cpp`, `libIME2/src/DebugLogConfig.cpp`); `%APPDATA%\Moqi` stores legacy cloud clipboard config (`MoqLauncher/Utils.cpp`, `MoqLauncher/PipeServer.cpp`).
- Backend subprocess environment adds `PYTHONIOENCODING=utf-8:ignore` for Unicode-safe backend stdio (`MoqLauncher/BackendServer.cpp`).

**Build:**
- Top-level build orchestration: `CMakeLists.txt`.
- MSVC runtime override: `CMAKE_MSVC_RUNTIME_LIBRARY` in `CMakeLists.txt`, plus `cmake/c_flag_overrides.cmake` and `cmake/cxx_flag_overrides.cmake`.
- Version source: `version.txt`, parsed by `CMakeLists.txt` and applied to resource templates such as `MoqiTextService/MoqiTextService.rc.in` and `MoqLauncher/version.rc.in`.
- Backend manifest: `backends.json` currently points to legacy `moqi-ime\server.exe`.
- IME language profile metadata: installed backend folders under `input_methods/*/ime.json`, scanned by `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiImeModule.cpp`, and `MoqLauncher/PipeServer.cpp`.
- Installer script: `installer/MoqiTsf.iss`; staging script: `scripts/install.ps1`; full packaging script: `scripts/_all_in_package.ps1`.

## Platform Requirements

**Development:**
- Windows with Visual Studio 2022, Windows SDK, CMake 3.21+, PowerShell 5.1, git submodules, and Inno Setup 6 for installer output (`README.md`, `scripts/build.ps1`, `installer/README.txt`).
- A `protoc.exe` path must be discoverable through `MOQI_PROTOC_EXECUTABLE`, `MOQI_PROTOBUF_ROOT`, `find_program(protoc)`, or CMake FetchContent protobuf build (`CMakeLists.txt`).
- Full package builds expect a sibling legacy backend checkout at `..\moqi-ime` unless `-MoqiImeRoot` or `-MoqiImeSource` is provided (`scripts/_all_in_package.ps1`, `scripts/install.ps1`).

**Production:**
- Installed payload currently targets `%ProgramFiles(x86)%\MoqiIM` via Inno Setup (`installer/MoqiTsf.iss`).
- TSF DLLs are copied into `SysWOW64\MoqiTextService.dll` and `System32\MoqiTextService.dll`, then registered with `regsvr32.exe` by `SetupHelper/SetupHelper.cpp`.
- Launcher autostart is configured at `HKCU\Software\Microsoft\Windows\CurrentVersion\Run\MoqiLauncher` (`installer/MoqiTsf.iss`).
- TypeDuck rewrite note: Moqi product names, paths, icons, Simplified Chinese installer language, legacy `moqi-ime` backend, Rime/fcitx references, and `%APPDATA%\Moqi` / `%LOCALAPPDATA%\MoqiIM` locations are current scaffold details, not target product behavior. Future TypeDuck work should keep the Windows TSF/COM/build architecture where useful while replacing legacy branding/config and installing under Chinese (Traditional, Hong Kong).

---

*Stack analysis: 2026-06-23*
