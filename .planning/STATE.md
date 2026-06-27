---
gsd_state_version: 1.0
milestone: v1.0
milestone_name: milestone
current_phase: 6
status: completed
stopped_at: Completed 06-06-PLAN.md
last_updated: "2026-06-27T11:38:58.875Z"
last_activity: 2026-06-27
last_activity_desc: Phase 6 marked complete
progress:
  total_phases: 7
  completed_phases: 6
  total_plans: 38
  completed_plans: 34
  percent: 86
current_phase_name: Privacy, Security, and Scaffold Cleanup
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-06-26)

**Core value:** Hong Kong users can install TypeDuck under Chinese (Traditional, Hong Kong) and type Cantonese with the same trusted behavior, bilingual settings, candidate details, and dictionary-like lookup experience as TypeDuck Web alpha.
**Current focus:** Phase 6: Privacy, Security, and Scaffold Cleanup

## Current Position

Phase: 6 — COMPLETE
Plan: 06-06 complete; Phase 6 complete
Status: Phase 6 complete
Last activity: 2026-06-27 — Phase 6 marked complete

Progress: [█████████░] 86% of planned milestone plans

## Performance Metrics

**Velocity:**

- Total plans completed: 27
- Average duration: 20 min
- Total execution time: 1.7 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01 | 2 | 22 min | 11 min |
| 02 | 3 | - | - |
| 3 | 3 | - | - |

**Recent Trend:**

- Last 5 plans: P01 5 min, P02 17 min, P03 10 min, P04 55 min, P05 13 min
- Trend: Phase 5 native candidate, dictionary, settings, About, tray, icon, and resource parity is accepted by iterative human verification and static/build/package guards; Phase 6 scaffold/privacy cleanup is next.

*Updated after each plan completion*
| Phase 01 P01 | 5 min | 2 tasks | 2 files |
| Phase 01 P02 | 17 min | 2 tasks | 8 files |
| Phase 02 P01 | 10 min | 3 tasks | 7 files |
| Phase 02 P02 | 55min | 3 tasks | 9 files |
| Phase 02 P03 | 13min | 3 tasks | 6 files |
| Phase 03 P01 | 9 min | 3 tasks | 8 files |
| Phase 03 P02 | 17 min | 3 tasks | 12 files |
| Phase 03 P03 | 33 min | 3 tasks | 10 files |
| Phase 04 P01 | 120 | 3 tasks | 9 files |
| Phase 04 P02 | 90 | 3 tasks | 10 files |
| Phase 04 P03 | 145 | 3 tasks | 7 files |
| Phase 04 P04 | live debug/UAT | 3 tasks | installer/backend/schema |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P01 | 8 min | 2 tasks | 4 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P02 | 12 min | 3 tasks | 10 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P03 | 27min | 3 tasks | 17 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P04 | 17 min | 3 tasks | 7 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P05 | 16min | 2 tasks | 14 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P07 | 46min | 2 tasks | 6 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P08 | 13 min | 2 tasks | 13 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P09 | 8 min | 3 tasks | 6 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P10 | 15 min | 3 tasks | 17 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P11 | 45min | 2 tasks | 8 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P12 | 15min | 2 tasks | 6 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P13 | 10min | 2 tasks | 12 files |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P14 | user-approved skip | 1 retired evidence path | docs |
| Phase 05-candidate-dictionary-settings-and-about-ui-parity P15 | closeout | UAT/verification/roadmap | docs |
| Phase 06 P01 | 7 min | 3 tasks | 4 files |
| Phase 06-privacy-security-and-scaffold-cleanup P02 | 70 min | 3 tasks | 7 files |
| Phase 06-privacy-security-and-scaffold-cleanup P03 | 13 min | 3 tasks | 11 files |
| Phase 06-privacy-security-and-scaffold-cleanup P07 | ~2h | 3 tasks | 21 files |
| Phase 06 P04 | 12min | 3 tasks | 16 files |
| Phase 06-privacy-security-and-scaffold-cleanup P05 | 7 min | 3 tasks | 5 files |
| Phase 06-privacy-security-and-scaffold-cleanup P06 | 10 min | 3 tasks | 5 files |

## Accumulated Context

### Decisions

Decisions are logged in PROJECT.md Key Decisions table.
Recent decisions affecting current work:

- [Project]: Treat Moqi as scaffold only; TypeDuck Web alpha is product truth.
- [Project]: Use coarse MVP phase structure while keeping high-risk engine, TSF registration, protocol, UI, cleanup, and verification work visible.
- [Project]: Use Quality model profile, parallel execution, git tracking, research, plan-check, verifier, and drift guard.
- [Phase 01]: TypeDuck product identity is centralized in .planning/product/TYPEDUCK-IDENTITY-CONTRACT.md before production renames.
- [Phase 01]: TypeDuck AppId, CLSID, and profile GUID values are proposed pending human review, while zh-HK bilingual profile text is locked by phase decisions.
- [Phase 01]: Moqi, fcitx, WebDAV/cloud clipboard, AI, Simplified-only copy, generic config tools, excessive customization, and legacy Moqi backend fallback are banned from v1 product surfaces.
- [Phase 01]: The 2026-06-23 Web alpha fixture is source-backed at TypeDuck-Web commit db21054 with dirty-worktree status recorded.
- [Phase 01]: Baseline settings/candidate screenshots, multilingual Indonesian-main settings/candidate/dictionary screenshots, supplemental `housam` compound dictionary detail, and Cangjie reverse lookup screenshots were captured at 1280x720.
- [Phase 01]: Web alpha dictionary detail reveal is movement-triggered: actual mouse movement over a candidate shows the panel, while a stationary pointer should not cause typing flicker.
- [Phase 01]: Phase 5 used the 2026-06-23 Web alpha fixture as source-backed layout authority; Phase 7 may refresh Web alpha fixtures if release verification needs fresh visual evidence.
- [Phase 02 Plan 01]: Lookup-filter evidence is recorded as static runtime inclusion: the TypeDuck-HK librime release `version-info.txt` names `rime-dictionary-lookup-filter` commit `3671814d4e4aeab8d616ceea3c7f6d88e96bba02`, tied to staged `rime.dll` SHA-256 evidence.
- [Phase 02 Plan 01]: Generated TypeDuck runtime and cache trees stay ignored; reproducible scripts and manifest evidence are committed instead.
- [Phase 02 Plan 02]: ENG-04 is proven through the existing moqi-ime framed protobuf process boundary, not direct in-process librime.
- [Phase 02 Plan 02]: TypeDuck-HK librime version labels are fork-specific; Rime API compatibility patches are based on API shape and commit history.
- [Phase 02]: Use the Plan 02-02 typing proof as ENG-05 source truth; no fabricated reverse-lookup rows were added when no leading \\v payload was observed.
- [Phase 02]: Preserve lookup-filter candidate comments as raw payload evidence with escaped notation, UTF-8 bytes, code points, control counts, and SHA-256 hashes.
- [Phase 02]: Keep Phase 2 lookup evidence raw-only; Phase 4 and Phase 5 map D-10 columns into structured protocol and native UI fields.
- [Phase 03]: TypeDuck TSF identity is centralized in MoqiTextService/TypeDuckProfile.* using the Phase 3 CLSID/profile GUID contract. — Plan 03-01 moved CLSID/profile GUID/display/DLL constants out of backend metadata and into first-party source.
- [Phase 03]: DllRegisterServer seeds the TypeDuck zh-HK profile before optional backend ime.json scanning. — Backend profile JSON may remain transition metadata but cannot replace or override the required TypeDuck profile.
- [Phase 03]: The CMake target remains MoqiTextService while output/module identity is TypeDuckTextService.dll. — This keeps source churn low while satisfying deployed TSF DLL identity.
- [Phase 03]: Deployed installer/setup names are TypeDuck-owned while internal scaffold target/source names may remain temporarily. — Plan 03-02 deploys TypeDuckLauncher.exe, TypeDuckSetupHelper.exe, TypeDuckTextService.dll, TypeDuckIME paths, and typeduck-windows-ime-setup.exe.
- [Phase 03]: TYPEDUCK_PROGRAM_DIR is the setup-helper registration environment variable. — MOQI_PROGRAM_DIR remains only as a transition-only compatibility alias during registration.
- [Phase 03]: TypeDuck-controlled installer/setup strings are bilingual; standard Inno wizard chrome may fall back to English until a vetted Traditional Chinese pack is bundled. — The local Inno Setup install lacked ChineseTraditional.isl.
- [Phase 03]: Plan 03-03 accepts VM registry/file/task evidence as automated zh-HK registration proof; the guest language list remained en-US, so a Settings screenshot remains optional product-review evidence.
- [Phase 03]: Installer cleanup uses code-safe GUID constants in Pascal registry paths while retaining escaped GUID constants for Inno identity fields.
- [Phase 03]: The VM verification harness never runs the installer on the host; install and uninstall actions run only through PowerShell Direct inside the named Hyper-V VM.
- [Phase 04]: Close typing MVP on live VM user UAT plus static/protocol guards; strict launcher fault-injection proof remains Phase 7 verification debt.
- [Phase 04]: TypeDuck schema packaging belongs in the sibling moqi-ime runtime build; moqi-im-windows consumes the packaged runtime and does not need a schema submodule.
- [Phase 04]: Bundle prebuilt Rime `build` artifacts produced by `rime_deployer.exe --build` on a filtered TypeDuck-HK schema source.
- [Phase 05 Plan 01]: Canonical TypeDuck appearance themes live at `D:/VSProjects/moqi-ime/input_methods/rime/appearance_themes.json`; the data-path copy is compatibility only and must remain byte-identical.
- [Phase 05 Plan 01]: Theme JSON uses native semantic palette roles with top-level fonts; backend code maps those roles into existing runtime appearance fields.
- [Phase 05 Plan 01]: `scripts/Test-TypeDuckAppearanceTheme.ps1` is the focused guard for theme schema, loader path order, and package-copy drift.
- [Phase 05]: TypeDuck lookup-filter parsing lives in MoqiTextService/TypeDuckCandidateInfo.* and is invoked at the candidate-window boundary, not in launcher/protocol/backend transport. — Preserves D-39 and keeps raw backend comments at the renderer trust boundary.
- [Phase 05]: Native display-language, Jyutping visibility, typeface, dictionary label, register, and part-of-speech maps mirror TypeDuck Web consts.ts. — Maintains LANG-03 and candidate/dictionary parity with the Web alpha source.
- [Phase 05]: Candidate window rendering now consumes structured TypeDuck fields and draws the native candidate/dictionary panel with accepted human-verified behavior.
- [Phase 05]: 05-03: TypeDuck settings JSON is saved before generated Rime side effects so deploy failure does not erase the readable source of truth.
- [Phase 05]: 05-03: Only Rime-affecting TypeDuck settings generate default/common custom YAML; interface-only settings remain JSON/native UI state.
- [Phase 05]: 05-03: TypeDuck-HK librime uses the reviewed generated-YAML bridge because the Web levers/custom-settings API is not exposed in this fork.
- [Phase 05]: 05-04: The TypeDuck candidate/dictionary popup remains native Win32/GDI and explicitly rejects Qt in the TSF popup path. — Implemented measured Win32/GDI drawing and a strict guard rejects Qt includes/libs in MoqiTextService.
- [Phase 05]: 05-04: Dictionary detail reveal follows the Web alpha movement threshold instead of passive pointer-rest hover. — Uses actual pointer movement counts before showing dictionary detail, avoiding flicker while typing when the pointer is stationary.
- [Phase 05]: 05-04: Preview screenshots were iteration evidence only and are no longer an acceptance gate. User explicitly replaced Phase 5 screenshot tests with human verification on 2026-06-26.
- [Phase ?]: [Phase 05]: 05-05: TypeDuckSettings is native Win32 and does not use Qt; standard controls and explicit layout anchors are sufficient for the settings/About executable.
- [Phase ?]: [Phase 05]: 05-05: Native settings UI applies through MoqLauncher/TypeDuckPreferences.* and saves TypeDuckPreferences.json rather than creating a parallel settings store.
- [Phase ?]: [Phase 05]: 05-05: About dialog attribution is the concise product copy: TypeDuck Windows IME 版本 Version, TypeDuck-HK librime fork v1.1.4, and TypeDuck-HK schema v2.0.0.
- [Phase ?]: [Phase 05]: 05-07: Install-time and post-install settings entry points launch the fixed first-party TypeDuckSettings.exe.
- [Phase ?]: [Phase 05]: 05-07: Installer staging treats TypeDuckSettings.exe as required payload and packages the newest launcher artifact for verification.
- [Phase ?]: [Phase 05]: 05-07: Backend-declared configTool metadata is no longer used for the Phase 5 settings surface.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-08: Use TypeDuck_Transparent.ico for staged executable surfaces. — D-28 requires transparent TypeDuck icons for executable surfaces; staging-time PE stamping makes the packaged output verifiable even with prebuilt artifacts.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-08: Use TypeDuck_Small.ico for the first-party TSF profile/input picker. — D-29 requires the small icon for Windows input picker; the implementation uses the installed icon path with DLL resource fallback.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-08: Use TypeDuck.ico for Inno setup/uninstall branding. — D-30 requires the full TypeDuck icon for installer, uninstaller, and broad product branding.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-08: Filter legacy moqi.png, mo.ico, mo.png, and moqi.ico from transitional backend staging. — D-31 bans those legacy Moqi images from packaged TypeDuck resource surfaces.
- [Phase 05]: 05-09: Active Phase 5 evidence slots no longer include cropped settings-two-column or incorrect About-dialog evidence.
- [Phase 05]: 05-09: Retry evidence must cite runtime-provenance.json for Windows, backend, TypeDuck Web, runtime DLL/schema/theme, and icon hashes.
- [Phase 05]: 05-09: Rejected UAT behavior is now an explicit RED-mode guard state, not an acceptable green baseline.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-10: VM candidate screenshots from stale external TypeDuck 1.1.2 runtime are not accepted as current package evidence.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-10: Candidate transport remains pass-through; raw_lookup_comment is parsed only at the CandidateInfo renderer boundary.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-10: Candidate preview BMPs are guarded source-backed iteration evidence and must cite runtime-provenance before use.
- [Phase 05]: 05-11: About is now a separate first-party Win32 executable; Settings no longer links or launches the About dialog. — Closes UAT gap 8 and removes the broken Settings-panel About button path.
- [Phase 05]: 05-11: About Start Menu access uses two concrete shortcuts, TypeDuck About and TypeDuck 關於, rather than a slash-delimited shortcut filename. — Windows shortcut filenames should not rely on a literal slash for a product entry point.
- [Phase 05]: 05-11: TypeDuckAbout.exe is staged in this plan, while new executable icon stamping remains 05-12 scope. — Keeps 05-11 inside settings/About accessibility and copy scope.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-12: TypeDuck input picker registration uses the compiled TypeDuckTextService.dll icon resource instead of an installed raw TypeDuck_Small.ico file.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-12: Uninstall display icon resolves from the compiled uninstaller executable instead of an app-root TypeDuck.ico file.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-12: Backend package build stamps server.exe from TypeDuck_Transparent.ico and removes legacy input_methods/rime/icon.ico.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-13: Keep installer/dist and installer/stage as ignored generated outputs; commit package SHA-256 and staged artifact hashes in provenance instead.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-14: The VM/manual screenshot manifest path was skipped by explicit user direction; `.planning/product/ui-fixtures` was removed and Phase 5 acceptance is human verification plus guards.
- [Phase 05-candidate-dictionary-settings-and-about-ui-parity]: 05-15: Final Phase 5 closeout lives in `05-UAT.md`, `05-VERIFICATION.md`, `05-06-SUMMARY.md`, `05-14-SUMMARY.md`, and `05-15-SUMMARY.md`.
- [Phase 06]: 06-01 removed Legacy Moqi registry/startup cleanup from the TypeDuck installer so TypeDuck uninstall does not touch separately installed Moqi state.
- [Phase 06]: 06-01 keeps TypeDuck runtime `server.exe` cleanup path-scoped to the TypeDuck app directory because `server.exe` is a generic process name.
- [Phase 06]: 06-01 renders restart guidance on final installer/uninstaller surfaces with reopen-apps-first wording instead of separate technical popups.
- [Phase 06]: 06-02 TypeDuckRuntime is the shipped runtime folder; legacy PowerShell parameter names remain only for caller compatibility. — TypeDuck-owned install paths are required while preserving script compatibility for existing callers.
- [Phase 06]: 06-02 package output keeps only canonical input_methods/rime/appearance_themes.json. — The former data-path theme copy is duplicate runtime payload and is now rejected by guards.
- [Phase 06]: 06-02 staged backends.json is removed. — Plan 06-03 owns the fixed in-code TypeDuck runtime bridge, so Plan 06-02 must not ship the legacy manifest.
- [Phase 06]: Launcher backend discovery is no longer file-driven; it always constructs one typeduck-runtime-bridge for TypeDuckRuntime/server.exe.
- [Phase 06]: Optional backend ime.json scanning remains compatibility-only and runs after first-party TypeDuck profile mapping.
- [Phase 06]: TSF Configure and launcher Settings stay fixed to TypeDuckSettings.exe and guards continue to reject backend configTool metadata.
- [Phase ?]: 06-07 removed active v1 backend entry points for AI, WebDAV/cloud clipboard, scheme-set download/update, auto-pair, and custom phrase surfaces.
- [Phase ?]: 06-07 uses targeted Rime tests for the privacy/security contract because the full legacy Rime package suite still has unrelated stale failures.
- [Phase 06]: TypeDuck diagnostics use %LOCALAPPDATA%\TypeDuckIME and TypeDuckLauncher.json; Legacy Moqi paths remain separate and untouched.
- [Phase 06]: Cloud clipboard protocol method 19 and request field 26 are removed from v1 and reserved in proto/moqi.proto.
- [Phase 06]: Routine quote-pair diagnostics record redacted lengths instead of raw committed text.
- [Phase 06]: TypeDuck launcher IPC hardening rejects only inspectable obvious mismatches; denied process inspection remains compatibility-tolerant and logs technical detail only.
- [Phase 06]: Named-pipe allow ACEs use explicit read/write/synchronize rights with the network deny ACE preserved.
- [Phase 06]: Phase 6 Plan 06: Strict aggregate guard now invokes focused installer, runtime, backend diagnostics, launcher protocol, settings/About, and icon packaging guards. — This makes VER-02 executable through one strict command and keeps focused guard failures visible in aggregate evidence.
- [Phase 06]: Phase 6 Plan 06: Release/nightly CI surfaces use TypeDuck-Windows, TypeDuck-Windows-backend, TypeDuck-HK/schema aap2-alpha, and typeduck-windows-ime artifact names. — Visible release surfaces must be TypeDuck-owned and schema packaging must use the TypeDuck-HK schema source instead of rime-frost.
- [Phase 06]: Phase 6 Plan 06: Guard evidence is compact JSON metadata only, avoiding raw typed samples and secrets. — The phase threat model requires pass/fail evidence without information disclosure.

### Pending Todos

Phase 6 is complete; Phase 7 compatibility and release verification is next.

### Blockers/Concerns

- [Phase 6]: Visible scaffold leakage, bilingual copy audit, data/log path cleanup, diagnostics safety, pipe hardening, and legacy feature removal remain the next risk area.
- [Phase 06 Plan 02]: Installer compile is blocked outside this plan by installer/MoqiTsf.iss unknown preprocessor directive around line 104.

## Deferred Items

| Category | Item | Status | Deferred At |
|----------|------|--------|-------------|
| Installer localization | Installer language picker showed duplicate English entries; decide whether to bundle/vet Traditional Chinese Inno language resources or keep product-controlled bilingual strings only. | Deferred to installer/UI polish | Phase 3 UAT |
| Installer shortcuts | Start Menu folder creation still warns on the bilingual path with a slash; correct shortcut folder naming in a later installer phase. | Deferred to installer/UI polish | Phase 3 UAT |
| Installed payload cleanup | Installed `backends.json` still references `moqi-ime` and `moqi-ime\\server.exe` inside `C:\Program Files (x86)\TypeDuckIME`; Phase 4/6 should remove or replace this scaffold manifest with a TypeDuck-owned runtime contract. | Deferred to protocol/scaffold cleanup | Phase 3 UAT |
| Typing MVP proof harness | `scripts\Test-TypeDuckTypingMvpProof.ps1 -Strict` still expects executable launcher fault-injection probes; Phase 4 is accepted on static guards plus live VM UAT, and deeper probes should move to release verification. | Deferred to compatibility/release verification | Phase 4 closeout |
| Settings-backed schema customization | `common.custom.yaml` should be generated and updated by frontend settings instead of treated as static packaged data. | Deferred to settings UI | Phase 4 closeout |
| Residual Moqi paths | Logs/config still use legacy paths such as `%LOCALAPPDATA%\MoqiIM`; product-owned path cleanup belongs with broader scaffold cleanup. | Deferred to scaffold cleanup | Phase 4 closeout |
| Release-grade screenshot matrix | Phase 5 screenshot fixtures and screenshot tests were explicitly retired and replaced by human verification; fresh screenshots, if desired, belong to final compatibility/release evidence. | Deferred to release verification | Phase 5 closeout |

## Session Continuity

Last session: 2026-06-27T11:37:28.460Z
Stopped at: Completed 06-06-PLAN.md
Resume file: None
