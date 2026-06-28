# TypeDuck Windows

TypeDuck Windows is a Cantonese input method for Windows users in Hong Kong, built on Microsoft Text Services Framework and a TypeDuck Rime runtime.

TypeDuck Windows 係為香港 Windows 用戶而設嘅粵語輸入法，使用 Microsoft Text Services Framework，並配合 TypeDuck Rime Runtime 提供候選字、粵拼同字典式提示。

## What You Get / 你會得到咩

- TypeDuck Cantonese IME registered under Chinese (Traditional, Hong Kong) / `zh-HK`.
- TypeDuck 粵語輸入法會註冊喺「中文（繁體，香港）」/ `zh-HK`。
- Candidate, Jyutping, and dictionary-style details in a Windows candidate window.
- 候選窗會顯示候選字、粵拼同字典式資料。
- A bilingual installer, settings window, about dialog, and Start Menu shortcuts.
- 雙語安裝程式、設定視窗、關於視窗同開始功能表捷徑。
- Local settings stored per user, with Rime-affecting options applied through the TypeDuck runtime.
- 每位用戶有獨立設定；影響 Rime 嘅選項會經 TypeDuck Runtime 套用。

## Install / 安裝

Download the latest installer from [TypeDuck-Windows releases](https://github.com/TypeDuck-HK/TypeDuck-Windows/releases), then run:

請到 [TypeDuck-Windows releases](https://github.com/TypeDuck-HK/TypeDuck-Windows/releases) 下載最新安裝程式，然後執行：

```text
typeduck-windows-ime-setup.exe
```

After installation, choose **TypeDuck 粵語輸入法 / TypeDuck Cantonese IME** under Chinese (Traditional, Hong Kong).

安裝完成後，請喺「中文（繁體，香港）」下面揀 **TypeDuck 粵語輸入法 / TypeDuck Cantonese IME**。

## First Use / 第一次使用

1. Open any Windows app that accepts text.
2. Switch to Chinese (Traditional, Hong Kong), then select TypeDuck.
3. Type Cantonese input codes and choose a candidate from the TypeDuck candidate window.
4. Open **輸入法設定 IME Settings** from the Start Menu or launcher tray icon to adjust preferences.

1. 開啟任何可以輸入文字嘅 Windows 應用程式。
2. 切換到中文（繁體，香港），再揀 TypeDuck。
3. 輸入粵語編碼，喺 TypeDuck 候選窗揀字。
4. 如要調整設定，可由開始功能表或工具列圖示開啟 **輸入法設定 IME Settings**。

## User Settings / 用戶設定

The settings app includes bilingual controls for candidate count, display languages, Chinese typeface, Jyutping visibility, completion, correction, sentence composition, input memory, reverse lookup display, and Cangjie version.

設定程式提供雙語控制項，包括每頁候選數量、顯示語言、中文字體、候選詞粵拼、自動完成、自動校正、自動組詞、輸入記憶、反查顯示同倉頡版本。

## Repositories

- Frontend and installer: [TypeDuck-HK/TypeDuck-Windows](https://github.com/TypeDuck-HK/TypeDuck-Windows)
- Backend runtime: [TypeDuck-HK/TypeDuck-Windows-backend](https://github.com/TypeDuck-HK/TypeDuck-Windows-backend)

## Architecture

TypeDuck Windows is split into two repositories that ship together:

```text
Windows TSF host apps
        |
        v
TypeDuckTextService.dll
        |
        | per-user named pipe, protobuf frames
        v
TypeDuckLauncher.exe
        |
        | stdin/stdout, protobuf frames
        v
TypeDuckRuntime/server.exe
        |
        v
librime + TypeDuck Rime data + dictionary lookup filter
```

The frontend repository owns TSF registration, COM entry points, candidate UI, launcher IPC, settings/about UI, setup helper, runtime staging, and the installer. The backend repository owns the Go runtime process, protobuf conversion, Rime service, librime binding, settings application to Rime configuration, and packaged runtime output.

## Build Prerequisites

- Windows 10 or Windows 11 for development and validation.
- Visual Studio 2022 with MSVC and Windows SDK.
- CMake 3.21 or newer.
- PowerShell 7 or newer; use `pwsh` for repository scripts.
- Inno Setup 6 for installer builds.
- Go matching the backend module requirement.
- Protocol Buffers `protoc` 33.5 or a local protobuf source tree.
- A TypeDuck Rime schema checkout to pass as `-RimeDataSource`.

## Build

Build the Windows frontend binaries:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
```

Build the full installer after the backend runtime and schema source are available:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/_all_in_package.ps1 -RimeDataSource <schema-source>
```

The installer is written to:

```text
installer/dist/typeduck-windows-ime-setup.exe
```

## Runtime Layout

The installed application uses this product layout:

```text
TypeDuckIME/
├── TypeDuckLauncher.exe
├── TypeDuckSettings.exe
├── TypeDuckAbout.exe
├── TypeDuckSetupHelper.exe
├── TypeDuckTextService.dll
├── x64/TypeDuckTextService.dll
├── resources/
└── TypeDuckRuntime/
    ├── server.exe
    └── input_methods/rime/
```

User data and logs use TypeDuck-specific folders:

- Preferences: `%APPDATA%\TypeDuckIME\TypeDuckPreferences.json`
- Rime user data: `%APPDATA%\TypeDuckIME\Rime`
- Logs: `%LOCALAPPDATA%\TypeDuckIME\Log`

## Testing

Build first, then run CTest against a configured Win32 build:

```powershell
ctest --test-dir build-vs32 -C Debug --output-on-failure
```

Useful focused checks:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckProtocolContract.ps1 -RepoRoot . -Strict
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckCandidateData.ps1 -RepoRoot . -Strict
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckSettingsPersistence.ps1 -RepoRoot . -Strict
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts/Test-TypeDuckReleaseArtifacts.ps1 -RepoRoot . -Strict
```

Cross-repo runtime verification should include the backend test suite and a runtime package build from `TypeDuck-Windows-backend`.

## Protocol

Frontend and backend communicate with Protocol Buffers over length-prefixed binary frames. Protocol changes must update both repository schemas and regenerated bindings:

- The frontend protobuf schema.
- The backend protobuf schema.
- Generated C++ and Go protobuf outputs.

Do not edit generated protobuf files directly.

## Release

The release workflows build on `windows-2022`, checkout both TypeDuck Windows repositories, prepare TypeDuck Rime schema data, download protobuf tooling, run the packaging script, and upload the installer asset. Release validation should cover:

- Installer file name and signature/hash evidence.
- Clean install, upgrade, uninstall, and reboot-required registration paths.
- Win32 and x64 TSF DLL registration.
- `zh-HK` language profile registration.
- Candidate window and dictionary lookup behavior.
- Settings persistence and Rime side effects.

## License

This repository is licensed under the [MIT License](LICENSE).

## Acknowledgement

Thanks to the Moqi IME project for its earlier Windows IME engineering work.
