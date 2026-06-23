# TypeDuck Phase 03 VM Installer Verification Notes

**Mode:** automated Hyper-V / PowerShell Direct
**Status:** complete
**VM:** My Virtual Machine
**Checkpoint:** TypeDuck-Phase03-BeforeInstall-20260624-005531 / 0d09d7f9-a5e3-40ea-a181-3616398d84ef
**Started:** 2026-06-23T16:55:30Z
**Completed:** 2026-06-23T16:55:55Z
**Installer SHA-256:** 8c678b0f19491319d7b5026911b98d97edf0f26f9ec02fdad8a33be33acdcfc0

## Verification Results

- Installer exit code: 0
- Uninstaller exit code: 0
- CLSID: {7D92985A-BC53-47B5-A5CC-6E47F86B9D18}
- Profile GUID: {C6E8F5DF-6504-44F9-B7CF-17A195373A83}
- Display text: TypeDuck 粵語輸入法 / TypeDuck Cantonese IME
- Win32 DLL: C:\Windows\SysWOW64\TypeDuckTextService.dll
- x64 DLL: C:\Windows\System32\TypeDuckTextService.dll
- Startup value: TypeDuckLauncher
- Scheduled task: TypeDuckIME-ReRegisterTSF

## After-Install Failures

None

## After-Uninstall Failures

None

## Screenshots / Settings Evidence

- Automation collected registry, file, task, language-list, and uninstall-entry snapshots.
- If a screenshot is needed for product review, capture Windows Settings showing TypeDuck 粵語輸入法 / TypeDuck Cantonese IME under Chinese (Traditional, Hong Kong) and save it as zh-hk-input-settings.png.
- Screenshot capture must avoid personal typed content.

## Limitations

- Get-WinUserLanguageList did not report zh-HK for the guest user; rely on CTF/TIP registry evidence and capture Settings screenshot if needed.

## Source Audit

GOAL Phase 3: COVERED by Plans 03-01, 03-02, and this VM evidence packet.
INST-01: Bilingual TypeDuck-branded installer, covered by Plan 03-02 source plus installer execution evidence and optional UI screenshot.
INST-02: Select TypeDuck under Chinese (Traditional, Hong Kong), covered by TypeDuck zh-HK profile registry/language evidence and optional Settings screenshot.
INST-03: Deterministic CLSID/profile GUID/zh-HK/display text, covered by Plan 03-01 constants and VM registry snapshots.
INST-04: Win32 and x64 TSF DLL registration, covered by SysWOW64/System32 file hashes and regsvr-created registry snapshots.
INST-05: TypeDuck-owned uninstall cleanup, covered by after-uninstall snapshots.
D-01 through D-04: Covered by Plan 03-01 first-party TypeDuck profile registration.
D-05 through D-16: Covered by Plan 03-02 installer/setup/staging changes.
D-17 through D-19: Covered by this VM checkpoint and install/uninstall evidence; no host install was attempted.
