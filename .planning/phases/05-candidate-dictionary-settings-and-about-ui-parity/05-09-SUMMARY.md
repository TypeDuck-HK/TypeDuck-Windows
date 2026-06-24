---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 09
subsystem: evidence-guard-provenance
tags: [typeduck, phase-05, uat, evidence, provenance, powershell]

requires:
  - phase: 05-08
    provides: TypeDuck icon and About resource packaging guard baseline
provides:
  - Phase 5 guards that RED-pass on rejected settings/About and icon packaging behavior
  - Clean Phase 5 evidence manifest without stale cropped settings or incorrect About slots
  - Runtime/Web provenance record for Windows repo, sibling backend, and TypeDuck Web source
affects: [05-10-gap-fixes, 05-11-candidate-parity, 05-12-settings-about-retry, 05-13-vm-evidence, 05-14-icon-packaging-retry, 05-15-closeout]

tech-stack:
  added: [PowerShell guard RED modes, JSON provenance manifest]
  patterns:
    - Guard rejected UAT behavior with explicit ExpectRed modes before retry fixes are accepted.
    - Treat evidence screenshots as named slots that must not be satisfied by stale files.
    - Require machine-readable provenance for dirty external repos before candidate/runtime/icon evidence is accepted.

key-files:
  created:
    - .planning/product/ui-fixtures/phase-05/runtime-provenance.json
  modified:
    - scripts/Test-TypeDuckSettingsAboutUi.ps1
    - scripts/Test-TypeDuckIconPackaging.ps1
    - scripts/Test-TypeDuckPhase05UiEvidence.ps1
    - .planning/product/ui-fixtures/phase-05/phase05-ui-evidence.json
    - .planning/product/ui-fixtures/phase-05/manual-uat-notes.md

key-decisions:
  - "05-09: Rejected UAT behavior is now an explicit RED-mode guard state, not an acceptable green baseline."
  - "05-09: Active Phase 5 evidence slots no longer include cropped settings-two-column or incorrect About-dialog evidence."
  - "05-09: Retry evidence must cite runtime-provenance.json for Windows, backend, TypeDuck Web, runtime DLL/schema/theme, and icon hashes."

patterns-established:
  - "Use -ExpectRed RejectedUatBehavior to prove current rejected behavior is caught before retry fixes."
  - "Use -ExpectRed VmEvidenceMissing while VM/manual evidence is intentionally incomplete."

requirements-completed: [INST-06, CAND-01, CAND-02, CAND-03, CAND-04, CAND-05, CAND-06, CAND-07, SET-01, SET-02, SET-03, SET-04, SET-05, SET-06, SET-07, SET-08, SET-09, SET-10, LANG-03]

duration: 8 min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 09: Gap-Closure Guard and Provenance Summary

**Rejected Phase 5 UAT behavior now fails guards, stale evidence slots are removed, and retry evidence is tied to exact runtime/Web provenance.**

## Performance

- **Duration:** 8 min
- **Started:** 2026-06-24T07:29:48Z
- **Completed:** 2026-06-24T07:37:00Z
- **Tasks:** 3/3
- **Files modified:** 6

## Accomplishments

- Added `RejectedUatBehavior` RED modes to the settings/About, icon packaging, and aggregate evidence guards.
- Made the settings guard reject visible `TypeDuckPreferences.json` UI copy, unsupported-state copy, missing 4-10 page-size tick labels, and an in-settings About button while About is moving to a separate executable.
- Made the icon guard reject raw staged `.ico` files under the installed product root and legacy packaged `moqi-ime/input_methods/rime/icon.ico`.
- Removed active `settingsTwoColumn` and `aboutDialog` evidence slots and replaced them with `settingsApplyPersistence`, `settingsPersistenceAfterRestart`, and `separateAboutExecutable`.
- Added `.planning/product/ui-fixtures/phase-05/runtime-provenance.json` with exact heads, dirty states, and hashes for the Windows repo, sibling backend, TypeDuck Web, runtime artifacts, Web fixture metadata, and TypeDuck icon sources.

## Task Commits

Each task was committed atomically:

1. **Task 1: Make rejected UAT behavior fail guards first** - `82cba56` (test)
2. **Task 2: Clean stale evidence screenshots and manifest slots** - `97636ea` (fix)
3. **Task 3: Record runtime and Web source provenance for retry evidence** - `606ca12` (docs)

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsAboutUi.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict -ExpectRed RejectedUatBehavior` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckIconPackaging.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict -ExpectRed RejectedUatBehavior` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckPhase05UiEvidence.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict -ExpectRed RejectedUatBehavior` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckPhase05UiEvidence.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict -ExpectRed VmEvidenceMissing` - PASS.

## Files Created/Modified

- `scripts/Test-TypeDuckSettingsAboutUi.ps1` - Adds rejected settings/About RED mode and hard failures for UAT 7/8 behavior.
- `scripts/Test-TypeDuckIconPackaging.ps1` - Adds rejected icon packaging RED mode and hard failures for raw staged icons and legacy runtime icon.
- `scripts/Test-TypeDuckPhase05UiEvidence.ps1` - Adds aggregate rejected-behavior RED routing, stale slot rejection, current required slot names, and runtime provenance validation.
- `.planning/product/ui-fixtures/phase-05/phase05-ui-evidence.json` - Removes stale settings/About slots and marks current required evidence states.
- `.planning/product/ui-fixtures/phase-05/manual-uat-notes.md` - Records removed slots and provenance requirement for retry evidence.
- `.planning/product/ui-fixtures/phase-05/runtime-provenance.json` - Records repo heads, dirty states, runtime/icon/Web fixture hashes, and lookup-filter provenance.

## Decisions Made

- Rejected UAT behavior is intentionally represented as RED-mode proof until later plans fix the product/runtime behavior.
- Stale screenshot filenames and stale slot names are banned from the active manifest, even when mentioned in guard source as rejected names.
- Provenance validation checks external backend and TypeDuck Web heads against the recorded values, while recording this repo’s shared-worktree base for traceability.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Allowed Task 2 VM-evidence RED check before Task 3 provenance exists**
- **Found during:** Task 2 verification
- **Issue:** The aggregate guard’s new provenance requirement initially blocked the Task 2 `VmEvidenceMissing` RED command before Task 3 created `runtime-provenance.json`.
- **Fix:** Kept provenance mandatory for normal acceptance, but allowed `-ExpectRed VmEvidenceMissing` to proceed before provenance exists. Once Task 3 creates the file, the same RED command validates it.
- **Files modified:** `scripts/Test-TypeDuckPhase05UiEvidence.ps1`
- **Verification:** Task 2 and Task 3 `VmEvidenceMissing` RED commands both passed.
- **Committed in:** `97636ea`

---

**Total deviations:** 1 auto-fixed (Rule 3 blocking)
**Impact on plan:** The adjustment preserves the plan’s task order and keeps provenance mandatory before any non-RED evidence acceptance.

## Issues Encountered

- One stale `vm-about-dialog.bmp` reference remained in manifest notes after the first cleanup patch. The aggregate guard caught it; the note was rewritten and verification passed.
- No installer was run on the host.

## Known Stubs

None. Stub scan found no TODO, FIXME, placeholder, coming-soon, or unavailable user-facing stubs in the files created or modified by this plan.

## Threat Flags

None beyond the plan threat model. The changed trust surfaces are planned evidence/provenance validation files; no new network endpoint, auth path, file-access runtime path, or schema trust boundary was introduced.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Ready for Plans 05-10 through 05-15. The guards now encode the rejected UAT baseline, active evidence slots are clean, and retry evidence has a provenance foundation for runtime, icon, candidate, Web fixture, and VM proof.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-09-SUMMARY.md`.
- Created provenance file exists at `.planning/product/ui-fixtures/phase-05/runtime-provenance.json`.
- Task commits found: `82cba56`, `97636ea`, `606ca12`.
- Rejected screenshot files `settings-two-column-layout.bmp` and `vm-about-dialog.bmp` are absent from the active screenshot directory.
- Required final verification commands passed.

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
