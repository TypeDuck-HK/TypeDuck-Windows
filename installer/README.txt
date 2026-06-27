TypeDuck Windows IME graphical installer (Inno Setup 6)
======================================================

Produces:
  - typeduck-windows-ime-setup.exe - wizard that installs Win32/x64 TSF DLLs plus launcher and TypeDuck runtime payload
  - After install: unins000.exe in the install directory - uninstaller

Prerequisites:
  1) A stage directory produced by scripts\install.ps1, containing:
     - win32\TypeDuckIME\TypeDuckLauncher.exe
     - win32\TypeDuckIME\TypeDuckSettings.exe
     - win32\TypeDuckIME\TypeDuckAbout.exe
     - win32\TypeDuckIME\TypeDuckTextService.dll
     - win32\TypeDuckIME\TypeDuckRuntime\server.exe
     - x64\TypeDuckIME\TypeDuckTextService.dll
  2) Inno Setup 6: https://jrsoftware.org/isdl.php

Build (PowerShell 7+):
  pwsh -NoProfile -ExecutionPolicy Bypass -File .\scripts\install.ps1
  or
  pwsh -NoProfile -ExecutionPolicy Bypass -File .\installer\build-installer.ps1 -StageDir C:\path\to\stage

Output:
  installer\dist\typeduck-windows-ime-setup.exe

Uninstall:
  - Settings -> Apps -> TypeDuck Windows IME, or
  - Run unins000.exe in the TypeDuckIME install folder, or
  - Start menu -> TypeDuckIME -> 解除安裝 Uninstall

Notes:
  - x64 only installer; it deploys Win32 payload to `%ProgramFiles(x86)%\TypeDuckIME` and x64 DLL to `%ProgramFiles%\TypeDuckIME`.
  - Installer requests Administrator for COM registration.
  - Fixed AppId inside `MoqiTsf.iss` should stay stable so Windows recognizes upgrades.
  - IME CLSID in the ISS must stay in sync with `MoqiTextService`.
