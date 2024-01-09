; weasel installation script
!include FileFunc.nsh
!include LogicLib.nsh
!include MUI2.nsh
!include x64.nsh

Unicode true

!define WEASEL_ROOT $INSTDIR\TypeDuck-${WEASEL_VERSION}

; The name of the installer
Name "TypeDuck ${WEASEL_VERSION}"

; The file to write
OutFile "archives\TypeDuck-Windows-${WEASEL_VERSION}-installer.exe"

VIProductVersion "${WEASEL_VERSION}.${WEASEL_BUILD}"
VIAddVersionKey "ProductName" "TypeDuck"
VIAddVersionKey "Comments" "Driven by Weasel"
VIAddVersionKey "CompanyName" "The Education University of Hong Kong"
VIAddVersionKey "LegalCopyright" "© The Education University of Hong Kong"
VIAddVersionKey "FileDescription" "TypeDuck IME"
VIAddVersionKey "FileVersion" "${WEASEL_VERSION}"

!define MUI_ICON ..\resource\TypeDuck.ico
!define MUI_UNICON ..\resource\TypeDuck.ico
!define MUI_WELCOMEFINISHPAGE_BITMAP ..\resource\Installer.bmp
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ..\resource\Installer.bmp
!define MUI_WELCOMEPAGE_TITLE "歡迎使用 TypeDuck$\r$\nWelcome to TypeDuck"
!define MUI_DIRECTORYPAGE_TEXT_TOP "\
安裝程式會將 TypeDuck 安裝至以下資料夾。按「安裝」以繼續。$\r$\n\
The Setup will install TypeDuck in the following folder. Click “Install” to continue.$\r$\n\
$\r$\n\
本程式乃根據 GNU 通用公眾特許條款第三版發佈。你可以前往 www.gnu.org/licenses/gpl，或於安裝後瀏覽以下安裝資料夾以閲覽其內容。$\r$\n\
This program is distributed under the GNU General Public License v3, which can be found at www.gnu.org/licenses/gpl or in the following destination folder after the installation."
!define MUI_TEXT_FINISH_INFO_TITLE "安裝完成$\r$\nInstallation Completed"
!define MUI_TEXT_FINISH_INFO_TEXT "\
你已成功安裝 TypeDuck 輸入法。$\r$\n\
TypeDuck has been sucessfully installed on your computer.$\r$\n\
$\r$\n\
如「TypeDuck」於輸入法功能表多次出現，請選擇帶有圖示之項目，或嘗試重新啟動電腦。$\r$\n\
If “TypeDuck” appears multiple times in the IME menu, please choose the one with an icon, or try restarting the computer.$\r$\n\
$\r$\n\
按「完成」以關閉安裝程式。$\r$\n\
Click “Finish” to close this Setup."
!define MUI_PAGE_CUSTOMFUNCTION_SHOW WelcomePageShowCallback

SetCompressor /SOLID lzma

Function WelcomePageShowCallback
${NSD_CreateLabel} 120u 38u 195u 150u "\
歡迎使用 TypeDuck 打得 —— 設有少數族裔語言提示粵拼輸入法！有字想打？一裝即用，毋須再等，即刻打得！$\r$\n\
Welcome to TypeDuck: a Cantonese input keyboard with minority language prompts! Got something you want to type? Have your fingers ready, get, set, TYPE DUCK!$\r$\n\
$\r$\n\
如有任何查詢，歡迎電郵至 info@typeduck.hk 或 lchaakming@eduhk.hk。$\r$\n\
Should you have any enquiries, please email info@typeduck.hk or lchaakming@eduhk.hk.$\r$\n\
$\r$\n\
本輸入法由香港教育大學語言學及現代語言系開發。特別鳴謝「語文教育及研究常務委員會」資助本計劃。$\r$\n\
This input method is developed by the Department of Linguistics and Modern Languages, the Education University of Hong Kong. Special thanks to the Standing Committee on Language Education and Research for funding this project."
Pop $0
SetCtlColors $0 "000000" "FFFFFF"
FunctionEnd

; The default installation directory
InstallDir $PROGRAMFILES\Rime

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Rime\TypeDuck" "InstallDir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

!insertmacro MUI_PAGE_WELCOME
; !insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------

; Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------

Function .onInit
  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\TypeDuck" \
  "UninstallString"
  StrCmp $R0 "" done

  StrCpy $0 "Upgrade"
  IfSilent uninst 0
  MessageBox MB_OKCANCEL|MB_ICONINFORMATION \
  "Detected an old version of TypeDuck.$\n$\nAre you sure you want to install? This will cause the old version to be removed." \
  IDOK uninst
  Abort

uninst:
  ; Backup data directory from previous installation, user files may exist
  ReadRegStr $R1 HKLM SOFTWARE\Rime\TypeDuck "TypeDuckRoot"
  StrCmp $R1 "" call_uninstaller
  IfFileExists $R1\data\*.* 0 call_uninstaller
  CreateDirectory $TEMP\TypeDuck_Backup
  CopyFiles $R1\data\*.* $TEMP\TypeDuck_Backup

call_uninstaller:
  ExecWait '$R0 /S'
  Sleep 800

done:
FunctionEnd

; The stuff to install
Section "TypeDuck"

  SectionIn RO

  ; Write the new installation path into the registry
  WriteRegStr HKLM SOFTWARE\Rime\TypeDuck "InstallDir" "$INSTDIR"

  ; Reset INSTDIR for the new version
  StrCpy $INSTDIR "${WEASEL_ROOT}"

  IfFileExists "$INSTDIR\TypeDuckServer.exe" 0 +2
  ExecWait '"$INSTDIR\TypeDuckServer.exe" /quit'

  SetOverwrite try
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  IfFileExists $TEMP\TypeDuck_Backup\*.* 0 program_files
  CreateDirectory $INSTDIR\data
  CopyFiles $TEMP\TypeDuck_Backup\*.* $INSTDIR\data
  RMDir /r $TEMP\TypeDuck_Backup

program_files:
  File "LICENSE.txt"
  File "typeduck.dll"
  ${If} ${RunningX64}
    File "typeduckx64.dll"
  ${EndIf}
  File "typeduckt.dll"
  ${If} ${RunningX64}
    File "typeducktx64.dll"
  ${EndIf}
  File "typeduck.ime"
  ${If} ${RunningX64}
    File "typeduckx64.ime"
  ${EndIf}
  File "typeduckt.ime"
  ${If} ${RunningX64}
    File "typeducktx64.ime"
  ${EndIf}
  File "TypeDuckDeployer.exe"
  File "TypeDuckServer.exe"
  File "TypeDuckSetup.exe"
  File "rime.dll"
  File "WinSparkle.dll"
  ; shared data files
  SetOutPath $INSTDIR\data
  File \
    /x "user.yaml" \
    /x "installation.yaml" \
    /x "trime.yaml" \
    /x "squirrel.yaml" \
    /x "jyut6ping3_mobile*.yaml" \
    /x "*_longpress*.yaml" \
    /x "*.custom.yaml" \
    "data\*.yaml"
  File /nonfatal "data\*.txt"
  File /nonfatal "data\*.gram"
  ; opencc data files
  SetOutPath $INSTDIR\data\opencc
  File "data\opencc\*.json"
  File "data\opencc\*.ocd*"

  SetOutPath $INSTDIR

  ExecWait "$INSTDIR\TypeDuckSetup.exe /t"

  ; run as user...
  ${If} ${Silent}
    ExecWait "$INSTDIR\TypeDuckDeployer.exe /setdefault"
  ${Else}
    ExecWait "$INSTDIR\TypeDuckDeployer.exe /install"
  ${EndIf}

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TypeDuck" "DisplayName" "TypeDuck"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TypeDuck" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TypeDuck" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TypeDuck" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Write autorun key
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "TypeDuckServer" "$INSTDIR\TypeDuckServer.exe"

  ${If} ${Silent}
    WriteRegStr HKLM SOFTWARE\Rime\TypeDuck\Updates "CheckForUpdates" 0
  ${EndIf}

  ; Start TypeDuckServer
  Exec "$INSTDIR\TypeDuckServer.exe"

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\TypeDuck"
  CreateShortCut "$SMPROGRAMS\TypeDuck\About TypeDuck….lnk" "$INSTDIR\TypeDuckDeployer.exe" "/about" "$SYSDIR\shell32.dll" 277
  CreateShortCut "$SMPROGRAMS\TypeDuck\IME Settings.lnk" "$INSTDIR\TypeDuckDeployer.exe" "" "$SYSDIR\shell32.dll" 21
  ; CreateShortCut "$SMPROGRAMS\TypeDuck\User Dictionary Management.lnk" "$INSTDIR\TypeDuckDeployer.exe" "/dict" "$SYSDIR\shell32.dll" 6
  ; CreateShortCut "$SMPROGRAMS\TypeDuck\User Data Sync.lnk" "$INSTDIR\TypeDuckDeployer.exe" "/sync" "$SYSDIR\shell32.dll" 26
  CreateShortCut "$SMPROGRAMS\TypeDuck\Redeploy.lnk" "$INSTDIR\TypeDuckDeployer.exe" "/deploy" "$SYSDIR\shell32.dll" 238
  CreateShortCut "$SMPROGRAMS\TypeDuck\Launch TypeDuck Server.lnk" "$INSTDIR\TypeDuckServer.exe" "" "$INSTDIR\TypeDuckServer.exe" 0
  ; CreateShortCut "$SMPROGRAMS\TypeDuck\User Folder.lnk" "$INSTDIR\TypeDuckServer.exe" "/userdir" "$SYSDIR\shell32.dll" 126
  ; CreateShortCut "$SMPROGRAMS\TypeDuck\Program Folder.lnk" "$INSTDIR\TypeDuckServer.exe" "/typeduckdir" "$SYSDIR\shell32.dll" 19
  ; CreateShortCut "$SMPROGRAMS\TypeDuck\Check for Updates.lnk" "$INSTDIR\TypeDuckServer.exe" "/update" "$SYSDIR\shell32.dll" 13
  ; CreateShortCut "$SMPROGRAMS\TypeDuck\Install Options.lnk" "$INSTDIR\TypeDuckSetup.exe" "" "$SYSDIR\shell32.dll" 162
  ; CreateShortCut "$SMPROGRAMS\TypeDuck\Uninstall TypeDuck.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"

  ExecWait '"$INSTDIR\TypeDuckServer.exe" /quit'

  ExecWait '"$INSTDIR\TypeDuckSetup.exe" /u'

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TypeDuck"
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "TypeDuckServer"
  DeleteRegKey HKLM SOFTWARE\Rime\TypeDuck

  ; Remove files and uninstaller
  SetOutPath $TEMP
  Delete /REBOOTOK "$INSTDIR\data\opencc\*.*"
  Delete /REBOOTOK "$INSTDIR\data\*.*"
  Delete /REBOOTOK "$INSTDIR\*.*"
  RMDir /REBOOTOK "$INSTDIR\data\opencc"
  RMDir /REBOOTOK "$INSTDIR\data"
  RMDir /REBOOTOK "$INSTDIR"
  SetShellVarContext all
  Delete /REBOOTOK "$SMPROGRAMS\TypeDuck\*.*"
  RMDir /REBOOTOK "$SMPROGRAMS\TypeDuck"

SectionEnd
