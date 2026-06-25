; TypeDuck Windows IME - Inno Setup 6 wizard (x64 only).
; Build: install Inno Setup 6, then run build-installer.ps1 -StageDir <stage root>.
; Source filename is kept as MoqiTsf.iss during the scaffold transition.

#define MyAppName "TypeDuck 粵語輸入法 / TypeDuck Cantonese IME"
#define MyAppPublisher "TypeDuck"
#define MyAppURL "https://www.typeduck.hk/"
#define MyAppId "{{9B52CF20-1C5D-4C74-9F5D-9E66377C8F37}"
#define ImeClsid "{{7D92985A-BC53-47B5-A5CC-6E47F86B9D18}}"
#define LegacyMoqiImeClsid "{{8F204C91-2D7A-4B3E-9E1F-6A5C0D8B2E7F}}"
#define ImeClsidCode "{7D92985A-BC53-47B5-A5CC-6E47F86B9D18}"
#define ImeProfileGuidCode "{C6E8F5DF-6504-44F9-B7CF-17A195373A83}"
#define LegacyMoqiImeClsidCode "{8F204C91-2D7A-4B3E-9E1F-6A5C0D8B2E7F}"

#ifndef StageDir
  #define StageDir "..\stage"
#endif

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion=1.0.0
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf32}\TypeDuckIME
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
CloseApplications=yes
RestartApplications=no
WizardStyle=modern
OutputDir=dist
OutputBaseFilename=typeduck-windows-ime-setup
Compression=lzma2/max
SolidCompression=yes
WizardSizePercent=110,100
DisableWelcomePage=no
SetupIconFile=..\TypeDuckSettings\assets\TypeDuck.ico
UninstallDisplayIcon={uninstallexe}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
; Traditional Chinese Hong Kong-compatible entry without the old Simplified-only
; vendored chrome. This Inno install may not include ChineseTraditional.isl, so
; TypeDuck-controlled copy below remains bilingual while standard chrome falls
; back to Inno's English resource until a vetted Traditional pack is bundled.
Name: "chinesetraditional"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#StageDir}\win32\TypeDuckIME\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}\TypeDuck About"; Filename: "{app}\TypeDuckAbout.exe"
Name: "{autoprograms}\{#MyAppName}\TypeDuck 關於"; Filename: "{app}\TypeDuckAbout.exe"
Name: "{autoprograms}\{#MyAppName}\解除安裝 / Uninstall"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\TypeDuckLauncher.exe"; Flags: nowait; Check: ShouldLaunchLauncher
Filename: "{app}\TypeDuckSettings.exe"; Parameters: "/apply-defaults"; Flags: runhidden waituntilterminated; Check: ShouldSeedDefaultSettings
Filename: "{app}\TypeDuckSettings.exe"; Description: "開啟 TypeDuck 設定 / Open TypeDuck Settings"; Flags: postinstall nowait skipifsilent; Check: ShouldLaunchSettings
Filename: "{app}\TypeDuckAbout.exe"; Description: "開啟 TypeDuck 關於 / Open TypeDuck About"; Flags: postinstall nowait skipifsilent; Check: ShouldLaunchAbout

[Registry]
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; \
  ValueType: string; ValueName: "TypeDuckLauncher"; \
  ValueData: """{app}\TypeDuckLauncher.exe"""; \
  Flags: uninsdeletevalue

[InstallDelete]
Type: filesandordirs; Name: "{app}\x64"
; Legacy Moqi migration cleanup: remove only scaffold payload folders that conflict
; with this TypeDuck install directory during the transition.
Type: filesandordirs; Name: "{app}\moqi-ime"

[Code]
const
  SetupHelperExitSuccess = 0;
  SetupHelperExitRestartRequired = 2;
  TypeDuckReregisterTaskName = 'TypeDuckIME-ReRegisterTSF';
  StartupSubkey = 'Software\Microsoft\Windows\CurrentVersion\Run';

var
  HelperInstallSucceeded: Boolean;
  HelperInstallNeedsRestart: Boolean;
  HelperUninstallNeedsRestart: Boolean;
  HadExistingInstall: Boolean;

function Bilingual(const Zh: String; const En: String): String;
begin
  Result := Zh + #13#10 + En;
end;

function ExistingImeInstallationPresent: Boolean;
begin
  Result :=
    FileExists(ExpandConstant('{app}\TypeDuckLauncher.exe')) or
    FileExists(ExpandConstant('{syswow64}\TypeDuckTextService.dll')) or
    FileExists(ExpandConstant('{sys}\TypeDuckTextService.dll')) or
    RegKeyExists(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\CTF\TIP\{#ImeClsidCode}') or
    RegKeyExists(HKEY_CURRENT_USER, 'Software\Microsoft\CTF\TIP\{#ImeClsidCode}') or
    RegKeyExists(HKEY_CLASSES_ROOT, 'CLSID\{#ImeClsidCode}') or
    RegKeyExists(HKEY_CURRENT_USER, 'Software\Classes\CLSID\{#ImeClsidCode}');
end;

procedure DeleteRegistryTreeIfPresent(const RootKey: Integer; const Subkey: String);
begin
  if RegKeyExists(RootKey, Subkey) then
    RegDeleteKeyIncludingSubkeys(RootKey, Subkey);
end;

procedure RegPurgeTypeDuckResiduals;
var
  ClsidKey: String;
  TipKey: String;
begin
  ClsidKey := 'CLSID\{#ImeClsidCode}';
  TipKey := 'SOFTWARE\Microsoft\CTF\TIP\{#ImeClsidCode}';
  DeleteRegistryTreeIfPresent(HKEY_CLASSES_ROOT, ClsidKey);
  DeleteRegistryTreeIfPresent(HKEY_LOCAL_MACHINE, TipKey);
  DeleteRegistryTreeIfPresent(HKEY_CURRENT_USER, 'Software\Microsoft\CTF\TIP\{#ImeClsidCode}\LanguageProfile\0x00000c04\{#ImeProfileGuidCode}');
  DeleteRegistryTreeIfPresent(HKEY_CURRENT_USER, 'Software\Microsoft\CTF\TIP\{#ImeClsidCode}\LanguageProfile\0x00000c04');
  DeleteRegistryTreeIfPresent(HKEY_CURRENT_USER, 'Software\Microsoft\CTF\TIP\{#ImeClsidCode}\LanguageProfile');
  DeleteRegistryTreeIfPresent(HKEY_CURRENT_USER, 'Software\Microsoft\CTF\TIP\{#ImeClsidCode}');
  DeleteRegistryTreeIfPresent(HKEY_CURRENT_USER, 'Software\Classes\CLSID\{#ImeClsidCode}');
  RegDeleteValue(HKEY_CURRENT_USER, StartupSubkey, 'TypeDuckLauncher');
end;

procedure RegPurgeLegacyMoqiResiduals;
begin
  // Legacy Moqi migration cleanup: allowlist only this scaffold CLSID/TIP/startup residue.
  DeleteRegistryTreeIfPresent(HKEY_CLASSES_ROOT, 'CLSID\{#LegacyMoqiImeClsidCode}');
  DeleteRegistryTreeIfPresent(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\CTF\TIP\{#LegacyMoqiImeClsidCode}');
  DeleteRegistryTreeIfPresent(HKEY_CURRENT_USER, 'Software\Microsoft\CTF\TIP\{#LegacyMoqiImeClsidCode}');
  DeleteRegistryTreeIfPresent(HKEY_CURRENT_USER, 'Software\Classes\CLSID\{#LegacyMoqiImeClsidCode}');
  RegDeleteValue(HKEY_CURRENT_USER, StartupSubkey, 'MoqiLauncher');
end;

procedure TryKillProcessImage(const ImageName: String);
var
  R: Integer;
begin
  Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /T /IM "' + ImageName + '"',
    '', SW_HIDE, ewWaitUntilTerminated, R);
end;

procedure StopTypeDuckProcesses;
begin
  TryKillProcessImage('TypeDuckLauncher.exe');
end;

procedure DeleteTypeDuckReregisterTask;
var
  R: Integer;
begin
  Exec(ExpandConstant('{sys}\schtasks.exe'),
    '/Delete /TN "' + TypeDuckReregisterTaskName + '" /F',
    '', SW_HIDE, ewWaitUntilTerminated, R);
end;

function GetSetupHelperPath: String;
begin
  Result := ExpandConstant('{app}\TypeDuckSetupHelper.exe');
end;

procedure EnsureSetupHelperExists;
begin
  if not FileExists(GetSetupHelperPath) then
    RaiseException(Bilingual(
      '找不到 TypeDuck 安裝工具: ' + GetSetupHelperPath,
      'TypeDuck setup helper not found: ' + GetSetupHelperPath));
end;

function RunSetupHelper(const Parameters: String; var ResultCode: Integer): Boolean;
begin
  EnsureSetupHelperExists;
  Result := Exec(GetSetupHelperPath, Parameters, ExpandConstant('{app}'),
    SW_HIDE, ewWaitUntilTerminated, ResultCode);
  if not Result then
    ResultCode := -1;
end;

function BuildInstallSetupHelperParameters(const Action: String): String;
begin
  Result := Action;
  if WizardSilent() then
    Result := Result + ' /s';
  Result := Result + ' --appdir "' + ExpandConstant('{app}') + '"';
end;

function BuildUninstallSetupHelperParameters(const Action: String): String;
begin
  Result := Action;
  if UninstallSilent() then
    Result := Result + ' /s';
  Result := Result + ' --appdir "' + ExpandConstant('{app}') + '"';
end;

procedure HandleSetupHelperResult(const Operation: String; const ResultCode: Integer);
begin
  RaiseException(Bilingual(
    Operation + ' 失敗，結束碼: ' + IntToStr(ResultCode),
    Operation + ' failed (exit code ' + IntToStr(ResultCode) + ').'));
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ResultCode: Integer;
begin
  if CurStep = ssInstall then
  begin
    HadExistingInstall := ExistingImeInstallationPresent;
    StopTypeDuckProcesses;
    DeleteTypeDuckReregisterTask;
  end;

  if CurStep = ssPostInstall then
  begin
    if not RunSetupHelper(BuildInstallSetupHelperParameters('/i'), ResultCode) then
      HandleSetupHelperResult('TypeDuck setup-helper install / TypeDuck 安裝工具安裝', ResultCode);

    if ResultCode = SetupHelperExitSuccess then
    begin
      HelperInstallSucceeded := True;
      if HadExistingInstall then
        SuppressibleMsgBox(
          Bilingual(
            '偵測到覆蓋安裝。如果目前 Windows 工作階段仍有舊 TSF 實例，TypeDuck 可能需要登出或重啟 Windows 後才會即時恢復正常輸入。',
            'An existing install was detected. If the current Windows session still has an old TSF instance loaded, TypeDuck may need sign-out or Windows restart before input resumes immediately.'),
          mbInformation, MB_OK, IDOK);
    end
    else if ResultCode = SetupHelperExitRestartRequired then
    begin
      HelperInstallSucceeded := True;
      HelperInstallNeedsRestart := True;
      SuppressibleMsgBox(
        Bilingual(
          '安裝器已更新 TypeDuck 應用程式檔案，但 TSF DLL 仍被 Windows 使用。請在安裝完成後盡快重啟 Windows；系統重啟後會自動完成 TSF 註冊。',
          'The installer updated TypeDuck application files, but the TSF DLL is still in use by Windows. Please restart Windows after setup; TSF registration will finish automatically after reboot.'),
        mbInformation, MB_OK, IDOK);
    end;
    if (ResultCode <> SetupHelperExitSuccess) and
       (ResultCode <> SetupHelperExitRestartRequired) then
      HandleSetupHelperResult('TypeDuck setup-helper install / TypeDuck 安裝工具安裝', ResultCode);
  end;
end;

function NeedRestart(): Boolean;
begin
  Result := HelperInstallNeedsRestart;
end;

function ShouldLaunchLauncher(): Boolean;
begin
  Result := HelperInstallSucceeded and (not HelperInstallNeedsRestart);
end;

function ShouldLaunchSettings(): Boolean;
begin
  Result := HelperInstallSucceeded and (not HelperInstallNeedsRestart);
end;

function ShouldSeedDefaultSettings(): Boolean;
begin
  Result := HelperInstallSucceeded and (not HelperInstallNeedsRestart);
end;

function ShouldLaunchAbout(): Boolean;
begin
  Result := HelperInstallSucceeded and (not HelperInstallNeedsRestart);
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ResultCode: Integer;
begin
  if CurUninstallStep = usUninstall then
  begin
    StopTypeDuckProcesses;
    DeleteTypeDuckReregisterTask;
    if not RunSetupHelper(BuildUninstallSetupHelperParameters('/u'), ResultCode) then
      HandleSetupHelperResult('TypeDuck setup-helper uninstall / TypeDuck 安裝工具解除安裝', ResultCode);
    if ResultCode = SetupHelperExitRestartRequired then
      HelperUninstallNeedsRestart := True
    else if ResultCode <> SetupHelperExitSuccess then
      HandleSetupHelperResult('TypeDuck setup-helper uninstall / TypeDuck 安裝工具解除安裝', ResultCode);
  end;
  if CurUninstallStep = usPostUninstall then
  begin
    RegPurgeTypeDuckResiduals;
    RegPurgeLegacyMoqiResiduals;
    if HelperUninstallNeedsRestart then
      SuppressibleMsgBox(
        Bilingual(
          '部分 TypeDuck TSF DLL 已安排在 Windows 重啟後刪除。請盡快重啟 Windows，以完成解除安裝清理。',
          'Some TypeDuck TSF DLL files were scheduled for deletion after Windows restarts. Please restart Windows soon to finish uninstall cleanup.'),
        mbInformation, MB_OK, IDOK);
  end;
end;
