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
OutFile "archives\TypeDuck-${WEASEL_VERSION}-dev.${WEASEL_BUILD}-installer.exe"

VIProductVersion "${WEASEL_VERSION}.${WEASEL_BUILD}"
VIAddVersionKey "ProductName" "TypeDuck"
VIAddVersionKey "Comments" "Driven by Weasel"
VIAddVersionKey "CompanyName" "The Education University of Hong Kong"
VIAddVersionKey "LegalCopyright" "© The Education University of Hong Kong"
VIAddVersionKey "FileDescription" "TypeDuck IME"
VIAddVersionKey "FileVersion" "${WEASEL_VERSION}"

!define MUI_ICON ..\resource\TypeDuck.ico
SetCompressor /SOLID lzma

; The default installation directory
InstallDir $PROGRAMFILES\Rime

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Rime\TypeDuck" "InstallDir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
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
  File "README.txt"
  File "7-zip-license.txt"
  File "7z.dll"
  File "7z.exe"
  File "COPYING-curl.txt"
  File "curl.exe"
  File "curl-ca-bundle.crt"
  File "rime-install.bat"
  File "rime-install-config.bat"
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
  File /x "user.yaml" /x "installation.yaml" /x "trime.yaml" /x "squirrel.yaml" "data\*.yaml"
  File /nonfatal "data\*.txt"
  File /nonfatal "data\*.gram"
  ; opencc data files
  SetOutPath $INSTDIR\data\opencc
  File "data\opencc\*.json"
  File "data\opencc\*.ocd*"

  SetOutPath $INSTDIR

  ExecWait "$INSTDIR\TypeDuckSetup.exe /t"

  ; run as user...
  ExecWait "$INSTDIR\TypeDuckDeployer.exe /install"

  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TypeDuck" "DisplayName" "TypeDuck"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TypeDuck" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TypeDuck" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\TypeDuck" "NoRepair" 1
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; Write autorun key
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "TypeDuckServer" "$INSTDIR\TypeDuckServer.exe"
  ; Start TypeDuckServer
  Exec "$INSTDIR\TypeDuckServer.exe"

  ; Prompt reboot
  StrCmp $0 "Upgrade" 0 +2
  SetRebootFlag true

SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\TypeDuck"
  CreateShortCut "$SMPROGRAMS\TypeDuck\IME Settings.lnk" "$INSTDIR\TypeDuckDeployer.exe" "" "$SYSDIR\shell32.dll" 21
  CreateShortCut "$SMPROGRAMS\TypeDuck\User Dictionary Management.lnk" "$INSTDIR\TypeDuckDeployer.exe" "/dict" "$SYSDIR\shell32.dll" 6
  CreateShortCut "$SMPROGRAMS\TypeDuck\User Data Sync.lnk" "$INSTDIR\TypeDuckDeployer.exe" "/sync" "$SYSDIR\shell32.dll" 26
  CreateShortCut "$SMPROGRAMS\TypeDuck\Redeploy.lnk" "$INSTDIR\TypeDuckDeployer.exe" "/deploy" "$SYSDIR\shell32.dll" 144
  CreateShortCut "$SMPROGRAMS\TypeDuck\TypeDuck Server.lnk" "$INSTDIR\TypeDuckServer.exe" "" "$INSTDIR\TypeDuckServer.exe" 0
  CreateShortCut "$SMPROGRAMS\TypeDuck\User Folder.lnk" "$INSTDIR\TypeDuckServer.exe" "/userdir" "$SYSDIR\shell32.dll" 126
  CreateShortCut "$SMPROGRAMS\TypeDuck\Program Folder.lnk" "$INSTDIR\TypeDuckServer.exe" "/typeduckdir" "$SYSDIR\shell32.dll" 19
  CreateShortCut "$SMPROGRAMS\TypeDuck\Check for Updates.lnk" "$INSTDIR\TypeDuckServer.exe" "/update" "$SYSDIR\shell32.dll" 13
  ; CreateShortCut "$SMPROGRAMS\TypeDuck\Install Options.lnk" "$INSTDIR\TypeDuckSetup.exe" "" "$SYSDIR\shell32.dll" 162
  CreateShortCut "$SMPROGRAMS\TypeDuck\Uninstall TypeDuck.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

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

  ; Prompt reboot
  SetRebootFlag true

SectionEnd
