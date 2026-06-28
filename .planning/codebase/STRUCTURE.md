# Codebase Structure

**Analysis Date:** 2026-06-28

## Directory Layout

```text
TypeDuck-Windows/
├── CMakeLists.txt                 # C++ build graph, dependencies, generated protobuf, target inclusion
├── MoqiTextService/               # TSF COM DLL, TypeDuck profile, frontend RPC, candidate UI
├── MoqLauncher/                   # Launcher process, named pipe server, backend bridge, preferences
├── SetupHelper/                   # Elevated TSF DLL copy/register/unregister helper
├── TypeDuckSettings/              # Settings and About Win32 applications and resources
├── Preview/                       # Standalone candidate window preview target
├── Tools/TypeduckBackendProbe/    # Local backend/protocol probe tool
├── Tests/                         # First-party C++ and PowerShell verification targets
├── proto/                         # Shared protobuf schema, generated C++ bindings, C++ framing helper
├── scripts/                       # Build, staging, installer, runtime, and verification scripts
├── installer/                     # Inno Setup script and installer build support
├── patches/                       # TypeDuck-owned patches applied to third-party submodules
├── runtime/                       # Staged TypeDuck runtime assets for local proof/package work
├── third_party/                   # External source checkouts/submodules used by product scripts
├── libIME2/                       # Reusable TSF/COM framework and vendored GoogleTest
├── libuv/                         # Vendored libuv dependency for launcher process/pipe IO
├── jsoncpp/                       # Vendored JsonCpp dependency
└── .planning/codebase/            # Shared codebase map documents

TypeDuck-Windows-backend/
├── server.go                      # Backend process entry point and request dispatcher
├── protocol_io.go                 # Length-prefixed protobuf stdin/stdout frame IO
├── go.mod                         # Go module/dependency manifest
├── proto/                         # Shared protobuf schema and generated Go bindings
├── imecore/                       # Backend request/response model and text service interface
├── input_methods/rime/            # TypeDuck Rime service, librime binding, settings, data metadata
├── input_methods/fcitx5/          # Additional registered service implementation
├── input_methods/moqi/            # Additional registered service implementation
├── mobilebridge/                  # Mobile bridge package and tests
├── icons/                         # Runtime and input mode icons
├── scripts/                       # Backend build/deploy/test scripts
├── tools/                         # Go tooling package
└── docs/                          # Backend design/training notes
```

## Directory Purposes

**`TypeDuck-Windows/MoqiTextService`:**
- Purpose: Windows TSF product DLL and in-host UI/RPC layer.
- Contains: COM exports, TypeDuck profile constants, text service subclass, RPC client, candidate window, language bar button, candidate lookup formatter, resources.
- Key files: `TypeDuck-Windows/MoqiTextService/DllEntry.cpp`, `TypeDuck-Windows/MoqiTextService/TypeDuckProfile.cpp`, `TypeDuck-Windows/MoqiTextService/MoqiImeModule.cpp`, `TypeDuck-Windows/MoqiTextService/MoqiTextService.cpp`, `TypeDuck-Windows/MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows/MoqiTextService/MoqiCandidateWindow.cpp`, `TypeDuck-Windows/MoqiTextService/TypeDuckCandidateInfo.cpp`.

**`TypeDuck-Windows/MoqLauncher`:**
- Purpose: User-session process that mediates between all TSF DLL instances and one backend runtime process.
- Contains: Named pipe server/client, backend subprocess bridge, TypeDuck preference validation/storage, tray window, pipe security, JSON helpers.
- Key files: `TypeDuck-Windows/MoqLauncher/MoqiLauncher.cpp`, `TypeDuck-Windows/MoqLauncher/PipeServer.cpp`, `TypeDuck-Windows/MoqLauncher/PipeClient.cpp`, `TypeDuck-Windows/MoqLauncher/BackendServer.cpp`, `TypeDuck-Windows/MoqLauncher/TypeDuckPreferences.cpp`.

**`TypeDuck-Windows/libIME2/src`:**
- Purpose: Reusable TSF/COM base layer.
- Contains: `Ime::TextService`, `Ime::ImeModule`, COM helpers, edit sessions, windows, display attributes, language bar, debug logging.
- Key files: `TypeDuck-Windows/libIME2/src/TextService.cpp`, `TypeDuck-Windows/libIME2/src/ImeModule.cpp`, `TypeDuck-Windows/libIME2/src/ComObject.h`, `TypeDuck-Windows/libIME2/src/ComPtr.h`, `TypeDuck-Windows/libIME2/src/EditSession.cpp`.

**`TypeDuck-Windows/SetupHelper`:**
- Purpose: Elevated install-time and uninstall-time TSF registration helper.
- Contains: Wide-character Win32 helper code for admin relaunch, DLL copy fallback, reboot scheduling, `regsvr32.exe`, and bilingual result messages.
- Key files: `TypeDuck-Windows/SetupHelper/SetupHelper.cpp`, `TypeDuck-Windows/SetupHelper/CMakeLists.txt`, `TypeDuck-Windows/SetupHelper/SetupHelper.rc`.

**`TypeDuck-Windows/TypeDuckSettings`:**
- Purpose: Win32 settings and about UI shipped with the installer.
- Contains: Settings window, about dialog, resources, version resources, TypeDuck icons and bitmaps.
- Key files: `TypeDuck-Windows/TypeDuckSettings/main.cpp`, `TypeDuck-Windows/TypeDuckSettings/TypeDuckSettingsWindow.cpp`, `TypeDuck-Windows/TypeDuckSettings/TypeDuckAboutMain.cpp`, `TypeDuck-Windows/TypeDuckSettings/TypeDuckAboutDialog.cpp`, `TypeDuck-Windows/TypeDuckSettings/resources/Installer.bmp`.

**`TypeDuck-Windows/proto`:**
- Purpose: C++ side of the cross-process protocol.
- Contains: `moqi.proto`, checked-in generated C++ bindings, and `ProtoFraming.h` for bounded little-endian length-prefix framing.
- Key files: `TypeDuck-Windows/proto/moqi.proto`, `TypeDuck-Windows/proto/moqi.pb.h`, `TypeDuck-Windows/proto/moqi.pb.cc`, `TypeDuck-Windows/proto/ProtoFraming.h`.

**`TypeDuck-Windows/scripts`:**
- Purpose: Build, installer staging, runtime staging, VM/install proof, protocol proof, and release verification automation.
- Contains: PowerShell scripts; run with `pwsh` for Unicode-safe execution.
- Key files: `TypeDuck-Windows/scripts/build.ps1`, `TypeDuck-Windows/scripts/Apply-TypeDuckSubmodulePatches.ps1`, `TypeDuck-Windows/scripts/install.ps1`, `TypeDuck-Windows/scripts/Stage-TypeDuckRuntime.ps1`, `TypeDuck-Windows/scripts/Invoke-TypeDuckReleaseVerification.ps1`, `TypeDuck-Windows/scripts/Test-TypeDuckProtocolContract.ps1`.

**`TypeDuck-Windows/patches`:**
- Purpose: Parent-owned patch files for third-party submodules that are not forked as TypeDuck repositories.
- Contains: Submodule-specific patch queues.
- Key files: `TypeDuck-Windows:patches/libIME2/0001-use-typeduck-diagnostics-paths.patch`.
- Notes: Apply through `TypeDuck-Windows:scripts/Apply-TypeDuckSubmodulePatches.ps1`; frontend CMake checks that the required `libIME2` patch is present before adding the submodule.

**`TypeDuck-Windows/installer`:**
- Purpose: Inno Setup packaging and install/uninstall orchestration.
- Contains: Graphical installer script, app metadata, messages, file copy, registry startup, helper invocation, process cleanup.
- Key files: `TypeDuck-Windows/installer/MoqiTsf.iss`, `TypeDuck-Windows/installer/build-installer.ps1`, `TypeDuck-Windows/installer/README.txt`.

**`TypeDuck-Windows/Tests`:**
- Purpose: First-party validation targets for candidate data, settings, and protocol behavior.
- Contains: C++ GoogleTest targets and PowerShell test scripts referenced by release verification.
- Key files: `TypeDuck-Windows/Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp`, `TypeDuck-Windows/Tests/TypeDuckSettings/TypeDuckPreferences_test.cpp`, `TypeDuck-Windows/Tests/TypeDuckProtocol/ProtoFraming_test.cpp`, `TypeDuck-Windows/Tests/TypeDuckProtocol/ProtocolRecovery_test.cpp`.

**`TypeDuck-Windows-backend`:**
- Purpose: Go backend executable root.
- Contains: Main server loop, frame IO, integration tests, Go module manifest.
- Key files: `TypeDuck-Windows-backend/server.go`, `TypeDuck-Windows-backend/protocol_io.go`, `TypeDuck-Windows-backend/server_test.go`, `TypeDuck-Windows-backend/server_integration_test.go`, `TypeDuck-Windows-backend/go.mod`.

**`TypeDuck-Windows-backend/imecore`:**
- Purpose: Backend-neutral request/response and service interface layer.
- Contains: `TextService` interface, base service, protobuf conversion, tray helpers.
- Key files: `TypeDuck-Windows-backend/imecore/client.go`, `TypeDuck-Windows-backend/imecore/service.go`, `TypeDuck-Windows-backend/imecore/protocol.go`, `TypeDuck-Windows-backend/imecore/tray.go`.

**`TypeDuck-Windows-backend/input_methods/rime`:**
- Purpose: TypeDuck Rime backend implementation.
- Contains: Rime service object, native librime binding, key mapping, settings/config update, appearance themes, candidate overlays, cloud clipboard code, Android support, runtime metadata, icons, templates, tests.
- Key files: `TypeDuck-Windows-backend/input_methods/rime/rime.go`, `TypeDuck-Windows-backend/input_methods/rime/librime.go`, `TypeDuck-Windows-backend/input_methods/rime/native_cgo.go`, `TypeDuck-Windows-backend/input_methods/rime/rime_keyevent.go`, `TypeDuck-Windows-backend/input_methods/rime/appearance_config.go`, `TypeDuck-Windows-backend/input_methods/rime/ime.json`.

**`TypeDuck-Windows-backend/proto`:**
- Purpose: Go side of the cross-process protocol.
- Contains: `moqi.proto` and generated Go bindings.
- Key files: `TypeDuck-Windows-backend/proto/moqi.proto`, `TypeDuck-Windows-backend/proto/moqi.pb.go`.

**`TypeDuck-Windows-backend/scripts`:**
- Purpose: Build and deploy the backend runtime package.
- Contains: Go build packaging, Rime data copy, test scripts, deploy scripts, build output under `scripts/build`.
- Key files: `TypeDuck-Windows-backend/scripts/build.ps1`, `TypeDuck-Windows-backend/scripts/deploy-server.ps1`, `TypeDuck-Windows-backend/scripts/Test-TypeDuckCandidateParity.ps1`.

## Key File Locations

**Entry Points:**
- `TypeDuck-Windows/MoqiTextService/DllEntry.cpp`: TSF DLL exports and TSF registration.
- `TypeDuck-Windows/MoqLauncher/MoqiLauncher.cpp`: Launcher `WinMain`.
- `TypeDuck-Windows/SetupHelper/SetupHelper.cpp`: Setup helper `wWinMain`.
- `TypeDuck-Windows/TypeDuckSettings/main.cpp`: Settings app `wWinMain`.
- `TypeDuck-Windows/TypeDuckSettings/TypeDuckAboutMain.cpp`: About app entry point.
- `TypeDuck-Windows-backend/server.go`: Backend `main`.

**Configuration:**
- `TypeDuck-Windows/CMakeLists.txt`: C++ build targets, dependencies, generated C++ proto.
- `TypeDuck-Windows/version.txt`: Product version source for resource generation.
- `TypeDuck-Windows/installer/MoqiTsf.iss`: Installer metadata, messages, file copy, registry, run/uninstall logic.
- `TypeDuck-Windows/MoqiTextService/TypeDuckProfile.cpp`: CLSID, profile GUID, zh-HK locale, installed names.
- `TypeDuck-Windows/MoqLauncher/TypeDuckPreferences.cpp`: User preference schema and validation.
- `TypeDuck-Windows-backend/go.mod`: Go module and dependencies.
- `TypeDuck-Windows-backend/input_methods/rime/ime.json`: TypeDuck Rime profile metadata.
- `TypeDuck-Windows-backend/input_methods/rime/appearance_themes.json`: Backend appearance theme data.

**Core Logic:**
- `TypeDuck-Windows/libIME2/src/TextService.cpp`: Generic TSF text service state and edit sessions.
- `TypeDuck-Windows/MoqiTextService/MoqiTextService.cpp`: Product TSF event bridge.
- `TypeDuck-Windows/MoqiTextService/MoqiClient.cpp`: Frontend RPC client and response application.
- `TypeDuck-Windows/MoqiTextService/MoqiCandidateWindow.cpp`: Candidate/dictionary panel rendering.
- `TypeDuck-Windows/MoqLauncher/PipeServer.cpp`: Launcher singleton, pipe server, tray, backend mapping.
- `TypeDuck-Windows/MoqLauncher/PipeClient.cpp`: Named pipe client request routing.
- `TypeDuck-Windows/MoqLauncher/BackendServer.cpp`: Backend process stdin/stdout bridge.
- `TypeDuck-Windows-backend/server.go`: Backend dispatcher and client session registry.
- `TypeDuck-Windows-backend/imecore/protocol.go`: Protobuf conversion contract.
- `TypeDuck-Windows-backend/input_methods/rime/rime.go`: Rime request handling and response assembly.
- `TypeDuck-Windows-backend/input_methods/rime/librime.go`: Native Rime DLL binding.

**Protocol:**
- `TypeDuck-Windows/proto/moqi.proto`: Frontend C++ schema source.
- `TypeDuck-Windows-backend/proto/moqi.proto`: Backend Go schema source.
- `TypeDuck-Windows/proto/ProtoFraming.h`: C++ bounded frame helper.
- `TypeDuck-Windows-backend/protocol_io.go`: Go frame helper.

**Installer and Runtime Packaging:**
- `TypeDuck-Windows/scripts/build.ps1`: Frontend build orchestration.
- `TypeDuck-Windows/scripts/install.ps1`: Installer stage tree creation and runtime copy.
- `TypeDuck-Windows/scripts/Stage-TypeDuckRuntime.ps1`: Runtime asset staging for librime/schema proof.
- `TypeDuck-Windows/installer/MoqiTsf.iss`: Inno installer.
- `TypeDuck-Windows-backend/scripts/build.ps1`: Backend runtime package creation.

**Testing:**
- `TypeDuck-Windows/Tests/TypeDuckProtocol/ProtoFraming_test.cpp`: C++ frame tests.
- `TypeDuck-Windows/Tests/TypeDuckProtocol/ProtocolRecovery_test.cpp`: Protocol recovery tests.
- `TypeDuck-Windows/Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp`: Candidate lookup parser tests.
- `TypeDuck-Windows/Tests/TypeDuckSettings/TypeDuckPreferences_test.cpp`: Preference validation tests.
- `TypeDuck-Windows-backend/server_test.go`: Backend server unit tests.
- `TypeDuck-Windows-backend/server_integration_test.go`: Backend integration tests.
- `TypeDuck-Windows-backend/imecore/protocol_test.go`: Backend protobuf conversion tests.
- `TypeDuck-Windows-backend/input_methods/rime/rime_test.go`: Rime service tests.

## Naming Conventions

**Files:**
- First-party C++ modules use PascalCase file stems in current code identifiers: `TypeDuck-Windows/MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows/MoqLauncher/PipeServer.cpp`, `TypeDuck-Windows/SetupHelper/SetupHelper.cpp`.
- TypeDuck-specific C++ additions use `TypeDuck*` file stems: `TypeDuck-Windows/MoqiTextService/TypeDuckProfile.cpp`, `TypeDuck-Windows/MoqLauncher/TypeDuckPreferences.cpp`, `TypeDuck-Windows/TypeDuckSettings/TypeDuckSettingsWindow.cpp`.
- Backend Go files use lowercase snake_case where descriptive: `TypeDuck-Windows-backend/protocol_io.go`, `TypeDuck-Windows-backend/input_methods/rime/rime_keyevent.go`, `TypeDuck-Windows-backend/input_methods/rime/appearance_config.go`.
- Test files use `_test.cpp` in frontend test targets and `_test.go` in backend packages.
- Generated protobuf files are `moqi.pb.*`; do not hand-edit `TypeDuck-Windows/proto/moqi.pb.h`, `TypeDuck-Windows/proto/moqi.pb.cc`, or `TypeDuck-Windows-backend/proto/moqi.pb.go`.

**Directories:**
- Frontend executable/library targets are top-level PascalCase directories: `TypeDuck-Windows/MoqiTextService`, `TypeDuck-Windows/MoqLauncher`, `TypeDuck-Windows/SetupHelper`, `TypeDuck-Windows/TypeDuckSettings`.
- Backend input methods live under `TypeDuck-Windows-backend/input_methods/<method>`.
- Backend package directories use lowercase package names: `TypeDuck-Windows-backend/imecore`, `TypeDuck-Windows-backend/mobilebridge`, `TypeDuck-Windows-backend/tools`.
- Runtime package layout must keep `server.exe` and `input_methods/` in the same `TypeDuckRuntime` directory.

**Code Identifiers:**
- C++ class and namespace identifiers currently include `Moqi::` for product frontend/launcher code and `Ime::` for reusable TSF code.
- C++ TypeDuck product constants live under `Moqi::TypeDuck` in `TypeDuck-Windows/MoqiTextService/TypeDuckProfile.cpp`.
- Protobuf package is `moqi.protocol` in both repositories.
- Go module imports currently use an older external module identity; treat those import paths as current code identifiers until the module is renamed.

## Where to Add New Code

**New TSF behavior:**
- Primary code: `TypeDuck-Windows/MoqiTextService/MoqiTextService.cpp` and `TypeDuck-Windows/MoqiTextService/MoqiClient.cpp`
- Shared TSF framework changes: `TypeDuck-Windows/libIME2/src`
- Tests: `TypeDuck-Windows/Tests/TypeDuckProtocol` or targeted PowerShell scripts under `TypeDuck-Windows/scripts`

**New candidate or dictionary UI behavior:**
- Parser/formatting: `TypeDuck-Windows/MoqiTextService/TypeDuckCandidateInfo.cpp`
- Rendering and interaction: `TypeDuck-Windows/MoqiTextService/MoqiCandidateWindow.cpp`
- Tests: `TypeDuck-Windows/Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp`

**New launcher IPC or backend process behavior:**
- Named pipe routing: `TypeDuck-Windows/MoqLauncher/PipeClient.cpp`
- Backend process lifecycle: `TypeDuck-Windows/MoqLauncher/BackendServer.cpp`
- Launcher singleton/tray: `TypeDuck-Windows/MoqLauncher/PipeServer.cpp`
- Protocol tests: `TypeDuck-Windows/Tests/TypeDuckProtocol`

**New settings preference:**
- Frontend preference model: `TypeDuck-Windows/MoqLauncher/TypeDuckPreferences.cpp`
- Settings UI: `TypeDuck-Windows/TypeDuckSettings/TypeDuckSettingsWindow.cpp`
- Protocol fields if needed: `TypeDuck-Windows/proto/moqi.proto` and `TypeDuck-Windows-backend/proto/moqi.proto`
- Backend application to Rime: `TypeDuck-Windows-backend/input_methods/rime/appearance_config.go` and `TypeDuck-Windows-backend/input_methods/rime/rime.go`
- Tests: `TypeDuck-Windows/Tests/TypeDuckSettings/TypeDuckPreferences_test.cpp` and backend `_test.go` files near the changed package.

**New protocol capability:**
- Schema: `TypeDuck-Windows/proto/moqi.proto` and `TypeDuck-Windows-backend/proto/moqi.proto`
- C++ framing/application: `TypeDuck-Windows/proto/ProtoFraming.h`, `TypeDuck-Windows/MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows/MoqLauncher/PipeClient.cpp`, `TypeDuck-Windows/MoqLauncher/BackendServer.cpp`
- Go conversion: `TypeDuck-Windows-backend/imecore/protocol.go`
- Tests: `TypeDuck-Windows/Tests/TypeDuckProtocol` and `TypeDuck-Windows-backend/imecore/protocol_test.go`

**New backend engine behavior:**
- Backend service dispatch: `TypeDuck-Windows-backend/server.go`
- Request/response model: `TypeDuck-Windows-backend/imecore/protocol.go`
- Rime session behavior: `TypeDuck-Windows-backend/input_methods/rime/rime.go`
- Rime native API usage: `TypeDuck-Windows-backend/input_methods/rime/librime.go` and `TypeDuck-Windows-backend/input_methods/rime/native_cgo.go`
- Tests: colocated `*_test.go` files under `TypeDuck-Windows-backend/input_methods/rime` or `TypeDuck-Windows-backend/imecore`

**New installer behavior:**
- Inno wizard/install/uninstall logic: `TypeDuck-Windows/installer/MoqiTsf.iss`
- Elevated copy/register behavior: `TypeDuck-Windows/SetupHelper/SetupHelper.cpp`
- Stage tree changes: `TypeDuck-Windows/scripts/install.ps1`
- Verification scripts: `TypeDuck-Windows/scripts/Test-TypeDuckInstallerSkeleton.ps1`, `TypeDuck-Windows/scripts/Invoke-TypeDuckReleaseVerification.ps1`

**New runtime package content:**
- Backend package builder: `TypeDuck-Windows-backend/scripts/build.ps1`
- Frontend installer staging copy/filtering: `TypeDuck-Windows/scripts/install.ps1`
- Librime/schema staging proof: `TypeDuck-Windows/scripts/Stage-TypeDuckRuntime.ps1`

**Utilities:**
- Frontend shared Win32 helpers: keep near the owning target unless reused by multiple targets.
- Launcher shared helpers: `TypeDuck-Windows/MoqLauncher/Utils.cpp` and `TypeDuck-Windows/MoqLauncher/Utils.h`.
- Backend shared request/response helpers: `TypeDuck-Windows-backend/imecore`.

## Special Directories

**`TypeDuck-Windows/build-vs32`, `TypeDuck-Windows/build-vs64`, `TypeDuck-Windows/build-vs32-settings`, `TypeDuck-Windows/build-vs32-settings-ui`:**
- Purpose: Local CMake/MSBuild output directories.
- Generated: Yes.
- Committed: No.

**`TypeDuck-Windows/runtime`:**
- Purpose: Staged runtime evidence/assets used by runtime proof and packaging workflows.
- Generated: Yes.
- Committed: Repository-specific; inspect git status before editing.

**`TypeDuck-Windows/.planning`:**
- Purpose: Product planning, product fixtures, codebase maps, and implementation artifacts.
- Generated: Yes.
- Committed: Project reference artifacts.

**`TypeDuck-Windows/libIME2`, `TypeDuck-Windows/libuv`, `TypeDuck-Windows/jsoncpp`:**
- Purpose: Vendored or submodule dependencies used by the frontend build.
- Generated: No.
- Committed: Yes.

**`TypeDuck-Windows-backend/scripts/build`:**
- Purpose: Backend build output and `TypeDuckRuntime` package directory.
- Generated: Yes.
- Committed: No for normal build outputs.

**`TypeDuck-Windows-backend/input_methods/rime/android`:**
- Purpose: Android native runtime assets for backend package variants.
- Generated: No for checked-in assets.
- Committed: Yes when present.

**`TypeDuck-Windows-backend/input_methods/rime/test`:**
- Purpose: Standalone Rime experiment/probe code.
- Generated: No.
- Committed: Yes.

---

*Structure analysis: 2026-06-28*
