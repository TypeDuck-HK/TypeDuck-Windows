# External Integrations

**Analysis Date:** 2026-06-23

## APIs & External Services

**Windows Platform APIs:**
- Microsoft Text Services Framework (TSF) - registers and runs the IME text service.
  - SDK/Client: Windows SDK COM/TSF headers through `libIME2/src/` and `MoqiTextService/`.
  - Auth: Windows COM registration and user language profile state; no env var.
- Win32 Shell, tray, clipboard, process, registry, ACL, and scheduled task APIs - launcher tray UI, clipboard listener, process startup, elevated setup, DLL registration, reboot fallback, and pipe security.
  - SDK/Client: Windows SDK functions in `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeSecurity.cpp`, `SetupHelper/SetupHelper.cpp`, and `installer/MoqiTsf.iss`.
  - Auth: User token/admin elevation; `SetupHelper.exe` relaunches with `runas` when needed.

**Local Backend Process:**
- TypeDuck runtime bridge - current fixed runtime process for TypeDuck candidate generation and settings/deploy requests.
  - SDK/Client: `MoqLauncher/PipeServer.cpp` constructs `typeduck-runtime-bridge` with command `TypeDuckRuntime\server.exe`; launcher process management remains in `MoqLauncher/BackendServer.cpp`.
  - Auth: Local process boundary only; no frontend secret.
- Legacy `moqi-ime` backend source - sibling backend repository still provides the packaged runtime build, but the Windows frontend no longer trusts source or staged `backends.json` for product runtime discovery.
  - SDK/Client: Sibling runtime is consumed through the packaged `TypeDuckRuntime` folder.
  - Auth: Local process boundary only; no frontend secret.

**GitHub / Build Downloads:**
- GitHub repositories and release assets - build fetches `spdlog`, `protobuf`, submodules, sibling backend, and CI artifacts/releases.
  - SDK/Client: CMake FetchContent in `CMakeLists.txt`, git submodules in `.gitmodules`, GitHub Actions in `.github/workflows/nightly.yml` and `.github/workflows/release.yml`.
  - Auth: `GH_TOKEN=${{ github.token }}` for release upload in workflows.
- Chocolatey package source - CI installs Inno Setup with `choco install innosetup`.
  - SDK/Client: `.github/workflows/nightly.yml`, `.github/workflows/release.yml`.
  - Auth: Not detected.

**Legacy Cloud/AI Services:**
- WebDAV sync/cloud clipboard - described as legacy backend functionality in `README.md`; Windows launcher only reads local enablement and forwards clipboard text to the backend over protobuf.
  - SDK/Client: `METHOD_CLOUD_CLIPBOARD_UPLOAD` in `proto/moqi.proto`; upload bridge in `MoqLauncher/PipeServer.cpp` and `MoqLauncher/BackendServer.cpp`.
  - Auth: Legacy config file `%APPDATA%\Moqi\cloud_clipboard.json`; README says password is stored in `cloud_clipboard.pw` with DPAPI, but this repo does not implement DPAPI encryption/decryption.
- AI model endpoint - described only by legacy README config (`base_url`, `api_key`, `model`) under `%APPDATA%\Moqi\Rime\ai_config.json`.
  - SDK/Client: Not implemented in this Windows frontend; delegated to backend/runtime config.
  - Auth: `api_key` in backend config, not read by this repo.

## Data Storage

**Databases:**
- Not detected.
  - Connection: Not applicable.
  - Client: Not applicable.

**File Storage:**
- Local install payload under `%ProgramFiles(x86)%\MoqiIM` and staged installer payload under `installer/stage/` (`installer/MoqiTsf.iss`, `scripts/install.ps1`).
- System TSF DLL copies under `SysWOW64\MoqiTextService.dll` and `System32\MoqiTextService.dll` (`SetupHelper/SetupHelper.cpp`).
- Launcher logs and config under `%LOCALAPPDATA%\MoqiIM`, including `MoqiLauncher.json` and `Log\*.log` (`MoqLauncher/PipeServer.cpp`, `libIME2/src/DebugLogConfig.cpp`).
- Legacy roaming config under `%APPDATA%\Moqi`, including cloud clipboard enablement (`MoqLauncher/Utils.cpp`, `MoqLauncher/PipeServer.cpp`).
- Backend language profile metadata under installed `moqi-ime\input_methods\*\ime.json` (`MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `MoqLauncher/PipeServer.cpp`).

**Caching:**
- No external cache service.
- Local build cache hints for protobuf source trees under `%USERPROFILE%\.cache\moqi-protobuf` (`scripts/build.ps1`, `scripts/_all_in_package.ps1`).
- Rotating launcher logs capped at 5 MB and 5 files (`MoqLauncher/PipeServer.cpp`).

## Authentication & Identity

**Auth Provider:**
- Windows identity and local session security.
  - Implementation: Named pipe paths include the Windows username (`\\.\pipe\<username>\MoqiIM\Launcher`) in `MoqiTextService/MoqiClient.cpp` and `MoqLauncher/PipeServer.cpp`; pipe security allows the logon SID and denies network access on Windows 8+ (`MoqLauncher/PipeSecurity.cpp`).
- COM/TSF registration identity.
  - Implementation: CLSID `{8F204C91-2D7A-4B3E-9E1F-6A5C0D8B2E7F}` in `MoqiTextService/MoqiImeModule.cpp` must stay aligned with `installer/MoqiTsf.iss`.
- Installer elevation.
  - Implementation: `SetupHelper.exe` relaunches itself with ShellExecute `runas` if not admin (`SetupHelper/SetupHelper.cpp`).

## Monitoring & Observability

**Error Tracking:**
- None external.

**Logs:**
- Launcher logs through `spdlog::rotating_logger_mt` to `%LOCALAPPDATA%\MoqiIM\Log\MoqiLauncher.log` (`MoqLauncher/PipeServer.cpp`).
- TSF/debug logs under `%LOCALAPPDATA%\MoqiIM\Log`, controlled by `MoqiLauncher.json` log level (`MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiTextService.cpp`, `libIME2/src/DebugLogConfig.cpp`).
- Backend stderr is captured and logged by `MoqLauncher/BackendServer.cpp`.
- Tray notifications can be emitted by backend protobuf responses (`ServerResponse.tray_notification` in `proto/moqi.proto`, handled in `MoqLauncher/BackendServer.cpp`).

## CI/CD & Deployment

**Hosting:**
- GitHub Releases for nightly and release installer assets (`.github/workflows/nightly.yml`, `.github/workflows/release.yml`).

**CI Pipeline:**
- GitHub Actions `windows-2022`.
- Nightly pipeline triggers on `main` push and manual dispatch; builds sibling `moqi-ime`, packages installer, uploads artifact, and updates a `nightly` prerelease (`.github/workflows/nightly.yml`).
- Release pipeline triggers on published release and manual dispatch; builds installer and uploads to the GitHub release when applicable (`.github/workflows/release.yml`).

## Environment Configuration

**Required env vars:**
- `MOQI_PROTOBUF_ROOT` - optional build-time root for local protobuf/protoc (`CMakeLists.txt`, `scripts/build.ps1`).
- `MOQI_PROTOBUF_SOURCE_DIR` - optional build-time protobuf source checkout (`CMakeLists.txt`, `scripts/build.ps1`).
- `MOQI_PROTOC_EXECUTABLE` - optional direct `protoc.exe` path (`CMakeLists.txt`).
- `MOQI_PROGRAM_DIR` - setup/runtime override for installed program directory during TSF registration and discovery (`SetupHelper/SetupHelper.cpp`, `MoqiTextService/MoqiImeModule.cpp`).
- `LOCALAPPDATA` - runtime logs and launcher config (`MoqiTextService/DllEntry.cpp`, `MoqLauncher/PipeServer.cpp`, `libIME2/src/DebugLogConfig.cpp`).
- `APPDATA` / `FOLDERID_RoamingAppData` - legacy `%APPDATA%\Moqi` config lookup (`MoqLauncher/Utils.cpp`).
- `GH_TOKEN` - GitHub release publishing token in workflows (`.github/workflows/nightly.yml`, `.github/workflows/release.yml`).
- `PROTOBUF_VERSION` - CI version selector set to `33.5` (`.github/workflows/nightly.yml`, `.github/workflows/release.yml`).

**Secrets location:**
- No repository secrets files detected.
- GitHub Actions use the built-in `${{ github.token }}` for release publishing.
- Legacy README references `api_key` in `%APPDATA%\Moqi\Rime\ai_config.json` and WebDAV password in `cloud_clipboard.pw`; those are backend/user runtime files and must not be committed.

## Webhooks & Callbacks

**Incoming:**
- Windows TSF/COM callbacks into `MoqiTextService.dll` for activation, key events, preserved keys, language profile changes, and menu/config operations (`MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiClient.cpp`).
- Local named pipe server listens at `\\.\pipe\<username>\MoqiIM\Launcher` (`MoqLauncher/PipeServer.cpp`).
- Windows clipboard updates arrive through `AddClipboardFormatListener` and `WM_CLIPBOARDUPDATE` in `MoqLauncher/PipeServer.cpp`.
- GitHub Actions workflow triggers: `push` to `main`, `workflow_dispatch`, and `release.published` (`.github/workflows/nightly.yml`, `.github/workflows/release.yml`).

**Outgoing:**
- Named pipe RPC from `MoqiTextService.dll` to `MoqiLauncher.exe` using `TransactNamedPipe`, framed protobuf payloads, and message mode pipes (`MoqiTextService/MoqiClient.cpp`, `proto/ProtoFraming.h`).
- Launcher subprocess stdio to backend `server.exe` using libuv pipes and framed protobuf (`MoqLauncher/BackendServer.cpp`, `proto/ProtoFraming.h`).
- ShellExecute launches `MoqiLauncher.exe`, IME config tools from `ime.json`, elevated `SetupHelper.exe`, and optional installer helpers (`MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `SetupHelper/SetupHelper.cpp`).
- GitHub release upload/edit commands run from CI (`.github/workflows/nightly.yml`, `.github/workflows/release.yml`).

---

*Integration audit: 2026-06-23*
