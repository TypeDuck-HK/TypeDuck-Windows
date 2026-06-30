# Technology Stack

**Analysis Date:** 2026-06-28

**Scope:** This map treats `https://github.com/TypeDuck-HK/TypeDuck-Windows` and `https://github.com/TypeDuck-HK/TypeDuck-Windows-backend` as equivalent parts of the TypeDuck Windows v1 product. Paths are repo-relative and prefixed with `TypeDuck-Windows:` or `TypeDuck-Windows-backend:`.

## Languages

- C++20 - Windows TSF text service, launcher, setup helper, settings/about apps, preview utility, backend probe, and reusable TSF layer in `TypeDuck-Windows:CMakeLists.txt`, `TypeDuck-Windows:MoqiTextService/`, `TypeDuck-Windows:MoqLauncher/`, `TypeDuck-Windows:SetupHelper/`, `TypeDuck-Windows:TypeDuckSettings/`, `TypeDuck-Windows:Preview/`, `TypeDuck-Windows:Tools/TypeduckBackendProbe/`, and `TypeDuck-Windows:libIME2/src/`.
- Go - backend runtime process, protocol adapter, Rime service, native Rime wrapper, settings application, runtime package builder inputs, and tests in `TypeDuck-Windows-backend:server.go`, `TypeDuck-Windows-backend:protocol_io.go`, `TypeDuck-Windows-backend:imecore/`, and `TypeDuck-Windows-backend:input_methods/rime/`.
- PowerShell - build, runtime staging, installer packaging, release verification, protocol guards, VM proof helpers, and backend runtime packaging in `TypeDuck-Windows:scripts/*.ps1`, `TypeDuck-Windows:installer/build-installer.ps1`, and `TypeDuck-Windows-backend:scripts/*.ps1`.
- Inno Setup Pascal Script - graphical installer, registry/startup orchestration, setup helper invocation, bilingual install/uninstall UI, and uninstall data prompts in `TypeDuck-Windows:installer/MoqiTsf.iss`.
- Protocol Buffers - shared frontend/backend IPC schema in `TypeDuck-Windows:proto/moqi.proto` and `TypeDuck-Windows-backend:proto/moqi.proto`.
- C - vendored native dependency code in `TypeDuck-Windows:libuv/`, `TypeDuck-Windows:jsoncpp/`, and native headers consumed by C++ targets.

## Runtime

- TypeDuck Windows is a Windows desktop IME using Microsoft Text Services Framework, COM DLL registration, Win32 windows/messages, shell tray APIs, named pipes, subprocess management, and a Go backend process.
- `TypeDuck-Windows` builds the installed Windows frontend payload: `TypeDuckTextService.dll`, `TypeDuckLauncher.exe`, `TypeDuckSetupHelper.exe`, `TypeDuckSettings.exe`, `TypeDuckAbout.exe`, runtime resources, and the Inno installer `typeduck-windows-ime-setup.exe`.
- `TypeDuck-Windows-backend` builds `TypeDuckRuntime/server.exe` plus `input_methods/rime/` runtime files used by the launcher.
- The installer is x64-only, installs under `%ProgramFiles(x86)%\TypeDuckIME`, and deploys both 32-bit and 64-bit `TypeDuckTextService.dll` for TSF host-process bitness coverage.
- The main TSF profile is Chinese (Traditional, Hong Kong) / `zh-HK` with GUID `{C6E8F5DF-6504-44F9-B7CF-17A195373A83}` in `TypeDuck-Windows:MoqiTextService/TypeDuckProfile.cpp` and `TypeDuck-Windows-backend:input_methods/rime/ime.json`.
- The launcher communicates with TSF clients through a per-user named pipe under the `TypeDuckIME` namespace and communicates with the backend through length-prefixed protobuf frames over stdin/stdout.
- User settings live in `%APPDATA%\TypeDuckIME\TypeDuckPreferences.json`; logs use `%LOCALAPPDATA%\TypeDuckIME\Log` where available.

## Frameworks

- Microsoft TSF / COM / Win32 API - language profile registration, key event sinks, composition ranges, candidate UI, display attributes, shell tray, startup registry, system DLL registration, and settings/about UI.
- libIME2 - reusable static TSF/COM abstraction layer built from `TypeDuck-Windows:libIME2/src/CMakeLists.txt`.
- libuv - launcher event loop, named pipe server, subprocess lifecycle, timers, async handoff, and stdio pipe handling in `TypeDuck-Windows:MoqLauncher/`.
- Protocol Buffers - C++ and Go generated IPC types; C++ framing helper in `TypeDuck-Windows:proto/ProtoFraming.h`; Go framing helper in `TypeDuck-Windows-backend:protocol_io.go`.
- JsonCpp - TypeDuck preference JSON, installed IME metadata, launcher config, and helper JSON parsing in the Windows repo.
- spdlog - launcher rotating file logging in `TypeDuck-Windows:MoqLauncher/PipeServer.cpp`.
- GoogleTest / GoogleMock - vendored C++ tests through `TypeDuck-Windows:libIME2/lib/googletest-release-1.10.0`.
- Go `testing` package - backend unit, integration, Rime, protocol, and packaging tests.
- librime - backend Rime engine loaded from packaged `input_methods/rime/rime.dll` by `TypeDuck-Windows-backend:input_methods/rime/librime.go`.
- rime-dictionary-lookup-filter - required Rime module evidence for dictionary-style candidate comments; backend requests the `dictionary_lookup` module in `TypeDuck-Windows-backend:input_methods/rime/librime.go`.

## Key Dependencies

- Visual Studio 2022, Windows SDK, MSBuild, and CMake 3.21+ for `TypeDuck-Windows`.
- Inno Setup 6 for installer compilation through `TypeDuck-Windows:installer/build-installer.ps1`.
- Protobuf 33.5 for frontend C++ generation and CI-provided `protoc`.
- Go 1.25.0 declared in `TypeDuck-Windows-backend:go.mod`; GitHub workflows currently set up Go 1.24.6.
- Go modules: `google.golang.org/protobuf`, `gopkg.in/yaml.v3`, `golang.org/x/mobile`, `golang.org/x/mod`, `golang.org/x/sync`, and `golang.org/x/tools`.
- Vendored/frontend dependencies: `TypeDuck-Windows:libIME2/`, `TypeDuck-Windows:libuv/`, `TypeDuck-Windows:jsoncpp/`, and FetchContent `spdlog` / `protobuf`.
- TypeDuck-owned frontend submodule patches: `TypeDuck-Windows:patches/libIME2/0001-use-typeduck-diagnostics-paths.patch`, applied by `TypeDuck-Windows:scripts/Apply-TypeDuckSubmodulePatches.ps1`.
- Runtime binary dependency: TypeDuck-HK librime Windows release staged by `TypeDuck-Windows:scripts/Stage-TypeDuckRuntime.ps1`.
- Runtime data dependency: TypeDuck schema data passed as `-RimeDataSource` to `TypeDuck-Windows-backend:scripts/build.ps1`; frontend packaging workflows download the prebuilt TypeDuck schema release artifact and pass its extracted directory.

## Configuration

- Frontend CMake cache variables: `MOQI_PROTOBUF_ROOT`, `MOQI_PROTOBUF_SOURCE_DIR`, and `MOQI_PROTOC_EXECUTABLE` in `TypeDuck-Windows:CMakeLists.txt` and `TypeDuck-Windows:scripts/build.ps1`.
- Frontend setup/runtime environment: `TYPEDUCK_PROGRAM_DIR` is set by `TypeDuck-Windows:SetupHelper/SetupHelper.cpp` during registration so the TSF DLL can resolve the installed payload. `MOQI_PROGRAM_DIR` remains as a compatibility code identifier in `TypeDuck-Windows:MoqiTextService/TypeDuckProfile.cpp`.
- Backend test/runtime environment variables include `MOQI_RIME_PACKAGE_DIR`, `MOQI_RIME_INIT_MAX_MS`, and `MOQI_REAL_APPDATA` in `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`.
- Runtime layout contract: installed app root contains `TypeDuckRuntime/server.exe` and `TypeDuckRuntime/input_methods/rime/`.
- Preference schema is implemented in `TypeDuck-Windows:MoqLauncher/TypeDuckPreferences.cpp` and applied to Rime by `TypeDuck-Windows-backend:input_methods/rime/appearance_config.go`.
- Installer identity is defined in `TypeDuck-Windows:installer/MoqiTsf.iss`, `TypeDuck-Windows:MoqiTextService/TypeDuckProfile.cpp`, and `TypeDuck-Windows:SetupHelper/SetupHelper.cpp`.

## Build and Packaging

```powershell
# TypeDuck-Windows: build Win32 and x64 frontend binaries
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1

# TypeDuck-Windows-backend: build the runtime package
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -RimeDataSource <schema-source>

# TypeDuck-Windows: build backend runtime, frontend binaries, and installer
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/_all_in_package.ps1 -RimeDataSource <schema-source>
```

- `TypeDuck-Windows:scripts/build.ps1` applies TypeDuck submodule patches, then configures Win32 and x64 Visual Studio builds. Win32 builds launcher/setup/settings/probe targets; x64 builds the TSF DLL target needed by 64-bit host processes.
- `TypeDuck-Windows:scripts/Apply-TypeDuckSubmodulePatches.ps1` applies required TypeDuck patches to checked-out third-party submodules; `TypeDuck-Windows:CMakeLists.txt` fails configuration if the required `libIME2` diagnostics path patch is missing.
- `TypeDuck-Windows-backend:scripts/build.ps1` cross-compiles `server.exe` for Windows amd64, stamps version/icon resources, copies `input_methods/rime`, prunes source-only/runtime-excluded paths, copies Rime shared data, and writes `scripts/build/TypeDuckRuntime`.
- `TypeDuck-Windows:scripts/install.ps1` creates the installer staging tree, stamps TypeDuck icons into executables, copies frontend binaries, filters `TypeDuckRuntime`, and invokes `installer/build-installer.ps1`.
- `TypeDuck-Windows:scripts/_all_in_package.ps1` coordinates the backend runtime build, frontend CMake build, and installer stage/build sequence.

## CI

- `TypeDuck-Windows:.github/workflows/nightly.yml` and `TypeDuck-Windows:.github/workflows/release.yml` run on `windows-2022`, checkout `TypeDuck-Windows` and `TypeDuck-Windows-backend`, download and extract the TypeDuck schema release artifact, apply TypeDuck submodule patches, install Inno Setup, download protoc 33.5, and produce installer artifacts.
- `TypeDuck-Windows-backend:.github/workflows/nightly.yml` and `TypeDuck-Windows-backend:.github/workflows/release.yml` run on `windows-2022`, set up Go, run the backend build script, and upload backend runtime zip artifacts.
- Dedicated CI steps for `ctest`, `go test ./...`, linting, generated-protobuf diffing, and coverage are not detected.

## Platform Requirements

- Windows 8+ target in the frontend top-level build through `_WIN32_WINNT=0x0602`; libuv is compiled with its own Windows target level.
- Windows 10/11 is the practical validation target for installer, TSF profile registration, HKCU startup, and user-facing settings.
- PowerShell scripts should be invoked with `pwsh -NoProfile -ExecutionPolicy Bypass -File ...` for Unicode-safe Traditional Chinese/Cantonese evidence and paths.
- Full installer packaging requires a built backend runtime package and a schema data source supplied through `-RimeDataSource`.

## Generated Files

- Do not hand-edit `TypeDuck-Windows:proto/moqi.pb.h`, `TypeDuck-Windows:proto/moqi.pb.cc`, or `TypeDuck-Windows-backend:proto/moqi.pb.go`.
- Protocol changes start in `TypeDuck-Windows:proto/moqi.proto` and `TypeDuck-Windows-backend:proto/moqi.proto`, then regenerate both language bindings.
- Frontend CMake can generate C++ protobuf outputs into the build tree when `protoc` is available, or use checked-in generated files for local proof builds.

---

*Stack analysis: 2026-06-28*
