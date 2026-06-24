---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 02
subsystem: candidate-dictionary-view-model
tags: [typeduck, candidate-ui, dictionary-ui, lookup-filter, gtest, powershell, win32]

requires:
  - phase: 04-typeduck-protocol-and-typing-mvp
    provides: TypeDuck raw lookup-filter comments preserved in the TSF candidate path
  - phase: 05-candidate-dictionary-settings-and-about-ui-parity
    provides: Plan 05-01 semantic theme and font contract
provides:
  - Native TypeDuck CandidateInfo and CandidateEntry parser/view-model contract
  - Fixture-backed parser tests for lookup-filter separators, CSV rows, multilingual definitions, compounds, Jyutping visibility, and typeface preference data
  - Candidate-window boundary that consumes structured CandidateInfo models without moving dictionary parsing into transport layers
affects: [phase-05-candidate-rendering, phase-05-settings-ui, phase-05-dictionary-panel, phase-06-scaffold-cleanup]

tech-stack:
  added: [GoogleTest target, PowerShell guard]
  patterns:
    - Parse raw lookup-filter comments near candidate-window view-model creation.
    - Keep Web-equivalent language/Jyutping/typeface constants in native TypeDuck product code.
    - Keep raw lookup comments as diagnostic data while visible rows derive from CandidateInfo fields.

key-files:
  created:
    - MoqiTextService/TypeDuckCandidateInfo.h
    - MoqiTextService/TypeDuckCandidateInfo.cpp
    - Tests/TypeDuckCandidateData/CMakeLists.txt
    - Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp
    - scripts/Test-TypeDuckCandidateData.ps1
    - .planning/product/candidate-fixtures/phase-05/candidate-data-contract.json
  modified:
    - CMakeLists.txt
    - MoqiTextService/CMakeLists.txt
    - MoqiTextService/MoqiCandidateWindow.h
    - MoqiTextService/MoqiCandidateWindow.cpp

key-decisions:
  - "TypeDuck lookup-filter parsing now lives in MoqiTextService/TypeDuckCandidateInfo.* and is invoked at the candidate-window boundary, not in launcher/protocol/backend transport."
  - "Native display-language, Jyutping visibility, typeface, dictionary label, register, and part-of-speech maps intentionally mirror TypeDuck Web consts.ts."
  - "The existing single-line candidate renderer remains a temporary consumer; exact dictionary panel drawing and placement stay owned by later Phase 5 rendering plans."

patterns-established:
  - "Use scripts/Test-TypeDuckCandidateData.ps1 as the strict static/runtime guard for D-32 through D-41."
  - "Use Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp for parser and candidate/dictionary view-model regressions."

requirements-completed: [CAND-02, CAND-03, CAND-04, CAND-05, LANG-03]

duration: 12 min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 02: Candidate Dictionary Data Model Summary

**Native TypeDuck CandidateInfo parser and candidate-window view-model boundary for lookup-filter dictionary data**

## Performance

- **Duration:** 12 min
- **Started:** 2026-06-24T03:23:01Z
- **Completed:** 2026-06-24T03:35:00Z
- **Tasks:** 3/3
- **Files modified:** 10

## Accomplishments

- Added a focused GoogleTest target for TypeDuck candidate data parsing and registered it in the root CMake build.
- Created a fixture-backed candidate data contract covering the D-10 header order, D-11 separators, `nei`, `housam`, multilingual definitions, Jyutping modes, and Sung/Hei preference data.
- Implemented `MoqiTextService/TypeDuckCandidateInfo.h/.cpp` with a bounded ConsumedString-style raw comment parser, CSV parser, Web-equivalent constant maps, `CandidateInfo`, and `CandidateEntry`.
- Updated the native candidate-window item boundary so visible text/comment output derives from `CandidateInfo` plus `DisplayPreferences`; raw comments remain diagnostic data.
- Added `scripts/Test-TypeDuckCandidateData.ps1` to reject parser drift into `MoqiClient`, `MoqLauncher`, `proto`, or backend transport files.

## Task Commits

Each task was committed atomically:

1. **Task 1: Add RED candidate data parser and mapping tests** - `7279046` (test)
2. **Task 2: Implement CandidateInfo and CandidateEntry near rendering** - `12e5efd` (feat)
3. **Task 3: Hand candidate window a settings-aware view model** - `ba207ab` (feat)

## Files Created/Modified

- `MoqiTextService/TypeDuckCandidateInfo.h` - Native candidate/dictionary view-model contract, preferences, language maps, and display helpers.
- `MoqiTextService/TypeDuckCandidateInfo.cpp` - Bounded lookup-filter raw comment parser, CSV parsing, dictionary field mapping, and Web-equivalent constants.
- `Tests/TypeDuckCandidateData/CMakeLists.txt` - GoogleTest target wiring for `TypeDuckCandidateInfo_test`.
- `Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp` - Parser and view-model behavior coverage for separators, CSV, multilingual rows, compounds, Jyutping visibility, and typeface preference data.
- `scripts/Test-TypeDuckCandidateData.ps1` - Strict guard for fixture schema, RED mode, parser placement, native model presence, and test executable execution.
- `.planning/product/candidate-fixtures/phase-05/candidate-data-contract.json` - Golden fixture contract for candidate/dictionary data.
- `CMakeLists.txt` - Registers `Tests/TypeDuckCandidateData`.
- `MoqiTextService/CMakeLists.txt` - Builds `TypeDuckCandidateInfo.*` into `MoqiTextService`.
- `MoqiTextService/MoqiCandidateWindow.h` / `MoqiTextService/MoqiCandidateWindow.cpp` - Candidate-window model boundary now consumes `CandidateInfo` and `DisplayPreferences`.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckCandidateData.ps1 -RepoRoot . -Strict -ExpectRed CandidateInfoMissing` - PASS during Task 1.
- `cmake -S . -B build-vs32 -G "Visual Studio 17 2022" -A Win32 && cmake --build build-vs32 --config Debug --target TypeDuckCandidateInfo_test -- /m:1 && build-vs32\Tests\TypeDuckCandidateData\Debug\TypeDuckCandidateInfo_test.exe` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckCandidateData.ps1 -RepoRoot . -Strict` - PASS.
- `cmake --build build-vs32 --config Debug --target TypeDuckCandidateInfo_test -- /m:1` - PASS.
- `build-vs32\Tests\TypeDuckCandidateData\Debug\TypeDuckCandidateInfo_test.exe` - PASS, 5/5 tests.
- `cmake --build build-vs32 --config Debug --target MoqiTextService -- /m:1` - PASS.

## Decisions Made

- Kept dictionary parsing in `MoqiTextService/TypeDuckCandidateInfo.*` and called it from candidate-window item creation, preserving D-39 by avoiding launcher/protocol/backend transport parsing.
- Kept later visual layout out of scope: the current renderer consumes structured fields, while exact dictionary panel drawing and placement remain for Plan 05-04.
- Stored backend strings as `std::wstring` data only; the renderer measures/draws them with GDI and does not interpret markup.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Corrected RED fixture control-character encoding**
- **Found during:** Task 1
- **Issue:** The fixture initially encoded D-11 separators as literal escape text, so JSON parsing produced `\u000b` text rather than U+000B.
- **Fix:** Changed the fixture separator fields to actual JSON `\u000b`, `\u000c`, and `\u000d` escapes.
- **Files modified:** `.planning/product/candidate-fixtures/phase-05/candidate-data-contract.json`
- **Verification:** Re-ran `scripts\Test-TypeDuckCandidateData.ps1 -Strict -ExpectRed CandidateInfoMissing` successfully.
- **Committed in:** `7279046`

**2. [Rule 1 - Bug] Fixed test references to temporary vectors**
- **Found during:** Task 2
- **Issue:** The GREEN test bound references to elements returned from temporary `matchedEntries()` and `otherLanguages()` vectors, causing an SEH access violation.
- **Fix:** Stored the returned vectors in local variables before indexing them.
- **Files modified:** `Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp`
- **Verification:** Rebuilt and reran `TypeDuckCandidateInfo_test.exe`; all 5 tests passed.
- **Committed in:** `12e5efd`

---

**Total deviations:** 2 auto-fixed (Rule 1 bugs)
**Impact on plan:** Both fixes corrected test/fixture harness issues encountered while implementing the planned behavior. No scope expansion.

## Issues Encountered

- The CMake configure/build emitted pre-existing third-party deprecation and conversion warnings from vendored dependencies; they did not block this plan and were not modified.

## Known Stubs

None - no placeholder or unwired stub patterns were found in the created/modified plan files. The candidate window intentionally remains a structured-model consumer rather than the final dictionary layout renderer because Plan 05-04 owns exact drawing and placement.

## Threat Flags

None - the only new trust-boundary code is the planned bounded parser for untrusted backend candidate comments. No new network endpoint, auth path, file access path, or transport/schema surface was introduced.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Ready for Plan 05-03 and downstream rendering work. Settings can now feed `DisplayPreferences`, and Plan 05-04 can draw candidate/dictionary rows from `CandidateInfo`/`CandidateEntry` without re-parsing lookup-filter payloads.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-02-SUMMARY.md`.
- Task commits exist in `moqi-im-windows`: `7279046`, `12e5efd`, `ba207ab`.
- Required verification commands passed.
- Parser placement guard found no `CandidateInfo`/lookup-filter parsing in `MoqiTextService/MoqiClient.cpp`, `MoqLauncher`, `proto`, or backend transport files.

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
