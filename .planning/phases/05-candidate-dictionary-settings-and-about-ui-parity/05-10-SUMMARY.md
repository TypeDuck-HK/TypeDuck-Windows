---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 10
subsystem: candidate-dictionary-ui
tags: [typeduck, candidate-window, dictionary, rime, parity, gdi, powershell]

requires:
  - phase: 05-09
    provides: Phase 5 rejected-UAT guards and runtime provenance baseline
provides:
  - Candidate parity guard covering runtime hashes, transport path, hou/housam/nei/multilingual/reverse lookup samples
  - VM-vs-preview divergence diagnosis for stale TypeDuck 1.1.2 runtime evidence
  - Native and preview dictionary rendering with multi-row entries and POS pills
  - Regenerated candidate preview BMP evidence with strict guard coverage
affects: [05-11-candidate-parity, 05-13-vm-evidence, phase-05-closeout]

tech-stack:
  added: [PowerShell runtime parity guard, Win32 GDI POS pill drawing]
  patterns:
    - Treat preview screenshots as source-backed iteration evidence only, not VM runtime acceptance proof.
    - Keep lookup-filter parsing at the CandidateInfo/rendering boundary and transport raw lookup comments unchanged.

key-files:
  created:
    - D:/VSProjects/moqi-ime/scripts/Test-TypeDuckCandidateParity.ps1
    - .planning/product/ui-fixtures/phase-05/candidate-preview/*.bmp
  modified:
    - MoqiTextService/MoqiCandidateWindow.cpp
    - MoqiTextService/MoqiCandidateWindow.h
    - Preview/main.cpp
    - Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp
    - scripts/Test-TypeDuckCandidateData.ps1
    - scripts/Test-TypeDuckCandidateWindow.ps1
    - .planning/product/candidate-fixtures/phase-05/candidate-data-contract.json
    - .planning/product/ui-fixtures/phase-05/runtime-provenance.json
    - .planning/product/ui-fixtures/phase-05/candidate-preview/capture-commands.md

key-decisions:
  - "05-10: VM candidate screenshots from stale external TypeDuck 1.1.2 runtime are not accepted as current package evidence."
  - "05-10: Candidate transport remains pass-through; raw_lookup_comment is parsed only at the CandidateInfo renderer boundary."
  - "05-10: Candidate preview BMPs are guarded source-backed iteration evidence and must cite runtime-provenance before use."

patterns-established:
  - "Use Test-TypeDuckCandidateParity.ps1 to reject stale runtime hashes, missing Web-alpha samples, and legacy candidate substitution."
  - "Use GDI rounded POS pills instead of bracketed part-of-speech text in native and preview dictionary panels."

requirements-completed: [CAND-01, CAND-02, CAND-03, CAND-04, CAND-05, CAND-06, CAND-07, LANG-03]

duration: 15 min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 10: Candidate Runtime Parity and Dictionary Rendering Summary

**Runtime-backed candidate parity guard with stale VM-runtime diagnosis, multi-row dictionary details, POS pills, and regenerated preview evidence**

## Performance

- **Duration:** 15 min
- **Started:** 2026-06-24T07:41:49Z
- **Completed:** 2026-06-24T07:54:38Z
- **Tasks:** 3/3
- **Files modified:** 17

## Accomplishments

- Added a sibling backend parity guard that checks TypeDuck runtime hashes, Web fixture coverage, candidate transport pass-through, and stale runtime exclusion.
- Recorded the VM-vs-preview root cause: a live stale `C:\Program Files (x86)\Rime\TypeDuck-1.1.2\TypeDuckServer.exe` path, while current TypeDuckIME host install paths were missing.
- Expanded candidate data tests and fixtures for `hou`, `housam`, `nei`, multilingual rows, reverse lookup buffer data, and non-bracket POS presentation.
- Updated native and preview GDI dictionary panels to draw POS values as rounded bordered pills and measure input-buffer text instead of estimating by character count.
- Regenerated seven candidate preview BMP captures and made the guard reject missing/truncated captures.

## Task Commits

1. **Task 1 RED: Prove candidate parity gaps fail** - `be52a85` (test) and backend `181336d` (test)
2. **Task 1 GREEN: Runtime parity diagnosis and data fixtures** - `69b4982` (feat)
3. **Task 2 RED: Renderer depth guard** - `c4c4b88` (test)
4. **Task 2 GREEN: POS pills and measured input buffer** - `a9722a1` (feat)
5. **Task 3: Preview evidence guard and captures** - `2f89a58` (test)

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File D:\VSProjects\moqi-ime\scripts\Test-TypeDuckCandidateParity.ps1 -RepoRoot D:\VSProjects\moqi-ime -WindowsRepoRoot . -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckCandidateData.ps1 -RepoRoot . -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckCandidateWindow.ps1 -RepoRoot . -Strict` - PASS.
- `cmake --build build-vs32 --config Debug --target TypeDuckCandidateInfo_test -- /m:1` - PASS.
- `build-vs32\Tests\TypeDuckCandidateData\Debug\TypeDuckCandidateInfo_test.exe` - PASS, 8 tests.
- `cmake --build build-vs32 --config Debug --target MoqiCandidatePreview -- /m:1` - PASS.
- `cmake --build build-vs32 --config Debug --target MoqiTextService -- /m:1` - PASS.

## VM-vs-Preview Divergence

Root cause: the host had a running `TypeDuckServer.exe` from `C:\Program Files (x86)\Rime\TypeDuck-1.1.2`, file version `1.1.2`, SHA-256 `24FCD4D463C8FC7272B735BFB0EAD40B2E20B0C2E712EB4C3B66905AA14AC031`. The current package paths `C:\Program Files (x86)\TypeDuckIME\moqi-ime\...` and host system TSF DLL paths were missing during diagnosis. Preview screenshots were generated from local Phase 5 fixtures/current build, so the old VM/host runtime could not be compared as current package evidence.

Fix/disposition: `runtime-provenance.json` now records the divergence and marks stale TypeDuck 1.1.2 evidence as `not-accepted-as-current-evidence`. The parity guard requires current TypeDuck-HK `rime.dll` SHA-256 `5783B84916FEFC0DD5DDA28F1D7292A9CA86C75F05132947FE816178B916C04B`, backend server SHA-256 `8058863ADEDF373826ABB877186E3ADF6F6292CD3B734F273E8AF28D752E305C`, and pass-through candidate transport. No installer was run on the host.

## Files Created/Modified

- `D:/VSProjects/moqi-ime/scripts/Test-TypeDuckCandidateParity.ps1` - New runtime/Web/transport parity guard.
- `.planning/product/candidate-fixtures/phase-05/candidate-data-contract.json` - Adds `hou`, reverse lookup, multilingual, and runtime parity contract samples.
- `.planning/product/ui-fixtures/phase-05/runtime-provenance.json` - Adds candidate runtime diagnosis, stale process evidence, bitness/install path checks, and transport disposition.
- `Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp` - Adds `hou`, reverse lookup, multilingual, and POS data assertions.
- `scripts/Test-TypeDuckCandidateData.ps1` - Requires the deeper candidate data tests and runtime parity contract fields.
- `scripts/Test-TypeDuckCandidateWindow.ps1` - Requires POS pills, measured input buffer/baseline anchors, divergence notes, and non-empty preview captures.
- `MoqiTextService/MoqiCandidateWindow.cpp` / `.h` - Draws POS pills, measures input buffer width, keeps cursor within measured bounds, and preserves multi-row rendering.
- `Preview/main.cpp` - Mirrors POS pills/measured input behavior in preview captures.
- `.planning/product/ui-fixtures/phase-05/candidate-preview/*.bmp` - Regenerated preview captures for `nei`, multilingual, `housam`, reverse lookup, edge, high-DPI, and fallback scenarios.

## Decisions Made

- Stale external TypeDuck 1.1.2 runtime captures are explicitly rejected for current package acceptance.
- Runtime candidate parity is enforced by source/runtime hashes and transport-path checks, not by changing Web-alpha fixture expectations.
- POS values are structured data drawn as native GDI pills; bracket text is now a guard failure.

## Deviations from Plan

None - plan executed as written. The stale runtime finding was the planned first diagnosis path and was recorded instead of masked.

## Issues Encountered

- The candidate preview directory initially had only capture commands and no BMP captures. The preview harness generated all required captures, and the guard now rejects missing or truncated files.
- The sibling backend repo retained pre-existing dirty state from earlier phases; only `scripts/Test-TypeDuckCandidateParity.ps1` was added and committed there.
- No installer was run on the host.

## Known Stubs

None. Stub scan found no TODO, FIXME, placeholder, coming-soon, or not-available markers in files touched by this plan.

## Threat Flags

None beyond the plan threat model. The new trust-surface checks are planned candidate runtime/provenance guards; no new network endpoint, auth path, or schema boundary was introduced.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plans 05-11/05-13 can use the parity guard and `runtime-provenance.json` to reject stale VM/runtime evidence and require current packaged TypeDuckIME proof.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-10-SUMMARY.md`.
- Task commits found in Windows repo: `be52a85`, `69b4982`, `c4c4b88`, `a9722a1`, `2f89a58`.
- Backend task commit found in sibling repo: `181336d`.
- Created preview captures exist and are non-empty.
- Required final verification commands passed.

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
