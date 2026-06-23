# Coding Conventions

**Analysis Date:** 2026-06-23

## Naming Patterns

**Files:**
- Use PascalCase for first-party C++ class modules: `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiCandidateWindow.h`, `MoqLauncher/PipeServer.cpp`, `libIME2/src/TextService.h`.
- Keep implementation/header pairs side by side in the owning module directory: `MoqiTextService/MoqiClient.cpp` with `MoqiTextService/MoqiClient.h`, `MoqLauncher/PipeClient.cpp` with `MoqLauncher/PipeClient.h`.
- Use lowercase for protocol/source schema files: `proto/moqi.proto`, `proto/ProtoFraming.h` is the small hand-written framing helper around generated protobuf code.
- Use PowerShell verb-noun script names for build/package automation: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/generate-glyph-icon.ps1`.
- Treat Moqi-branded filenames and user-facing text as legacy scaffold for replacement, not product naming to preserve: `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/PipeServer.cpp`, `installer/MoqiTsf.iss`, `backends.json`.

**Functions:**
- Existing C++ uses lower camelCase for methods and free functions: `waitForRpcConnection()` in `MoqiTextService/MoqiClient.cpp`, `formatCodePoints()` in `MoqiTextService/MoqiClient.cpp`, `getPipeName()` in `MoqLauncher/PipeServer.cpp`.
- Windows entry points and callbacks keep Win32 naming/signatures: `wWinMain()` in `SetupHelper/SetupHelper.cpp`, `CALLBACK Client::onAsyncPollTimer()` in `MoqiTextService/MoqiClient.cpp`, `DllRegisterServer()` in `MoqiTextService/DllEntry.cpp`.
- Test names use GoogleTest `TEST(SuiteName, CaseName)` with PascalCase case names: `TEST(TestComPtr, DefaultsToNull)` in `libIME2/test/ComPtr_test.cpp`, `TEST(TestIUnknownImpl, QueryInterfaceTSF)` in `libIME2/test/ComObject_test.cpp`.
- PowerShell helper functions use approved verb-style names or local action names with PascalCase: `Invoke-Step` and `Resolve-ProtobufRoot` in `scripts/build.ps1`, `Copy-MoqiImeRuntime` in `scripts/install.ps1`.

**Variables:**
- C++ class members use trailing underscores: `pipe_`, `rpcInProgress_`, `pendingAsyncResponses_` in `MoqiTextService/MoqiClient.h`; `singleInstanceMutex_` and `logger_` in `MoqLauncher/PipeServer.h`.
- Local variables use lower camelCase in newer first-party C++: `serializedReply` in `MoqiTextService/MoqiClient.cpp`, `targetWindow` in `MoqiTextService/MoqiClient.cpp`, `reboot_required` is also present in newer helper code in `SetupHelper/SetupHelper.cpp`.
- Constants are mixed. Legacy code uses all caps `MAX_LOG_FILE_SIZE` in `MoqLauncher/PipeServer.cpp`; newer helper and preview code use `k`-prefixed constants such as `kExitSuccess` in `SetupHelper/SetupHelper.cpp` and `kWindowClassName` in `Preview/main.cpp`. Prefer `k`-prefixed constants for new TypeDuck code.
- CMake variables are uppercase and project-prefixed: `MOQI_PROTOBUF_ROOT`, `MOQI_GENERATED_PROTO_DIR`, `MOQI_PROTO_CPP` in `CMakeLists.txt`. Rename or introduce TypeDuck-specific variables consistently when replacing scaffold-owned build config.

**Types:**
- Classes and structs use PascalCase: `Ime::ComPtr` in `libIME2/src/ComPtr.h`, `Moqi::Client` in `MoqiTextService/MoqiClient.h`, `PipeSecurityAttributes` in `MoqLauncher/PipeSecurity.h`.
- Namespaces separate framework and product layers: `Ime` for the reusable TSF/libIME layer in `libIME2/src/ComObject.h`, `Moqi` for product scaffold code in `MoqiTextService/MoqiClient.h` and `MoqLauncher/PipeServer.cpp`, `Moqi::Proto` for framing in `proto/ProtoFraming.h`.
- Generated protobuf types live under `moqi::protocol` in `proto/moqi.pb.h`; do not hand-edit `proto/moqi.pb.h` or `proto/moqi.pb.cc`.

## Code Style

**Formatting:**
- Formatting tool: Not detected. There is no `.clang-format`, `.editorconfig`, `.clang-tidy`, or lint config at repo root.
- C++ formatting is inconsistent across legacy and newer code. Legacy `libIME2/src/ImeEngine.cpp` uses 4-space indentation and compact `if(...)` style; newer `SetupHelper/SetupHelper.cpp`, `Preview/main.cpp`, and `proto/ProtoFraming.h` use 2-space indentation, braces on the same line, and spaces inside control statements.
- For new TypeDuck-owned files, use the newer 2-space C++ style from `SetupHelper/SetupHelper.cpp` and `proto/ProtoFraming.h`. Do not reformat untouched legacy files in `libIME2/src` or large scaffold files such as `MoqiTextService/MoqiClient.cpp` as part of feature changes.
- Keep C++ standard at C++20 to match `CMakeLists.txt`, `libIME2/CMakeLists.txt`, and `MoqiTextService/CMakeLists.txt`.
- Use Unicode-aware Windows APIs and wide strings for Windows UI/paths: `CreateWindowExW()` in `Preview/main.cpp`, `MessageBoxW()` and `std::wstring` in `SetupHelper/SetupHelper.cpp`, `StringFromCLSID()` conversion helpers in `MoqiTextService/TsfLog.cpp`.

**Linting:**
- Tool used: Not detected.
- Key rules: Not enforced by config. Compile-time safety comes from MSVC flags in `CMakeLists.txt`, including `/utf-8`, disabled RTTI via `/GR-`, `_UNICODE`, `UNICODE`, and C++20.
- Manual review should check COM reference-counting, HRESULT branches, handle closure, Win32 resource cleanup, generated-file boundaries, and legacy Moqi user-facing text.

## Import Organization

**Order:**
1. Prefer local header first in `.cpp` files, as in `MoqiTextService/MoqiClient.cpp` including `"MoqiClient.h"` first and `MoqLauncher/PipeServer.cpp` including `"PipeServer.h"` first.
2. Then include nearby project headers: `libIME2/src/Utils.h`, `proto/ProtoFraming.h`, `MoqiTextService.h` in `MoqiTextService/MoqiClient.cpp`.
3. Then include Windows/platform headers and standard library headers: `<Windows.h>`, `<Shellapi.h>`, `<filesystem>`, `<sstream>` in `MoqLauncher/PipeServer.cpp` and `SetupHelper/SetupHelper.cpp`.
4. Then include third-party library headers: `<json/json.h>`, `<uv.h>`, and `<spdlog/spdlog.h>` in `MoqLauncher/PipeServer.cpp`.

**Path Aliases:**
- CMake include directories provide root-relative includes from `${CMAKE_SOURCE_DIR}` and generated protobuf root in `MoqiTextService/CMakeLists.txt` and `MoqLauncher/CMakeLists.txt`.
- Include generated protobuf as `proto/moqi.pb.h`, as in `MoqiTextService/MoqiClient.cpp` and `MoqiTextService/MoqiClient.h`.
- Include libIME headers either through `libIME2/src/...` for cross-module usage (`MoqiTextService/MoqiClient.h`) or direct local names inside `libIME2/src` (`libIME2/src/ImeEngine.h`).

## Error Handling

**Patterns:**
- COM and TSF functions return `HRESULT` and compare against `S_OK`, `FAILED(...)`, or `SUCCEEDED(...)`: `libIME2/src/ComObject.h`, `libIME2/src/ImeModule.cpp`, `MoqiTextService/MoqiTextService.cpp`.
- Win32 helper executables return integer exit codes and build user-visible failure strings: `kExitFailure`, `FormatWindowsErrorMessage()`, and `ShowFailureAndReturn()` in `SetupHelper/SetupHelper.cpp`.
- RPC and pipe failures generally return `false`, close the pipe, and reset text service state: `Client::callRpcMethod()` and `Client::closeRpcConnection()` in `MoqiTextService/MoqiClient.cpp`.
- Exceptions are used sparingly around operations that can fail during setup/logging: `MoqLauncher/PipeSecurity.cpp`, `MoqLauncher/PipeServer.cpp`, and PowerShell scripts in `scripts/build.ps1` and `scripts/install.ps1`.
- PowerShell scripts set `$ErrorActionPreference = "Stop"` and throw on failed external commands: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`.

## Logging

**Framework:** `spdlog`, Win32 debug output, file append logging, PowerShell console output.

**Patterns:**
- Launcher logging uses `spdlog::rotating_logger_mt` with fallback to stderr in `MoqLauncher/PipeServer.cpp`.
- TSF/debug logging writes to `%LOCALAPPDATA%\MoqiIM\Log\tsf-debug.log` and `OutputDebugStringW()` in `MoqiTextService/TsfLog.cpp`, `MoqiTextService/DllEntry.cpp`, and `MoqiTextService/MoqiClient.cpp`.
- Debug logging gates are controlled through `Ime::isDebugLoggingEnabled()` and `Ime::isTraceLoggingEnabled()` in `libIME2/src/DebugLogConfig.cpp` and `MoqiTextService/MoqiClient.cpp`.
- PowerShell scripts report progress with `Write-Host` and warnings with `Write-Warning`: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`.
- For TypeDuck replacement work, rename log directories and logger names when product-owned paths are changed; do not leave new user-facing TypeDuck behavior writing to Moqi-branded locations unless explicitly preserving migration compatibility.

## Comments

**When to Comment:**
- Use comments for Win32/TSF constraints, COM ownership, and non-obvious Windows behavior. Good examples are TSF DLL placement comments in `SetupHelper/SetupHelper.cpp`, app-container pipe security notes in `MoqLauncher/PipeServer.cpp`, and COM `IUnknown` pointer comments in `libIME2/src/ComObject.h`.
- Keep FIXME/TODO comments specific and actionable. Existing debt examples are in `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/BackendServer.cpp`, `libIME2/src/CandidateWindow.cpp`, and `libIME2/test/ComObject_test.cpp`.
- Do not add comments that only restate simple code; existing useful comments explain Windows API limitations, TSF lifecycle, or migration-sensitive behavior.

**JSDoc/TSDoc:**
- Not applicable. This is a C++/PowerShell codebase.
- C++ has no Doxygen convention in first-party files; comments are plain `//` and block license headers.

## Function Design

**Size:** Keep new functions focused. Legacy files contain very large functions and files, especially `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiTextService.cpp`, and `MoqLauncher/PipeServer.cpp`; new TypeDuck logic should prefer smaller helpers like `proto/ProtoFraming.h` and `SetupHelper/SetupHelper.cpp`.

**Parameters:** Use references for required mutable outputs and const references for read-only strings/objects: `serializeMessage(const Message&, std::string&)` in `proto/ProtoFraming.h`, `ScheduleReregisterTask(const Options&, std::wstring&)` in `SetupHelper/SetupHelper.cpp`, `updateUI(const Json::Value&)` in `MoqiTextService/MoqiClient.cpp`.

**Return Values:** Use `bool` for recoverable operation success (`Client::waitForRpcConnection()` in `MoqiTextService/MoqiClient.cpp`, `PipeServer::isCloudClipboardEnabled()` in `MoqLauncher/PipeServer.cpp`), `HRESULT` for COM contracts (`libIME2/src/ImeModule.h`), and explicit process exit codes for executables (`SetupHelper/SetupHelper.cpp`).

## Module Design

**Exports:** 
- TSF COM exports live in `MoqiTextService/DllEntry.cpp` and `MoqiTextService/MoqiTextService.def`.
- Reusable IME/TSF primitives are built as `libIME2_static` from `libIME2/src/CMakeLists.txt`.
- Product TSF DLL code is built as `MoqiTextService` in `MoqiTextService/CMakeLists.txt`; launcher code is built as `MoqiLauncher` in `MoqLauncher/CMakeLists.txt`; installer helper code is built from `SetupHelper/CMakeLists.txt`.

**Barrel Files:** Not used. Include headers directly from their owning module, for example `MoqiTextService/MoqiClient.h`, `MoqLauncher/PipeServer.h`, `libIME2/src/ComPtr.h`, and `proto/ProtoFraming.h`.

**Legacy Scaffold Boundary:**
- Moqi branding, Simplified Chinese UI strings, backend names, and fcitx/Moqi-specific configuration are scaffold details in paths such as `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/PipeServer.cpp`, `installer/MoqiTsf.iss`, and `backends.json`.
- New product-owned UI strings should be bilingual Traditional Hong Kong Chinese and English where user-facing; do not copy unused Moqi/fcitx language-bar or menu clutter from `MoqLauncher/PipeServer.cpp` or `MoqiTextService/MoqiClient.cpp`.
- TypeDuck librime fork and dictionary lookup filter plugin work should use new TypeDuck names in new code/config while preserving required Windows TSF conventions from `libIME2/src` and installer registration patterns from `SetupHelper/SetupHelper.cpp`.

---

*Convention analysis: 2026-06-23*
