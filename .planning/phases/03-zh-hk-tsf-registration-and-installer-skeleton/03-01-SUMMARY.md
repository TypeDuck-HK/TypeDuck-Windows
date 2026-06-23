---
phase: 03-zh-hk-tsf-registration-and-installer-skeleton
plan: 01
subsystem: installer-registration
tags: [windows-tsf, com, zh-hk, resources, cmake, powershell]

requires:
  - phase: 02-engine-runtime-contract-spike
    provides: TypeDuck-HK runtime proof and lookup-filter evidence for downstream Windows integration
provides:
  - First-party TypeDuck text service CLSID and zh-HK profile GUID constants
  - Deterministic TypeDuck Cantonese TSF profile registration seeded before backend metadata scanning
  - Static PowerShell contract check for TypeDuck TSF identity
  - TypeDuckTextService.dll build output and resource metadata
affects: [phase-03-installer, phase-04-protocol, phase-06-cleanup]

tech-stack:
  added: [PowerShell contract check, TypeDuckProfile C++ helper]
  patterns: [first-party TSF identity constants, backend metadata as optional transition data]

key-files:
  created:
    - scripts/Test-TypeDuckTsfIdentity.ps1
    - MoqiTextService/TypeDuckProfile.h
    - MoqiTextService/TypeDuckProfile.cpp
  modified:
    - MoqiTextService/MoqiImeModule.cpp
    - MoqiTextService/DllEntry.cpp
    - MoqiTextService/MoqiTextService.rc.in
    - MoqiTextService/CMakeLists.txt
    - MoqiTextService/MoqiTextService.def

key-decisions:
  - "TypeDuck TSF identity is centralized in MoqiTextService/TypeDuckProfile.* using the Phase 3 CLSID/profile GUID contract."
  - "DllRegisterServer seeds the TypeDuck zh-HK profile before optional backend ime.json scanning, and duplicate/overriding backend TypeDuck profile metadata is ignored."
  - "The CMake target name remains MoqiTextService for low churn, while the produced DLL and module definition use TypeDuckTextService.dll."

patterns-established:
  - "Registration authority pattern: required product TSF profiles come from first-party source before transition-time backend metadata."
  - "Identity guard pattern: scripts/Test-TypeDuckTsfIdentity.ps1 statically checks production files for TypeDuck CLSID/profile/resource/output identity."

requirements-completed: ["INST-02", "INST-03"]

duration: 9 min
completed: 2026-06-23
status: complete
---

# Phase 03 Plan 01: TypeDuck TSF Identity and zh-HK Profile Summary

**TypeDuck zh-HK TSF registration now uses first-party CLSID/profile/resource metadata and builds TypeDuckTextService.dll.**

## Performance

- **Duration:** 9 min
- **Started:** 2026-06-23T15:45:24Z
- **Completed:** 2026-06-23T15:54:04Z
- **Tasks:** 3
- **Files modified:** 8

## Accomplishments

- Added `scripts/Test-TypeDuckTsfIdentity.ps1`, a static guard for TypeDuck CLSID, profile GUID, `zh-HK`, bilingual profile text, resource metadata, and DLL output identity.
- Added `MoqiTextService/TypeDuckProfile.*` as the first-party source for TypeDuck text service CLSID, zh-HK profile GUID, display name, environment variable names, install directory, and DLL name.
- Updated `DllRegisterServer` to always seed the TypeDuck Cantonese profile before optional backend metadata, while filtering backend attempts to override the required TypeDuck profile identity.
- Updated TSF resource metadata and build output naming so Release builds produce `TypeDuckTextService.dll`.

## Task Commits

1. **Task 1: Add the TypeDuck TSF identity contract check** - `3623f3b` (test)
2. **Task 2: Centralize first-party TypeDuck profile metadata** - `589a03a` (feat)
3. **Task 3: Register the deterministic zh-HK profile and TypeDuck DLL resource identity** - `2357577` (feat)

**Plan metadata:** pending final docs commit

## Files Created/Modified

- `scripts/Test-TypeDuckTsfIdentity.ps1` - Static TypeDuck TSF identity and registration-authority contract guard.
- `MoqiTextService/TypeDuckProfile.h` - Public TypeDuck profile metadata declarations.
- `MoqiTextService/TypeDuckProfile.cpp` - TypeDuck CLSID/profile GUID, locale, display, DLL, env-var, and install-directory constants.
- `MoqiTextService/MoqiImeModule.cpp` - Uses the TypeDuck CLSID and prefers `TYPEDUCK_PROGRAM_DIR`, with `MOQI_PROGRAM_DIR` kept as a transition fallback.
- `MoqiTextService/DllEntry.cpp` - Seeds the required TypeDuck profile and filters backend metadata that would override it.
- `MoqiTextService/MoqiTextService.rc.in` - Replaces Simplified/Moqi metadata with TypeDuck Traditional Hong Kong and English resource strings.
- `MoqiTextService/CMakeLists.txt` - Compiles the TypeDuck profile helper and sets `OUTPUT_NAME` to `TypeDuckTextService`.
- `MoqiTextService/MoqiTextService.def` - Aligns the module definition library name with `TypeDuckTextService.dll`.

## Decisions Made

- Kept the internal CMake target and source directory names as scaffold names for now, per Phase 3 D-11, while changing deployed DLL identity.
- Used the loaded TSF DLL module path as the profile icon source for registration metadata until Phase 3 installer work provides final deployed icon packaging.
- Left backend input-method scanning in place only as optional transition metadata; it cannot replace or override the required TypeDuck zh-HK profile.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical] Aligned module definition with TypeDuck DLL output**
- **Found during:** Task 3 (deterministic profile and DLL resource identity)
- **Issue:** `MoqiTextService.def` still declared `LIBRARY MoqiTextService.dll`, which would leave linker/module-definition identity inconsistent with the requested `TypeDuckTextService.dll` output.
- **Fix:** Changed the DEF `LIBRARY` line to `TypeDuckTextService.dll`.
- **Files modified:** `MoqiTextService/MoqiTextService.def`
- **Verification:** Release build produced `build-vs32/MoqiTextService/Release/TypeDuckTextService.dll` and `build-vs64/MoqiTextService/Release/TypeDuckTextService.dll`.
- **Committed in:** `2357577`

---

**Total deviations:** 1 auto-fixed (1 missing critical)
**Impact on plan:** The fix was required to keep DLL identity coherent. No feature scope was added.

## Issues Encountered

- Task 2 used the monolithic contract guard from Task 1; it still reported the planned Task 3 registration/resource/output-name failures until Task 3 was implemented. Final task-level and plan-level verification passed.
- Release build completed with existing non-blocking warnings: missing local `protoc` caused fallback to checked-in protobuf sources, vendored CMake deprecation warnings, and an existing `NDEBUG` macro redefinition warning.

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckTsfIdentity.ps1 -RepoRoot . -Strict`  
  **Result:** PASS - `TypeDuck TSF identity check passed.`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\build.ps1 -RepoRoot . -Configuration Release`  
  **Result:** PASS - Win32 full solution and x64 `MoqiTextService` target built successfully.
- Build outputs verified:
  - `build-vs32\MoqiTextService\Release\TypeDuckTextService.dll`
  - `build-vs64\MoqiTextService\Release\TypeDuckTextService.dll`

## Known Stubs

- `MoqiTextService/MoqiImeModule.cpp:145` and `MoqiTextService/MoqiImeModule.cpp:189` contain pre-existing scaffold `FIXME` comments around backend config-tool lookup/localization. They do not block this plan's registration identity goal and are covered by later settings/security cleanup phases.

## Threat Flags

None - the modified trust boundaries are the ones explicitly covered by the plan threat model: backend metadata no longer owns required TypeDuck profile registration, and resource/output identity is asserted by the new contract check.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 03-02 can now update installer, setup helper, and staging paths against a deterministic TypeDuck TSF DLL/profile identity. Full Windows profile appearance and uninstall behavior still require the planned VM-backed Phase 3 verification.

## Self-Check: PASSED

- Created files exist: `scripts/Test-TypeDuckTsfIdentity.ps1`, `MoqiTextService/TypeDuckProfile.h`, `MoqiTextService/TypeDuckProfile.cpp`.
- Task commits exist: `3623f3b`, `589a03a`, `2357577`.
- Final verification commands passed.
- Only remaining dirty status is the pre-existing `jsoncpp` submodule marker.

---
*Phase: 03-zh-hk-tsf-registration-and-installer-skeleton*
*Completed: 2026-06-23*
