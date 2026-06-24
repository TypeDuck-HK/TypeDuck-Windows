---
phase: 05-candidate-dictionary-settings-and-about-ui-parity
plan: 03
subsystem: settings
tags: [typeduck, settings, rime, json, protobuf, tsf, launcher, backend]

requires:
  - phase: 05-01
    provides: TypeDuck semantic theme foundation and bilingual UI conventions.
  - phase: 05-02
    provides: Structured TypeDuck candidate data model used by later settings-controlled rendering.
provides:
  - TypeDuck settings JSON source of truth shaped like the Web alpha DEFAULT_PREFERENCES contract.
  - Launcher and TSF protocol bridge for TypeDuck settings snapshot/update messages.
  - Rime default/common custom side effects generated from engine-affecting settings.
  - Guard and unit test coverage for settings scope, validation, side effects, and bounded apply failure.
affects: [05-05-settings-ui, 05-06-installer-settings, candidate-rendering, rime-runtime]

tech-stack:
  added: [GoogleTest target TypeDuckPreferences_test, PowerShell guards]
  patterns:
    - TypeDuck preferences persist in JSON; Rime YAML is generated output only.
    - Engine-affecting settings are capability-gated and batched behind Apply.
    - Backend reads only Rime-relevant JSON fields so interface-only settings cannot create Rime patches.

key-files:
  created:
    - Tests/TypeDuckSettings/CMakeLists.txt
    - Tests/TypeDuckSettings/TypeDuckPreferences_test.cpp
    - scripts/Test-TypeDuckSettingsPersistence.ps1
    - D:/VSProjects/moqi-ime/scripts/Test-TypeDuckSettingsCustomization.ps1
    - MoqLauncher/TypeDuckPreferences.h
    - MoqLauncher/TypeDuckPreferences.cpp
  modified:
    - CMakeLists.txt
    - MoqLauncher/CMakeLists.txt
    - MoqLauncher/PipeClient.cpp
    - MoqLauncher/PipeClient.h
    - MoqLauncher/BackendServer.cpp
    - MoqLauncher/BackendServer.h
    - MoqiTextService/MoqiClient.cpp
    - MoqiTextService/MoqiClient.h
    - proto/moqi.proto
    - D:/VSProjects/moqi-ime/input_methods/rime/appearance_config.go
    - D:/VSProjects/moqi-ime/input_methods/rime/rime.go

key-decisions:
  - "Settings JSON is saved before Rime side effects are attempted, so deploy failures do not erase the readable source of truth."
  - "Only pageSize, enableCompletion, enableCorrection, enableSentence, enableLearning, and isCangjie5 generate Rime customization."
  - "The current TypeDuck-HK librime fork uses a reviewed generated-YAML bridge because the Web levers/custom-settings API is not exposed."

patterns-established:
  - "Use MoqLauncher/TypeDuckPreferences.* as the native settings contract for future settings UI work."
  - "Return bilingual bounded settings errors for failed apply/deploy paths."
  - "Keep interface-only settings in JSON/protocol/native UI state, not default.custom.yaml or common.custom.yaml."

requirements-completed: [SET-03, SET-04, SET-05, SET-06, SET-07, SET-08, SET-09, CAND-04, CAND-05, LANG-03]

duration: 27min
completed: 2026-06-24
status: complete
---

# Phase 05 Plan 03: TypeDuck Settings Persistence and Rime Side Effects Summary

**TypeDuck settings now persist as Web-alpha-shaped JSON, bridge through launcher/TSF protobuf messages, and generate bounded Rime customization side effects.**

## Performance

- **Duration:** 27 min
- **Started:** 2026-06-24T03:41:34Z
- **Completed:** 2026-06-24T04:08:11Z
- **Tasks:** 3
- **Files modified:** 17

## Accomplishments

- Added RED and GREEN coverage for TypeDuck settings defaults, Web-alpha order, page-size range, interface-only setting isolation, Rime patch mapping, and apply failure behavior.
- Implemented `MoqLauncher/TypeDuckPreferences.*` with JSON persistence under `TypeDuckIME/TypeDuckPreferences.json`, validation, capability metadata, and protobuf snapshot/update fields.
- Connected settings snapshot/update helpers through launcher and TSF client code so Plan 05-05 can render a real native settings UI against the runtime contract.
- Added Rime side effects for `default.custom.yaml` `menu/page_size` and `common.custom.yaml` `__patch` entries, with backend redeploy/reconfigure request flow and bilingual bounded failure messages.
- Added sibling backend handling that reads only engine-affecting TypeDuck JSON fields, writes generated Rime custom files, and redeploys without making YAML the persisted settings store.

## Task Commits

1. **Task 1: Add RED preferences and side-effect mapping tests** - `ccc8dcd` (windows test), `5edf5b9` (backend test)
2. **Task 2: Implement TypeDuck JSON preferences and protocol bridge** - `916cb95` (windows feat)
3. **Task 3: Apply Rime custom settings and redeploy semantics** - `cd4ea8a` (windows feat), `f792eb3` (backend feat)

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsPersistence.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict -ExpectRed PreferencesMissing` - PASS during RED.
- `cmake -S . -B build-vs32-settings -G "Visual Studio 17 2022" -A Win32` - PASS with existing third-party CMake warnings.
- `cmake --build build-vs32-settings --config Debug --target TypeDuckPreferences_test -- /m:1` - PASS.
- `build-vs32-settings\Tests\TypeDuckSettings\Debug\TypeDuckPreferences_test.exe` - PASS, 5/5 tests.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsPersistence.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict` - PASS.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File D:\VSProjects\moqi-ime\scripts\Test-TypeDuckSettingsCustomization.ps1 -RepoRoot D:\VSProjects\moqi-ime -WindowsRepoRoot . -Strict` - PASS.
- `go test ./input_methods/rime -run TestDoesNotExist -count=0` - PASS compile-only.
- `cmake --build build-vs32-settings --config Debug --target MoqiLauncher -- /m:1` - PASS, with existing MSVC `stdext::checked_array_iterator` warnings from vendored spdlog/fmt usage.
- `cmake --build build-vs32-settings --config Debug --target MoqiTextService -- /m:1` - PASS.

## Files Created/Modified

- `Tests/TypeDuckSettings/TypeDuckPreferences_test.cpp` - GoogleTest coverage for defaults, validation, JSON source preservation, and Rime side-effect mapping.
- `scripts/Test-TypeDuckSettingsPersistence.ps1` - Windows guard for JSON persistence, Web-alpha scope, and no YAML-as-source regression.
- `D:/VSProjects/moqi-ime/scripts/Test-TypeDuckSettingsCustomization.ps1` - Backend guard for generated Rime customization and redeploy semantics.
- `MoqLauncher/TypeDuckPreferences.*` - Native TypeDuck settings model, validation, JSON load/save, capability metadata, and side-effect calculation.
- `MoqLauncher/PipeClient.*` - Settings snapshot/update handling and bounded settings responses.
- `MoqLauncher/BackendServer.*` - Generated Rime file writer and backend redeploy request for settings apply.
- `MoqiTextService/MoqiClient.*` - TSF-side helpers to request snapshots and apply updates through the launcher protocol.
- `proto/moqi.proto` - TypeDuck settings snapshot/update protocol fields and settings apply error code.
- `D:/VSProjects/moqi-ime/input_methods/rime/appearance_config.go` - Backend TypeDuck JSON reader and Rime customization writer.
- `D:/VSProjects/moqi-ime/input_methods/rime/rime.go` - TypeDuck apply command hook and redeploy integration.

## Decisions Made

- JSON remains the persisted source of truth; `default.custom.yaml` and `common.custom.yaml` are generated side effects only.
- Settings apply is batched behind an Apply-style transaction for the future UI, not applied per toggle.
- Interface-only settings stay out of Rime customization. This preserves Web-alpha settings scope while allowing native UI rendering and future candidate presentation controls.
- The backend code documents the generated YAML bridge because this TypeDuck-HK librime fork does not expose the Web levers/custom-settings API.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical Functionality] Preserved JSON source-of-truth on side-effect failure**
- **Found during:** Task 3
- **Issue:** The initial Task 2 bridge applied Rime side effects before saving JSON, which could make generated YAML the effective success gate for persisted settings.
- **Fix:** Changed `TypeDuck::applyPreferences` to validate and save JSON first, then attempt Rime side effects and return a bounded bilingual failure if deploy/customization fails.
- **Files modified:** `MoqLauncher/TypeDuckPreferences.cpp`, `MoqLauncher/PipeClient.cpp`, `MoqLauncher/BackendServer.*`
- **Verification:** `TypeDuckPreferences.FailedApplyDoesNotCorruptJsonSourceOfTruth`, persistence guard, customization guard, launcher build.
- **Committed in:** `cd4ea8a`

**Total deviations:** 1 auto-fixed (Rule 2)
**Impact on plan:** Preserves the plan's JSON-source-of-truth invariant; no scope expansion.

## Issues Encountered

- A combined `MoqiLauncher`/`MoqiTextService` build command exceeded the shell timeout while MSBuild continued running. Incremental reruns for both targets completed successfully.
- The first backend customization guard invocation used the wrong relative script path and failed before running the script. The absolute backend script path from the plan passed.
- Full `go test ./input_methods/rime` exceeded the 120 second command timeout in the dirty sibling backend checkout. Compile-only `go test ./input_methods/rime -run TestDoesNotExist -count=0` passed.
- Backend repo had pre-existing unrelated dirty files (`go.mod`, icons, `ime.json`, `librime.go`, untracked icon assets). They were not staged or committed.

## Known Stubs

None. Stub scan found only pre-existing legacy FIXMEs/null checks in scaffold files and one existing backend error string, not new placeholder behavior from this plan.

## Threat Flags

None beyond the plan's threat model. The new settings trust boundary validates known language IDs, bounded page-size range, boolean engine settings, and invalid JSON fallback before applying side effects.

## User Setup Required

None.

## Next Phase Readiness

Plan 05-05 can build the native settings UI against the snapshot/update contract, ordered descriptors, and capability metadata in `MoqLauncher/TypeDuckPreferences.*`. Plan 05-06 can reuse the same JSON file for installer-time settings and rely on generated Rime YAML as deploy-time output.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-03-SUMMARY.md`.
- Windows commits found: `ccc8dcd`, `916cb95`, `cd4ea8a`.
- Backend commits found: `5edf5b9`, `f792eb3`.

---
*Phase: 05-candidate-dictionary-settings-and-about-ui-parity*
*Completed: 2026-06-24*
