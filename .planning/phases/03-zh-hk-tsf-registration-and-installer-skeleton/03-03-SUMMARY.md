---
phase: 03-zh-hk-tsf-registration-and-installer-skeleton
plan: 03
subsystem: installer-registration
tags: [windows-tsf, hyper-v, powershell-direct, inno-setup, zh-hk, vm-evidence]

requires:
  - phase: 03-zh-hk-tsf-registration-and-installer-skeleton
    provides: TypeDuck TSF identity and TypeDuck installer skeleton from Plans 03-01 and 03-02
provides:
  - Hyper-V/PowerShell Direct VM verification harness for TypeDuck installer evidence
  - VM install/register/uninstall evidence for TypeDuck CLSID, zh-HK profile, dual-bitness DLLs, startup entry, scheduled-task state, and cleanup
  - Code-safe Inno uninstall cleanup for TypeDuck HKCU CTF TIP residue
affects: [phase-04-protocol, phase-05-settings-ui, phase-07-release-verification]

tech-stack:
  added: [PowerShell Direct verification harness, VM registry/file/task evidence snapshots]
  patterns: [host-safe VM installer verification, machine-readable installer evidence packet, manual fallback checklist]

key-files:
  created:
    - scripts/Test-TypeDuckVmInstallerVerification.ps1
    - scripts/Invoke-TypeDuckVmInstallerVerification.ps1
    - .planning/product/installer-fixtures/phase-03/registry-before.json
    - .planning/product/installer-fixtures/phase-03/registry-after-install.json
    - .planning/product/installer-fixtures/phase-03/registry-after-uninstall.json
    - .planning/product/installer-fixtures/phase-03/vm-install-registration-uninstall.json
    - .planning/product/installer-fixtures/phase-03/verification-notes.md
  modified:
    - installer/MoqiTsf.iss
    - .planning/PROJECT.md

key-decisions:
  - "Plan 03-03 accepts VM registry/file/task evidence as the automated proof for zh-HK registration; the guest user language list remained en-US, so a Settings screenshot remains optional product-review evidence rather than a blocking automated check."
  - "Installer cleanup uses code-safe GUID constants in Pascal registry paths while retaining escaped GUID constants for Inno identity fields."
  - "The verification harness never runs the installer on the host; install and uninstall actions are performed only through PowerShell Direct inside the named Hyper-V VM."

patterns-established:
  - "VM evidence packet pattern: registry-before, registry-after-install, registry-after-uninstall, notes, and aggregate JSON live together under .planning/product/installer-fixtures/phase-03."
  - "Credential hygiene pattern: guest credentials are supplied as runtime SecureString values and are not written to repo artifacts."

requirements-completed: ["INST-01", "INST-02", "INST-03", "INST-04", "INST-05"]

duration: 33 min active across checkpoint
completed: 2026-06-24
status: complete
---

# Phase 03 Plan 03: VM Installer Verification Summary

**Hyper-V VM evidence proves the TypeDuck installer registers zh-HK TSF metadata, dual-bitness DLLs, startup state, and clean uninstall behavior.**

## Performance

- **Duration:** 33 min active across checkpoint
- **Started:** 2026-06-23T16:24:27Z
- **Completed:** 2026-06-23T16:57:27Z
- **Tasks:** 3
- **Files modified:** 10

## Accomplishments

- Added `scripts/Invoke-TypeDuckVmInstallerVerification.ps1`, a host-safe Hyper-V/PowerShell Direct harness with a manual checklist fallback.
- Verified `installer\dist\typeduck-windows-ime-setup.exe` inside Hyper-V VM `My Virtual Machine`, using checkpoint `TypeDuck-Phase03-BeforeInstall-20260624-005531` (`0d09d7f9-a5e3-40ea-a181-3616398d84ef`).
- Captured before/install/uninstall VM snapshots proving TypeDuck CLSID `{7D92985A-BC53-47B5-A5CC-6E47F86B9D18}`, profile GUID `{C6E8F5DF-6504-44F9-B7CF-17A195373A83}`, `0x00000c04` zh-HK language profile, display text, SysWOW64/System32 DLL hashes, launcher startup entry, scheduled-task state, and cleanup.
- Fixed a VM-discovered uninstall cleanup bug where HKCU CTF TIP residue could remain because Inno Pascal registry strings used escaped GUID constants.

## Task Commits

1. **Task 1: Add the VM verification harness and checklist** - `a8bee6b` (test), `7b0d728` (feat)
2. **Checkpoint packet: Record VM credential gate** - `7c8956b` (docs)
3. **Task 1 follow-up: Render checklist values and UTC timestamps** - `ac2c25f`, `55e7e04` (fix)
4. **Task 2: Run clean VM install and registration verification** - `94c0a01` (test)
5. **Task 3: Run VM uninstall cleanup verification and publish source audit** - `a5c5a69` (fix)

**Plan metadata:** pending final docs commit

## Files Created/Modified

- `scripts/Test-TypeDuckVmInstallerVerification.ps1` - Contract check for harness parameters and checklist-only output.
- `scripts/Invoke-TypeDuckVmInstallerVerification.ps1` - Hyper-V/PowerShell Direct installer verification harness with manual fallback.
- `installer/MoqiTsf.iss` - Uses code-safe TypeDuck GUID constants in Pascal registry cleanup paths.
- `.planning/product/installer-fixtures/phase-03/registry-before.json` - VM baseline snapshot.
- `.planning/product/installer-fixtures/phase-03/registry-after-install.json` - VM after-install registry/file/task/language evidence.
- `.planning/product/installer-fixtures/phase-03/registry-after-uninstall.json` - VM after-uninstall cleanup evidence.
- `.planning/product/installer-fixtures/phase-03/vm-install-registration-uninstall.json` - Aggregate machine-readable evidence, status `complete`.
- `.planning/product/installer-fixtures/phase-03/verification-notes.md` - Human-readable VM notes, limitations, and source audit.

## Decisions Made

- Used PowerShell Direct with a runtime-only guest credential and did not persist credential material in files or commits.
- Treated `Get-WinUserLanguageList` returning only `en-US` as a documented limitation, while accepting CTF/TIP `LanguageProfile\0x00000c04\{C6E8...}` registry evidence as automated proof that the profile is registered for Chinese (Traditional, Hong Kong).
- Kept the installer artifact out of git; it remains generated at `installer\dist\typeduck-windows-ime-setup.exe` with SHA-256 `8c678b0f19491319d7b5026911b98d97edf0f26f9ec02fdad8a33be33acdcfc0`.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Fixed harness checklist rendering and UTC timestamp formatting**
- **Found during:** Task 1 and credential-gate evidence review
- **Issue:** Early checklist/evidence packets rendered some placeholders and culture-shaped timestamps.
- **Fix:** Updated the harness/test so checklist values and timestamps render deterministically.
- **Files modified:** `scripts/Invoke-TypeDuckVmInstallerVerification.ps1`, `scripts/Test-TypeDuckVmInstallerVerification.ps1`, `.planning/product/installer-fixtures/phase-03/verification-notes.md`, `.planning/product/installer-fixtures/phase-03/vm-install-registration-uninstall.json`
- **Verification:** `scripts\Test-TypeDuckVmInstallerVerification.ps1` passed.
- **Committed in:** `ac2c25f`, `55e7e04`

**2. [Rule 1 - Bug] Fixed harness failure aggregation**
- **Found during:** Task 2/3 VM run
- **Issue:** PowerShell collapsed zero/one-item failure lists to scalars, so `.Count` access failed after uninstall evidence collection.
- **Fix:** Wrapped install and uninstall failure collections in arrays.
- **Files modified:** `scripts/Invoke-TypeDuckVmInstallerVerification.ps1`
- **Verification:** Full VM harness completed and wrote status `complete`.
- **Committed in:** `a5c5a69`

**3. [Rule 1 - Bug] Fixed TypeDuck HKCU TIP cleanup**
- **Found during:** Task 3 VM uninstall verification
- **Issue:** After uninstall, `HKCU\Software\Microsoft\CTF\TIP\{7D92985A-BC53-47B5-A5CC-6E47F86B9D18}` remained.
- **Fix:** Added code-safe TypeDuck and legacy Moqi GUID constants for Inno Pascal registry paths, and explicitly removed the TypeDuck zh-HK profile subtree before deleting the parent TIP key.
- **Files modified:** `installer/MoqiTsf.iss`
- **Verification:** Rebuilt installer and reran VM harness; after-uninstall failures are empty.
- **Committed in:** `a5c5a69`

---

**Total deviations:** 3 auto-fixed (3 bugs)
**Impact on plan:** All fixes were required to make VM evidence truthful and to satisfy INST-05 cleanup. No broad cleanup or settings-flow scope was added.

## Authentication Gates

- PowerShell Direct initially required a guest credential. Execution paused at a human-action checkpoint after creating checkpoint `TypeDuck-Phase03-BeforeInstall-20260624-002900`; the credential was later supplied at runtime as a SecureString. No credential material was written to repository files, evidence artifacts, commit messages, summary, or final output.

## Issues Encountered

- The VM user language list did not include `zh-HK`; evidence therefore records CTF/TIP registry profile data under `0x00000c04` and notes that a Windows Settings screenshot can still be captured for product review.
- Inno logged a non-blocking Start Menu directory creation warning caused by the slash in the bilingual app name. Install/register/uninstall verification still passed; Start Menu polish is outside the Phase 3 acceptance boundary.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckVmInstallerVerification.ps1 -RepoRoot .`  
  **Result:** PASS
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckInstallerSkeleton.ps1 -RepoRoot . -Strict`  
  **Result:** PASS
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -SkipMoqiImeCopy`  
  **Result:** PASS - rebuilt `installer\dist\typeduck-windows-ime-setup.exe`.
- `scripts\Invoke-TypeDuckVmInstallerVerification.ps1` via PowerShell Direct against `My Virtual Machine`  
  **Result:** PASS - aggregate evidence status `complete`, install exit `0`, uninstall exit `0`, after-install failures `[]`, after-uninstall failures `[]`.
- Credential scan over scripts, installer, phase summary path, and evidence fixtures  
  **Result:** PASS - no credential text found.

## Known Stubs

None - no placeholder or unwired stub patterns were found in plan-owned script/evidence files. The evidence limitation around Settings screenshots is documented, not a stubbed result.

## Threat Flags

None - the trust boundaries exercised by this plan are covered by the plan threat model: host-to-VM installer transfer, checkpointed VM baseline, installer OS registration, and evidence artifact generation.

## User Setup Required

None - VM verification completed. The temporary guest credential used for PowerShell Direct should be removed from the VM when no longer needed.

## Next Phase Readiness

Phase 3 is complete. Phase 4 can build on a VM-verified TypeDuck installer skeleton with deterministic zh-HK TSF registration and clean uninstall behavior. Phase 5/7 should capture a visual Windows Settings screenshot when UI verification becomes the focus.

## Self-Check: PASSED

- Created files exist: `scripts/Invoke-TypeDuckVmInstallerVerification.ps1`, `scripts/Test-TypeDuckVmInstallerVerification.ps1`, `registry-before.json`, `registry-after-install.json`, `registry-after-uninstall.json`, `vm-install-registration-uninstall.json`, and `verification-notes.md`.
- Task commits exist: `a8bee6b`, `7b0d728`, `7c8956b`, `ac2c25f`, `55e7e04`, `94c0a01`, `a5c5a69`.
- VM evidence JSON status is `complete`, with install/uninstall exit code `0` and no install/uninstall failures.
- Credential scan passed.
- Only remaining dirty status is the pre-existing `jsoncpp` submodule marker.

---
*Phase: 03-zh-hk-tsf-registration-and-installer-skeleton*
*Completed: 2026-06-24*
