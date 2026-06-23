# Phase 3: zh-HK TSF Registration and Installer Skeleton - Context

**Gathered:** 2026-06-23
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase makes TypeDuck installable and removable as a deterministic Chinese (Traditional, Hong Kong) Windows TSF IME. It covers the installer skeleton, TypeDuck-owned AppId/CLSID/profile GUID use, first-party `zh-HK` language profile registration, dual-bitness TSF DLL deployment/registration, launcher startup entry, and uninstall cleanup. It should not implement the final TypeDuck protocol, typing behavior, candidate/dictionary UI, settings dialog, About dialog, broad privacy cleanup, or release verification beyond the install/register/uninstall skeleton required by INST-01 through INST-05.

</domain>

<decisions>
## Implementation Decisions

### Profile Registration Authority
- **D-01:** Make the TypeDuck Cantonese `zh-HK` TSF profile a first-party registration source in this repo, not a value discovered solely from backend `ime.json`.
- **D-02:** Use the Phase 1 proposed TypeDuck identifiers as the Phase 3 implementation defaults unless planning discovers a concrete collision: AppId `{9B52CF20-1C5D-4C74-9F5D-9E66377C8F37}`, text service CLSID `{7D92985A-BC53-47B5-A5CC-6E47F86B9D18}`, and profile GUID `{C6E8F5DF-6504-44F9-B7CF-17A195373A83}`.
- **D-03:** Register the profile under `zh-HK` with bilingual display text `TypeDuck 粵語輸入法 / TypeDuck Cantonese IME`. Use fallback locale only as a compatibility detail if Windows APIs require it; it must not change the product target away from Chinese (Traditional, Hong Kong).
- **D-04:** Backend `ime.json` may remain as optional engine/config metadata during the transition, but it must not be able to override the first-party TypeDuck profile GUID, locale, display text, or icon used for TSF registration.

### Installer Identity Scope
- **D-05:** Phase 3 should rename/reword the installer and registration-facing product identity now: installer app name/publisher, AppId, default install directory, Start menu entries, launcher startup key, setup helper messages, restart-required messages, and setup/uninstall operation labels.
- **D-06:** Use bilingual Traditional Hong Kong Chinese and English for all Phase 3 user-facing installer/setup text. Do not keep the vendored Simplified Chinese installer language as the TypeDuck installer chrome.
- **D-07:** Keep Phase 3 focused on installer/registration identity. Broad README rewrite, CI release artifact naming, protocol package names, tray menu cleanup, candidate UI strings, settings/About UI, logs/data directory cleanup, and automated banned-string checks remain later phases unless they block install/register/uninstall behavior.
- **D-08:** Do not implement the install-time settings flow in this phase. INST-06 and the real settings dialog belong to Phase 5; Phase 3 may leave a placeholder-free installer that simply installs and registers TypeDuck cleanly.

### DLL and Bitness Strategy
- **D-09:** Rename the externally deployed TSF DLL to `TypeDuckTextService.dll` in the installer/system-directory registration path during Phase 3. This keeps Windows COM/TSF registration, resource metadata, and uninstall cleanup aligned with the TypeDuck product identity.
- **D-10:** Preserve the existing dual-bitness deployment model: copy/register the Win32 TSF DLL under `SysWOW64` and the x64 TSF DLL under `System32`, using the correct `regsvr32.exe` for each bitness.
- **D-11:** Source directories and internal CMake target names may remain Moqi-scaffolded in Phase 3 if renaming them would add churn. The user-visible/deployed binary names, resource strings, COM registration names, and installer paths are the priority.
- **D-12:** Update `SetupHelper` and installer logic in one coordinated change whenever the DLL name changes. Avoid mixed `MoqiTextService.dll`/`TypeDuckTextService.dll` registration paths because that would create hard-to-debug orphaned COM or TSF entries.

### Uninstall and Migration Cleanup
- **D-13:** Uninstall should remove TypeDuck-owned installer files, startup entries, scheduled tasks, COM/TSF registration, system DLL copies, and TypeDuck profile residue without purging unrelated user state.
- **D-14:** Include a narrow, explicit legacy Moqi cleanup path only for residues that would conflict with the TypeDuck migration or leave duplicate/broken profiles from this scaffold. This should be allowlisted and documented, not a broad "delete anything Moqi-like" sweep.
- **D-15:** Rename the scheduled re-registration task to `TypeDuckIME-ReRegisterTSF` and ensure uninstall removes that task when present.
- **D-16:** Avoid broad process kills where practical. If Phase 3 still needs image-name termination for locked DLL replacement, use TypeDuck-specific process names and leave broader process-cleanup hardening to Phase 6.

### the agent's Discretion
The user selected all gray areas with the agent's recommendations. The planner may choose the exact implementation split across plans, but should keep the first pass small enough to verify clean install, profile appearance under Chinese (Traditional, Hong Kong), dual-bitness registration, and uninstall cleanup before adding broader product cleanup.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Planning
- `.planning/PROJECT.md` - Product truth: TypeDuck Web alpha is authoritative; Moqi is scaffold only.
- `.planning/REQUIREMENTS.md` - Phase 3 requirement mapping for INST-01 through INST-05 and boundaries for later phases.
- `.planning/ROADMAP.md` - Phase 3 goal, dependencies, success criteria, and later phase boundaries.
- `.planning/STATE.md` - Current project position and known Phase 3 concern about fragile zh-HK TSF registration and uninstall cleanup.
- `.planning/phases/01-identity-and-web-parity-contract/01-CONTEXT.md` - Prior decisions for TypeDuck identity authority, zh-HK bilingual profile target, and banned legacy surfaces.
- `.planning/phases/02-engine-runtime-contract-spike/02-CONTEXT.md` - Prior decisions for TypeDuck-HK runtime proof and the internal adapter boundary.

### Product Contracts and Fixtures
- `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` - Source of truth for TypeDuck AppId, CLSID, profile GUID, display names, install paths, DLL/executable names, scheduled task names, and verification guidance.
- `.planning/product/TYPEDUCK-BANNED-SURFACES.md` - Banned legacy Moqi, fcitx, WebDAV/cloud clipboard, AI, Simplified-only, arbitrary config-tool, excessive customization, and legacy fallback surfaces.
- `.planning/product/TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md` - Phase 2 runtime staging contract; confirms the engine/runtime proof but keeps installer integration for later phases.
- `.planning/product/TYPEDUCK-LOOKUP-FILTER-RAW-CONTRACT.md` - Raw lookup-filter evidence; useful only as context that final protocol/UI mapping is not Phase 3 scope.
- `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` - Product parity fixture authority for later UI/settings phases; Phase 3 only needs installer/profile wording consistency.

### Codebase Maps
- `.planning/codebase/STRUCTURE.md` - File locations for TSF registration, installer script, setup helper, staging scripts, and packaging.
- `.planning/codebase/INTEGRATIONS.md` - Windows TSF/COM, installer elevation, system DLL copy, backend metadata, and registry/profile integration points.
- `.planning/codebase/CONCERNS.md` - Known risks around backend-declared profile metadata, undocumented language-profile registry writes, dual-bitness DLL deployment, and Simplified installer chrome.

### Registration and Installer Code
- `MoqiTextService/MoqiImeModule.cpp` - Current text service CLSID, program directory discovery, and backend-driven config tool lookup.
- `MoqiTextService/DllEntry.cpp` - Current registration export and backend `ime.json` profile scanning path to replace or override with first-party TypeDuck profile metadata.
- `libIME2/src/ImeModule.cpp` - Generic TSF registration/unregistration implementation, including `RegisterProfile`, `EnableLanguageProfile`, per-user language profile registry writes, and cleanup.
- `SetupHelper/SetupHelper.cpp` - Elevated copy/register/unregister/reboot-required logic for dual-bitness TSF DLLs and scheduled re-registration.
- `installer/MoqiTsf.iss` - Inno Setup AppId, language, install path, file staging, startup entry, setup helper invocation, process stop, and registry cleanup logic.
- `scripts/install.ps1` - Installer staging paths and artifact copy locations for Win32/x64 payloads.
- `scripts/_all_in_package.ps1` - Full package orchestration that may still assume legacy backend/staging names.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `libIME2/src/ImeModule.cpp`: Already performs TSF COM registration, category registration, `RegisterProfile`, profile enablement, and unregister cleanup. Reuse this rather than inventing a new TSF registration stack.
- `SetupHelper/SetupHelper.cpp`: Already handles admin elevation, system-directory copy, 32-bit and 64-bit `regsvr32`, locked-DLL/reboot-required replacement, and scheduled re-registration. Update constants and paths instead of replacing the helper wholesale.
- `installer/MoqiTsf.iss`: Already stages payload files, invokes `SetupHelper`, writes launcher startup registry entries, and handles install/uninstall hooks. It is the right skeleton for Phase 3, but its product strings and Simplified language are wrong.
- `scripts/install.ps1`: Already stages Win32 and x64 binaries into installer payload roots. Use it to align `TypeDuckIME` staging directories and deployed binary names.

### Established Patterns
- Registration currently scans backend `input_methods/*/ime.json` for profile GUID, locale, display name, and icon. Phase 3 should reverse that authority for the TypeDuck profile: first-party registration metadata is authoritative, backend metadata is optional.
- The installer is x64-only but deploys both Win32 and x64 TSF DLLs. This pattern remains valid for TypeDuck and should be preserved.
- `SetupHelper` uses `MOQI_PROGRAM_DIR` to tell the TSF DLL where the installed payload lives during registration. Phase 3 should introduce `TYPEDUCK_PROGRAM_DIR` and may keep `MOQI_PROGRAM_DIR` only as a temporary compatibility alias if needed.
- Current installer cleanup purges Moqi CLSID/TIP keys tied to the scaffold CLSID. Phase 3 should switch to TypeDuck-owned cleanup while making any legacy Moqi cleanup explicit and narrow.

### Integration Points
- `MoqiTextService/MoqiImeModule.cpp` and `installer/MoqiTsf.iss` must agree on the TypeDuck text service CLSID.
- `MoqiTextService/DllEntry.cpp`, `libIME2/src/ImeModule.cpp`, and the identity contract must agree on the TypeDuck profile GUID, locale, display text, and icon.
- `SetupHelper/SetupHelper.cpp`, `scripts/install.ps1`, `MoqiTextService/MoqiTextService.def`, and resource metadata must agree on the deployed TSF DLL filename.
- `installer/MoqiTsf.iss` and `MoqLauncher/PipeServer.cpp`/launcher target naming must agree on launcher executable name and startup registry value if the launcher is renamed in this phase.

</code_context>

<specifics>
## Specific Ideas

- Implement the TypeDuck profile as a small first-party profile metadata table or helper in the TSF DLL registration path, seeded from `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md`.
- Keep any backend-driven profile scanning behind a transition guard or optional path, but ensure absence/malformed `ime.json` cannot prevent the TypeDuck `zh-HK` profile from registering.
- Prefer bilingual labels in the compact form `Traditional Hong Kong Chinese / English`, for example `TypeDuck 粵語輸入法 / TypeDuck Cantonese IME`.
- Treat Phase 3 verification as install/register/uninstall evidence: installer launches, Windows exposes TypeDuck under Chinese (Traditional, Hong Kong), both bitness DLLs register, and uninstall removes TypeDuck-owned residue.

</specifics>

<deferred>
## Deferred Ideas

- Install-time settings flow and the real settings dialog belong to Phase 5.
- Final TypeDuck protocol and typing behavior belong to Phase 4.
- Candidate/dictionary UI parity, About dialog, and tray/settings UI copy belong to Phase 5 and Phase 6.
- Full banned-surface cleanup, log/data path privacy cleanup, pipe security hardening, arbitrary config-tool removal, and automated leak checks belong mainly to Phase 6.
- Release artifact naming, CI release upload changes, and full compatibility/release evidence belong to Phase 7 unless a smaller change is necessary to package the Phase 3 installer skeleton.

</deferred>

---

*Phase: 3-zh-HK TSF Registration and Installer Skeleton*
*Context gathered: 2026-06-23*
