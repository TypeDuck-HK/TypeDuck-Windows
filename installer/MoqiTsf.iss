; TypeDuck Windows IME - Inno Setup 6 wizard (x64 only).
; Build: install Inno Setup 6, then run build-installer.ps1 -StageDir <stage root>.
; Moqi scaffold compatibility: source filename is kept as MoqiTsf.iss during the transition.

#define MyAppName "TypeDuck 粵語輸入法 / TypeDuck Cantonese IME"
#define MyAppPublisher "香港教育大學 The Education University of Hong Kong"
#define MyAppURL "https://www.typeduck.hk/"
; Inno Setup directives escape a literal leading "{" as "{{".
; AppId therefore intentionally uses a doubled opening brace; code and registry
; strings use normal single-braced GUID constants below.
#define MyAppId "{{9B52CF20-1C5D-4C74-9F5D-9E66377C8F37}"
#define ImeClsidCode "{7D92985A-BC53-47B5-A5CC-6E47F86B9D18}"
#define ImeProfileGuidCode "{C6E8F5DF-6504-44F9-B7CF-17A195373A83}"

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
CloseApplications=no
RestartApplications=no
WizardStyle=modern
OutputDir=dist
OutputBaseFilename=typeduck-windows-ime-setup
Compression=lzma2/max
SolidCompression=yes
WizardSizePercent=110,100
DisableWelcomePage=no
SetupIconFile=..\TypeDuckSettings\assets\TypeDuck.ico
WizardImageFile=..\TypeDuckSettings\resources\Installer.bmp
UninstallDisplayIcon={uninstallexe}
DefaultDialogFontName=Microsoft JhengHei UI

[Messages]
SetupAppTitle=TypeDuck 安裝 / TypeDuck Setup
SetupWindowTitle=TypeDuck 安裝 / TypeDuck Setup
UninstallAppTitle=TypeDuck 解除安裝 / TypeDuck Uninstall
UninstallAppFullTitle=TypeDuck 解除安裝 / TypeDuck Uninstall
ConfirmTitle=確認 / Confirm
ErrorTitle=錯誤 / Error
ButtonBack=< 返回 Back
ButtonNext=下一步 Next >
ButtonInstall=安裝 Install
ButtonOK=確定 OK
ButtonCancel=取消 Cancel
ButtonYes=是 Yes
ButtonNo=否 No
ButtonFinish=完成 Finish
ButtonBrowse=瀏覽 Browse...
ButtonWizardBrowse=瀏覽 Browse...
ExitSetupTitle=離開安裝程式 / Exit Setup
ExitSetupMessage=TypeDuck 尚未完成安裝。如現在離開，TypeDuck 不會安裝至此電腦。%n要離開安裝程式嗎？%n%nTypeDuck setup is not complete. If you exit now, TypeDuck will not be installed on this computer.%nExit Setup?
WizardSelectDir=選擇安裝位置 / Select Destination Location
SelectDirDesc=TypeDuck 應安裝在何處？ / Where should TypeDuck be installed?
SelectDirLabel3=安裝程式會將 TypeDuck 安裝到以下資料夾。%nSetup will install TypeDuck into the following folder.
SelectDirBrowseLabel=要繼續，請按「下一步 Next」。如要選擇其他資料夾，請按「瀏覽 Browse」。%nTo continue, click Next. To select a different folder, click Browse.
DiskSpaceGBLabel=至少需要 [gb] GB 可用磁碟空間。%nAt least [gb] GB of free disk space is required.
DiskSpaceMBLabel=至少需要 [mb] MB 可用磁碟空間。%nAt least [mb] MB of free disk space is required.
WizardReady=準備安裝 / Ready to Install
ReadyLabel1=TypeDuck 已準備好安裝至此電腦。%nSetup is ready to install TypeDuck on this computer.
ReadyLabel2a=按「安裝 Install」開始安裝；如要檢查或更改設定，請按「返回 Back」。%nClick Install to begin. Click Back to review or change settings.
ReadyLabel2b=按「安裝 Install」開始安裝。%nClick Install to begin.
ReadyMemoDir=安裝位置 / Destination location:
ReadyMemoGroup=開始功能表資料夾 / Start Menu folder:
ReadyMemoTasks=其他工作 / Additional tasks:
WizardInstalling=安裝中 / Installing
InstallingLabel=請稍候，TypeDuck 正在安裝至此電腦。%nPlease wait while TypeDuck is installed on this computer.
StatusClosingApplications=正在關閉 TypeDuck / Closing TypeDuck...
StatusCreateDirs=正在建立資料夾 / Creating folders...
StatusExtractFiles=正在解壓縮檔案 / Extracting files...
StatusCreateIcons=正在建立捷徑 / Creating shortcuts...
StatusCreateRegistryEntries=正在寫入設定 / Saving settings...
StatusSavingUninstall=正在儲存解除安裝資訊 / Saving uninstall information...
StatusRunProgram=正在完成安裝 / Finishing installation...
StatusRollback=正在復原變更 / Rolling back changes...
FinishedHeadingLabel=安裝完成 / Installation Completed
FinishedLabelNoIcons=TypeDuck 安裝已完成。%nTypeDuck setup has finished.
FinishedLabel=TypeDuck 安裝已完成。%nTypeDuck setup has finished.
WizardUninstalling=解除安裝狀態 / Uninstall Status
UninstallStatusLabel=請稍候，TypeDuck 正在從此電腦移除。%nPlease wait while TypeDuck is removed from this computer.
StatusUninstalling=正在解除安裝 TypeDuck / Uninstalling TypeDuck...
ConfirmUninstall=是否要移除 TypeDuck 及其所有元件？%n%nDo you want to remove TypeDuck and all of its components?
UninstalledAll=TypeDuck 已解除安裝。如 TypeDuck 仍然出現，請重新啟動電腦。%nTypeDuck is uninstalled. If TypeDuck still appears, restart your computer.
UninstalledMost=TypeDuck 已解除安裝，但有部分檔案需重新啟動電腦方可自動移除。如 TypeDuck 仍然出現，請重新啟動電腦。%nTypeDuck uninstall is complete, but some files require a computer restart in order to be removed automatically. If TypeDuck still appears, restart your computer.
UninstalledAndNeedsRestart=TypeDuck 已解除安裝。如 TypeDuck 仍然出現，請重新啟動電腦。%nTypeDuck is uninstalled. If TypeDuck still appears, restart your computer.

[Files]
Source: "{#StageDir}\win32\TypeDuckIME\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\TypeDuckIME\輸入法設定 IME Settings"; Filename: "{app}\TypeDuckSettings.exe"
Name: "{autoprograms}\TypeDuckIME\關於 About TypeDuck…"; Filename: "{app}\TypeDuckAbout.exe"
Name: "{autoprograms}\TypeDuckIME\解除安裝 Uninstall"; Filename: "{uninstallexe}"

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
Type: filesandordirs; Name: "{app}\TypeDuckRuntime"
Type: filesandordirs; Name: "{app}\resources"

[UninstallDelete]
Type: filesandordirs; Name: "{app}"
Type: filesandordirs; Name: "{localappdata}\TypeDuckIME"
Type: filesandordirs; Name: "{userappdata}\TypeDuckIME"; Check: ShouldDeleteUserDataOnUninstall

[Code]
const
  SetupHelperExitSuccess = 0;
  SetupHelperExitRestartRequired = 2;
  TypeDuckReregisterTaskName = 'TypeDuckIME-ReRegisterTSF';
  TypeDuckTextServiceDllName = 'TypeDuckTextService.dll';
  StartupSubkey = 'Software\Microsoft\Windows\CurrentVersion\Run';

var
  HelperInstallSucceeded: Boolean;
  HelperInstallFailed: Boolean;
  HelperInstallNeedsRestart: Boolean;
  HelperUninstallFailed: Boolean;
  HelperUninstallNeedsRestart: Boolean;
  HadExistingInstall: Boolean;
  DeleteUserDataOnUninstall: Boolean;

function Bilingual(const Zh: String; const En: String): String;
begin
  Result := Zh + #13#10 + En;
end;

function AboutTextBlock: String;
begin
  Result :=
    '歡迎使用 TypeDuck 打得 —— 設有少數族裔語言提示粵拼輸入法！有字想打？一裝即用，毋須再等，即刻打得！' + #13#10 +
    'Welcome to TypeDuck: a Cantonese input keyboard with minority language prompts! Got something you want to type? Have your fingers ready, get, set, TYPE DUCK!' + #13#10 +
    '' + #13#10 +
    '如有任何查詢，歡迎電郵至 info@typeduck.hk 或 lchaakming@eduhk.hk。' + #13#10 +
    'Should you have any enquiries, please email info@typeduck.hk or lchaakming@eduhk.hk.' + #13#10 +
    '' + #13#10 +
    '本輸入法由香港教育大學語言學及現代語言系開發。特別鳴謝「語文教育及研究常務委員會」資助本計劃。' + #13#10 +
    'This input method is developed by the Department of Linguistics and Modern Language Studies, the Education University of Hong Kong. Special thanks to the Standing Committee on Language Education and Research for funding this project.';
end;

procedure InitializeWizard;
begin
  WizardForm.WelcomeLabel1.Caption := Bilingual('歡迎使用 TypeDuck', 'Welcome to TypeDuck');
  WizardForm.WelcomeLabel1.Top := ScaleY(4);
  WizardForm.WelcomeLabel1.Height := ScaleY(52);
  WizardForm.WelcomeLabel1.Font.Size := 14;
  WizardForm.WelcomeLabel2.Caption := AboutTextBlock;
  WizardForm.WelcomeLabel2.Top := ScaleY(58);
  WizardForm.WelcomeLabel2.Height := ScaleY(300);
  WizardForm.WelcomeLabel2.Font.Size := 9;
end;

function InstallFinishedText: String;
begin
  if HelperInstallFailed then
    Result := Bilingual(
      'TypeDuck 未能完成安裝。請重新啟動電腦，然後再次執行安裝程式。',
      'TypeDuck could not finish installation. Please restart your computer, then run the installer again.')
  else if HadExistingInstall then
    Result := Bilingual(
      'TypeDuck 已安裝完成。請關閉並重新開啟欲使用 TypeDuck 新版本的應用程式。如未能輸入，請重新啟動電腦。',
      'TypeDuck is installed. Close and reopen the apps where you want to use the new version of TypeDuck. If you are unable to type, restart your computer.')
  else
    Result := Bilingual(
      'TypeDuck 已安裝完成。如未能輸入，請重新啟動電腦。',
      'TypeDuck is installed. If you are unable to type, restart your computer.');
end;

function UninstallFinishedText: String;
begin
  if HelperUninstallFailed then
    Result := Bilingual(
      'TypeDuck 未能完成解除安裝。請重新啟動電腦，然後再次執行解除安裝。',
      'TypeDuck could not finish uninstalling. Please restart your computer, then run uninstall again.')
  else
    Result := Bilingual(
      'TypeDuck 已解除安裝。如 TypeDuck 仍然出現，請重新啟動電腦。',
      'TypeDuck is uninstalled. If TypeDuck still appears, restart your computer.');
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpFinished then
    WizardForm.FinishedLabel.Caption := InstallFinishedText;
end;

procedure DeleteRegistryTreeIfPresent(const RootKey: Integer; const Subkey: String);
begin
  if RegKeyExists(RootKey, Subkey) then
    RegDeleteKeyIncludingSubkeys(RootKey, Subkey);
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

procedure TryKillProcessImage(const ImageName: String);
var
  R: Integer;
begin
  Exec(ExpandConstant('{sys}\taskkill.exe'), '/F /T /IM "' + ImageName + '"',
    '', SW_HIDE, ewWaitUntilTerminated, R);
end;

procedure TryKillProcessInAppDir(const ProcessName: String);
var
  R: Integer;
  Command: String;
begin
  Command :=
    '-NoProfile -ExecutionPolicy Bypass -Command "' +
    '$app = ''' + ExpandConstant('{app}') + '''; ' +
    'Get-Process -Name ''' + ProcessName + ''' -ErrorAction SilentlyContinue | ' +
    'Where-Object { $_.Path -and $_.Path.StartsWith($app, [System.StringComparison]::OrdinalIgnoreCase) } | ' +
    'Stop-Process -Force"';
  Exec(ExpandConstant('{sys}\WindowsPowerShell\v1.0\powershell.exe'),
    Command, '', SW_HIDE, ewWaitUntilTerminated, R);
end;

procedure StopTypeDuckProcesses;
begin
  TryKillProcessImage('TypeDuckLauncher.exe');
  TryKillProcessImage('TypeDuckSettings.exe');
  TryKillProcessImage('TypeDuckAbout.exe');
  TryKillProcessInAppDir('server');
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
      '找不到 TypeDuck 安裝工具：' + GetSetupHelperPath,
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
  Result := Action + ' /s';
  Result := Result + ' --appdir "' + ExpandConstant('{app}') + '"';
end;

function BuildUninstallSetupHelperParameters(const Action: String): String;
begin
  Result := Action + ' /s';
  Result := Result + ' --appdir "' + ExpandConstant('{app}') + '"';
end;

procedure HandleInstallSetupHelperFailure;
begin
  HelperInstallFailed := True;
  MsgBox(Bilingual(
    'TypeDuck 未能完成安裝。請重新啟動電腦，然後再次執行安裝程式。',
    'TypeDuck could not finish installation. Please restart your computer, then run the installer again.'),
    mbError, MB_OK);
end;

procedure HandleUninstallSetupHelperFailure;
begin
  HelperUninstallFailed := True;
  MsgBox(Bilingual(
    'TypeDuck 未能完成解除安裝。請重新啟動電腦，然後再次執行解除安裝。',
    'TypeDuck could not finish uninstalling. Please restart your computer, then run uninstall again.'),
    mbError, MB_OK);
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
    begin
      HandleInstallSetupHelperFailure;
      Exit;
    end;

    if ResultCode = SetupHelperExitSuccess then
    begin
      HelperInstallSucceeded := True;
    end
    else if ResultCode = SetupHelperExitRestartRequired then
    begin
      HelperInstallSucceeded := True;
      HelperInstallNeedsRestart := True;
    end;
    if (ResultCode <> SetupHelperExitSuccess) and
       (ResultCode <> SetupHelperExitRestartRequired) then
    begin
      HandleInstallSetupHelperFailure;
      Exit;
    end;
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

function PromptDeleteUserDataOnUninstall(): Boolean;
var
  Form: TSetupForm;
  PromptLabel: TNewStaticText;
  DataCheckBox: TNewCheckBox;
  ContinueButton: TNewButton;
  CancelButton: TNewButton;
  ContentLeft: Integer;
  ContentWidth: Integer;
  ButtonWidth: Integer;
  ButtonHeight: Integer;
  ButtonGap: Integer;
  ButtonTop: Integer;
  ButtonsLeft: Integer;
begin
  Form := CreateCustomForm(ScaleX(360), ScaleY(132), False, True);
  try
    Form.Caption := 'TypeDuck 解除安裝選項 / TypeDuck Uninstall Options';
    Form.Color := clWhite;
    Form.Font.Size := 10;

    ContentLeft := ScaleX(16);
    ContentWidth := Form.ClientWidth - (ContentLeft * 2);
    ButtonWidth := ScaleX(116);
    ButtonHeight := ScaleY(28);
    ButtonGap := ScaleX(12);
    ButtonTop := Form.ClientHeight - ScaleY(48);
    ButtonsLeft := (Form.ClientWidth - ((ButtonWidth * 2) + ButtonGap)) div 2;

    PromptLabel := TNewStaticText.Create(Form);
    PromptLabel.Parent := Form;
    PromptLabel.Color := clWhite;
    PromptLabel.SetBounds(ContentLeft, ScaleY(18), ContentWidth, ScaleY(68));
    PromptLabel.AutoSize := False;
    PromptLabel.WordWrap := True;
    PromptLabel.Caption := Bilingual(
      'TypeDuck 可以保留你的個人設定和詞庫資料，方便日後重新安裝時繼續使用。',
      'TypeDuck can keep your personal settings and dictionary data so they remain available after reinstalling.');

    DataCheckBox := TNewCheckBox.Create(Form);
    DataCheckBox.Parent := Form;
    DataCheckBox.Color := clWhite;
    DataCheckBox.SetBounds(ContentLeft, ScaleY(72), ContentWidth, ScaleY(32));
    DataCheckBox.Caption := '同時刪除 TypeDuck 個人資料 / Also delete TypeDuck user data';
    DataCheckBox.Checked := False;

    ContinueButton := TNewButton.Create(Form);
    ContinueButton.Parent := Form;
    ContinueButton.SetBounds(ButtonsLeft, ButtonTop, ButtonWidth, ButtonHeight);
    ContinueButton.Caption := '繼續 Continue';
    ContinueButton.Default := True;
    ContinueButton.ModalResult := mrOk;

    CancelButton := TNewButton.Create(Form);
    CancelButton.Parent := Form;
    CancelButton.SetBounds(ButtonsLeft + ButtonWidth + ButtonGap, ButtonTop, ButtonWidth, ButtonHeight);
    CancelButton.Caption := '取消 Cancel';
    CancelButton.Cancel := True;
    CancelButton.ModalResult := mrCancel;

    Result := Form.ShowModal = mrOk;
    if Result then
      DeleteUserDataOnUninstall := DataCheckBox.Checked;
  finally
    Form.Free;
  end;
end;

function InitializeUninstall(): Boolean;
begin
  DeleteUserDataOnUninstall := False;
  if UninstallSilent then
  begin
    Result := True;
    Exit;
  end;
  Result := PromptDeleteUserDataOnUninstall;
end;

function ShouldDeleteUserDataOnUninstall(): Boolean;
begin
  Result := DeleteUserDataOnUninstall;
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
    begin
      HandleUninstallSetupHelperFailure;
      Exit;
    end;
    if ResultCode = SetupHelperExitRestartRequired then
      HelperUninstallNeedsRestart := True
    else if ResultCode <> SetupHelperExitSuccess then
    begin
      HandleUninstallSetupHelperFailure;
      Exit;
    end;
  end;
  if CurUninstallStep = usPostUninstall then
  begin
    RegPurgeTypeDuckResiduals;
  end;
end;
