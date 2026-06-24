---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 01
subsystem: ui-runtime-theme-contract
tags: [typeduck, candidate-ui, dictionary-ui, theme-contract, powershell, go]

requires:
  - phase: 04-typeduck-protocol-and-typing-mvp
    provides: TypeDuck protocol and runtime packaging boundary
provides:
  - TypeDuck semantic light/dark appearance theme contract
  - Top-level HK Chinese and display-language font contract
  - Backend loader mapping from semantic palettes to existing runtime appearance fields
  - Strict guard for schema, loader path order, and package copy identity
affects: [phase-05-candidate-ui, phase-05-settings-ui, phase-05-dictionary-ui, phase-06-scaffold-cleanup]

tech-stack:
  added: [PowerShell guard script]
  patterns:
    - Semantic TypeDuck appearance roles are stored in JSON and mapped at the backend edge.
    - Fonts are top-level contract data, not per-theme palette data.

key-files:
  created:
    - scripts/Test-TypeDuckAppearanceTheme.ps1
  modified:
    - D:/VSProjects/moqi-ime/input_methods/rime/appearance_themes.json
    - D:/VSProjects/moqi-ime/input_methods/rime/appearance_themes.go
    - D:/VSProjects/moqi-ime/scripts/build.ps1

key-decisions:
  - "The canonical TypeDuck theme source is D:/VSProjects/moqi-ime/input_methods/rime/appearance_themes.json."
  - "The data-path appearance_themes.json copy remains transition compatibility only and must be byte-identical to the canonical root file."
  - "Theme JSON exposes semantic native roles and top-level fonts; runtime-specific appearance fields are derived by the backend loader."

patterns-established:
  - "Use scripts/Test-TypeDuckAppearanceTheme.ps1 for focused theme/font contract and package-copy guards."
  - "Keep Web-alpha-derived palette values but avoid Tailwind/DaisyUI token names in native JSON."

requirements-completed: [CAND-01, LANG-03]

duration: 8 min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 01: TypeDuck Semantic Theme Foundation Summary

**TypeDuck semantic light/dark theme and font contract with backend loader/package guards**

## Performance

- **Duration:** 8 min
- **Started:** 2026-06-24T03:10:16Z
- **Completed:** 2026-06-24T03:17:24Z
- **Tasks:** 2/2
- **Files modified:** 4

## Accomplishments

- Replaced the scaffold Moqi/Simplified novelty theme list with a TypeDuck-owned `version` 2 JSON contract containing top-level `fonts` and exactly `light`/`dark` semantic palettes.
- Updated the backend loader to decode `palette` and top-level `fonts`, then map semantic palette roles into existing runtime appearance fields.
- Kept the packaged `input_methods/rime/data/appearance_themes.json` compatibility copy, with build-time and guard-time byte-identity checks against the canonical root file.
- Added `scripts/Test-TypeDuckAppearanceTheme.ps1` to reject legacy theme IDs/labels, Tailwind/DaisyUI palette keys, theme-level font data, missing semantic roles, loader path-order regressions, and package-copy drift.

## Task Commits

Each task was committed atomically:

1. **Task 1: Replace scaffold theme JSON with TypeDuck semantic contract**
   - `1eba786` in `moqi-im-windows`: `feat(05-01): add TypeDuck appearance theme guard`
   - `4daea81` in `moqi-ime`: `feat(05-01): replace scaffold appearance themes`
2. **Task 2: Canonicalize theme loader and package-copy behavior**
   - `bc2e8da` in `moqi-im-windows`: `test(05-01): extend appearance theme guard for packaging`
   - `29a4566` in `moqi-ime`: `feat(05-01): canonicalize TypeDuck theme loading`

## Files Created/Modified

- `scripts/Test-TypeDuckAppearanceTheme.ps1` - strict schema, loader, and package-copy guard.
- `D:/VSProjects/moqi-ime/input_methods/rime/appearance_themes.json` - canonical TypeDuck semantic theme/font contract.
- `D:/VSProjects/moqi-ime/input_methods/rime/appearance_themes.go` - semantic palette decoding and compatibility mapping to existing appearance fields.
- `D:/VSProjects/moqi-ime/scripts/build.ps1` - canonical theme package copy and byte-identity assertion for the transition data-path copy.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckAppearanceTheme.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File D:\VSProjects\moqi-ime\scripts\build.ps1 -RepoRoot D:\VSProjects\moqi-ime -RimeDataSource I:\GitHub\TypeDuck-Web\schema` - PASS; built `server.exe`, staged Rime data, and copied canonical plus compatibility theme files.
- Post-build repeat guard - PASS; verified freshly rebuilt package copies remain byte-identical to the canonical backend source theme file.

## Decisions Made

- Followed D-12/D-13 by keeping `input_methods/rime/appearance_themes.json` as the owner and retaining the `data` copy only as byte-identical compatibility baggage.
- Followed D-14/D-16 by keeping fonts outside `themes` and limiting each theme to a `palette` object.
- Followed D-07/D-10 by using semantic role names such as `panel_background`, `dictionary_background`, `selection_background`, and `definition_text` instead of Tailwind/DaisyUI names.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Fixed absolute backend path handling in the guard**
- **Found during:** Task 1
- **Issue:** The new guard incorrectly joined an absolute `-BackendRoot` with the current repo path, producing `D:\VSProjects\moqi-im-windows\D:\VSProjects\moqi-ime\...`.
- **Fix:** Updated `Resolve-FullPath` to return rooted paths directly.
- **Files modified:** `scripts/Test-TypeDuckAppearanceTheme.ps1`
- **Verification:** Re-ran the strict guard successfully.
- **Committed in:** `1eba786`

---

**Total deviations:** 1 auto-fixed (Rule 1 bug)
**Impact on plan:** The fix was required for the planned verification command to work with the specified absolute backend path. No scope expansion.

## Issues Encountered

- The sibling backend had unrelated pre-existing dirty files before execution, including icons, `go.mod`, `ime.json`, and `librime.go`; they were left untouched and uncommitted.
- `D:/VSProjects/moqi-ime/scripts/build.ps1` already contained dirty `-RimeDataSource` support needed by the plan's build verification. The Task 2 commit includes that build-script state together with the new theme-copy identity assertion because the plan verification depends on it.

## Known Stubs

None - no placeholder or unwired stub patterns were found in the created/modified plan files.

## Threat Flags

None - this plan changed static theme/font contract data, loader decoding, and package-copy checks only. No new network endpoint, auth path, file trust boundary, or schema boundary was introduced beyond the planned theme JSON trust boundary.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Ready for Plan 05-02. Downstream candidate and dictionary view-model work can consume a stable TypeDuck semantic palette/font contract without carrying Moqi novelty themes or Tailwind token names.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-01-SUMMARY.md`.
- Task commits exist in `moqi-im-windows`: `1eba786`, `bc2e8da`.
- Task commits exist in `moqi-ime`: `4daea81`, `29a4566`.
- Required verification commands passed.

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
