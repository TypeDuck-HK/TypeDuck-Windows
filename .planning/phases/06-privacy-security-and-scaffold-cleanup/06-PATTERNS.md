# Phase 6: Privacy, Security, and Scaffold Cleanup - Pattern Map

**Mapped:** 2026-06-27
**Files analyzed:** 46
**Analogs found:** 44 / 46

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|-------------------|------|-----------|----------------|---------------|
| `installer/MoqiTsf.iss` | installer config | batch | `installer/MoqiTsf.iss` | exact |
| `.gitmodules` | config | batch | `.gitmodules` | exact |
| `installer/Inno-Setup-Chinese-Simplified-Translation/**` | config/vendor removal | batch | `.gitmodules` lines 10-12 | role-match |
| `SetupHelper/SetupHelper.cpp` | utility | file-I/O + request-response | `SetupHelper/SetupHelper.cpp` | exact |
| `scripts/install.ps1` | packaging script | file-I/O + batch | `scripts/install.ps1` | exact |
| `scripts/_all_in_package.ps1` | packaging script | batch | `scripts/_all_in_package.ps1` | exact |
| `installer/build-installer.ps1` | packaging script | batch | `scripts/install.ps1` | role-match |
| `.github/workflows/release.yml` | CI config | batch | `.github/workflows/nightly.yml` | role-match |
| `.github/workflows/nightly.yml` | CI config | batch | `.github/workflows/release.yml` | role-match |
| `backends.json` | config/removal | request-response config | `backends.json` | exact |
| `MoqLauncher/PipeServer.cpp` | service | event-driven + request-response | `MoqLauncher/PipeServer.cpp` | exact |
| `MoqLauncher/PipeServer.h` | service header | event-driven | `MoqLauncher/PipeServer.h` | exact |
| `MoqLauncher/PipeSecurity.cpp` | security utility | request-response | `MoqLauncher/PipeSecurity.cpp` | exact |
| `MoqLauncher/PipeSecurity.h` | security utility header | request-response | `MoqLauncher/PipeSecurity.h` | exact |
| `MoqLauncher/PipeClient.cpp` | service | streaming + request-response | `MoqLauncher/PipeClient.cpp` | exact |
| `MoqLauncher/PipeClient.h` | service header | streaming | `MoqLauncher/PipeClient.h` | exact |
| `MoqLauncher/BackendServer.cpp` | service | streaming + event-driven | `MoqLauncher/BackendServer.cpp` | exact |
| `MoqLauncher/BackendServer.h` | service header | streaming | `MoqLauncher/BackendServer.h` | exact |
| `MoqLauncher/Utils.cpp` | utility | file-I/O | `MoqLauncher/PipeServer.cpp` path/config helpers | role-match |
| `MoqLauncher/TypeDuckPreferences.*` | model/service | file-I/O | `MoqLauncher/TypeDuckPreferences.cpp` via Phase 5 guards | role-match |
| `MoqiTextService/MoqiClient.cpp` | service | request-response + streaming | `MoqiTextService/MoqiClient.cpp` | exact |
| `MoqiTextService/MoqiClient.h` | service header | request-response | `MoqiTextService/MoqiClient.h` | exact |
| `MoqiTextService/MoqiImeModule.cpp` | service/module | request-response | `MoqiTextService/MoqiImeModule.cpp` | exact |
| `MoqiTextService/TsfLog.cpp` | utility | file-I/O | `MoqiTextService/TsfLog.cpp` | exact |
| `libIME2/src/DebugLogConfig.cpp` | utility | file-I/O | `libIME2/src/DebugLogConfig.cpp` | exact |
| `proto/ProtoFraming.h` | utility | streaming transform | `proto/ProtoFraming.h` | exact |
| `proto/moqi.proto` | protocol model | request-response | `scripts/Test-TypeDuckProtocolContract.ps1` | role-match |
| `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1` (new) | test/guard | batch + transform | `scripts/Test-TypeDuckIconPackaging.ps1` | role-match |
| `scripts/Test-TypeDuckRuntimePackagePruning.ps1` (new) | test/guard | file-I/O + batch | `scripts/Test-TypeDuckIconPackaging.ps1` | role-match |
| `Tests/TypeDuckProtocolFraming/*` (possible new) | test | streaming transform | `proto/ProtoFraming.h` + `libIME2/test/*` | role-match |
| `D:\VSProjects\moqi-ime\scripts\build.ps1` | sibling packaging script | file-I/O + batch | `D:\VSProjects\moqi-ime\scripts\build.ps1` | exact |
| `D:\VSProjects\moqi-ime\input_methods\rime\*cloud*`, `cloudclipboard/**`, `ai_*`, `ai_config.json`, `templates/**`, `test/**`, `android/**`, `icons/**`, `ime.json` | sibling runtime assets/services | file-I/O + event-driven | `D:\VSProjects\moqi-ime\scripts\build.ps1` package filters | role-match |
| `D:\VSProjects\moqi-ime\server.go` | backend service | streaming + request-response | `D:\VSProjects\moqi-ime\server.go` | exact |
| `D:\VSProjects\moqi-ime\server_test.go` | backend test | file-I/O + transform | `D:\VSProjects\moqi-ime\server_test.go` | exact |
| `D:\VSProjects\moqi-ime\server_integration_test.go` | backend integration test | request-response | `D:\VSProjects\moqi-ime\server_integration_test.go` | exact |
| `D:\VSProjects\moqi-ime\input_methods\rime\rime.go` | backend IME service | event-driven + request-response | `D:\VSProjects\moqi-ime\input_methods\rime\rime.go` | exact |
| `D:\VSProjects\moqi-ime\input_methods\rime\debug_logging.go` | backend diagnostic utility | file-I/O | `libIME2/src/DebugLogConfig.cpp` | role-match |
| `D:\VSProjects\moqi-ime\input_methods\rime\appearance_config.go` | backend config service | file-I/O + request-response | `D:\VSProjects\moqi-ime\input_methods\rime\appearance_config.go` | exact |
| `D:\VSProjects\moqi-ime\input_methods\rime\appearance_themes.go` | backend config service | file-I/O | `D:\VSProjects\moqi-ime\input_methods\rime\appearance_themes.go` | exact |
| `D:\VSProjects\moqi-ime\input_methods\rime\config_update.go` | backend config service | event-driven | `D:\VSProjects\moqi-ime\input_methods\rime\config_update.go` | exact |
| `D:\VSProjects\moqi-ime\input_methods\rime\ai_client.go` | backend service | network request-response | `D:\VSProjects\moqi-ime\input_methods\rime\ai_client.go` | exact stale-feature target |
| `D:\VSProjects\moqi-ime\input_methods\rime\ai_config.go` | backend config service | file-I/O + environment | `D:\VSProjects\moqi-ime\input_methods\rime\ai_config.go` | exact stale-feature target |
| `D:\VSProjects\moqi-ime\input_methods\rime\cloud_clipboard*.go` and `cloudclipboard/**` | backend service/config | network + file-I/O + event-driven | `D:\VSProjects\moqi-ime\input_methods\rime\cloud_clipboard.go` | exact stale-feature target |
| `D:\VSProjects\moqi-ime\input_methods\rime\scheme_set_download.go` | backend service | network + file-I/O + event-driven | `D:\VSProjects\moqi-ime\input_methods\rime\scheme_set_download.go` | exact stale-feature target |
| `D:\VSProjects\moqi-ime\input_methods\rime\auto_pair_symbols.go` | backend config service | file-I/O | `D:\VSProjects\moqi-ime\input_methods\rime\auto_pair_symbols.go` | exact stale-feature/gating target |
| `D:\VSProjects\moqi-ime\input_methods\rime\custom_phrase.go` | backend config service | file-I/O + transform | `D:\VSProjects\moqi-ime\input_methods\rime\custom_phrase.go` | exact stale-feature/gating target |

## Pattern Assignments

### `installer/MoqiTsf.iss` (installer config, batch)

**Analog:** `installer/MoqiTsf.iss`

**Identity and language pattern** (lines 5-50):
```iss
#define MyAppName "TypeDuck 粵語輸入法 / TypeDuck Cantonese IME"
#define MyAppPublisher "香港教育大學 The Education University of Hong Kong"
DefaultDirName={autopf32}\TypeDuckIME
OutputBaseFilename=typeduck-windows-ime-setup
SetupIconFile=..\TypeDuckSettings\assets\TypeDuck.ico

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "chinesetraditional"; MessagesFile: "compiler:Default.isl"
```

**Planner note:** Phase 6 must replace the current `compiler:Default.isl` fallback for `chinesetraditional` with product-owned bilingual installer messages, set the publisher to `香港教育大學 The Education University of Hong Kong`, and remove the Simplified submodule instead of using official Simplified chrome.

**Start Menu and install launch pattern** (lines 55-64):
```iss
[Icons]
Name: "{autoprograms}\{#MyAppName}\TypeDuck About"; Filename: "{app}\TypeDuckAbout.exe"
Name: "{autoprograms}\{#MyAppName}\TypeDuck 關於"; Filename: "{app}\TypeDuckAbout.exe"
Name: "{autoprograms}\{#MyAppName}\解除安裝 / Uninstall"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\TypeDuckSettings.exe"; Parameters: "/apply-defaults"; Flags: runhidden waituntilterminated; Check: ShouldSeedDefaultSettings
Filename: "{app}\TypeDuckSettings.exe"; Description: "開啟 TypeDuck 設定 / Open TypeDuck Settings"; Flags: postinstall nowait skipifsilent; Check: ShouldLaunchSettings
```

**Planner note:** Update to the locked Phase 6 names: Start Menu folder `TypeDuckIME`, entries `輸入法設定 IME Settings`, `關於 About TypeDuck…`, `解除安裝 Uninstall`. Avoid slash-delimited shortcut names.

**Process cleanup and uninstall pattern** (lines 141-152, 275-301):
```iss
procedure TryKillProcessImage(const ImageName: String);
begin
  Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /T /IM "' + ImageName + '"',
    '', SW_HIDE, ewWaitUntilTerminated, R);
end;

procedure StopTypeDuckProcesses;
begin
  TryKillProcessImage('TypeDuckLauncher.exe');
end;
```

```iss
if CurUninstallStep = usUninstall then
begin
  StopTypeDuckProcesses;
  DeleteTypeDuckReregisterTask;
  if not RunSetupHelper(BuildUninstallSetupHelperParameters('/u'), ResultCode) then
    HandleSetupHelperResult('TypeDuck 安裝工具解除安裝', 'TypeDuck setup-helper uninstall', ResultCode);
end;
```

**Use this for:** automated TypeDuck-specific process cleanup. Extend only with allowlisted TypeDuck process names. Do not kill, inspect, or repair Legacy Moqi processes; TypeDuck and Moqi must coexist.

**Final-page restart guidance note:** installer/uninstaller restart wording should be placed on the final wizard page rather than a popup. It must be plain bilingual user text with no `TSF`, `DLL`, `COM`, registration, or internal implementation terms. Suggest reopening apps first and restarting Windows only when necessary.

### `.gitmodules` and `installer/Inno-Setup-Chinese-Simplified-Translation/**` (config/vendor removal, batch)

**Analog:** `.gitmodules`

**Submodule pattern to remove** (lines 10-12):
```gitconfig
[submodule "installer/Inno-Setup-Chinese-Simplified-Translation"]
	path = installer/Inno-Setup-Chinese-Simplified-Translation
	url = https://github.com/kira-96/Inno-Setup-Chinese-Simplified-Translation
```

**Use this for:** delete the submodule entry and remove the vendored directory from the working tree/package surface. Add a guard that fails if this path, `ChineseSimplified.isl`, or `chinesesimplified` appears under installer/product surfaces.

### `SetupHelper/SetupHelper.cpp` (utility, file-I/O + request-response)

**Analog:** `SetupHelper/SetupHelper.cpp`

**Imports and constants pattern** (lines 1-22):
```cpp
#include <windows.h>
#include <shellapi.h>
#include <filesystem>
#include <string>
#include <vector>

constexpr int kExitSuccess = 0;
constexpr int kExitFailure = 1;
constexpr int kExitRestartRequired = 2;
constexpr wchar_t kProgramDirEnvVar[] = L"TYPEDUCK_PROGRAM_DIR";
constexpr wchar_t kReregisterTaskName[] = L"TypeDuckIME-ReRegisterTSF";
constexpr wchar_t kTextServiceDllName[] = L"TypeDuckTextService.dll";
```

**Bilingual user failure pattern** (lines 655-685):
```cpp
int ShowFailureAndReturn(const std::wstring& message, const bool silent) {
  ShowMessage(message, kSetupHelperCaption, MB_ICONERROR | MB_OK, silent);
  return kExitFailure;
}

if (!RunRegsvr(regsvr32, dest32, app_dir, false)) {
  return ShowFailureAndReturn(
      Bilingual(L"未能註冊 Win32 TypeDuck TSF DLL。",
                L"Failed to register Win32 TypeDuck TSF DLL."),
      options.silent);
}
```

**Registration/copy pattern** (lines 692-783):
```cpp
const fs::path source32 = app_dir / kTextServiceDllName;
const fs::path source64 = app_dir / L"x64" / kTextServiceDllName;
const fs::path dest32 = fs::path(GetSyswow64DirectoryPath()) / kTextServiceDllName;
const fs::path dest64 = fs::path(GetNativeSystemDirectoryPath()) / kTextServiceDllName;

bool reboot_required = false;
if (!CopyFileWithFallback(source32, dest32, reboot_required, &copy_error,
                          &initial_copy_error, &fallback_error)) {
  if (!((initial_copy_error == ERROR_SHARING_VIOLATION ||
         initial_copy_error == ERROR_ACCESS_DENIED ||
         fallback_error == ERROR_SHARING_VIOLATION ||
         fallback_error == ERROR_ACCESS_DENIED) &&
        ScheduleReplaceOnReboot(source32, dest32, reboot_required,
                                &copy_error))) {
    return ShowFailureAndReturn(...);
  }
}
```

**CLI parsing pattern** (lines 817-912):
```cpp
const std::wstring help_text =
    L"用法 / Usage: TypeDuckSetupHelper.exe /i|/r|/u [/s] [--appdir <path>]\n"
    L"  /i       安裝或更新 TypeDuck TSF DLLs / Install or upgrade the TypeDuck TSF DLLs.\n";

if (!ParseOptions(args, options, error)) {
  ShowMessage(error, kSetupHelperCaption, MB_ICONERROR | MB_OK, false);
  ShowUsage();
  return kExitInvalidArgs;
}
if (!IsRunningAsAdmin()) {
  return RestartElevated(args, options.silent);
}
```

**Use this for:** keep setup-helper HRESULT/Win32 exit-code structure. For Phase 6, move technical details into logs where user action is not needed; keep only bounded bilingual user dialogs.

### `scripts/install.ps1` (packaging script, file-I/O + batch)

**Analog:** `scripts/install.ps1`

**Script structure pattern** (lines 34-52):
```powershell
param(
    [string] $RepoRoot = "",
    [string] $Win32BuildDir = "",
    [string] $X64BuildDir = "",
    [string] $MoqiImeSource = "",
    [switch] $SkipMoqiImeCopy,
    [string] $StageDir = "",
    [string] $IssPath = ""
)

$ErrorActionPreference = "Stop"
```

**Runtime copy filter pattern to extend** (lines 279-324):
```powershell
function Copy-MoqiImeRuntime {
    param(
        [string] $SourceRoot,
        [string] $DestinationRoot
    )

    $bannedLegacyIconNames = @(
        "moqi.png",
        "mo.ico",
        "mo.png",
        "moqi.ico",
        "About_Banner.bmp",
        "Credit_Logos.bmp",
        "Installer.bmp"
    )
    $files = Get-ChildItem -Path $SourceRoot -Recurse -Force -File | Where-Object {
        $relativePath = $_.FullName.Substring($SourceRoot.Length).TrimStart('\', '/')
        $_.Extension -ne ".go" -and
        $_.FullName -notmatch '[\\/]\.git(?:[\\/]|$)' -and
        $relativePath -notmatch '^input_methods[\\/]rime[\\/]icon\.ico$' -and
        ($bannedLegacyIconNames -notcontains $_.Name.ToLowerInvariant())
    }
}
```

**Staging pattern** (lines 341-437):
```powershell
$stageWin32Root = Join-Path $StageDir "win32\TypeDuckIME"
$stageResourceRoot = Join-Path $stageWin32Root "resources"
$transparentIcon = Join-Path $iconSourceRoot "TypeDuck_Transparent.ico"

$backends = Join-Path $RepoRoot "backends.json"
Copy-Item -LiteralPath $backends -Destination (Join-Path $stageWin32Root "backends.json") -Force

$settingsExe = Resolve-ArtifactPath -Label "TypeDuckSettings.exe" -Candidates @(...)
Copy-IfExists -Source $settingsExe -Destination (Join-Path $stageWin32Root "TypeDuckSettings.exe")

$imeDest = Join-Path $stageWin32Root "moqi-ime"
Copy-MoqiImeRuntime -SourceRoot $MoqiImeSource -DestinationRoot $imeDest
```

**Planner note:** Replace `backends.json` staging with in-code backend registration and rename `$imeDest` to a TypeDuck-owned runtime folder. Extend filters for `android`, `cloudclipboard`, `templates`, `test`, `ai_config.json`, `ime.json`, duplicate icon folders, and `input_methods/rime/data/appearance_themes.json`.

### `scripts/_all_in_package.ps1` (packaging script, batch)

**Analog:** `scripts/_all_in_package.ps1`

**pwsh invocation pattern** (lines 124-160):
```powershell
Invoke-Step -FilePath "pwsh" -ArgumentList @(
    "-NoProfile",
    "-ExecutionPolicy", "Bypass",
    "-File", "`"$moqiImeBuildScript`"",
    "-RepoRoot", "`"$MoqiImeRoot`""
) -WorkingDirectory $MoqiImeRoot

Invoke-Step -FilePath "pwsh" -ArgumentList $windowsBuildArgs -WorkingDirectory $RepoRoot

Invoke-Step -FilePath "pwsh" -ArgumentList @(
    "-NoProfile",
    "-ExecutionPolicy", "Bypass",
    "-File", "`"$windowsInstallScript`"",
    "-RepoRoot", "`"$RepoRoot`"",
    "-MoqiImeSource", "`"$moqiImeRuntimeDir`""
) -WorkingDirectory $RepoRoot
```

**Use this for:** package-time guard integration. Keep `pwsh -NoProfile -ExecutionPolicy Bypass -File ...` style and fail hard on guard failures before installer build success is reported.

### `backends.json` removal / inlining (config, request-response)

**Analog:** `backends.json`

**Current manifest to replace** (lines 1-9):
```json
[
    {
        "name": "typeduck-runtime-bridge",
        "displayName": "TypeDuck Runtime Bridge",
        "command": "moqi-ime\\server.exe",
        "workingDir": "moqi-ime",
        "params": ""
    }
]
```

**Consumers:** `MoqLauncher/PipeServer.cpp` lines 178-185 and `MoqiTextService/MoqiImeModule.cpp` lines 81-90.

**Planner note:** Inline a single TypeDuck-owned backend entry in code and delete staged `backends.json`. Keep name `typeduck-runtime-bridge` if useful internally, but change `command`/`workingDir` away from `moqi-ime`.

### `MoqLauncher/PipeServer.cpp` / `.h` (service, event-driven + request-response)

**Analog:** `MoqLauncher/PipeServer.cpp`

**Current constants and legacy names to replace** (lines 55-89):
```cpp
PipeServer *PipeServer::singleton_ = nullptr;
wchar_t PipeServer::singleInstanceMutexName_[] = L"MoqiLauncherMutex";
static constexpr char kTypeDuckBackendBridgeName[] = "typeduck-runtime-bridge";
static constexpr UINT ID_RESTART_Moqi_BACKENDS = 1002;
static constexpr UINT ID_EXIT_Moqi = 1003;
static constexpr wchar_t CONFIG_FILE_REL_PATH[] = L"\\MoqiLauncher.json";
static constexpr wchar_t TYPEDUCK_SETTINGS_EXE[] = L"\\TypeDuckSettings.exe";
```

**Data/log path pattern to replace** (lines 136-175):
```cpp
void PipeServer::initDataDir() {
  dataDirPath_ = getAppLocalDir() + L"\\MoqiIM";
  makeDirs(dataDirPath_);
}

auto logFile = Ime::DebugLogFile::prepareDailyLogFilePath(
    logDirPath, L"MoqiLauncher.log");
logger_ = spdlog::rotating_logger_mt("MoqiLauncher", logFile,
                                     MAX_LOG_FILE_SIZE, NUM_LOG_FILES);
```

**Fixed first-party launch pattern** (lines 399-423):
```cpp
std::wstring settingsPath = topDirPath_ + TYPEDUCK_SETTINGS_EXE;
HINSTANCE result = ::ShellExecuteW(hwnd_, L"open", settingsPath.c_str(),
                                   nullptr, topDirPath_.c_str(),
                                   SW_SHOWNORMAL);
if (reinterpret_cast<INT_PTR>(result) <= 32) {
  ::MessageBoxW(hwnd_,
      L"未能開啟 TypeDuck 設定。請確認 TypeDuckSettings.exe 已安裝。\n"
      L"Unable to open TypeDuck Settings. Please confirm TypeDuckSettings.exe is installed.",
      L"TypeDuck 設定 / TypeDuck Settings", MB_OK | MB_ICONWARNING);
}
```

**Pipe namespace pattern to replace** (lines 449-461):
```cpp
pipeName = L"\\\\.\\pipe\\";
pipeName += username;
pipeName += L"\\MoqiIM\\";
pipeName += baseName;
```

**Tray menu pattern** (lines 787-804):
```cpp
::AppendMenu(hmenu, MF_STRING | MF_ENABLED, ID_OPEN_TYPEDUCK_SETTINGS,
             L"輸入法設定 / IME Settings");
::AppendMenu(hmenu, MF_STRING | MF_ENABLED, ID_OPEN_TYPEDUCK_ABOUT,
             L"關於 / About TypeDuck…");
::AppendMenu(hmenu, MF_STRING | MF_ENABLED, ID_RESTART_Moqi_BACKENDS,
             L"重新啟動 TypeDuck 引擎 / Restart TypeDuck Engine");
```

**Cloud clipboard removal target** (lines 812-819, 912-918):
```cpp
bool PipeServer::isCloudClipboardEnabled() const {
  const auto configPath = getMoqiAppDataDir() + L"\\cloud_clipboard.json";
  Json::Value config;
  if (!loadJsonFile(configPath, config)) {
    return false;
  }
  return config.get("enabled", false).asBool();
}
```

```cpp
if (::AddClipboardFormatListener(hwnd_)) {
  clipboardListenerRegistered_ = true;
  initClipboardAsync();
  logger_->info("Cloud clipboard listener enabled");
}
```

**Planner note:** remove listener/async/upload paths entirely for v1. Rename data dir to `%LOCALAPPDATA%\TypeDuckIME`, config to `TypeDuckLauncher.json`, logger name/file to TypeDuck-owned names, and pipe namespace/mutex/window IDs to TypeDuck-owned names without changing internal C++ class names.

### `MoqLauncher/PipeSecurity.cpp` / `.h` (security utility, request-response)

**Analog:** `MoqLauncher/PipeSecurity.cpp`

**Imports and SID lookup pattern** (lines 1-10, 26-68):
```cpp
#include "PipeSecurity.h"
#include <AclAPI.h>
#include <sddl.h>
#include <VersionHelpers.h>

#define PROCESS_ALL_ACCESS_HEX  TEXT("0x1fffff")
```

```cpp
static std::wstring getLogonSid() {
    HANDLE processToken = INVALID_HANDLE_VALUE;
    try {
        if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &processToken)) {
            throw std::runtime_error("Fail to open process token");
        }
        ...
    }
    catch (...) {
        if (processToken != INVALID_HANDLE_VALUE) {
            ::CloseHandle(processToken);
        }
    }
    return logonSidStr;
}
```

**ACL pattern to harden** (lines 123-140):
```cpp
if (::IsWindows8OrGreater()) {
    securityDescriptorStr += L"(" SDDL_ACCESS_DENIED L";;" SDDL_GENERIC_ALL L";;;" SDDL_NETWORK L")";
    securityDescriptorStr += L"(" SDDL_ACCESS_ALLOWED L";;" SDDL_GENERIC_ALL L";;;" SDDL_LOCAL_SYSTEM L")";
    securityDescriptorStr += L"(" SDDL_ACCESS_ALLOWED L";;" SDDL_GENERIC_ALL L";;;" SDDL_BUILTIN_ADMINISTRATORS L")";
    securityDescriptorStr += L"(" SDDL_ACCESS_ALLOWED L";;" SDDL_GENERIC_ALL L";;;" SDDL_ALL_APP_PACKAGES L")";
}
securityDescriptorStr += L"(" SDDL_ACCESS_ALLOWED L";;" PROCESS_ALL_ACCESS_HEX L";;;" + logonSid + L")";
```

**Planner note:** Replace broad `GENERIC_ALL`/`PROCESS_ALL_ACCESS_HEX` rights with explicit named-pipe read/write/connect rights while preserving app-container compatibility and same-logon usability.

### `MoqLauncher/PipeClient.cpp` / `.h` (service, streaming + request-response)

**Analog:** `MoqLauncher/PipeClient.cpp`

**Frame parse/error pattern** (lines 180-219):
```cpp
readBuffer_.append(readBuf, len);
if (readBuffer_.hasViolation()) {
    handleClientFrameViolation();
    return;
}

std::string payload;
while (readBuffer_.nextFrame(payload)) {
    moqi::protocol::ClientRequest request;
    if (!Proto::parsePayload(payload, request)) {
        logger()->error("Failed to parse protobuf request from client {}", clientId_);
        writeTypeDuckErrorResponse(
            0,
            moqi::protocol::TYPEDUCK_ERROR_MALFORMED_PAYLOAD,
            "Malformed TypeDuck client protobuf payload",
            moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
            true);
        continue;
    }
}
```

**Bounded error response pattern** (lines 258-296):
```cpp
moqi::protocol::ServerResponse response;
response.set_client_id(clientId_);
response.set_seq_num(responseSeqNum);
response.set_success(false);
response.set_error(message);

auto* health = response.mutable_typeduck_engine_health();
health->set_status(healthStatus);
health->set_message(message);
health->set_recoverable(recoverable);

auto* error = response.mutable_typeduck_error();
error->set_code(errorCode);
error->set_message(message);
error->set_recoverable(recoverable);
error->set_detail(detail);
```

**Violation handling pattern** (lines 299-320):
```cpp
const auto frameError = readBuffer_.lastError();
const auto errorCode =
    frameError == Proto::FrameError::PayloadTooLarge
        ? moqi::protocol::TYPEDUCK_ERROR_PAYLOAD_TOO_LARGE
        : moqi::protocol::TYPEDUCK_ERROR_MALFORMED_FRAME;
logger()->error("Rejecting client {} frame: {}", clientId_, detail);
readBuffer_.clear();
writeTypeDuckErrorResponse(...);
```

**Use this for:** all new malformed-frame and bounded diagnostic behavior.

### `MoqLauncher/BackendServer.cpp` / `.h` (service, streaming + event-driven)

**Analog:** `MoqLauncher/BackendServer.cpp`

**Backend stdout frame violation pattern** (lines 343-364):
```cpp
stdoutFrameBuf_.append(buf, len);
if (stdoutFrameBuf_.hasViolation()) {
  const auto frameError = stdoutFrameBuf_.lastError();
  pipeServer_->notifyClientsOfBackendError(
      this,
      frameError == Proto::FrameError::PayloadTooLarge
          ? moqi::protocol::TYPEDUCK_ERROR_PAYLOAD_TOO_LARGE
          : moqi::protocol::TYPEDUCK_ERROR_BACKEND_RESTART,
      frameError == Proto::FrameError::PayloadTooLarge
          ? "TypeDuck backend stdout frame exceeds launcher payload limit"
          : "Malformed TypeDuck backend stdout frame",
      moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
      true,
      frameError == Proto::FrameError::MalformedHeader
          ? "FrameError::MalformedHeader"
          : "FrameError::PayloadTooLarge");
  stdoutFrameBuf_.clear();
  restartProcess();
  return;
}
```

**Parse/restart pattern** (lines 421-436):
```cpp
while (stdoutFrameBuf_.nextFrame(payload)) {
  moqi::protocol::ServerResponse response;
  if (!Proto::parsePayload(payload, response)) {
    logger()->error("Failed to parse protobuf response from backend {}", name_);
    pipeServer_->notifyClientsOfBackendError(
        this,
        moqi::protocol::TYPEDUCK_ERROR_BACKEND_RESTART,
        "Malformed TypeDuck backend protobuf response",
        moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
        true,
        "FrameError::MalformedHeader");
    stdoutFrameBuf_.clear();
    restartProcess();
    return;
  }
}
```

**Planner note:** Remove `cloud_clipboard` request/upload code paths and any logs that expose raw backend stderr content if it may contain typed text. Keep bounded TypeDuck error messages.

### `MoqiTextService/MoqiClient.cpp` / `.h` (service, request-response + streaming)

**Analog:** `MoqiTextService/MoqiClient.cpp`

**Current log path to replace** (lines 68-97, 135-150):
```cpp
std::wstring rpcGuardLogPath() {
  const wchar_t *localAppData = _wgetenv(L"LOCALAPPDATA");
  if (!localAppData || !*localAppData) {
    return L"";
  }
  return std::wstring(localAppData) + L"\\MoqiIM\\Log\\tsf-debug.log";
}

void appendRpcGuardLog(const std::wstring &message) {
  if (!Ime::isTraceLoggingEnabled()) {
    return;
  }
  ...
  stream << L"[" << timestamp << L"] " << message << L"\n";
}
```

**Failure reset pattern** (lines 765-778):
```cpp
appendRpcGuardLog(hasTypeduckFailure
                    ? L"[TypeDuck degraded] resetting TSF client state"
                    : L"[RPC failure] resetting TSF client state");
markRpcDegraded(hasTypeduckFailure ? L"typeDuckFailure" : L"rpcFailure");
closeRpcConnection();
resetTextServiceState();
if (textService_ != nullptr) {
  textService_->resetTypeDuckDegradedState(session);
}
return false;
```

**Bounded RPC call pattern** (lines 1876-1925):
```cpp
if (!Proto::serializeMessageBounded(
        request, serializedRequest, Proto::kMaxClientFramePayloadBytes)) {
  return false;
}

if (callRpcPipe(pipe_, serializedRequest, serializedResponse)) {
  while (true) {
    Proto::FrameBuffer responseBuffer{Proto::kMaxClientFramePayloadBytes};
    responseBuffer.append(serializedResponse.data(), serializedResponse.size());
    if (responseBuffer.hasViolation()) {
      success = false;
      break;
    }
    ...
    success = responseBuffer.nextFrame(payload) &&
              Proto::parsePayload(payload, protoResponse);
  }
}
```

**Server identity gap** (lines 1956-1965):
```cpp
bool Client::isPipeCreatedByMoqiServer(HANDLE pipe) {
  ULONG serverPid;
  if (GetNamedPipeServerProcessId(pipe, &serverPid)) {
    // FIXME: check the command line of the server?
  }
  return true;
}
```

**Launcher/pipe path pattern to replace** (lines 1990-2012, 2115-2125):
```cpp
std::wstring launcherPath = module->programDir();
launcherPath += L"\\TypeDuckLauncher.exe";
HINSTANCE result =
    ::ShellExecuteW(nullptr, L"open", launcherPath.c_str(), nullptr,
                    module->programDir().c_str(), SW_SHOWNORMAL);
```

```cpp
pipeName += username.get();
pipeName += L"\\MoqiIM\\";
pipeName += base_name;
```

**Planner note:** Harden identity compatibility-tolerantly: verify owner/path sanity and TypeDuck-owned namespace, not strict version/signature checks. Rename pipe namespace and logs to TypeDuck. Keep degraded reset semantics.

### `MoqiTextService/MoqiImeModule.cpp` (service/module, request-response)

**Analog:** `MoqiTextService/MoqiImeModule.cpp`

**Fixed settings launch pattern** (lines 38-79, 162-176):
```cpp
constexpr const wchar_t* kTypeDuckSettingsExecutable = L"TypeDuckSettings.exe";

std::wstring buildTypeDuckSettingsPath(const std::wstring& programDir) {
  if (programDir.empty()) {
    return kTypeDuckSettingsExecutable;
  }
  std::wstring settingsPath = programDir;
  settingsPath += L"\\";
  settingsPath += kTypeDuckSettingsExecutable;
  return settingsPath;
}

bool launchTypeDuckSettings(HWND hwndParent, const std::wstring& programDir) {
  const std::wstring settingsPath = buildTypeDuckSettingsPath(programDir);
  const HINSTANCE result = ::ShellExecuteW(
      hwndParent, L"open", settingsPath.c_str(), nullptr,
      programDir.empty() ? nullptr : programDir.c_str(), SW_SHOWNORMAL);
  return reinterpret_cast<INT_PTR>(result) > 32;
}
```

```cpp
bool ImeModule::onConfigure(HWND hwndParent, LANGID langid,
                            REFGUID rguidProfile) {
  (void)langid;
  (void)rguidProfile;

  if (!launchTypeDuckSettings(hwndParent, programDir_)) {
    ::MessageBoxW(...);
    return false;
  }
  return true;
}
```

**Backend manifest reader to remove/replace** (lines 81-90):
```cpp
void loadBackendDirs(const std::wstring& programDir,
                     std::vector<std::wstring>& backendDirs) {
  backendDirs.clear();
  std::ifstream fp(programDir + L"\\backends.json", std::ifstream::binary);
  if (!fp) {
    return;
  }
```

**Use this for:** preserve fixed first-party settings launch and delete arbitrary backend `configTool` launch assumptions. Replace backend-dir discovery with a single TypeDuck-owned runtime dir constant.

### `MoqiTextService/TsfLog.cpp` and `libIME2/src/DebugLogConfig.cpp` (utility, file-I/O)

**Analogs:** `MoqiTextService/TsfLog.cpp`, `libIME2/src/DebugLogConfig.cpp`

**TSF log file pattern to rename** (`MoqiTextService/TsfLog.cpp` lines 12-21):
```cpp
std::wstring logFilePath() {
    wchar_t localAppData[MAX_PATH] = {0};
    if (::ExpandEnvironmentStringsW(L"%LOCALAPPDATA%", localAppData, MAX_PATH) == 0) {
        return L"";
    }
    std::wstring baseDir = std::wstring(localAppData) + L"\\MoqiIM";
    std::wstring logDir = baseDir + L"\\Log";
    ::CreateDirectoryW(baseDir.c_str(), nullptr);
    ::CreateDirectoryW(logDir.c_str(), nullptr);
    return logDir + L"\\tsf-debug.log";
}
```

**Debug config path to rename** (`libIME2/src/DebugLogConfig.cpp` lines 19-25):
```cpp
std::wstring configFilePath() {
    const wchar_t* localAppData = _wgetenv(L"LOCALAPPDATA");
    if (!localAppData || !*localAppData) {
        return L"";
    }
    return std::wstring(localAppData) + L"\\MoqiIM\\MoqiLauncher.json";
}
```

**Planner note:** rename to `%LOCALAPPDATA%\TypeDuckIME\Log\...` and `TypeDuckLauncher.json`. Keep opt-in caching behavior from `isDebugLoggingEnabled()` / `isTraceLoggingEnabled()`.

### `proto/ProtoFraming.h` (utility, streaming transform)

**Analog:** `proto/ProtoFraming.h`

**Frame bound pattern** (lines 11-21, 23-40):
```cpp
inline constexpr std::size_t kMaxClientFramePayloadBytes = 1024 * 1024;
inline constexpr std::size_t kMaxBackendFramePayloadBytes = 1024 * 1024;

enum class FrameError {
  None,
  Incomplete,
  PayloadTooLarge,
  MalformedHeader,
};

class FrameBuffer {
public:
  explicit FrameBuffer(
      std::size_t maxPayloadBytes = kDefaultMaxFramePayloadBytes)
      : maxPayloadBytes_(maxPayloadBytes) {}

  void append(const char *data, size_t len) {
    if (len > maxBufferedBytes() || buffer_.size() > maxBufferedBytes() - len) {
      fail(FrameError::PayloadTooLarge);
      return;
    }
```

**Parse/serialize pattern** (lines 57-75, 110-148, 161-167):
```cpp
if (payloadSize > maxPayloadBytes_) {
  fail(FrameError::PayloadTooLarge);
  return false;
}
payload.assign(buffer_.data() + sizeof(payloadSize), payloadSize);
buffer_.erase(0, sizeof(payloadSize) + payloadSize);
```

```cpp
inline bool tryFramePayload(const std::string &payload,
                            std::string &framedPayload,
                            std::size_t maxPayloadBytes =
                                kDefaultMaxFramePayloadBytes) {
  framedPayload.clear();
  if (payload.size() > maxPayloadBytes ||
      payload.size() >
          static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)())) {
    return false;
  }
```

**Use this for:** frame-size hardening and tests. Add test coverage rather than replacing this helper.

### `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1` (new guard, batch + transform)

**Analog:** `scripts/Test-TypeDuckIconPackaging.ps1`

**Guard structure pattern** (lines 1-49):
```powershell
param(
  [string] $RepoRoot = ".",
  [string] $BackendRoot = "D:\VSProjects\moqi-ime",
  [switch] $Strict,
  [ValidateSet("", "RejectedUatBehavior")]
  [string] $ExpectRed = ""
)

$ErrorActionPreference = "Stop"

function Assert-True([bool] $Condition, [string] $Message) {
  if (-not $Condition) {
    throw $Message
  }
}

function Assert-Text([string] $Text, [string] $Pattern, [string] $Message) {
  if ($Text -notmatch $Pattern) {
    throw $Message
  }
}

function Assert-NotText([string] $Text, [string] $Pattern, [string] $Message) {
  if ($Text -match $Pattern) {
    throw $Message
  }
}
```

**Stage scan pattern** (lines 235-271):
```powershell
$stageRoot = Join-Path $repo "installer/stage/win32/TypeDuckIME"
$rejectedBehavior = [System.Collections.Generic.List[string]]::new()
if (Test-Path -LiteralPath $stageRoot -PathType Container) {
  $rawStageIcons = @(Get-ChildItem -LiteralPath $stageRoot -File -Filter "*.ico" -ErrorAction SilentlyContinue)
  foreach ($icon in $rawStageIcons) {
    Add-Violation $rejectedBehavior "Raw standalone icon is staged under installed product root: $($icon.FullName)"
  }
}

Assert-True ($rejectedBehavior.Count -eq 0) "Rejected UAT icon packaging behavior found: $($rejectedBehavior -join '; ')"
```

**Use this for:** banned visible string scans. Guard must allow source identifiers but fail in product/user-facing surfaces for visible Moqi, fcitx, WebDAV/cloud clipboard, AI, Simplified-only installer strings, Simplified Chinese log/debug/printf strings, and Simplified Inno resources.

### `scripts/Test-TypeDuckRuntimePackagePruning.ps1` (new guard, file-I/O + batch)

**Analog:** `scripts/Test-TypeDuckIconPackaging.ps1`

**Package assertions pattern** (lines 257-286):
```powershell
if (Test-Path -LiteralPath $stageRoot -PathType Container) {
  foreach ($resourceName in @("About_Banner.bmp", "Credit_Logos.bmp", "Installer.bmp")) {
    Assert-True (Test-Path -LiteralPath (Join-Path $stageRoot "resources/$resourceName")) "Missing staged frontend resource: resources/$resourceName"
  }
  foreach ($bannedName in @("moqi.png", "mo.ico", "mo.png", "moqi.ico")) {
    $bannedStageFile = Get-ChildItem -LiteralPath $stageRoot -Recurse -Force -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Name -ieq $bannedName } |
    Select-Object -First 1
    Assert-True ($null -eq $bannedStageFile) "Banned legacy Moqi image was staged: $($bannedStageFile.FullName)"
  }
}
```

**Use this for:** fail if staged/packaged TypeDuck payload contains `input_methods\rime\android`, `cloudclipboard`, `templates`, `test`, `ai_config.json`, `ime.json`, duplicate icon folders, `input_methods\rime\data\appearance_themes.json`, or `backends.json`. Do not treat Legacy Moqi roaming filenames as TypeDuck package cleanup targets.

### `D:\VSProjects\moqi-ime\scripts\build.ps1` (sibling packaging script, file-I/O + batch)

**Analog:** `D:\VSProjects\moqi-ime\scripts\build.ps1`

**Script conventions** (lines 19-26, 44-50):
```powershell
param(
    [string] $RepoRoot = "",
    [string] $BuildRoot = "",
    [string] $PackageDir = "",
    [string] $RimeDataSource = ""
)

$ErrorActionPreference = "Stop"

function Remove-IfExists {
    param([string] $Path)
    if (Test-Path -LiteralPath $Path) {
        Remove-Item -LiteralPath $Path -Recurse -Force
    }
}
```

**Package layout pattern to change** (lines 212-234):
```powershell
if (-not $PackageDir) { $PackageDir = Join-Path $BuildRoot "moqi-ime" }
$ServerExe = Join-Path $PackageDir "server.exe"
$BackendSnippet = Join-Path $BuildRoot "backends.moqi-ime.json"
$PackageRimeDir = Join-Path $PackageDir "input_methods\rime"
$PackageRimeDataDir = Join-Path $PackageRimeDir "data"
$ServerIcon = Join-Path $IconsDir "TypeDuck_Transparent.ico"
```

**Existing prune pattern to extend** (rg hits around lines 303-355):
```powershell
Copy-DirectoryContents -Source $RimeDir -Destination (Join-Path $packageInputMethodsDir "rime")
Remove-IfExists -Path (Join-Path $PackageRimeDir "icon.ico")
...
$pathsToRemove = @(
    @{ Path = Join-Path $PackageDir "input_methods\rime\data\others"; Label = "rime shared data others directory" },
    @{ Path = Join-Path $PackageDir "input_methods\rime\icons\icons"; Label = "nested icons directory" }
)
...
$packagedGoFiles = Get-ChildItem -LiteralPath (Join-Path $PackageDir "input_methods\rime") -Filter "*.go" -File -ErrorAction SilentlyContinue
```

**Planner note:** rename default package dir to TypeDuck-owned runtime folder, stop emitting `backends.moqi-ime.json`, remove Moqi console text, remove `data\appearance_themes.json` compatibility copy, and prune the Phase 6 banned asset directories/files at source packaging time.

### Sibling runtime stale/off-scope files (runtime assets/services, file-I/O + event-driven)

**Analogs:** `D:\VSProjects\moqi-ime\input_methods\rime\ime.json`, `D:\VSProjects\moqi-ime\input_methods\rime\cloud_clipboard*.go`, `D:\VSProjects\moqi-ime\input_methods\rime\cloudclipboard\*.go`, `D:\VSProjects\moqi-ime\input_methods\rime\ai_*.go`, `D:\VSProjects\moqi-ime\input_methods\fcitx5\*`, `D:\VSProjects\moqi-ime\input_methods\moqi\*`

**Profile metadata still present** (`D:\VSProjects\moqi-ime\input_methods\rime\ime.json` lines 1-10):
```json
{
	"name": "TypeDuck 粵語輸入法 / TypeDuck Cantonese IME",
	"version": "0.1",
	"guid": "{C6E8F5DF-6504-44F9-B7CF-17A195373A83}",
	"locale": "zh-HK",
	"fallbackLocale": "zh-HK",
	"icon": "../../icons/TypeDuck.ico",
	"win8_icon": "",
	"moduleName": "rime",
	"serviceName": "RimeTextService"
}
```

**Sensitive config pointer:** `D:\VSProjects\moqi-ime\input_methods\rime\ai_config.json` contains off-scope AI action configuration and secret-like API config. Do not quote it into plans; delete or exclude it from v1 shipped package and add guards that fail on its presence.

**Use this for:** remove/cut off all shipped and callable paths for fcitx/Moqi input methods, cloud clipboard/WebDAV, AI, Android artifacts, templates, tests, duplicate icon folders, and runtime `ime.json` if profile authority is first-party in the Windows frontend.

## Backend Enrichment (D-27)

Phase 6 planning must treat the sibling backend as first-class scope. D-27 says most Simplified diagnostics live in `D:\VSProjects\moqi-ime`; do not assign diagnostics cleanup only to Windows frontend files.

### Backend Diagnostic String Hotspots

**Primary server diagnostics:** `D:\VSProjects\moqi-ime\server.go`

**Chinese log/error excerpts to translate to English** (lines 110-140, 180-198, 276-301, 382-489):
```go
log.Printf("注册输入法服务: %s", guid)
log.Println("Moqi Go 后端服务器已启动")
return fmt.Errorf("读取错误: %w", err)
log.Printf("处理消息错误: 缺少 client_id")
log.Printf("初始化失败 client=%s seq=%d 原因=缺少guid id=%q data=%s", clientID, req.SeqNum, req.ID.StringValue(), stringifyData(req.Data))
return &imecore.Response{SeqNum: req.SeqNum, Success: false, Error: fmt.Sprintf("未知的输入法: %s", guid)}
log.Printf("请求失败 client=%s seq=%d method=%s 原因=客户端未初始化", clientID, req.SeqNum, req.Method)
return fmt.Errorf("构建 protobuf 响应失败: %w", err)
candidates = append(candidates, filepath.Join(localAppData, "MoqiIM", "Log", "moqi-ime.log"))
log.Printf("无法创建日志文件，改用标准错误输出: %v", err)
log.Println("Moqi Go 后端启动")
```

**Rime diagnostics and UI/menu leakage:** `D:\VSProjects\moqi-ime\input_methods\rime\rime.go`

**Hotspots** (lines 22, 281-334, 394, 425-488, 682-684, 1017-1047, 1145-1157, 3193-3437, 3497-3502):
```go
APP         = "Moqi"
cfg, err := loadAIConfig()
if err := ime.reloadAIConfig(); err != nil {
traceLogf("RIME 输入法处理请求 client=%s seq=%d method=%s", ime.Client.ID, req.SeqNum, req.Method)
case "cloudClipboardUpload":
debugLogf("RIME 输入法已激活")
debugLogf("云剪贴板候选显示中，忽略非强制 composition terminated active=%t pending=%t", ime.cloudClipboardActive, ime.cloudClipboardPending)
ime.openPath(ime.userDir())
ime.openCloudClipboardSettingsAsync(resp)
debugLogf("RIME 输入法初始化完成 elapsed=%s firstRun=%t backendAvailable=%t", time.Since(initStart), firstRun, backendAvailable)
Tooltip:   "中英文切换",
Text: "设置",
map[string]interface{}{"id": ID_DOWNLOAD_SCHEME_SET, "text": "下载方案集(&D)"},
map[string]interface{}{"text": "输入设置", "submenu": []map[string]interface{}{
ime.cloudClipboardMenuSection(),
return filepath.Join(localAppData, "MoqiIM", "Log")
```

**Rime helper diagnostics:** `appearance_config.go`, `config_update.go`, `scheme_set_download.go`, `cloud_clipboard*.go`, `auto_pair_symbols.go`, `custom_phrase.go`.

**Representative excerpts**:
```go
// D:\VSProjects\moqi-ime\input_methods\rime\config_update.go lines 57-115
resp.TrayNotification = trayNotification("未检测到 Git 命令", imecore.TrayNotificationIconError)
resp.TrayNotification = trayNotification("当前方案集目录不是 Git 仓库", imecore.TrayNotificationIconError)
message := "更新配置失败"
return "请检查 Git 输出"
```

```go
// D:\VSProjects\moqi-ime\input_methods\rime\scheme_set_download.go lines 64-224
resp.TrayNotification = trayNotification("方案集下载已在进行中", imecore.TrayNotificationIconInfo)
log.Printf("下载方案集失败: %v", err)
return "", fmt.Errorf("创建墨奇用户目录失败: %w", err)
return nil, errors.New("下载内容过大")
return fmt.Errorf("方案集校验失败")
```

```go
// D:\VSProjects\moqi-ime\input_methods\rime\cloud_clipboard.go lines 31-82, 168-222, 294-490
Title:   "云剪贴板",
{"id": ID_WEBDAV_SETTINGS, "text": "WebDAV 配置..."},
debugLogf("云剪贴板快捷键触发 event=down key=%d char=%d hotkey=%s", req.KeyCode, req.CharCode, action.Hotkey)
resp.ShowMessage = &imecore.MessageWindow{Message: "请先在设置中开启云剪贴板", Duration: 2500}
resp.CompositionString = fmt.Sprintf("云剪贴板 %d/%d", ime.cloudClipboardPage+1, pageCount)
resp.ShowMessage = &imecore.MessageWindow{Message: "云剪贴板已清空", Duration: 2500}
```

```go
// D:\VSProjects\moqi-ime\input_methods\rime\auto_pair_symbols.go lines 14-61, 169-197
autoPairSymbolsFileName    = "moqi_auto_pair_symbols.txt"
builder.WriteString("# 成对符号\n")
log.Printf("加载成对符号配置失败: %v", err)
resp.TrayNotification = trayNotification("创建成对符号文件失败", imecore.TrayNotificationIconError)
```

```go
// D:\VSProjects\moqi-ime\input_methods\rime\custom_phrase.go lines 17-63, 133-139, 420-435
customPhraseFileName         = "moqi_custom_phrase.txt"
content = []byte("# 置顶短语\n# 词汇<Tab>编码<Tab>权重\n")
log.Printf("加载置顶短语失败: %v", err)
resp.TrayNotification = trayNotification("创建置顶短语文件失败", imecore.TrayNotificationIconError)
```

**Tests that must change with English diagnostics and TypeDuck paths:**
- `D:\VSProjects\moqi-ime\server_test.go` lines 98-127 currently assert `MoqiIM\Log` and `moqi-ime.log`.
- `D:\VSProjects\moqi-ime\server_integration_test.go` lines 145-152 currently assert `response.GetError() == "客户端未初始化"`.
- `D:\VSProjects\moqi-ime\input_methods\rime\rime_test.go` line 4362 asserts `rimeLogDir` uses `MoqiIM\Log`.
- `D:\VSProjects\moqi-ime\input_methods\rime\rime_test.go` lines 1885, 1970-2018, 2082, 2872-2878 assert Simplified scheme-set/update/auto-pair menu strings.
- `D:\VSProjects\moqi-ime\input_methods\rime\scheme_set_download_test.go` lines 131 and 164 assert Simplified error/notification text.
- AI/cloud tests (`ai_client_test.go`, `ai_config_test.go`, `rime_ai_test.go`, `cloud_clipboard*_test.go`, `cloudclipboard/*_test.go`) should be deleted, compile-gated, or rewritten if Phase 6 removes those features from callable v1 paths.

### Backend Package Pruning Hotspots

**Backend build output filter:** `D:\VSProjects\moqi-ime\scripts\build.ps1`

**Current package layout and stale output** (lines 213-239, 326-339, 364-393):
```powershell
if (-not $PackageDir) { $PackageDir = Join-Path $BuildRoot "moqi-ime" }
$BackendSnippet = Join-Path $BuildRoot "backends.moqi-ime.json"
Write-Host " Moqi IME Go Backend Build Script"
$packageAppearanceThemesData = Join-Path $PackageRimeDataDir "appearance_themes.json"
Copy-Item -LiteralPath $sourceAppearanceThemes -Destination $packageAppearanceThemesData -Force
Write-Step -Title "Step 7: Generate backends.json snippet"
name       = "moqi-ime"
command    = "moqi-ime\server.exe"
workingDir = "moqi-ime"
Write-Host "  C:\Program Files (x86)\MoqiIM\moqi-ime"
```

**Existing prune pattern to extend** (`D:\VSProjects\moqi-ime\scripts\build.ps1` lines 303-355):
```powershell
Copy-DirectoryContents -Source $RimeDir -Destination (Join-Path $packageInputMethodsDir "rime")
Remove-IfExists -Path (Join-Path $PackageRimeDir "icon.ico")
$pathsToRemove = @(
    @{ Path = Join-Path $PackageDir "input_methods\rime\data\others"; Label = "rime shared data others directory" },
    @{ Path = Join-Path $PackageDir "input_methods\rime\icons\icons"; Label = "nested icons directory" }
)
$packagedGoFiles = Get-ChildItem -LiteralPath (Join-Path $PackageDir "input_methods\rime") -Filter "*.go" -File -ErrorAction SilentlyContinue
```

**Required Phase 6 pruning targets:**
- `D:\VSProjects\moqi-ime\input_methods\rime\android`
- `D:\VSProjects\moqi-ime\input_methods\rime\cloudclipboard`
- `D:\VSProjects\moqi-ime\input_methods\rime\templates`
- `D:\VSProjects\moqi-ime\input_methods\rime\test`
- `D:\VSProjects\moqi-ime\input_methods\rime\ai_config.json`
- `D:\VSProjects\moqi-ime\input_methods\rime\ime.json`
- duplicate `D:\VSProjects\moqi-ime\input_methods\rime\data\appearance_themes.json`
- root/package icon folder duplication between `icons` and `input_methods\rime\icons`

**Planner action:** add backend-side pruning in `D:\VSProjects\moqi-ime\scripts\build.ps1` and frontend-side staging pruning in `scripts/install.ps1`. Do not rely on only the Windows frontend stage filter; the backend package should already be TypeDuck-v1-clean before it is copied into the installer stage.

### Backend Stale Feature Removal/Gating Hotspots

**AI runtime:** `ai_client.go`, `ai_config.go`, and AI fields/calls in `rime.go`.

**Removal/gating anchors**:
```go
// D:\VSProjects\moqi-ime\input_methods\rime\ai_client.go lines 71-131
func (c *aiClient) GenerateReviewCandidates(input aiGenerateRequest) ([]string, error) {
  ...
  Content: "你是一个中文输入法助手。请严格按用户要求输出候选文案，每条单独一行，不要编号，不要项目符号，不要解释，不要输出思考过程。",
  ...
  return nil, fmt.Errorf("AI API returned empty review content")
}
```

```go
// D:\VSProjects\moqi-ime\input_methods\rime\ai_config.go lines 55-167, 230-255
func loadAIConfig() (*aiRuntimeConfig, error) {
  if err := ensureUserAIConfigCopied(); err != nil {
    return nil, err
  }
  ...
}
BaseURL: strings.TrimRight(strings.TrimSpace(os.Getenv("MOQI_AI_BASE_URL")), "/"),
action.Name = "AI"
action.Name = "写好评"
return "请围绕“{{composition}}”生成最多 3 条适合直接发布的中文好评，每条 20 字左右。"
```

**Cloud/WebDAV runtime:** `cloud_clipboard*.go` and `cloudclipboard/**`.

**Removal/gating anchors**:
```go
// D:\VSProjects\moqi-ime\server.go lines 255-261
case "cloudClipboardUpload":
  rime.UploadCloudClipboardFromLauncher(req.CloudClipboardText)
  return &imecore.Response{SeqNum: req.SeqNum, Success: true, ReturnValue: 1}
```

```go
// D:\VSProjects\moqi-ime\input_methods\rime\cloud_clipboard_config.go lines 14-27, 137-142
const cloudClipboardConfigFileName = "cloud_clipboard.json"
ID_WEBDAV_SETTINGS          = 5004
// UploadCloudClipboardFromLauncher handles clipboard uploads from MoqiLauncher.
func UploadCloudClipboardFromLauncher(text string) {
  globalCloudClipboardService().UploadFromRequest(text)
}
```

**Scheme download/config update surfaces:** `scheme_set_download.go`, `config_update.go`, and `rime.go` menu command wiring.

**Removal/gating anchors:**
```go
// D:\VSProjects\moqi-ime\input_methods\rime\rime.go lines 3323-3332
map[string]interface{}{"id": ID_DOWNLOAD_SCHEME_SET, "text": "下载方案集(&D)"},
items = append(items, map[string]interface{}{
  "text":    "输入方案(&I)",
  "submenu": schemaItems,
})
```

```go
// D:\VSProjects\moqi-ime\input_methods\rime\scheme_set_download.go lines 64-115
resp.TrayNotification = trayNotification("打开方案集下载窗口...", imecore.TrayNotificationIconInfo)
go func() {
  ...
  ime.sendAsyncTrayNotification(trayNotification("方案集下载安装成功: "+name, imecore.TrayNotificationIconInfo))
}()
```

**Appearance/customization leftovers:** `appearance_config.go`, `auto_pair_symbols.go`, `custom_phrase.go`.

**Planner action:** decide per file whether to delete, compile-gate, or make unreachable. For v1, AI/cloud/WebDAV/scheme download/config update must not be visible or callable. Auto-pair and custom-phrase surfaces should be removed/gated unless explicitly accepted by Web alpha scope; if retained internally, remove Moqi filenames (`moqi_auto_pair_symbols.txt`, `moqi_custom_phrase.txt`) and Simplified menu strings.

**Likely tests to remove or rewrite:**
- `input_methods/rime/ai_client_test.go`, `ai_config_test.go`, `rime_ai_test.go`.
- `input_methods/rime/cloud_clipboard*_test.go` and `input_methods/rime/cloudclipboard/*_test.go`.
- `input_methods/rime/scheme_set_download_test.go` if scheme downloading is removed from v1.
- `input_methods/rime/rime_test.go` sections covering AI, cloud clipboard, scheme-set download/update, auto-pair/custom-phrase menus, and Moqi log path.
- `server_integration_test.go` fcitx/simple-pinyin flows if `server.go` stops importing/registering `input_methods/fcitx5` and `input_methods/moqi`.

### Backend Diagnostic Guard Script Pattern

Add or extend a guard script so Phase 6 fails on Han characters in backend log/debug/printf/error strings while allowing candidate/dictionary/test fixture data. Use a parser-oriented allowlist, not a blanket repo-wide Han ban.

**Suggested guard shape** (new Windows repo script, e.g. `scripts/Test-TypeDuckBackendDiagnostics.ps1`, or backend repo equivalent):
```powershell
param(
  [string] $BackendRoot = "D:\VSProjects\moqi-ime"
)

$ErrorActionPreference = "Stop"
$sourceFiles = Get-ChildItem -LiteralPath $BackendRoot -Recurse -Include *.go,*.ps1 -File |
  Where-Object {
    $_.FullName -notmatch '[\\/]\.git[\\/]' -and
    $_.FullName -notmatch '[\\/]rime-frost[\\/](cn_dicts|opencc)([\\/]|$)' -and
    $_.FullName -notmatch '[\\/]scripts[\\/]build[\\/]'
  }

$diagnosticPatterns = @(
  'log\.(Printf|Println|Fatal|Fatalf)\s*\(',
  'debugLogf\s*\(',
  'traceLogf\s*\(',
  'fmt\.Errorf\s*\(',
  'errors\.New\s*\(',
  'return\s+.*Error:',
  'Write-(Host|Warning|Error)\s+'
)

$allowedFixturePaths = @(
  '[\\/]rime-frost[\\/]',
  '[\\/]input_methods[\\/]rime[\\/]data[\\/]',
  '[\\/]input_methods[\\/]rime[\\/]test[\\/]',
  '_test\.go$'
)
```

**Guard requirements:**
- Fail on Han characters inside diagnostic call arguments: `log.*`, `debugLogf`, `traceLogf`, `fmt.Errorf`, `errors.New`, `log.Fatal`, PowerShell `Write-Host`/`Write-Warning`/`throw` diagnostics.
- Exclude legitimate candidate/dictionary/schema fixture data, e.g. TypeDuck dictionaries, Rime schema data, lookup fixture CSV/YAML, and tests whose purpose is candidate content. Do not exclude tests that assert diagnostic strings or runtime paths.
- Also fail on backend package output containing `MoqiIM`, `moqi-ime`, `WebDAV`, `cloud clipboard`, `AI`, `fcitx`, `input_methods\rime\data\appearance_themes.json`, `ai_config.json`, and `ime.json` where those are v1-banned shipped surfaces.
- The guard should be called by Windows packaging (`scripts/_all_in_package.ps1`) before installer success is reported, because Phase 6 spans both repos.

### Planner Revision Notes

- Phase 6 plans must include backend tasks for `D:\VSProjects\moqi-ime`; backend diagnostics, package pruning, stale AI/cloud/WebDAV/scheme-download removal, and backend guard coverage are not optional follow-ups.
- Keep frontend work for installer strings, pipe security, TypeDuck paths, and staging filters, but assign diagnostics cleanup across both `moqi-im-windows` and `moqi-ime`.
- Backend log/path changes must update tests at the same time; do not leave `server_test.go` or `rime_test.go` expecting `MoqiIM`, `moqi-ime.log`, or Simplified error text.
- Do not quote or preserve AI prompts/API config in plans except as removal targets. Treat `ai_config.json` and environment variables `MOQI_AI_*` as stale v1 surfaces.
- Product package acceptance should inspect the final staged installer tree and the backend package tree independently; both must be clean.

## Shared Patterns

### Bilingual User-Facing Copy
**Source:** `SetupHelper/SetupHelper.cpp` lines 677-685, `MoqLauncher/PipeServer.cpp` lines 405-423, `MoqiTextService/MoqiImeModule.cpp` lines 168-173  
**Apply to:** installer pages/messages, setup helper dialogs, tray/menu, settings launch failures.
```cpp
Bilingual(L"未能註冊 Win32 TypeDuck TSF DLL。",
          L"Failed to register Win32 TypeDuck TSF DLL.")
```

```cpp
L"未能開啟 TypeDuck 設定。請確認 TypeDuckSettings.exe 已安裝。\n"
L"Unable to open TypeDuck Settings. Please confirm TypeDuckSettings.exe is installed."
```

### English-Only Diagnostics
**Source:** `MoqLauncher/PipeClient.cpp` lines 191-195, 314; `MoqLauncher/BackendServer.cpp` lines 426-430  
**Apply to:** logs, debug/trace files, `printf`/`Write-Host` diagnostics.
```cpp
logger()->error("Failed to parse protobuf request from client {}", clientId_);
...
"Malformed TypeDuck client protobuf payload"
```

### TypeDuck-Owned Paths
**Source:** paths to replace in `MoqLauncher/PipeServer.cpp` lines 136-144, `MoqiTextService/TsfLog.cpp` lines 12-21, `libIME2/src/DebugLogConfig.cpp` lines 19-25  
**Apply to:** launcher config/logs, TSF logs, debug config, pipe namespace, roaming data.
```cpp
dataDirPath_ = getAppLocalDir() + L"\\MoqiIM";
...
return std::wstring(localAppData) + L"\\MoqiIM\\MoqiLauncher.json";
```

**Planner action:** replace with `TypeDuckIME` and `TypeDuckLauncher.json`. Do not add migration or cleanup for old `%APPDATA%\Moqi` files; Legacy Moqi remains a separate product and TypeDuck must not touch its state.

### Frame Bounds and Malformed Payloads
**Source:** `proto/ProtoFraming.h` lines 11-21, `MoqLauncher/PipeClient.cpp` lines 180-219, `MoqLauncher/BackendServer.cpp` lines 343-364  
**Apply to:** TSF client, launcher pipe clients, backend stdout.
```cpp
inline constexpr std::size_t kMaxClientFramePayloadBytes = 1024 * 1024;
...
if (payloadSize > maxPayloadBytes_) {
  fail(FrameError::PayloadTooLarge);
  return false;
}
```

### Fixed Settings Launch
**Source:** `MoqiTextService/MoqiImeModule.cpp` lines 38-79, 162-176; `MoqLauncher/PipeServer.cpp` lines 399-423  
**Apply to:** TSF configuration callback and tray Settings command.
```cpp
constexpr const wchar_t* kTypeDuckSettingsExecutable = L"TypeDuckSettings.exe";
...
const HINSTANCE result = ::ShellExecuteW(
    hwndParent, L"open", settingsPath.c_str(), nullptr,
    programDir.empty() ? nullptr : programDir.c_str(), SW_SHOWNORMAL);
```

### Guard Script Shape
**Source:** `scripts/Test-TypeDuckIconPackaging.ps1` lines 1-60 and 235-286  
**Apply to:** new privacy/security/scaffold cleanup guards.
```powershell
$ErrorActionPreference = "Stop"
function Assert-NotText([string] $Text, [string] $Pattern, [string] $Message) {
  if ($Text -match $Pattern) {
    throw $Message
  }
}
```

### Package-Time Pruning
**Source:** `scripts/install.ps1` lines 279-324; `D:\VSProjects\moqi-ime\scripts\build.ps1` package filters around lines 303-355  
**Apply to:** runtime package pruning in both repos.
```powershell
$files = Get-ChildItem -Path $SourceRoot -Recurse -Force -File | Where-Object {
    $relativePath = $_.FullName.Substring($SourceRoot.Length).TrimStart('\', '/')
    $_.Extension -ne ".go" -and
    $_.FullName -notmatch '[\\/]\.git(?:[\\/]|$)' -and
    $relativePath -notmatch '^input_methods[\\/]rime[\\/]icon\.ico$'
}
```

## No Analog Found

| File | Role | Data Flow | Reason |
|------|------|-----------|--------|
| `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1` | test/guard | batch + transform | No existing guard understands Phase 6's source-identifier allowance and user-facing surface distinction. Use Phase 5 guard script shape. |
| `Tests/TypeDuckProtocolFraming/*` | test | streaming transform | No first-party product framing unit test exists. Use `proto/ProtoFraming.h` behavior plus GoogleTest style from `libIME2/test/*`. |

## Metadata

**Analog search scope:** `installer/`, `SetupHelper/`, `MoqLauncher/`, `MoqiTextService/`, `libIME2/src/`, `proto/`, `scripts/`, `.github/workflows/`, `.gitmodules`, `backends.json`, and sibling `D:\VSProjects\moqi-ime`.
**Files scanned:** 80+ frontend/backend files and planning references.
**Pattern extraction date:** 2026-06-27
