# Coding Conventions

**Analysis Date:** 2026-06-28

**Repositories:**
- `TypeDuck-Windows` means public repo `https://github.com/TypeDuck-HK/TypeDuck-Windows`.
- `TypeDuck-Windows-backend` means public repo `https://github.com/TypeDuck-HK/TypeDuck-Windows-backend`.

## Naming Patterns

**Files:**
- Use PascalCase for first-party Windows C++ modules in `TypeDuck-Windows`: `MoqiTextService/TypeDuckCandidateInfo.cpp`, `MoqLauncher/TypeDuckPreferences.cpp`, `TypeDuckSettings/TypeDuckSettingsWindow.cpp`, `SetupHelper/SetupHelper.cpp`.
- Keep Windows C++ implementation/header pairs side by side in the owning directory: `TypeDuck-Windows:MoqiTextService/TypeDuckCandidateInfo.cpp` with `TypeDuck-Windows:MoqiTextService/TypeDuckCandidateInfo.h`, and `TypeDuck-Windows:MoqLauncher/TypeDuckPreferences.cpp` with `TypeDuck-Windows:MoqLauncher/TypeDuckPreferences.h`.
- Current Windows build targets and product modules retain `Moqi` identifiers where they are code identifiers: `TypeDuck-Windows:CMakeLists.txt` defines project `MoqiIMWindows`; `TypeDuck-Windows:MoqiTextService/CMakeLists.txt`, `TypeDuck-Windows:MoqLauncher/CMakeLists.txt`, and namespace `Moqi::TypeDuck` are current naming surfaces.
- Use lowercase for protocol schema files and generated outputs: `TypeDuck-Windows:proto/moqi.proto`, `TypeDuck-Windows:proto/moqi.pb.h`, `TypeDuck-Windows:proto/moqi.pb.cc`, `TypeDuck-Windows-backend:proto/moqi.proto`, `TypeDuck-Windows-backend:proto/moqi.pb.go`.
- Use `Test-TypeDuck*.ps1` for guard scripts and `Invoke-TypeDuck*.ps1` for evidence-producing scripts in `TypeDuck-Windows:scripts/` and `TypeDuck-Windows-backend:scripts/`.
- Use Go package directories by domain in `TypeDuck-Windows-backend`: root server files in `server.go` and `protocol_io.go`, protocol abstractions in `imecore/`, Rime implementation in `input_methods/rime/`, mobile bridge in `mobilebridge/`, and protocol generated code in `proto/`.
- Use Go `_test.go` suffixes next to the package under test: `TypeDuck-Windows-backend:server_test.go`, `TypeDuck-Windows-backend:imecore/protocol_test.go`, `TypeDuck-Windows-backend:input_methods/rime/rime_test.go`, `TypeDuck-Windows-backend:mobilebridge/bridge_test.go`.

**Functions:**
- Use lower camelCase for first-party Windows C++ functions and methods: `formatJyutping()` in `TypeDuck-Windows:MoqiTextService/TypeDuckCandidateInfo.cpp`, `preferenceAffectsRime()` in `TypeDuck-Windows:MoqLauncher/TypeDuckPreferences.cpp`, and `transactLauncher()` in `TypeDuck-Windows:TypeDuckSettings/TypeDuckSettingsWindow.cpp`.
- Keep Win32, COM, and TSF entry points in their required names/signatures: `DllRegisterServer()` in `TypeDuck-Windows:MoqiTextService/DllEntry.cpp`, `wWinMain()` in `TypeDuck-Windows:SetupHelper/SetupHelper.cpp`, and Inno event handlers in `TypeDuck-Windows:installer/MoqiTsf.iss`.
- Use approved or action-oriented PowerShell function names: `Invoke-Step` and `Resolve-ProtobufRoot` in `TypeDuck-Windows:scripts/build.ps1`, `Write-Step` and `Invoke-External` in `TypeDuck-Windows-backend:scripts/build.ps1`, and `Assert-File` / `Assert-Contains` in `TypeDuck-Windows-backend:scripts/Test-TypeDuckCandidateParity.ps1`.
- Use Go exported PascalCase only for API surfaces consumed by other packages: `NewServer()` in `TypeDuck-Windows-backend:server.go`, `BuildProtoResponse()` in `TypeDuck-Windows-backend:imecore/protocol.go`, and `NewSession()` in `TypeDuck-Windows-backend:mobilebridge/bridge.go`.
- Use Go lower camelCase for package-private helpers and state: `decodeClientRequest()` in `TypeDuck-Windows-backend:protocol_io.go`, `registerTypeDuckRimeService()` in `TypeDuck-Windows-backend:server.go`, and `realRimeTestDirs()` in `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`.

**Variables:**
- Use trailing underscores for Windows C++ class members in existing product and TSF code: `pipe_`, `rpcInProgress_`, and `pendingAsyncResponses_` in `TypeDuck-Windows:MoqiTextService/MoqiClient.h`.
- Use lower camelCase for local C++ variables and PowerShell script variables: `serializedReply` in `TypeDuck-Windows:MoqiTextService/MoqiClient.cpp`, `$commonConfigureArgs` in `TypeDuck-Windows:scripts/build.ps1`, and `$PackageRimeDataDir` in `TypeDuck-Windows-backend:scripts/build.ps1`.
- Use Go mixedCaps for variables and struct fields: `clientID`, `reqMsg`, `typeDuckProfileGUID` in `TypeDuck-Windows-backend:server.go`, and `CandidateEntries`, `TypeDuckCandidatePage` in `TypeDuck-Windows-backend:imecore/protocol.go`.
- Environment/config keys are current protocol/build identifiers and remain uppercase where present: `MOQI_PROTOBUF_ROOT`, `MOQI_PROTOC_EXECUTABLE`, and `TYPEDUCK_PROGRAM_DIR` in `TypeDuck-Windows`; `MOQI_RIME_PACKAGE_DIR`, `MOQI_REAL_APPDATA`, and `MOQI_RIME_INIT_MAX_MS` in `TypeDuck-Windows-backend` tests.

**Types:**
- Use PascalCase for C++ classes/structs/enums: `CandidateInfo`, `CandidateEntry`, `DisplayPreferences`, `ChineseTypeface`, and `JyutpingVisibility` in `TypeDuck-Windows:MoqiTextService/TypeDuckCandidateInfo.h`.
- Use PascalCase for Go exported types and lower camelCase for package-private types: `Server`, `Client`, `ServiceFactory`, and `TrayNotification` in `TypeDuck-Windows-backend:server.go` and `TypeDuck-Windows-backend:imecore/protocol.go`; `testBackend` and `candidateItem` in `TypeDuck-Windows-backend:input_methods/rime/rime_test.go`.
- Keep protobuf generated types under the generated package/namespace from `proto/moqi.proto`: C++ uses `moqi::protocol` in `TypeDuck-Windows:proto/moqi.pb.h`; Go imports generated types as `moqipb` from `TypeDuck-Windows-backend:proto/moqi.pb.go`.

## Code Style

**Formatting:**
- Formatting tool: Go formatting is implicit through `gofmt`; no `.clang-format`, `.editorconfig`, `.clang-tidy`, `.golangci.yml`, or global formatter config is detected in either repo.
- For new first-party Windows C++ code in `TypeDuck-Windows`, use the 2-space style visible in `MoqiTextService/TypeDuckCandidateInfo.cpp`, `MoqLauncher/TypeDuckPreferences.cpp`, `SetupHelper/SetupHelper.cpp`, and `proto/ProtoFraming.h`.
- Do not reformat vendored or broad framework code during focused changes: `TypeDuck-Windows:libIME2/src/`, `TypeDuck-Windows:libuv/`, `TypeDuck-Windows:jsoncpp/`, and `TypeDuck-Windows:libIME2/lib/googletest-release-1.10.0/`.
- For Go in `TypeDuck-Windows-backend`, run `gofmt` on edited `.go` files. The current style uses tabs, grouped standard-library imports before first-party imports, and `t.Fatalf` assertions in tests.
- For PowerShell in both repos, use `param(...)` blocks, `$ErrorActionPreference = "Stop"`, explicit helper functions, `Test-Path -LiteralPath`, and `throw` for failed guard conditions as shown in `TypeDuck-Windows:scripts/Test-TypeDuckRuntimeContract.ps1` and `TypeDuck-Windows-backend:scripts/build.ps1`.
- For Inno Setup Pascal Script in `TypeDuck-Windows:installer/MoqiTsf.iss`, keep constants in PascalCase or product-prefixed names, keep user-facing text bilingual, and use helper functions such as `Bilingual()` for repeated Chinese/English message pairs.

**Linting:**
- Tool used: Not detected for Windows C++, PowerShell, Inno Setup, or backend Go.
- Use compilation and tests as the current enforcement layer: MSVC/CMake for `TypeDuck-Windows:CMakeLists.txt`, GoogleTest targets in `TypeDuck-Windows:Tests/`, `go test` for `TypeDuck-Windows-backend`, and PowerShell guard scripts in both repos.
- Use manual review for COM lifetime, Win32 handle cleanup, HRESULT branches, protobuf schema compatibility, PowerShell destructive path checks, and Rime runtime path assumptions.

**Build conventions:**
- Use `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1` for local Windows repo builds in `TypeDuck-Windows`; the script configures Win32 and x64 CMake builds and uses `MOQI_PROTOBUF_ROOT`, `MOQI_PROTOBUF_SOURCE_DIR`, and `MOQI_PROTOC_EXECUTABLE`.
- Use `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/_all_in_package.ps1` from `TypeDuck-Windows` for full installer packaging when a backend runtime and Rime data source are available.
- Use `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -RimeDataSource <schema-source>` from `TypeDuck-Windows-backend` for runtime package builds. The backend build sets `GOOS=windows`, `GOARCH=amd64`, and `CGO_ENABLED=0` while producing `server.exe` and `input_methods/rime/`.
- Do not rely on implicit local absolute paths in plans or docs. Pass explicit repo-relative or CI workspace arguments such as `-RimeDataSource`, `-MoqiImeRoot`, and `-ProtobufRoot`.

## Import Organization

**Order:**
1. Windows C++ includes local project headers first when the file owns the implementation: `#include "TypeDuckCandidateInfo.h"` in `TypeDuck-Windows:MoqiTextService/TypeDuckCandidateInfo.cpp`.
2. Add C++ standard library headers next: `<algorithm>`, `<array>`, `<cwctype>`, `<set>`, `<utility>` in `TypeDuck-Windows:MoqiTextService/TypeDuckCandidateInfo.cpp`.
3. Add third-party headers after standard headers: `<json/json.h>` in `TypeDuck-Windows:MoqLauncher/TypeDuckPreferences.cpp`, and generated protobuf headers such as `proto/moqi.pb.h` where protocol types are required.
4. Go imports standard library packages first, then a blank line, then first-party module imports, then third-party imports where present: `TypeDuck-Windows-backend:server.go`, `TypeDuck-Windows-backend:imecore/protocol.go`, and `TypeDuck-Windows-backend:input_methods/rime/scheme_set_download_test.go`.
5. PowerShell scripts declare parameters and preferences before helper functions and executable body: `TypeDuck-Windows:scripts/build.ps1`, `TypeDuck-Windows-backend:scripts/build.ps1`.

**Path Aliases:**
- C++ include roots come from CMake include directories, especially `${PROJECT_SOURCE_DIR}` and `${MOQI_GENERATED_PROTO_ROOT}` in `TypeDuck-Windows:Tests/TypeDuckProtocol/CMakeLists.txt`, `TypeDuck-Windows:MoqLauncher/CMakeLists.txt`, and `TypeDuck-Windows:MoqiTextService/CMakeLists.txt`.
- The Go module path in `TypeDuck-Windows-backend:go.mod` currently uses an older external module identity; imports under that module are current code identifiers until the module is renamed.
- PowerShell scripts resolve repo roots with `$PSScriptRoot` and `Join-Path`; keep file references relative to the resolved repo root as in `TypeDuck-Windows:scripts/Test-TypeDuckRuntimeContract.ps1` and `TypeDuck-Windows-backend:scripts/Test-TypeDuckCandidateParity.ps1`.

## Error Handling

**Patterns:**
- Windows TSF/COM paths return `HRESULT` and use `FAILED(...)`, `SUCCEEDED(...)`, `S_OK`, and `E_*` semantics in `TypeDuck-Windows:libIME2/src/`, `TypeDuck-Windows:MoqiTextService/DllEntry.cpp`, and `TypeDuck-Windows:MoqiTextService/MoqiTextService.cpp`.
- Windows setup and preview helpers use bool/int return values plus bounded user-facing messages: `FormatWindowsErrorMessage()` and `ShowFailureAndReturn()` in `TypeDuck-Windows:SetupHelper/SetupHelper.cpp`.
- Windows protocol framing reports bounded parser state instead of throwing: `FrameBuffer::lastError()`, `FrameError::PayloadTooLarge`, and `hasViolation()` in `TypeDuck-Windows:proto/ProtoFraming.h`.
- Windows probe tooling can throw `std::runtime_error` for command-line and process-boundary failures because it is an executable harness: `TypeDuck-Windows:Tools/TypeduckBackendProbe/main.cpp`.
- PowerShell guards fail with `throw` or `Write-Error` and a nonzero exit code; successful guards print a compact pass line, as in `TypeDuck-Windows:scripts/Test-TypeDuckProtocolContract.ps1` and `TypeDuck-Windows-backend:scripts/Test-TypeDuckCandidateParity.ps1`.
- Go functions return `error` with wrapping where context matters: `fmt.Errorf("read error: %w", err)` in `TypeDuck-Windows-backend:server.go`, `fmt.Errorf("invalid empty frame")` in `TypeDuck-Windows-backend:protocol_io.go`, and `fmt.Errorf("unsupported return data type %T", value)` in `TypeDuck-Windows-backend:imecore/protocol.go`.
- Go request handlers convert recoverable runtime failures into protocol responses instead of panics: `handleRequest()` and `sendResponse()` in `TypeDuck-Windows-backend:server.go`, `errorResponse()` in `TypeDuck-Windows-backend:mobilebridge/bridge.go`.
- Go tests use `t.Helper()` in helper functions and `t.Fatalf` / `t.Fatal` for setup and assertion failures: `TypeDuck-Windows-backend:server_test.go`, `TypeDuck-Windows-backend:input_methods/rime/rime_test.go`, and `TypeDuck-Windows-backend:input_methods/rime/scheme_set_download_test.go`.

## Logging

**Framework:** `console`, Windows debug output, `spdlog`, and Go standard `log`.

**Patterns:**
- Windows TSF debug logging writes through `OutputDebugStringW()` and file logging helpers in `TypeDuck-Windows:MoqiTextService/TsfLog.cpp`, with gates in `TypeDuck-Windows:libIME2/src/DebugLogConfig.cpp`.
- Windows launcher logging uses `spdlog::rotating_logger_mt` in `TypeDuck-Windows:MoqLauncher/PipeServer.cpp`; keep high-volume typed content out of routine logs.
- Backend server logging uses Go `log` in `TypeDuck-Windows-backend:server.go`; it logs request/session metadata and deliberately keeps routine response payloads out of logs in `logResponseSummary()`.
- Backend Rime slow-path diagnostics use `debugLogf()` and `log.Printf()` around Rime initialization, redeploy, and slow operations in `TypeDuck-Windows-backend:input_methods/rime/native_cgo.go`, `TypeDuck-Windows-backend:input_methods/rime/librime.go`, and `TypeDuck-Windows-backend:mobilebridge/bridge.go`.
- Backend file logs live under the TypeDuck log directory verified by `TypeDuck-Windows-backend:server_test.go` and `TypeDuck-Windows-backend:input_methods/rime/rime_test.go`.
- PowerShell scripts use `Write-Host` for progress and pass summaries, `Write-Warning` for nonfatal build conditions, and `throw`/`Write-Error` for failures: `TypeDuck-Windows:scripts/build.ps1`, `TypeDuck-Windows-backend:scripts/build.ps1`.

## Comments

**When to Comment:**
- Comment Windows API, TSF, COM, installer, and protocol constraints that are not obvious from the call itself: `TypeDuck-Windows:CMakeLists.txt`, `TypeDuck-Windows:installer/MoqiTsf.iss`, `TypeDuck-Windows:SetupHelper/SetupHelper.cpp`, and `TypeDuck-Windows:proto/ProtoFraming.h`.
- Comment Go build tags and platform boundaries: `TypeDuck-Windows-backend:input_methods/rime/librime.go`, `TypeDuck-Windows-backend:input_methods/rime/native_cgo.go`, `TypeDuck-Windows-backend:input_methods/rime/native_stub.go`, and `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`.
- Do not add comments that restate simple code. Use comments for protocol compatibility, Rime module requirements, user-data boundaries, and test fixture assumptions.

**JSDoc/TSDoc:**
- Not applicable. This product codebase uses C++, Go, PowerShell, Inno Setup, and protobuf.

## Function Design

**Size:** Keep new code focused around one boundary: TSF/window behavior in `TypeDuck-Windows:MoqiTextService/`, launcher/runtime orchestration in `TypeDuck-Windows:MoqLauncher/`, setup behavior in `TypeDuck-Windows:SetupHelper/`, backend request handling in `TypeDuck-Windows-backend:server.go`, and Rime behavior in `TypeDuck-Windows-backend:input_methods/rime/`.

**Parameters:** Prefer explicit value objects at boundaries. Use protobuf messages for frontend/backend IPC in `TypeDuck-Windows:proto/moqi.proto` and `TypeDuck-Windows-backend:proto/moqi.proto`; use `Preferences` / `RimeSideEffects` in `TypeDuck-Windows:MoqLauncher/TypeDuckPreferences.h`; use `imecore.Request` / `imecore.Response` in `TypeDuck-Windows-backend:imecore/protocol.go`.

**Return Values:** Return structured results for recoverable validation and apply operations: `ValidationResult` and `ApplyResult` in `TypeDuck-Windows:MoqLauncher/TypeDuckPreferences.cpp`, `*imecore.Response` in `TypeDuck-Windows-backend:server.go`, and `(value, error)` for Go helpers that can fail.

## Module Design

**Exports:** 
- Windows TSF exports stay in `TypeDuck-Windows:MoqiTextService/DllEntry.cpp` and `TypeDuck-Windows:MoqiTextService/MoqiTextService.def`.
- Reusable TSF mechanics stay in `TypeDuck-Windows:libIME2/src/`; product behavior should live in `TypeDuck-Windows:MoqiTextService/`, `TypeDuck-Windows:MoqLauncher/`, `TypeDuckSettings/`, `SetupHelper/`, or focused `Tests/` subtrees.
- Backend protocol mapping stays in `TypeDuck-Windows-backend:imecore/protocol.go`; backend process framing stays in `TypeDuck-Windows-backend:protocol_io.go`; Rime-specific behavior stays in `TypeDuck-Windows-backend:input_methods/rime/`.

**Barrel Files:** Not detected. Include/import concrete module files or packages directly.

**Generated File Boundaries:**
- Edit `TypeDuck-Windows:proto/moqi.proto` and `TypeDuck-Windows-backend:proto/moqi.proto` for protocol changes. Do not hand-edit `TypeDuck-Windows:proto/moqi.pb.h`, `TypeDuck-Windows:proto/moqi.pb.cc`, or `TypeDuck-Windows-backend:proto/moqi.pb.go`.
- `TypeDuck-Windows:CMakeLists.txt` generates C++ protobuf outputs into the build tree when `protoc` is available and falls back to checked-in `proto/moqi.pb.*` for local proof builds.
- `TypeDuck-Windows-backend:proto/moqi.pb.go` records `protoc-gen-go v1.36.11` and `protoc v6.33.5`; regenerate it from `TypeDuck-Windows-backend:proto/moqi.proto` with the matching Go protobuf toolchain when the schema changes.
- Do not edit vendored/generated third-party files under `TypeDuck-Windows:libIME2/lib/googletest-release-1.10.0/`, `TypeDuck-Windows:jsoncpp/`, or `TypeDuck-Windows:libuv/` unless the task is explicitly about vendor maintenance.

**CI Behavior:**
- `TypeDuck-Windows:.github/workflows/nightly.yml` and `TypeDuck-Windows:.github/workflows/release.yml` run on `windows-2022`, use `pwsh`, checkout both public repos, download the TypeDuck schema release artifact, install Inno Setup, download protobuf `33.5`, and run `scripts/_all_in_package.ps1`.
- `TypeDuck-Windows-backend:.github/workflows/nightly.yml` and `TypeDuck-Windows-backend:.github/workflows/release.yml` run on `windows-2022`, use `powershell`, set up Go `1.24.6`, run `scripts/build.ps1`, and upload runtime zip artifacts.
- No dedicated lint, `go test`, `ctest`, or coverage workflow is detected in either repo. Add explicit CI test steps when a change relies on test execution rather than package-build compilation.

---

*Convention analysis: 2026-06-28*
