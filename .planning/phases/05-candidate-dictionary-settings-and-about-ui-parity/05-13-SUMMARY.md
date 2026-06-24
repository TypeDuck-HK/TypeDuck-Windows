---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 13
subsystem: ui-evidence
tags: [typeduck, evidence, installer, candidate-parity, package-provenance, vm-uat]
requires:
  - phase: 05-candidate-dictionary-settings-and-about-ui-parity
    provides: "05-10 candidate parity guard, 05-11 separate About executable, and 05-12 icon packaging fixes."
provides:
  - "Fresh rebuilt TypeDuck backend runtime, Windows Release binaries, staged payload, and installer package provenance."
  - "Aggregate Phase 5 evidence manifest aligned to fixed child guards and explicit VM/manual missing slots for 05-14."
  - "Final guard evidence including sibling candidate parity and aggregate VmEvidenceMissing expected-red validation."
affects: [phase-05, 05-14-vm-uat, 05-15-closeout, installer, evidence]
tech-stack:
  added: []
  patterns:
    - "Record generated package binaries through committed SHA-256 provenance instead of force-adding ignored installer outputs."
    - "Use VmEvidenceMissing aggregate red mode only for VM/manual slots; fixed child guards must run green."
key-files:
  created:
    - ".planning/product/ui-fixtures/phase-05/05-13-build-command-results.json"
    - ".planning/product/ui-fixtures/phase-05/05-13-preview-capture-results.json"
    - ".planning/product/ui-fixtures/phase-05/05-13-final-guard-results.json"
    - ".planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-13-SUMMARY.md"
  modified:
    - "scripts/Test-TypeDuckPhase05UiEvidence.ps1"
    - ".planning/product/ui-fixtures/phase-05/phase05-ui-evidence.json"
    - ".planning/product/ui-fixtures/phase-05/runtime-provenance.json"
    - ".planning/product/ui-fixtures/phase-05/screenshots/candidate-baseline-nei.bmp"
    - ".planning/product/ui-fixtures/phase-05/screenshots/candidate-multilingual-indonesian-main.bmp"
    - ".planning/product/ui-fixtures/phase-05/screenshots/dictionary-detail-housam-compound.bmp"
    - ".planning/product/ui-fixtures/phase-05/screenshots/dictionary-detail-multilingual.bmp"
    - ".planning/product/ui-fixtures/phase-05/screenshots/reverse-lookup-cangjie-onf.bmp"
key-decisions:
  - "05-13: Keep installer/dist and installer/stage as ignored generated outputs; commit package SHA-256 and staged artifact hashes in provenance instead."
  - "05-13: VmEvidenceMissing expected-red mode now keeps fixed settings/About and icon child guards green, and only treats VM/manual slots as unresolved."
  - "05-13: All required VM/manual evidence slots remain status=missing for Plan 05-14, even where older VM BMP files still exist on disk."
patterns-established:
  - "Final package evidence must include sibling candidate parity, package hash, staged executable hashes, runtime hashes, icon hashes, and repo dirty-state records."
  - "Non-VM preview captures are supporting evidence only and cannot satisfy VM-required candidate/settings/icon behavior."
requirements-completed: [INST-06, CAND-01, CAND-02, CAND-03, CAND-04, CAND-05, CAND-06, CAND-07, SET-01, SET-02, SET-03, SET-04, SET-05, SET-06, SET-07, SET-08, SET-09, SET-10, LANG-03]
duration: 10min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 13: Automated Evidence and Package Rebuild Summary

**Fresh TypeDuck runtime/package provenance with candidate parity and aggregate evidence validation ready for 05-14 VM UAT**

## Performance

- **Duration:** 10 min
- **Started:** 2026-06-24T08:31:05Z
- **Completed:** 2026-06-24T08:40:47Z
- **Tasks:** 2
- **Files modified:** 12

## Accomplishments

- Rebuilt the sibling TypeDuck runtime, rebuilt Release Win32/x64 Windows binaries, staged the runtime, and compiled `installer/dist/typeduck-windows-ime-setup.exe` without running the installer on the host.
- Recorded current repository heads/dirty states, package SHA-256, staged executable hashes, runtime file hashes, icon hashes, preview capture hashes, and command results in the evidence manifest/provenance files.
- Regenerated non-VM candidate preview/screenshots from the rebuilt Release preview executable.
- Fixed the aggregate validator so `-ExpectRed VmEvidenceMissing` proves only VM/manual evidence is absent while settings/About and icon child guards run green.
- Marked all required VM/manual evidence slots as explicit `missing` for Plan 05-14.

## Package Evidence

- **Installer:** `installer/dist/typeduck-windows-ime-setup.exe`
- **SHA-256:** `59757083C0B11A7D9676C1AE86B9176A4FF8382AB12283DFE5690DCB64F55CE5`
- **Bytes:** `22692039`
- **Backend server expected hash:** `058CDC6C2D1E9DA30C002C13F50842CECE22955BBD302FBC5FE9FC53180A9511`
- **TypeDuck `rime.dll` hash:** `5783B84916FEFC0DD5DDA28F1D7292A9CA86C75F05132947FE816178B916C04B`

## Task Commits

1. **Task 1: Run full guard/build/package path and refresh provenance** - `6cb8741` (test)
2. **Task 2: Refresh non-VM evidence and aggregate slots** - `bb4c007` (test)

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File D:\VSProjects\moqi-ime\scripts\build.ps1 -RepoRoot D:\VSProjects\moqi-ime -RimeDataSource I:\GitHub\TypeDuck-Web\schema` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\build.ps1 -RepoRoot . -Configuration Release` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -MoqiImeSource D:\VSProjects\moqi-ime\scripts\build\moqi-ime` - PASS, compiled installer/package.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File D:\VSProjects\moqi-ime\scripts\Test-TypeDuckCandidateParity.ps1 -RepoRoot D:\VSProjects\moqi-ime -WindowsRepoRoot D:\VSProjects\moqi-im-windows -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckAppearanceTheme.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckCandidateData.ps1 -RepoRoot . -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckCandidateWindow.ps1 -RepoRoot . -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsPersistence.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsAboutUi.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckIconPackaging.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckPhase05UiEvidence.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict -ExpectRed VmEvidenceMissing` - PASS RED only for missing VM/manual evidence.

## Remaining VM Evidence Slots

`installerFirstRunSettings`, `postInstallSettingsEntryPoint`, `settingsApplyPersistence`, `settingsPersistenceAfterRestart`, `separateAboutExecutable`, `notepadCandidateNei`, `browserCandidateHousam`, `movementReveal`, `stationaryPointerNoFlicker`, `highDpiPlacement`, `multiMonitorPlacement`, `uiLessHost`, `imperfectCompositionRectangle`, `inputPickerAndExecutableIcons`.

## Files Created/Modified

- `.planning/product/ui-fixtures/phase-05/runtime-provenance.json` - Current repo state, runtime/package hashes, command results, and provenance for the rebuilt package.
- `.planning/product/ui-fixtures/phase-05/phase05-ui-evidence.json` - Current package/preview evidence plus explicit VM/manual missing slots for 05-14.
- `.planning/product/ui-fixtures/phase-05/05-13-build-command-results.json` - Rebuild/stage/package command result record.
- `.planning/product/ui-fixtures/phase-05/05-13-preview-capture-results.json` - Release preview capture command/hash record.
- `.planning/product/ui-fixtures/phase-05/05-13-final-guard-results.json` - Final strict guard result record.
- `.planning/product/ui-fixtures/phase-05/screenshots/*.bmp` - Refreshed active non-VM candidate/dictionary preview evidence.
- `scripts/Test-TypeDuckPhase05UiEvidence.ps1` - Fixes aggregate red-mode handling for `VmEvidenceMissing`.

## Decisions Made

- Do not force-add ignored generated binaries under `installer/dist` or `installer/stage`; the committed evidence records the exact package and staged hashes.
- Keep VM/manual slots unresolved in the manifest for 05-14. Preview captures remain supporting evidence only.
- Treat the initial prebuild candidate-parity hash mismatch as stale-provenance sequencing, not a product failure; final parity passed after rebuild/provenance refresh.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Fixed aggregate red-mode handling for VM evidence**
- **Found during:** Task 2 final aggregate verification
- **Issue:** `Test-TypeDuckPhase05UiEvidence.ps1 -ExpectRed VmEvidenceMissing` incorrectly invoked settings/About and icon child guards with `-ExpectRed RejectedUatBehavior`, causing a failure after those child guards were correctly fixed green.
- **Fix:** Limited child `RejectedUatBehavior` red mode to `-ExpectRed RejectedUatBehavior`; `VmEvidenceMissing` now runs child guards green and checks only missing VM/manual slots.
- **Files modified:** `scripts/Test-TypeDuckPhase05UiEvidence.ps1`
- **Verification:** Final aggregate command passed with only `VmEvidenceMissing`.
- **Committed in:** `bb4c007`

---

**Total deviations:** 1 auto-fixed bug.
**Impact on plan:** Required to prove the intended 05-13 state: all automated/package/preview guards green, VM/manual evidence still missing for 05-14.

## Issues Encountered

- A prebuild candidate parity attempt failed because `runtime-provenance.json` still expected the previous backend `server.exe` hash. The backend was rebuilt, provenance was refreshed to `058CDC6C...`, and the final candidate parity guard passed.
- Inno Setup emitted existing warnings for deprecated `x64` architecture naming and HKCU usage under admin install mode. The compile succeeded and the resulting installer SHA is recorded.
- The sibling backend repo remains dirty from prior icon/runtime source changes; 05-13 records that state in provenance and does not commit sibling changes.

## Known Stubs

None. Stub scan found no blocking TODO/FIXME/placeholder/coming-soon/not-available markers in touched files. The only empty-string match was the validator's optional `ExpectRed` parameter default.

## Threat Flags

None beyond the plan threat model. The package-to-evidence tampering risk is mitigated by recorded hashes and final guard results; preview evidence remains separated from VM-required behavior.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 05-14 can install and test the current package identified by SHA-256 `59757083C0B11A7D9676C1AE86B9176A4FF8382AB12283DFE5690DCB64F55CE5` and fill the explicit VM/manual evidence slots.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-13-SUMMARY.md`.
- Task commits found: `6cb8741`, `bb4c007`.
- Current installer package exists at `installer/dist/typeduck-windows-ime-setup.exe` with SHA-256 `59757083C0B11A7D9676C1AE86B9176A4FF8382AB12283DFE5690DCB64F55CE5`.
- Final aggregate validator passed with `-ExpectRed VmEvidenceMissing`.

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
