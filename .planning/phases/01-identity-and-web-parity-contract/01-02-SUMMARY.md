---
phase: 01-identity-and-web-parity-contract
plan: 02
subsystem: product-fixtures
tags: [typeduck, web-alpha, fixtures, settings, candidates, dictionary]

requires:
  - phase: 01-identity-and-web-parity-contract
    provides: TypeDuck identity and banned-surface contracts from 01-01
provides:
  - Dated TypeDuck Web alpha fixture contract for VER-01
  - Source/runtime provenance JSON for the 2026-06-23 Web alpha capture
  - Settings, candidate-list, and dictionary-detail fixture JSON
  - Settings and candidate-list desktop screenshots with dictionary screenshot status recorded as partial
affects: [phase-5-ui-parity, phase-7-release-verification]

tech-stack:
  added: []
  patterns:
    - Source-backed Web alpha fixture directory under .planning/product/web-alpha-fixtures/YYYY-MM-DD
    - JSON fixtures kept ASCII-safe for Windows PowerShell 5.1 validation
    - Screenshot capture limitations recorded as partial/blocked metadata

key-files:
  created:
    - .planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md
    - .planning/product/web-alpha-fixtures/2026-06-23/source-metadata.json
    - .planning/product/web-alpha-fixtures/2026-06-23/settings-order.json
    - .planning/product/web-alpha-fixtures/2026-06-23/candidate-list-sample.json
    - .planning/product/web-alpha-fixtures/2026-06-23/dictionary-detail-sample.json
    - .planning/product/web-alpha-fixtures/2026-06-23/screenshots/settings-desktop-1280x720.png
    - .planning/product/web-alpha-fixtures/2026-06-23/screenshots/candidate-desktop-1280x720.png
  modified: []

key-decisions:
  - "The 2026-06-23 Web alpha fixture is source-backed at TypeDuck-Web commit db21054 with dirty-worktree status recorded."
  - "Settings and candidate-list screenshots were captured at 1280x720; dictionary-detail screenshot status is partial because automation did not display the panel."
  - "Phase 5 and Phase 7 must refresh Web alpha fixtures if the alpha drifts before UI implementation or release verification."

patterns-established:
  - "Fixture metadata must include URL, source path, commit/hash, dirty status, viewport, capture tool, screenshots, and limitations."
  - "Blocked or partial runtime evidence is acceptable only when the exact attempted source and limitation are recorded."

requirements-completed:
  - VER-01

duration: 17 min
completed: 2026-06-23
status: complete
---

# Phase 1 Plan 2: Web Alpha Fixture Contract Summary

**Source-backed TypeDuck Web alpha fixtures for settings order, candidate presentation, dictionary fields, and dated visual references**

## Performance

- **Duration:** 17 min
- **Started:** 2026-06-23T07:23:08Z
- **Completed:** 2026-06-23T07:40:32Z
- **Tasks:** 2
- **Files modified:** 8

## Accomplishments

- Created the dated VER-01 fixture contract at `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md`.
- Captured source provenance from `I:\GitHub\TypeDuck-Web` at commit `db21054`, branch `aap2-alpha`, with dirty worktree status recorded.
- Created small JSON fixtures for settings order/defaults, candidate-list presentation, and dictionary-detail structure.
- Captured settings and candidate-list desktop screenshots at `1280x720`; recorded dictionary-detail screenshot as partial with the exact Playwright hover/touch limitation.

## Task Commits

1. **Task 1: Capture Web alpha provenance and small fixture assets** - `b835f77` (docs)
2. **Task 2: Create the dated Web alpha fixture contract** - `002873b` (docs)

**Plan metadata:** recorded in the final docs commit.

## Files Created/Modified

- `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` - Human-readable dated Web alpha fixture contract.
- `.planning/product/web-alpha-fixtures/2026-06-23/source-metadata.json` - Machine-readable source/runtime/screenshot provenance.
- `.planning/product/web-alpha-fixtures/2026-06-23/settings-order.json` - Settings labels, order, defaults, options, and Display Languages behavior.
- `.planning/product/web-alpha-fixtures/2026-06-23/candidate-list-sample.json` - Candidate presentation fields and runtime `nei` sample.
- `.planning/product/web-alpha-fixtures/2026-06-23/dictionary-detail-sample.json` - Dictionary panel field contract and partial screenshot status.
- `.planning/product/web-alpha-fixtures/2026-06-23/screenshots/settings-desktop-1280x720.png` - Captured settings screenshot.
- `.planning/product/web-alpha-fixtures/2026-06-23/screenshots/candidate-desktop-1280x720.png` - Captured candidate-list screenshot.

## Decisions Made

- Treat `db21054` plus dirty source status as the provenance for this fixture, not as a permanently clean product baseline.
- Mark dictionary-detail screenshot capture partial instead of fabricating a visual reference from source structure.
- Keep the fixture JSON ASCII-safe so the plan's Windows PowerShell validation commands parse consistently.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Made fixture JSON and path checks PowerShell-validation safe**
- **Found during:** Task 1 and Task 2 verification
- **Issue:** Windows PowerShell 5.1 parsed UTF-8 fixture JSON incorrectly under the plan's default `Get-Content`, and the markdown verifier looked for an escaped Windows path form.
- **Fix:** Rewrote JSON fixtures with ASCII `\uXXXX` escapes and added an explicit escaped source-path metadata row in the markdown.
- **Files modified:** JSON fixtures under `.planning/product/web-alpha-fixtures/2026-06-23/`, `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md`
- **Verification:** Both task-level PowerShell checks and the plan-level verification command pass.
- **Committed in:** `b835f77` and `002873b`

---

**Total deviations:** 1 auto-fixed (1 blocking validation compatibility issue).
**Impact on plan:** No scope creep; the fixture content remains source-backed and easier to validate on the target Windows shell.

## Issues Encountered

- Dictionary-detail screenshot capture is partial: Playwright successfully captured settings and candidate-list states, but the dictionary panel did not appear under controlled hover/move/touch attempts. The dictionary field contract is still source-backed through `DictionaryPanel.tsx`, `CandidateInfo.ts`, `consts.ts`, and `index.css`.

## Known Stubs

None.

## Authentication Gates

None.

## Verification

- `powershell -NoProfile -Command '<Task 1 JSON/provenance assertions>'` - PASS
- `powershell -NoProfile -Command '<Task 2 markdown/source assertions>'` - PASS
- `powershell -NoProfile -Command '<plan-level fixture existence, JSON parse, and production diff guard>'` - PASS
- Stub scan over created markdown and JSON fixtures - PASS

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Phase 1 artifacts now cover IDEN-02 and VER-01. Phase 2 can proceed to the engine/runtime spike, while Phase 5 and Phase 7 should refresh this Web alpha fixture if the alpha changes before UI implementation or release verification.

## Self-Check: PASSED

- Found `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md`
- Found `.planning/product/web-alpha-fixtures/2026-06-23/source-metadata.json`
- Found `.planning/product/web-alpha-fixtures/2026-06-23/settings-order.json`
- Found `.planning/product/web-alpha-fixtures/2026-06-23/candidate-list-sample.json`
- Found `.planning/product/web-alpha-fixtures/2026-06-23/dictionary-detail-sample.json`
- Found `.planning/product/web-alpha-fixtures/2026-06-23/screenshots/settings-desktop-1280x720.png`
- Found `.planning/product/web-alpha-fixtures/2026-06-23/screenshots/candidate-desktop-1280x720.png`
- Found task commit `b835f77`
- Found task commit `002873b`

---
*Phase: 01-identity-and-web-parity-contract*
*Completed: 2026-06-23*
