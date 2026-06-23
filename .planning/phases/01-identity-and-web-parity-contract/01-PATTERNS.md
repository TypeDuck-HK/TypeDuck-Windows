# Phase 1: Identity and Web Parity Contract - Pattern Map

**Mapped:** 2026-06-23
**Files analyzed:** 4 planned artifacts, plus 18 source/evidence areas
**Analogs found:** 4 / 4

## File Classification

| New/Modified File or Artifact | Role | Data Flow | Closest Analog | Match Quality |
|-------------------------------|------|-----------|----------------|---------------|
| `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` | product contract | transform | `.planning/codebase/CONCERNS.md` + `.planning/codebase/INTEGRATIONS.md` | role-match |
| `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` | fixture contract | file-I/O, transform | `.planning/phases/01-identity-and-web-parity-contract/01-CONTEXT.md` | exact |
| `.planning/product/web-alpha-fixtures/2026-06-23/` | fixture asset directory | file-I/O | `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` pattern to be created | partial |
| `.planning/product/TYPEDUCK-BANNED-SURFACES.md` | negative contract, audit map | transform | `.planning/REQUIREMENTS.md` Out of Scope + `.planning/codebase/CONCERNS.md` | exact |
| Optional capture helper under `.planning/product/web-alpha-fixtures/2026-06-23/` | utility | file-I/O | `scripts/install.ps1` | role-match |

## Existing Code Areas to Inspect

| Existing Area | Role | Data Flow | Why It Matters |
|---------------|------|-----------|----------------|
| `MoqiTextService/MoqiImeModule.cpp` | config, registration | file-I/O, request-response | CLSID, program dir, backend `ime.json`, arbitrary config tool launch |
| `MoqiTextService/DllEntry.cpp` | registration | file-I/O, batch | TSF profile registration, locale/profile metadata, debug log path |
| `installer/MoqiTsf.iss` | installer config | batch, file-I/O | AppId, install path, language, startup registry, setup helper, cleanup |
| `SetupHelper/SetupHelper.cpp` | installer helper | batch, file-I/O | system DLL names, regsvr32 flow, scheduled re-registration task |
| `MoqLauncher/PipeServer.cpp` | launcher service | event-driven, pub-sub | mutex, pipe path, log path, tray strings, cloud clipboard |
| `MoqiTextService/MoqiClient.cpp` | IPC client | request-response | launcher exe name, pipe path, log paths, weak server identity check |
| `MoqiTextService/MoqiTextService.rc.in` | resource config | transform | file/product metadata strings |
| `scripts/install.ps1`, `scripts/_all_in_package.ps1` | packaging scripts | batch, file-I/O | stage paths, artifact names, backend copy assumptions |
| `.github/workflows/release.yml`, `.github/workflows/nightly.yml` | release config | batch | CI artifact paths and backend checkout assumptions |
| `README.md`, `TODO.md`, `proto/moqi.proto` | docs/protocol | transform, request-response | banned Moqi/fcitx/cloud/AI/protocol surfaces |
| `MoqiTextService/MoqiCandidateWindow.cpp`, `Preview/main.cpp` | visual reference consumers | event-driven | future candidate fixture consumers only; no Phase 1 code changes |

## Pattern Assignments

### `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` (product contract, transform)

**Analog:** `.planning/codebase/CONCERNS.md` for issue/evidence structure, `.planning/codebase/INTEGRATIONS.md` for integration inventory, and `01-CONTEXT.md` for required fields.

**Required contract shape** (`01-CONTEXT.md` lines 17-25):
```markdown
- **D-01:** Create a durable product contract at `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md`.
- **D-02:** The contract must cover executable names, DLL names, AppId, CLSID, profile GUIDs, install directories, log/data directories, pipe and mutex names, registry keys, resource strings, installer/release artifact names, and user-visible display names.
- **D-03:** Every identity entry must include current Moqi scaffold value when discoverable, target TypeDuck value, ownership/status (`locked`, `proposed`, or `deferred`), affected files, and verification notes.
- **D-05:** Prefer new deterministic TypeDuck-owned CLSID/profile GUID/AppId values instead of preserving Moqi identifiers.
- **D-07:** The TSF profile target must be Chinese (Traditional, Hong Kong) / `zh-HK`, with bilingual Traditional Hong Kong Chinese and English display text.
```

**Evidence inventory pattern** (`.planning/codebase/CONCERNS.md` lines 7-11):
```markdown
**Legacy Moqi product identity is embedded across code, installer, protocol, CI, and runtime paths:**
- Issue: Current binaries, namespaces, directories, registry cleanup, log paths, tray text, installer labels, release artifacts, and documentation are Moqi-specific.
- Files: `CMakeLists.txt`, `backends.json`, `README.md`, `TODO.md`, `MoqiTextService/MoqiImeModule.cpp`, ...
- Fix approach: Create a single TypeDuck product identity contract with executable names, DLL names, AppId, TSF CLSID, profile GUIDs, install directories, log directories, release artifact names, installer text, tray strings, and protocol package names.
```

**Current identity source excerpts to inventory:**
```cpp
// MoqiTextService/MoqiImeModule.cpp:33-39
// CLSID of Moqi Text Service (must stay in sync with installer/registration
// cleanup) {8F204C91-2D7A-4B3E-9E1F-6A5C0D8B2E7F}
const GUID g_textServiceClsid = {
    0x8f204c91, 0x2d7a, 0x4b3e,
    {0x9e, 0x1f, 0x6a, 0x5c, 0x0d, 0x8b, 0x2e, 0x7f}};
```

```pascal
; installer/MoqiTsf.iss:5-8,16-23,32,40,43,50,53-55
#define MyAppName "墨奇输入法"
#define MyAppPublisher "Moqi"
#define MyAppURL "https://github.com/gaboolic/moqi-im-windows"
#define MyAppId "{{C7A6A2D5-16C7-4BE4-8F52-E96D6D6A9E42}"
AppId={#MyAppId}
AppName={#MyAppName}
DefaultDirName={autopf32}\MoqiIM
OutputBaseFilename=moqi-im-windows-setup
Name: "chinesesimplified"; MessagesFile: ".\Inno-Setup-Chinese-Simplified-Translation\ChineseSimplified.isl"
Source: "{#StageDir}\win32\MoqiIM\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Filename: "{app}\MoqiLauncher.exe"; Flags: nowait; Check: ShouldLaunchLauncher
ValueName: "MoqiLauncher";
ValueData: """{app}\MoqiLauncher.exe""";
```

```cpp
// MoqLauncher/PipeServer.cpp:58-60,116-126,343-352
wchar_t PipeServer::singleInstanceMutexName_[] = L"MoqiLauncherMutex";
wchar_t PipeServer::wndClassName_[] = L"MoqiLauncherWnd";
dataDirPath_ = getAppLocalDir() + L"\\MoqiIM";
auto logFile = Ime::DebugLogFile::prepareDailyLogFilePath(logDirPath, L"MoqiLauncher.log");
logger_ = spdlog::rotating_logger_mt("MoqiLauncher", logFile,
pipeName = L"\\\\.\\pipe\\";
pipeName += username;
pipeName += L"\\MoqiIM\\";
pipeName += baseName;
```

```cpp
// SetupHelper/SetupHelper.cpp:18,621-656
constexpr wchar_t kReregisterTaskName[] = L"MoqiIM-ReRegisterTSF";
const fs::path source32 = app_dir / L"MoqiTextService.dll";
const fs::path source64 = app_dir / L"x64" / L"MoqiTextService.dll";
const fs::path dest32 = fs::path(GetSyswow64DirectoryPath()) / L"MoqiTextService.dll";
const fs::path dest64 = fs::path(GetNativeSystemDirectoryPath()) / L"MoqiTextService.dll";
```

**Planner instruction:** Use a table with columns: `Category`, `Current Moqi Scaffold Value`, `Target TypeDuck Value`, `Status`, `Affected Files`, `Verification Notes`. Do not change these source files in Phase 1; cite them as current-value evidence.

---

### `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` (fixture contract, file-I/O/transform)

**Analog:** `01-CONTEXT.md` fixture decisions and `REQUIREMENTS.md` Web-alpha-facing requirements.

**Fixture requirements** (`01-CONTEXT.md` lines 28-31, 65-66):
```markdown
- **D-08:** Create a dated fixture set at `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` plus any small supporting assets under `.planning/product/web-alpha-fixtures/2026-06-23/`.
- **D-09:** Fixtures must be source-backed, not memory-backed. Capture the alpha URL, local TypeDuck Web source path, source commit/hash when available, capture date, browser/viewport details for screenshots, and any access limitations.
- **D-10:** The fixture set must cover settings order, settings labels, defaults/options, display-language behavior, candidate list presentation, dictionary/detail panel data, visual references, and known Web alpha gaps or unsupported Windows mappings.
- **D-11:** If `http://localhost:5173/TypeDuck-Web/aap2-alpha/` or `I:\GitHub\TypeDuck-Web` is unavailable during execution, the fixture artifact must record that as blocked/partial instead of fabricating parity data.
```

**Acceptance coverage pattern** (`.planning/REQUIREMENTS.md` lines 52-67, 89):
```markdown
- [ ] **CAND-01**: User sees a native candidate panel styled to match TypeDuck Web alpha visual tone...
- [ ] **CAND-03**: User can inspect a selected or highlighted candidate's dictionary-like detail panel...
- [ ] **SET-03**: Settings dialog mirrors TypeDuck Web alpha settings and order, with Display Languages first.
- [ ] **SET-04**: User can enable display languages from the Web alpha language set: English, Hindi, Indonesian, Nepali, and Urdu.
- [ ] **VER-01**: Developer has a dated TypeDuck Web alpha parity fixture set covering settings defaults, labels, candidate list, dictionary panel, and visual references.
```

**Future consumer references only** (`.planning/codebase/STRUCTURE.md` lines 151-155):
```markdown
**New TypeDuck TSF Behavior:**
- Candidate visual feel matching TypeDuck Web alpha: `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/MoqiCandidateWindow.h`.
- Lang-bar and preserved-key controls: `MoqiTextService/MoqiLangBarButton.cpp`, `MoqiTextService/MoqiClient.cpp`.
```

**Current candidate limitations to record for unsupported mappings:**
```cpp
// proto/moqi.proto:73-76,177
message CandidateEntry {
  string text = 1;
  string comment = 2;
}
repeated CandidateEntry candidate_entries = 27;
```

```cpp
// MoqiTextService/MoqiCandidateWindow.cpp:22-28,84-86
constexpr COLORREF kWindowBackground = RGB(255, 255, 255);
constexpr COLORREF kWindowBorder = RGB(150, 150, 150);
constexpr COLORREF kDividerColor = RGB(220, 220, 220);
constexpr COLORREF kSelectedBackground = RGB(198, 221, 249);
constexpr int kDefaultCandidateSpacing = 20;
std::wstring logDir = std::wstring(localAppData) + L"\\MoqiIM\\Log";
```

**Planner instruction:** Structure the fixture doc as metadata first, then tables for `Settings`, `Candidate List`, `Dictionary Detail`, `Visual References`, `Unsupported/Blocked Windows Mappings`, and `Source Evidence`. If the local web runtime or `I:\GitHub\TypeDuck-Web` is unavailable in a future refresh, mark the relevant section with exact unavailable-evidence metadata, attempted source, and date.

---

### `.planning/product/web-alpha-fixtures/2026-06-23/` (fixture asset directory, file-I/O)

**Analog:** The fixture markdown artifact above. No existing fixture asset directory exists.

**Required asset naming pattern:** Keep filenames date-scoped and semantic so later phases can reference them without guessing:
```text
.planning/product/web-alpha-fixtures/2026-06-23/
  source-metadata.json
  settings-order.json
  candidate-list-sample.json
  dictionary-detail-sample.json
  screenshots/
    settings-desktop-1280x720.png
    candidate-desktop-1280x720.png
```

**Planner instruction:** This directory is optional if capture is blocked, but the fixture markdown must still document attempted capture and missing assets. Do not put large generated dumps here; keep small source-backed fixtures.

---

### `.planning/product/TYPEDUCK-BANNED-SURFACES.md` (negative contract, transform/audit map)

**Analog:** `.planning/REQUIREMENTS.md` Out of Scope table and `.planning/codebase/CONCERNS.md` banned-feature sections.

**Out-of-scope table pattern** (`.planning/REQUIREMENTS.md` lines 118-129):
```markdown
## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Feature | Reason |
|---------|--------|
| Moqi product behavior or branding | The repo is scaffold only; user explicitly wants TypeDuck. |
| Visible fcitx features or references | Not part of the Windows TypeDuck target and would look like unfinished scaffold leakage. |
| WebDAV/cloud clipboard | Outside stated scope and privacy-sensitive. |
| AI writing, translation, or explanation controls | Scaffold leftover and not part of TypeDuck Windows v1. |
| Excessive customization beyond Web alpha settings | Parity and testability matter more than broad configurability. |
| Installing as Chinese (Simplified) | Explicitly rejected; target is Chinese (Traditional, Hong Kong). |
```

**Known leakage pattern** (`.planning/codebase/CONCERNS.md` lines 37-41, 101-105):
```markdown
**User-facing fcitx, AI, WebDAV, and Moqi feature clutter exists in docs and launcher behavior:**
- Issue: README and launcher code describe or implement features outside the TypeDuck target, including fcitx-in-progress text, WebDAV cloud clipboard, AI config, multiple Moqi schemes, and Moqi tray controls.
- Files: `README.md`, `TODO.md`, `MoqLauncher/PipeServer.cpp`, `MoqLauncher/BackendServer.cpp`, `proto/moqi.proto`, `MoqiTextService/MoqiClient.cpp`
- Fix approach: Remove or compile-gate non-TypeDuck feature surfaces.
```

**Concrete banned-surface excerpts:**
```markdown
README.md:7: 当前状态：已实现 Rime 输入法接入，fcitx5 输入法接入中。
README.md:32: 同步设置：设置WebDAV后，支持跨设备同步用户词库（WebDAV）
README.md:33: 云剪贴板（WebDAV）...
README.md:34: 原生ai功能...
```

```cpp
// proto/moqi.proto:21-28,147
METHOD_CLOUD_CLIPBOARD_UPLOAD = 19;
optional string cloud_clipboard_text = 26;
```

```cpp
// MoqLauncher/PipeServer.cpp:666-679
::AppendMenu(hmenu, MF_STRING | MF_ENABLED, ID_RESTART_Moqi_BACKENDS,
             L"重启墨奇引擎");
::AppendMenu(hmenu, MF_STRING | MF_ENABLED, ID_EXIT_Moqi,
             L"退出墨奇引擎");
const auto configPath = getMoqiAppDataDir() + L"\\cloud_clipboard.json";
```

```cpp
// MoqiTextService/MoqiImeModule.cpp:154-166
std::wstring configCommand;
std::wstring configParams;
std::wstring configDir;
configCommand = utf8ToUtf16(info.get("configTool", "").asCString());
configParams = utf8ToUtf16(info.get("configToolParams", "").asCString());
configDir = utf8ToUtf16(info.get("configToolDir", "").asCString());
```

**Planner instruction:** Use columns `Banned Surface`, `Why Banned`, `Audit Patterns`, `Known Files`, `Allowed Replacement`, `Cleanup Phase`. This phase documents the rules and evidence; Phase 6 owns broad cleanup and automated checks.

---

### Optional capture helper (utility, file-I/O)

**Analog:** `scripts/install.ps1` PowerShell style if the planner decides a small helper is needed.

**Script style pattern** (`scripts/install.ps1` lines 1-4, 34-39, 157-166):
```powershell
#Requires -Version 5.1
<#
.SYNOPSIS
  Stage Moqi IM for Windows binaries and invoke the installer builder.
#>
param(
    [string] $RepoRoot = "",
    [string] $Win32BuildDir = "",
    [string] $X64BuildDir = "",
    [string] $MoqiImeSource = "",
    [switch] $SkipMoqiImeCopy,
)
```

**Planner instruction:** Prefer no helper unless it makes fixture capture reproducible. If created, place it under `.planning/product/web-alpha-fixtures/2026-06-23/`, keep it read-only against production code, use PowerShell 5.1, and record its outputs in the fixture markdown.

## Shared Patterns

### Source-Backed Documentation
**Source:** `01-CONTEXT.md` lines 28-31 and 39-41  
**Apply to:** All Phase 1 artifacts
```markdown
Fixtures must be source-backed, not memory-backed.
If the alpha URL or local source is unavailable, record blocked/partial instead of fabricating parity data.
Phase 1 should be documentation and fixture capture only.
```

### Requirement Traceability
**Source:** `.planning/REQUIREMENTS.md` lines 139-143 and 193-194  
**Apply to:** Identity contract and fixture contract
```markdown
| Requirement | Phase | Status |
| IDEN-02 | Phase 1 | Pending |
| VER-01 | Phase 1 | Pending |
| VER-02 | Phase 6 | Pending |
```

### Identity Surface Inventory
**Source:** `.planning/codebase/INTEGRATIONS.md` lines 48-64  
**Apply to:** Identity contract
```markdown
- System TSF DLL copies under `SysWOW64\MoqiTextService.dll` and `System32\MoqiTextService.dll`.
- Launcher logs and config under `%LOCALAPPDATA%\MoqiIM`.
- Named pipe paths include `\\.\pipe\<username>\MoqiIM\Launcher`.
- CLSID `{8F204C91-2D7A-4B3E-9E1F-6A5C0D8B2E7F}` in `MoqiTextService/MoqiImeModule.cpp` must stay aligned with `installer/MoqiTsf.iss`.
```

### Banned Surface Audit Terms
**Source:** `01-CONTEXT.md` lines 33-36 and `.planning/REQUIREMENTS.md` lines 124-129  
**Apply to:** Banned-surface contract
```text
Moqi, 墨奇, fcitx, WebDAV, cloud clipboard, cloud_clipboard, AI, ai_config,
ChineseSimplified, chinesesimplified, moqi-ime, generic configTool/configToolParams/configToolDir
```

### Future Consumer Boundaries
**Source:** `01-CONTEXT.md` lines 74-85 and `.planning/codebase/STRUCTURE.md` lines 151-178  
**Apply to:** All Phase 1 plans
```markdown
- `.planning/codebase/*.md` identify most identity, installer, protocol, and banned-feature surfaces.
- `Preview/main.cpp` and `MoqiTextService/MoqiCandidateWindow.cpp` are useful later for candidate visual parity, but Phase 1 should only reference them.
- Identity contract feeds Phases 3, 4, 5, 6, and 7.
- Web alpha fixtures feed Phase 5 and Phase 7.
```

## No Analog Found

| File | Role | Data Flow | Reason |
|------|------|-----------|--------|
| `.planning/product/web-alpha-fixtures/2026-06-23/` | fixture asset directory | file-I/O | No existing source-backed TypeDuck Web fixture directory exists yet. Use the required shape from `01-CONTEXT.md` and keep assets small. |

## Metadata

**Analog search scope:** `.planning/**/*.md`, `MoqiTextService/`, `MoqLauncher/`, `SetupHelper/`, `installer/`, `scripts/`, `.github/workflows/`, `proto/`, `README.md`, `backends.json`, `CMakeLists.txt`  
**Files scanned:** 30+ targeted files via `rg` and narrow line-range reads  
**Pattern extraction date:** 2026-06-23
