# Integrations

**Analysis Date:** 2026-06-28

**Scope:** This map treats `https://github.com/TypeDuck-HK/TypeDuck-Windows` and `https://github.com/TypeDuck-HK/TypeDuck-Windows-backend` as equivalent parts of the TypeDuck Windows v1 product. Paths are repo-relative and prefixed with `TypeDuck-Windows:` or `TypeDuck-Windows-backend:`.

## External Systems

| Integration | Direction | Purpose | Evidence |
|-------------|-----------|---------|----------|
| Microsoft Text Services Framework | Windows loads frontend DLL | Registers and hosts the TypeDuck zh-HK IME profile in TSF host processes. | `TypeDuck-Windows:MoqiTextService/DllEntry.cpp`, `TypeDuck-Windows:MoqiTextService/TypeDuckProfile.cpp`, `TypeDuck-Windows:libIME2/src/` |
| Windows COM / Registry | Installer and TSF DLL write system/user state | COM class registration, TIP profile registration, startup value, uninstall cleanup, and re-registration fallback. | `TypeDuck-Windows:SetupHelper/SetupHelper.cpp`, `TypeDuck-Windows:installer/MoqiTsf.iss` |
| Windows named pipes | TSF frontend to launcher | Per-user IPC between in-process TSF DLL clients and `TypeDuckLauncher.exe`. | `TypeDuck-Windows:MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows:MoqLauncher/PipeServer.cpp`, `TypeDuck-Windows:MoqLauncher/PipeSecurity.cpp` |
| Backend stdin/stdout | Launcher to backend runtime | Length-prefixed protobuf transport to `TypeDuckRuntime/server.exe`. | `TypeDuck-Windows:MoqLauncher/BackendServer.cpp`, `TypeDuck-Windows-backend:server.go`, `TypeDuck-Windows-backend:protocol_io.go` |
| librime / Rime DLL | Backend to native engine | Cantonese composition, candidate generation, schema deploy, and dictionary lookup comments. | `TypeDuck-Windows-backend:input_methods/rime/librime.go`, `TypeDuck-Windows-backend:input_methods/rime/native_cgo.go` |
| Rime schema data | Backend runtime data | Shared data copied into the runtime package for `input_methods/rime/data`. | `TypeDuck-Windows-backend:scripts/build.ps1`, `TypeDuck-Windows:scripts/Stage-TypeDuckRuntime.ps1` |
| rime-dictionary-lookup-filter | Rime module inside packaged engine | Supplies structured dictionary-like data through Rime candidate comments. | `TypeDuck-Windows-backend:input_methods/rime/librime.go`, `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`, `TypeDuck-Windows:scripts/Stage-TypeDuckRuntime.ps1` |
| GitHub Actions | CI/release automation | Builds nightly and release installer/backend artifacts. | `TypeDuck-Windows:.github/workflows/*.yml`, `TypeDuck-Windows-backend:.github/workflows/*.yml` |
| GitHub Releases | CI artifact publishing | Publishes installer and backend runtime artifacts from workflow outputs. | `TypeDuck-Windows:.github/workflows/nightly.yml`, `TypeDuck-Windows:.github/workflows/release.yml`, `TypeDuck-Windows-backend:.github/workflows/*.yml` |

## IPC Contract

- Schema sources are `TypeDuck-Windows:proto/moqi.proto` and `TypeDuck-Windows-backend:proto/moqi.proto`.
- C++ transport framing is implemented in `TypeDuck-Windows:proto/ProtoFraming.h`; Go transport framing is implemented in `TypeDuck-Windows-backend:protocol_io.go`.
- Frames use a 32-bit little-endian size prefix followed by serialized protobuf payload.
- Frontend clients send `ClientRequest` messages through the launcher pipe. The launcher forwards compatible requests to backend stdin and returns `ServerResponse` messages.
- Candidate/dictionary payloads flow through `CandidateEntry.text`, `CandidateEntry.comment`, and TypeDuck candidate page metadata.
- Settings are carried through `METHOD_TYPEDUCK_SETTINGS_UPDATE`; backend redeploy is carried through `METHOD_TYPEDUCK_DEPLOY`.

## Runtime Package Integration

```text
TypeDuck-Windows installer payload
└── TypeDuckRuntime/
    ├── server.exe
    └── input_methods/
        └── rime/
            ├── rime.dll
            ├── appearance_themes.json
            └── data/
```

- `TypeDuck-Windows-backend:scripts/build.ps1` creates the runtime package.
- `TypeDuck-Windows:scripts/install.ps1` filters and copies the runtime package into the installer stage tree.
- `TypeDuck-Windows:MoqLauncher/PipeServer.cpp` defines the fixed runtime bridge command `TypeDuckRuntime\server.exe` and working directory `TypeDuckRuntime`.
- `TypeDuck-Windows:MoqiTextService/MoqiImeModule.cpp` scans installed `TypeDuckRuntime/input_methods/*/ime.json` where metadata is available.
- `TypeDuck-Windows-backend:server.go` registers the fixed TypeDuck Rime profile GUID and can scan `input_methods/*/ime.json` from the backend executable directory.

## Settings Integration

- The Windows settings app writes the TypeDuck preference JSON through shared code in `TypeDuck-Windows:MoqLauncher/TypeDuckPreferences.cpp`.
- The settings app reaches the launcher through the same named-pipe namespace used by TSF clients, implemented in `TypeDuck-Windows:TypeDuckSettings/TypeDuckSettingsWindow.cpp`.
- `TypeDuck-Windows:MoqLauncher/PipeClient.cpp` validates preference changes and forwards Rime-affecting side effects through `BackendServer::applyTypeDuckPreferences`.
- `TypeDuck-Windows-backend:input_methods/rime/appearance_config.go` maps settings into Rime custom YAML and redeploy/reload behavior.
- Interface-only preferences such as display language, Chinese typeface, romanization visibility, and reverse-code display remain frontend-owned.

## Installer and Windows Shell Integration

- `TypeDuck-Windows:installer/MoqiTsf.iss` configures an x64 installer, TypeDuck bilingual wizard text, Start Menu shortcuts, HKCU startup entry, postinstall settings/about launch, uninstall data prompt, and cleanup.
- `TypeDuck-Windows:SetupHelper/SetupHelper.cpp` copies `TypeDuckTextService.dll` into `SysWOW64` and `System32`, runs matching `regsvr32.exe`, handles admin elevation, and schedules `TypeDuckIME-ReRegisterTSF` when locked DLLs require reboot-time registration.
- `TypeDuck-Windows:MoqLauncher/PipeServer.cpp` owns tray menu, launcher mutex/window class, logging, and backend lifecycle.
- `TypeDuck-Windows:TypeDuckSettings/` owns settings/about executables and resources.

## CI and Release Integration

- Frontend workflows checkout both product repos, download and extract the TypeDuck schema release artifact, install Inno Setup, download protoc, run `TypeDuck-Windows:scripts/_all_in_package.ps1`, upload installer artifacts, and publish release/nightly assets.
- Backend workflows run `TypeDuck-Windows-backend:scripts/build.ps1` and package backend runtime zip artifacts.
- The frontend installer artifact naming contract is `typeduck-windows-ime-setup.exe` plus tag-or-sha variants in release workflows.
- Release verification guards live primarily in `TypeDuck-Windows:scripts/Test-TypeDuckRelease*.ps1` and `TypeDuck-Windows:scripts/Invoke-TypeDuckReleaseVerification.ps1`.

## Data Storage

- User preferences: `%APPDATA%\TypeDuckIME\TypeDuckPreferences.json`.
- Rime user data: `%APPDATA%\TypeDuckIME\Rime`.
- Frontend/launcher logs: `%LOCALAPPDATA%\TypeDuckIME\Log`.
- Backend logs: `%LOCALAPPDATA%\TypeDuckIME\Log\TypeDuckBackend-YYYY-MM-DD.log`, with temp/current-directory fallback in `TypeDuck-Windows-backend:server.go`.
- Installer app files: `%ProgramFiles(x86)%\TypeDuckIME`.

## Security and Trust Boundaries

- The TSF DLL runs inside arbitrary host processes; engine work remains out-of-process behind the launcher/backend boundary.
- Named-pipe access is constrained by local Windows security attributes in `TypeDuck-Windows:MoqLauncher/PipeSecurity.cpp`.
- Runtime staging downloads and extracts binary engine dependencies in `TypeDuck-Windows:scripts/Stage-TypeDuckRuntime.ps1`; the script records SHA-256 and lookup-filter provenance evidence.
- The installer performs elevated system DLL registration through `TypeDuck-Windows:SetupHelper/SetupHelper.cpp`; changes need install/upgrade/uninstall/reboot verification.

## Integration Gaps

- Cross-repo protobuf schema compatibility is not enforced by one canonical generator/diff command.
- Backend frame reads do not enforce the same payload maximum as the C++ framing helper.
- Frontend workflows consume the backend runtime package, while backend workflows also publish standalone runtime zips; release authority should stay explicit.
- GitHub workflows build/package artifacts but do not currently run full `ctest`, `go test ./...`, or generated-protobuf drift checks as dedicated CI gates.

---

*Integration analysis: 2026-06-28*
