---
phase: 01-identity-and-web-parity-contract
plan: 01
subsystem: product-contract
tags: [typeduck, identity, tsf, installer, banned-surfaces]

requires: []
provides:
  - TypeDuck identity contract for names, identifiers, paths, registry keys, resources, and artifacts
  - TypeDuck banned-surface contract with audit patterns and cleanup ownership
affects: [phase-2-engine-runtime, phase-3-installer-registration, phase-4-protocol, phase-5-ui-parity, phase-6-cleanup, phase-7-release]

tech-stack:
  added: []
  patterns:
    - Source-backed planning contracts under .planning/product
    - Current scaffold value to target TypeDuck value inventory
    - Negative product-surface audit table

key-files:
  created:
    - .planning/product/TYPEDUCK-IDENTITY-CONTRACT.md
    - .planning/product/TYPEDUCK-BANNED-SURFACES.md
  modified: []

key-decisions:
  - "TypeDuck product identity is centralized in .planning/product/TYPEDUCK-IDENTITY-CONTRACT.md before production renames."
  - "TypeDuck AppId, CLSID, and profile GUID values are proposed pending human review, while zh-HK bilingual profile text is locked by phase decisions."
  - "Moqi, fcitx, WebDAV/cloud clipboard, AI, Simplified-only copy, generic config tools, excessive customization, and legacy Moqi backend fallback are banned from v1 product surfaces."

patterns-established:
  - "Identity rows must include current scaffold value, target TypeDuck value, status, owner phase, affected files, and verification notes."
  - "Banned surfaces must include rationale, audit patterns, known files, allowed replacement, and cleanup phase."

requirements-completed:
  - IDEN-02

duration: 5 min
completed: 2026-06-23
status: complete
---

# Phase 1 Plan 1: Identity and Banned-Surface Contract Summary

**Source-backed TypeDuck identity and banned-surface contracts for downstream installer, TSF, protocol, UI, cleanup, and release phases**

## Performance

- **Duration:** 5 min
- **Started:** 2026-06-23T07:14:04Z
- **Completed:** 2026-06-23T07:18:29Z
- **Tasks:** 2
- **Files modified:** 2

## Accomplishments

- Created the TypeDuck identity contract with current Moqi scaffold values and target TypeDuck values for executables, DLLs, AppId, CLSID, profile GUID, zh-HK profile text, install/log/data paths, pipe/mutex names, registry keys, resources, release artifacts, environment variables, scheduled task names, tray/window names, and protocol/package names.
- Created the banned-surface contract covering visible Moqi branding, fcitx, WebDAV/cloud clipboard, AI controls, Simplified-only copy, backend-declared config tools, excessive customization, legacy Moqi backend fallback, and Moqi runtime paths.
- Verified both artifacts with PowerShell source assertions and confirmed production source, installer, protocol, registry, runtime path, and workflow files were not modified.

## Task Commits

1. **Task 1: Create the TypeDuck identity contract** - `b98b23a` (docs)
2. **Task 2: Create the banned-surface contract** - `36a77e9` (docs)

**Plan metadata:** recorded in the final docs commit.

## Files Created/Modified

- `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` - Product identity authority for TypeDuck naming, identifiers, paths, registry locations, resources, profile display text, and release artifacts.
- `.planning/product/TYPEDUCK-BANNED-SURFACES.md` - Negative product contract with audit patterns and cleanup ownership for legacy scaffold surfaces.

## Decisions Made

- TypeDuck identity implementation must use the contract instead of one-off downstream names or GUIDs.
- AppId, CLSID, and profile GUID values are proposed and pending human review before production implementation.
- `zh-HK`, Chinese (Traditional, Hong Kong), and bilingual Traditional Hong Kong Chinese plus English profile/display text are locked by the phase context.
- Banned-surface cleanup remains documentation-only in this plan; later phases perform production cleanup and automated checks.

## Deviations from Plan

None - plan executed exactly as written.

**Total deviations:** 0 auto-fixed.
**Impact on plan:** No scope changes; production files remained untouched.

## Issues Encountered

None.

## Known Stubs

None.

## User Setup Required

None - no external service configuration required.

## Verification

- `powershell -NoProfile -Command '<Task 1 identity contract assertions>'` - PASS
- `powershell -NoProfile -Command '<Task 2 banned-surface assertions>'` - PASS
- `powershell -NoProfile -Command '<plan-level artifact existence and production diff guard>'` - PASS

## Next Phase Readiness

Ready for `01-02-PLAN.md`, which should create the dated TypeDuck Web alpha fixture markdown and supporting fixture assets for VER-01.

## Self-Check: PASSED

- Found `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md`
- Found `.planning/product/TYPEDUCK-BANNED-SURFACES.md`
- Found `.planning/phases/01-identity-and-web-parity-contract/01-01-SUMMARY.md`
- Found task commit `b98b23a`
- Found task commit `36a77e9`

---
*Phase: 01-identity-and-web-parity-contract*
*Completed: 2026-06-23*
