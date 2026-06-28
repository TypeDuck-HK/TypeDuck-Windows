# AGENTS.md

## Product

TypeDuck Windows is a v1 Windows Cantonese IME product made from two equivalent repositories:

- `https://github.com/TypeDuck-HK/TypeDuck-Windows` - Windows TSF frontend, launcher, settings/about apps, setup helper, installer, runtime staging, and release verification.
- `https://github.com/TypeDuck-HK/TypeDuck-Windows-backend` - Go backend runtime, Rime service, librime binding, TypeDuck settings application, packaged runtime output, and backend tests.

Do not reference local checkout paths in documentation, plans, commits, or review comments. Use public repo names plus repo-relative paths, for example `TypeDuck-Windows:MoqiTextService/TypeDuckProfile.cpp` or `TypeDuck-Windows-backend:server.go`.

## Current Product Truth

- TypeDuck Windows is a separate v1 product.
- The installed IME profile is Chinese (Traditional, Hong Kong) / `zh-HK`.
- The user-facing profile name is `TypeDuck 粵語輸入法 / TypeDuck Cantonese IME`.
- The primary TSF profile GUID is `{C6E8F5DF-6504-44F9-B7CF-17A195373A83}`.
- User-facing strings must be bilingual Traditional Hong Kong Chinese and English.
- The installer must show settings during installation and keep language/settings actions prominent.
- The shipped runtime is `TypeDuckRuntime` under the installed app directory.
- Candidate and dictionary display depend on the TypeDuck librime runtime and `rime-dictionary-lookup-filter` evidence.

## Repository Responsibilities

### TypeDuck-Windows

- TSF COM DLL and profile registration: `MoqiTextService/DllEntry.cpp`, `MoqiTextService/TypeDuckProfile.cpp`, `MoqiTextService/MoqiImeModule.cpp`.
- Product TSF behavior and frontend RPC: `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiClient.cpp`.
- Candidate and dictionary UI: `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/TypeDuckCandidateInfo.cpp`.
- Launcher, named pipe server, backend bridge, tray, and preferences: `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeClient.cpp`, `MoqLauncher/BackendServer.cpp`, `MoqLauncher/TypeDuckPreferences.cpp`.
- Settings/about apps: `TypeDuckSettings/`.
- Installer and elevated registration helper: `installer/MoqiTsf.iss`, `SetupHelper/SetupHelper.cpp`.
- Runtime and release scripts: `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`, `scripts/Stage-TypeDuckRuntime.ps1`, `scripts/Test-TypeDuck*.ps1`, `scripts/Invoke-TypeDuck*.ps1`.
- C++ protocol schema and framing: `proto/moqi.proto`, `proto/ProtoFraming.h`.

### TypeDuck-Windows-backend

- Backend process entry point and client/session dispatch: `server.go`.
- Backend frame IO: `protocol_io.go`.
- Request/response model and protobuf conversion: `imecore/protocol.go`.
- Backend service abstraction: `imecore/client.go`, `imecore/service.go`.
- TypeDuck Rime service: `input_methods/rime/rime.go`.
- Native librime binding and Rime API wrapper: `input_methods/rime/librime.go`, `input_methods/rime/native_cgo.go`, `input_methods/rime/native_stub.go`.
- Rime key translation: `input_methods/rime/rime_keyevent.go`.
- TypeDuck settings to Rime config: `input_methods/rime/appearance_config.go`, `input_methods/rime/config_update.go`.
- Runtime package build: `scripts/build.ps1`.
- Backend protocol schema and generated Go bindings: `proto/moqi.proto`, `proto/moqi.pb.go`.

## Naming and Identity

- Public repo names are `TypeDuck-Windows` and `TypeDuck-Windows-backend`.
- Product binaries are `TypeDuckTextService.dll`, `TypeDuckLauncher.exe`, `TypeDuckSetupHelper.exe`, `TypeDuckSettings.exe`, `TypeDuckAbout.exe`, and `TypeDuckRuntime/server.exe`.
- Installed app/user folders use `TypeDuckIME`.
- Current source still contains `Moqi` and `moqi` identifiers in target names, namespaces, protocol package names, generated protobuf names, and some script parameters. Treat these as current implementation identifiers or stale code pending removal, not as product names.
- Do not perform single-file identity renames. TSF CLSIDs, profile GUIDs, installer cleanup keys, COM registration, executable names, scripts, tests, and backend profile metadata must change together.

## Architecture

```text
Windows TSF host process
        |
        v
TypeDuckTextService.dll
        |
        | named pipe + protobuf frame
        v
TypeDuckLauncher.exe
        |
        | stdin/stdout + protobuf frame
        v
TypeDuckRuntime/server.exe
        |
        v
librime + TypeDuck Rime data + dictionary lookup filter
```

- Keep TSF host-process work thin. Engine work belongs in the backend process.
- Keep all TSF-to-backend traffic behind the launcher bridge.
- Keep protobuf framing consistent across C++ and Go.
- Preserve COM reference counting and Win32 handle ownership rules.
- Keep user-facing TypeDuck settings in `TypeDuckSettings` and preference code, not in backend menu surfaces.

## Build

Use `pwsh`, not Windows PowerShell, when documenting or running repository scripts unless testing Windows PowerShell compatibility specifically.

```powershell
# TypeDuck-Windows
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1

# TypeDuck-Windows-backend
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -RimeDataSource <schema-source>

# TypeDuck-Windows full package
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/_all_in_package.ps1 -RimeDataSource <schema-source>
```

Frontend build requirements include Visual Studio 2022, Windows SDK, CMake 3.21+, Inno Setup 6 for installer builds, git submodules, and protobuf tooling. Backend build requirements include Go, protobuf Go bindings, a TypeDuck Rime schema source, and packaged `rime.dll` assets.

## Testing

Choose focused tests for the edited boundary.

### TypeDuck-Windows

```powershell
ctest --test-dir build-vs32 -C Debug --output-on-failure
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckProtocolContract.ps1 -RepoRoot . -Strict
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckCandidateData.ps1 -RepoRoot . -Strict
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckSettingsPersistence.ps1 -RepoRoot . -Strict
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckRuntimePackagePruning.ps1 -RepoRoot . -Strict
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckReleaseArtifacts.ps1 -RepoRoot . -Strict
```

### TypeDuck-Windows-backend

```powershell
go test ./...
go test ./imecore ./input_methods/rime ./mobilebridge
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckCandidateParity.ps1 -RepoRoot .
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckSettingsCustomization.ps1 -RepoRoot .
```

Native Rime tests are environment-gated. Use `MOQI_RIME_PACKAGE_DIR`, `MOQI_RIME_INIT_MAX_MS`, and `MOQI_REAL_APPDATA` only for tests that explicitly require them.

## Coding Conventions

- Use the newer 2-space C++ style visible in TypeDuck-owned files such as `MoqiTextService/TypeDuckCandidateInfo.cpp`, `MoqLauncher/TypeDuckPreferences.cpp`, `SetupHelper/SetupHelper.cpp`, and `proto/ProtoFraming.h`.
- Do not reformat broad framework/vendor code in `libIME2/`, `libuv/`, `jsoncpp/`, or vendored GoogleTest as part of product changes.
- Use Unicode-aware Windows APIs and wide strings for Windows UI, registry, paths, installer messaging, and TSF-facing strings.
- Keep C++ members with trailing underscores where the surrounding class does so.
- Run `gofmt` on edited Go files.
- PowerShell scripts should use `param(...)`, `$ErrorActionPreference = "Stop"`, `Test-Path -LiteralPath`, repo-root resolution through `$PSScriptRoot`, and native PowerShell filesystem operations.
- Avoid comments that restate code. Comment TSF/COM ownership, Win32 lifecycle constraints, Rime runtime constraints, and protocol compatibility.

## Protocol Rules

- Edit `.proto` schemas first, then regenerate bindings.
- Do not hand-edit generated protobuf files:
  - `TypeDuck-Windows:proto/moqi.pb.h`
  - `TypeDuck-Windows:proto/moqi.pb.cc`
  - `TypeDuck-Windows-backend:proto/moqi.pb.go`
- Keep frontend and backend schema fields, enum values, and TypeDuck settings payloads aligned.
- C++ framing has a payload ceiling in `proto/ProtoFraming.h`; keep Go framing behavior compatible when changing protocol limits.
- Candidate dictionary payloads must preserve raw lookup comments until the frontend parser formats them.

## User-Facing Text

- Every user-facing string added to Windows UI, installer UI, setup helper messages, tray messages, and settings/about UI must be bilingual Traditional Hong Kong Chinese and English.
- Prefer concise paired strings, for example `設定 / Settings`, `安裝完成 / Installation completed`, or full Traditional Chinese sentence followed by English sentence.
- Do not expose implementation filenames, JSON file names, unsupported-feature placeholders, or internal protocol terms in UI copy unless the screen is explicitly a developer diagnostic.

## Installer and Runtime Safety

- Installer work must consider both 32-bit and 64-bit TSF DLL registration.
- `SetupHelper` copies TSF DLLs into Windows system directories and invokes matching `regsvr32.exe`; changes require install/uninstall/reboot-path verification.
- `TypeDuckLauncher` startup registration is under HKCU and should remain per-user.
- The runtime package must include `TypeDuckRuntime/server.exe`, `input_methods/rime/rime.dll`, Rime data, and `appearance_themes.json`.
- Runtime packaging should keep non-v1 source-only surfaces out of the shipped runtime. Source folders that are stale or pending removal must not become user-visible through installer staging.

## Security and Privacy

- TSF DLL code runs inside third-party host processes. Keep blocking IO, native engine work, network calls, and long operations out of the TSF DLL.
- Treat named pipe and backend stdin/stdout as trust boundaries. Validate frame sizes, parse errors, backend timeouts, and same-user pipe assumptions.
- Do not log typed content, clipboard content, candidate payload bodies, secrets, or raw settings values unless a debug gate explicitly requires bounded diagnostic output.
- Runtime staging downloads binary dependencies; keep hashes, provenance, and release evidence current.
- Installer and setup helper changes can affect elevated system state. Review path validation, reboot scheduling, registry cleanup, and DLL source integrity.

## Documentation Rules

- Documentation should describe current TypeDuck Windows v1 behavior, not project history.
- Do not reference local checkout paths.
- Use public repo names and repo-relative paths.
- The frontend `.planning/codebase` directory is the shared codebase map for both repositories and should cover both equivalently.
- The single `TypeDuck-Windows:AGENTS.md` may include backend context and stale code pending removal when it affects engineering work.
- Keep stale source-only surfaces out of user-facing docs unless they are relevant to an engineering risk.
- Keep READMEs user/developer focused; put deep implementation constraints in `AGENTS.md` and `.planning/codebase`.

## High-Risk Areas

- TSF profile identity: `MoqiTextService/TypeDuckProfile.cpp`, `MoqiTextService/DllEntry.cpp`, `SetupHelper/SetupHelper.cpp`, `installer/MoqiTsf.iss`, and backend `input_methods/rime/ime.json` must stay synchronized.
- Candidate dictionary display: backend Rime comments, protobuf candidate entries, `TypeDuckCandidateInfo.cpp`, and `MoqiCandidateWindow.cpp` must be tested together.
- Settings: frontend preference validation, settings UI, launcher apply flow, backend Rime config writes, and redeploy/reload behavior must be changed together.
- Runtime packaging: backend `scripts/build.ps1`, frontend `scripts/install.ps1`, frontend release workflows, and runtime pruning tests must agree on the shipped layout.
- Cross-repo protocol: any schema drift can break candidate data, settings, runtime health, or recovery behavior.

## Review Checklist

- Does the change preserve `zh-HK` TypeDuck profile registration?
- Are all user-facing strings bilingual Traditional Hong Kong Chinese and English?
- Are frontend and backend protobuf schemas still compatible?
- Are generated protobuf files updated only through generation?
- Does the installer still stage `TypeDuckRuntime` and both TSF DLL bitnesses?
- Are TypeDuck settings persisted, validated, and applied without corrupting existing settings?
- Are logs privacy-safe?
- Were tests run in the repository that owns the changed behavior, plus cross-repo checks when protocol/runtime packaging changed?
