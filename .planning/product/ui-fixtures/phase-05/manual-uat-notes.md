# Phase 5 Manual / VM UAT Notes

Status: partial, blocked
VM: My Virtual Machine
Interactive route: active console user `user`
PowerShell Direct credential used for orchestration: `typeduckverify`
Installer safety: installer was copied into the VM and run inside the VM only; it was not run on the host.

## What Worked

- Active-session scheduled tasks with `/RU user /IT /RL HIGHEST` worked from PowerShell Direct.
- Installed package inside the VM returned exit code 0.
- Installed `TypeDuckSettings.exe` launched from `C:\Program Files (x86)\TypeDuckIME`.
- Settings UI screenshot shows Display Languages first and the two-column settings layout.
- Applying settings displayed `設定已儲存 / Settings saved` and wrote `C:\Users\User\AppData\Local\TypeDuckIME\TypeDuckPreferences.json`.
- The active user default input method override accepted the TypeDuck zh-HK TIP: `0C04:{7D92985A-BC53-47B5-A5CC-6E47F86B9D18}{C6E8F5DF-6504-44F9-B7CF-17A195373A83}`.
- Notepad typing `nei` showed the TypeDuck candidate popup and Cantonese candidates.
- Notepad typing `housam` showed TypeDuck candidate rows for the compound sample.

## Blocking Gaps

- Visible installer-first-run settings dialog was not captured. The VM install used `/VERYSILENT`, and the Inno `[Run]` entry has `skipifsilent`.
- About dialog visible capture failed. Source/package guards verify its content, but active VM attempts by control message, keyboard Enter, and coordinate click did not show a usable About dialog capture.
- Dictionary detail reveal failed in live VM screenshots: pointer movement over `nei` and `housam` candidates did not show the side dictionary panel.
- Browser host-app evidence was not captured.
- High-DPI, multi-monitor/extended desktop, UI-less TSF host, imperfect composition rectangle fallback, and visible input-picker/icon evidence were not exercised.

## Evidence Files

- `screenshots/vm-postinstall-settings-entrypoint.bmp`
- `screenshots/vm-settings-apply-persistence.bmp`
- `screenshots/vm-notepad-candidate-nei.bmp`
- `screenshots/vm-browser-candidate-housam.bmp` (captured in Notepad; browser evidence still missing)
- `screenshots/vm-movement-reveal-nei.bmp` (candidate list visible; dictionary detail did not reveal)
- `screenshots/vm-housam-movement-reveal.bmp` (candidate list visible; dictionary detail did not reveal)
- `screenshots/vm-stationary-pointer-no-flicker.bmp`

## Current Conclusion

Phase 5 automated/package evidence is strong, and live VM confirms settings launch plus basic Notepad candidate behavior. The phase should remain pending because Task 3 requires dictionary detail reveal, About dialog visibility, browser host-app, high-DPI, multi-monitor, UI-less, imperfect rectangle, and visible icon/input-picker evidence.
