---
phase: 06-privacy-security-and-scaffold-cleanup
plan: 06
subsystem: verification
tags: [privacy, security, ci, installer, runtime, powershell]

requires:
  - phase: 06-01
    provides: TypeDuck-only installer cleanup boundaries
  - phase: 06-02
    provides: TypeDuckRuntime package pruning contract
  - phase: 06-03
    provides: fixed TypeDuck runtime bridge
  - phase: 06-04
    provides: TypeDuck diagnostics path and privacy cleanup
  - phase: 06-05
    provides: launcher protocol and IPC hardening
  - phase: 06-07
    provides: backend off-scope feature removal
provides:
  - Strict aggregate Phase 6 guard covering focused installer, runtime, backend, settings/About, icon, and IPC checks
  - TypeDuck-owned release/nightly artifact names and checkout paths
  - Compact Phase 6 guard evidence JSON
affects: [phase-07-release-verification, ci-release, installer-packaging]

tech-stack:
  added: []
  patterns:
    - Aggregate PowerShell guard orchestrates focused guards through named-parameter splatting
    - Guard evidence is compact JSON with UTC timestamp, commands, status, and category names only

key-files:
  created:
    - .planning/product/privacy-security/phase-06-guard-results.json
  modified:
    - scripts/Test-TypeDuckPrivacySecurityCleanup.ps1
    - .github/workflows/release.yml
    - .github/workflows/nightly.yml
    - installer/README.txt

key-decisions:
  - "Phase 6 verification is centralized in scripts/Test-TypeDuckPrivacySecurityCleanup.ps1 -Strict, which now invokes the focused guards instead of relying on separate manual runs."
  - "Release and nightly workflows use TypeDuck-Windows, TypeDuck-Windows-backend, TypeDuck-HK/schema on aap2-alpha, and TypeDuck-owned installer artifact names."
  - "Guard evidence records only status, categories, command lines, and roots; it does not include raw typed samples or secrets."

patterns-established:
  - "Focused guard orchestration: aggregate guards should collect focused failures as guard violations, then write pass/fail evidence before exiting."
  - "CI package surface checks: release verification should reject legacy workflow paths, artifact names, rime-frost, powershell.exe script invocation, and standalone schema/build uploads."

requirements-completed: [IDEN-01, IDEN-03, IDEN-04, LANG-01, LANG-02, SEC-01, SEC-02, SEC-03, SEC-04, SEC-05, VER-02]

duration: 10 min
completed: 2026-06-27
status: complete
---

# Phase 06 Plan 06: Aggregate Guard and Release Artifact Naming Summary

**Strict Phase 6 privacy/security guard with backend-aware coverage, TypeDuck CI artifact naming, and compact guard evidence JSON**

## Performance

- **Duration:** 10 min
- **Started:** 2026-06-27T11:26:00Z
- **Completed:** 2026-06-27T11:36:00Z
- **Tasks:** 3
- **Files modified:** 5

## Accomplishments

- Made `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1 -Strict` invoke the installer, runtime pruning, backend diagnostics, launcher protocol, settings/About, and icon packaging guards.
- Added explicit aggregate coverage for D-18/D-19 Legacy Moqi coexistence, D-28 publisher text, D-29 final-page restart guidance, D-23 banned surfaces, D-24 internal identifier allowances, and D-27 backend diagnostics/package checks.
- Renamed release/nightly workflow surfaces to `typeduck-windows-ime-*`, `TypeDuck-Windows`, and `TypeDuck-Windows-backend`; added TypeDuck schema checkout on `aap2-alpha` and Rime deployer runtime build preparation.
- Recorded `.planning/product/privacy-security/phase-06-guard-results.json` with pass status, UTC timestamp, guard commands, and compact category coverage.

## Task Commits

1. **Task 1: Finalize aggregate banned-surface guard** - `628f08a` (`feat`)
2. **Task 2: Rename CI artifact surfaces and script invocations** - `c867594` (`feat`)
3. **Task 3: Record final Phase 6 guard evidence** - `aeea464` (`test`)

## Files Created/Modified

- `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1` - Aggregate guard orchestration, CI/staging-doc checks, D-18/D-19/D-28/D-29 assertions, and JSON evidence writer.
- `.github/workflows/release.yml` - TypeDuck checkout paths, TypeDuck schema checkout/deployer preparation, `pwsh` script invocation, and TypeDuck release artifact/upload names.
- `.github/workflows/nightly.yml` - TypeDuck checkout paths, TypeDuck schema checkout/deployer preparation, `pwsh` script invocation, and TypeDuck nightly artifact/release names.
- `installer/README.txt` - TypeDuck installer output and `TypeDuckIME`/`TypeDuckRuntime` staging examples.
- `.planning/product/privacy-security/phase-06-guard-results.json` - Final strict guard evidence.

## Decisions Made

- Centralized Phase 6 verification through the aggregate guard so VER-02 has one strict command path.
- Kept legacy PowerShell parameter names such as `-MoqiImeRoot` only as script compatibility surfaces while workflow checkout/path names are TypeDuck-owned.
- Recorded evidence as compact metadata only to avoid leaking raw typed content, candidate payloads, or secrets.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Fixed focused guard argument binding**
- **Found during:** Task 1
- **Issue:** The first aggregate implementation passed focused-guard arguments as a positional array, causing PowerShell to misbind parameters.
- **Fix:** Switched focused guard execution to hashtable splatting with explicit named parameters.
- **Files modified:** `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1`
- **Verification:** `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckPrivacySecurityCleanup.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- **Committed in:** `628f08a`

**2. [Rule 1 - Bug] Narrowed schema upload negative check**
- **Found during:** Task 2
- **Issue:** A broad regex could have matched the word `schema` across unrelated workflow sections rather than only standalone schema/build upload paths.
- **Fix:** Replaced it with targeted checks for schema checkout/build paths in upload locations.
- **Files modified:** `scripts/Test-TypeDuckPrivacySecurityCleanup.ps1`
- **Verification:** Aggregate guard passed after the CI workflow scan was added.
- **Committed in:** `c867594`

**Total deviations:** 2 auto-fixed bugs.
**Impact on plan:** Both fixes improved guard correctness without changing product runtime code or expanding scope.

## Issues Encountered

- The raw JSON timestamp validation initially parsed `generatedAt` through `ConvertFrom-Json`, which displays DateTime values in local culture. Rechecked the raw JSON text instead; the file contains the required UTC string ending in `Z`.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckPrivacySecurityCleanup.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- Raw JSON evidence check for `status: pass`, UTC `generatedAt`, required category names, and absence of raw sample/secret-like content.
- `rg` spot checks for TypeDuck workflow artifact names, checkout paths, schema source, Rime deployer, `pwsh` invocation, and absence of legacy workflow names.

## Known Stubs

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Phase 6 now has one strict aggregate guard and committed guard evidence. Phase 7 can use this as the static/privacy/security release baseline while it handles broader release verification.

## Self-Check: PASSED

- Summary file created at `.planning/phases/06-privacy-security-and-scaffold-cleanup/06-06-SUMMARY.md`.
- Task commits found: `628f08a`, `c867594`, `aeea464`.
- Final aggregate guard passed.
- Guard evidence JSON exists and passes the raw contract check.

---
*Phase: 06-privacy-security-and-scaffold-cleanup*
*Completed: 2026-06-27*
