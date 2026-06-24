---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 12
subsystem: packaging
tags: [installer, icons, executable-resources, tsf, backend-runtime, uat-gap-closure]
requires:
  - phase: 05-candidate-dictionary-settings-and-about-ui-parity
    provides: "05-08 icon source/resource decisions and 05-11 TypeDuckAbout.exe staging foundation."
provides:
  - "Strict icon packaging guard for UAT gap 9."
  - "Installed app-root staging without raw TypeDuck*.ico files."
  - "Packaged runtime without input_methods/rime/icon.ico."
  - "TypeDuck executable/icon resources for launcher, settings, About, setup helper, backend server, TSF input picker, and installer/uninstaller surfaces."
affects: [phase-05, installer, packaging, backend-runtime, uat]
tech-stack:
  added: []
  patterns:
    - "Use source icon assets only for PE resource stamping and installer metadata, not app-root raw files."
    - "Use staged PE payload inspection in PowerShell guards for executable icon evidence."
key-files:
  created:
    - ".planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-12-SUMMARY.md"
  modified:
    - "scripts/Test-TypeDuckIconPackaging.ps1"
    - "scripts/install.ps1"
    - "installer/MoqiTsf.iss"
    - "MoqiTextService/TypeDuckProfile.cpp"
    - "D:/VSProjects/moqi-ime/scripts/build.ps1"
    - "D:/VSProjects/moqi-ime/input_methods/rime/icon.ico"
key-decisions:
  - "TypeDuck input picker registration now uses the compiled TypeDuckTextService.dll icon resource instead of an installed raw TypeDuck_Small.ico file."
  - "Uninstall display icon now resolves from the compiled uninstaller executable instead of an app-root TypeDuck.ico file."
  - "The backend package build stamps server.exe from TypeDuck_Transparent.ico and removes the legacy runtime input_methods/rime/icon.ico after copying Rime metadata."
patterns-established:
  - "Icon packaging guard checks both static packaging rules and staged PE icon payloads."
  - "Installer staging applies executable icon resources without copying TypeDuck icon assets into the app root."
requirements-completed: [INST-06, CAND-04, CAND-05, LANG-03]
duration: 15min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 12: Icon Packaging Gap Closure Summary

**TypeDuck icon packaging now uses executable, TSF, backend, and installer resources without raw app-root icon leakage or the legacy Rime runtime icon.**

## Performance

- **Duration:** 15 min
- **Started:** 2026-06-24T08:14:51Z
- **Completed:** 2026-06-24
- **Tasks:** 2
- **Files modified:** 6

## Accomplishments

- Added strict guard coverage that rejects raw `TypeDuck*.ico` files in `installer/stage/win32/TypeDuckIME`, rejects packaged `moqi-ime/input_methods/rime/icon.ico`, and inspects staged PE icon payloads.
- Removed app-root raw icon staging and changed TSF/uninstaller icon surfaces to use compiled resources instead of raw installed `.ico` files.
- Updated the sibling backend package build to stamp `server.exe` from `TypeDuck_Transparent.ico` and removed the tracked legacy `input_methods/rime/icon.ico`.
- Stamped `TypeDuckLauncher.exe`, `TypeDuckSettings.exe`, `TypeDuckAbout.exe`, `TypeDuckSetupHelper.exe`, and packaged `moqi-ime/server.exe` during staging.

## Task Commits

1. **Task 1 RED: Reject raw icon leakage and legacy runtime icon files** - `ef171a3` (test)
2. **Task 1 GREEN: Backend runtime icon cleanup** - `2a54521` in `D:/VSProjects/moqi-ime` (fix)
3. **Task 1 GREEN: Windows app-root icon cleanup** - `59ee70e` (fix)
4. **Task 2 RED: Stamp executable, input-picker, and installer icon surfaces** - `582647a` (test)
5. **Task 2 GREEN: Stamp TypeDuckAbout executable icon** - `6160dd7` (fix)

## Files Created/Modified

- `scripts/Test-TypeDuckIconPackaging.ps1` - Adds static and staged-package checks for raw icon leakage, legacy runtime icon removal, PE icon payloads, TSF profile icon resource, and installer icon metadata.
- `scripts/install.ps1` - Stops copying raw TypeDuck icons into the app root, filters `input_methods/rime/icon.ico`, and stamps `TypeDuckAbout.exe`.
- `installer/MoqiTsf.iss` - Uses `{uninstallexe}` for uninstall display icon so no app-root `TypeDuck.ico` is required.
- `MoqiTextService/TypeDuckProfile.cpp` - Registers the TypeDuck profile with the DLL icon resource path and resource index.
- `D:/VSProjects/moqi-ime/scripts/build.ps1` - Uses `TypeDuck_Transparent.ico` for `server.exe` version resources and removes packaged `input_methods/rime/icon.ico`.
- `D:/VSProjects/moqi-ime/input_methods/rime/icon.ico` - Deleted intentionally; legacy runtime icon file must not be packaged.

## Decisions Made

- Use the compiled `TypeDuckTextService.dll` `TypeDuck_Small.ico` resource for TSF/input-picker registration rather than an installed raw icon file.
- Use the compiled uninstaller executable as the uninstall display icon source, preserving `TypeDuck.ico` branding without copying `TypeDuck.ico` into the installed app root.
- Keep backend source icon assets under the sibling backend `icons/` source/package subtree; the ban applies to raw icon files in the installed app root and the legacy Rime `input_methods/rime/icon.ico`.

## Deviations from Plan

None - plan executed within the icon packaging/stamping scope.

## Issues Encountered

- The first Task 1 guard draft overmatched `programDirEnvVar()` in `TypeDuckProfile.cpp`; it was narrowed before the green commit to reject only the raw icon lookup helper and filename.
- The sibling backend had pre-existing dirty files (`go.mod`, Rime icon files, `ime.json`, `librime.go`, and untracked source icons). They were left unstaged and uncommitted except for this plan's `scripts/build.ps1` change and `input_methods/rime/icon.ico` deletion.
- Inno Setup emitted existing warnings for deprecated `x64` architecture naming and HKCU usage under admin install mode; compile still succeeded.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File D:\VSProjects\moqi-ime\scripts\build.ps1 -RepoRoot D:\VSProjects\moqi-ime -RimeDataSource I:\GitHub\TypeDuck-Web\schema`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -MoqiImeSource D:\VSProjects\moqi-ime\scripts\build\moqi-ime`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckIconPackaging.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`

## Known Stubs

None. Stub scan found only benign script parameter defaults and version-info empty metadata fields, not UI/rendering stubs.

## Threat Flags

None. The plan's executable-icon spoofing and install-tree tampering mitigations are covered by staged PE icon checks and raw-file rejection.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

05-12 closes UAT gap 9 at the packaging/guard level. 05-13 can rebuild/package evidence from the committed scripts without expecting raw root icon files or legacy `input_methods/rime/icon.ico`.

## Self-Check: PASSED

- FOUND: `scripts/Test-TypeDuckIconPackaging.ps1`
- FOUND: `scripts/install.ps1`
- FOUND: `installer/MoqiTsf.iss`
- FOUND: `MoqiTextService/TypeDuckProfile.cpp`
- FOUND: `D:/VSProjects/moqi-ime/scripts/build.ps1`
- FOUND: `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-12-SUMMARY.md`
- FOUND: `ef171a3`
- FOUND: `2a54521`
- FOUND: `59ee70e`
- FOUND: `582647a`
- FOUND: `6160dd7`

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
