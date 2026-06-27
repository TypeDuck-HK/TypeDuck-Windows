# Phase 6: Privacy, Security, and Scaffold Cleanup - Context

**Gathered:** 2026-06-27T06:55:00Z
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase removes or hardens remaining TypeDuck v1 product risks before release verification: user-visible scaffold leakage, installer-owned localization, log/data path identity, diagnostic privacy, IPC/frame bounds, fixed first-party settings launch, installer process cleanup, and automated guard coverage for banned strings/features. Source code folder, file, class, and variable names may remain scaffold-shaped for v1 if they are not user-facing. Non-test product code changes should be done in Phase 6 unless Phase 7 later detects a release-blocking failure.

</domain>

<decisions>
## Implementation Decisions

### Product Identity and Scaffold Leakage
- **D-01:** Source code folder names, file names, class names, variables, and other non-visible implementation identifiers may remain Moqi/scaffold-shaped for v1. Do not spend Phase 6 churn renaming internals just to satisfy cosmetic source identity.
- **D-02:** User-facing installation folders, log folders, log filenames, app names, resource metadata, installer strings, tray/menu surfaces, candidate/settings/About/status/error text, and release artifact names must be TypeDuck-owned.
- **D-03:** No v1-visible surface should expose Moqi, fcitx, WebDAV/cloud clipboard, AI, Simplified-only wording, backend-declared generic config tools, or other off-scope scaffold concepts.
- **D-04:** The installed installer page or surface that says TypeDuck and the backend engine are currently running should be eliminated. The installer already knows which TypeDuck processes to stop; it should automate that cleanup instead of showing this page to the user.
- **D-05:** For the sibling `moqi-ime` runtime, stale/off-scope behavior should be removed from shipped TypeDuck runtime/package and callable paths. Source-level deletion is preferred for risky or product-confusing features, but conservative compile/package exclusion is acceptable for low-risk implementation leftovers that are fully inaccessible and testably absent from v1.

### Logs, Diagnostics, and Strings
- **D-06:** Log/debug/printf lines must be entirely English. There should be no Simplified Chinese log/debug/printf text.
- **D-07:** The user's "be conservative" guidance applies when auditing diagnostics privacy and deciding whether a log line should be removed or redacted. It does not mean being conservative about translations; non-English/Simplified log lines should be translated to English.
- **D-08:** Runtime logs and diagnostics should use TypeDuck-owned paths and avoid raw typed content by default. Technical details belong in logs, not routine user-facing dialogs.
- **D-09:** User-facing v1 strings are expected to be mostly bilingual already except installer. Phase 6 should still inspect source/resources and add automated guards where practical.
- **D-10:** Remove the Simplified Chinese Inno language submodule/translation dependency immediately. The TypeDuck installer should supply all installer strings itself instead of relying on official Inno translations.
- **D-11:** Installer strings should be product-owned bilingual Traditional Hong Kong Chinese and English. Standard installer chrome should not be accepted merely because an official translation exists.

### IPC, Launch, and Failure Behavior
- **D-12:** Harden named pipe access, frame-size limits, malformed-frame handling, and bounded diagnostics, but do not add checks that reject legitimate executables or services from past or future TypeDuck versions.
- **D-13:** Any server/client identity check must be compatibility-tolerant. Prefer TypeDuck-owned namespace, ACL, owner/path sanity, and frame/protocol bounds over strict version/signature rules that would break upgrades, downgrades, or future release channels.
- **D-14:** Remove arbitrary backend-provided config tool launching. Configuration should route to the fixed first-party `TypeDuckSettings.exe` entry point.
- **D-15:** Cloud clipboard/WebDAV/AI/fcitx feature paths should be removed or compile-gated out of v1 if removal is temporarily risky. They must not be visible or callable as TypeDuck product behavior.
- **D-16:** User-facing failure behavior should hide technical details by default. Show technical detail only in logs except when TypeDuck typing is no longer possible and restart cannot recover it, or when the implementation believes the user explicitly tampered with something and rejects it.

### Installer and Process Cleanup
- **D-17:** Process termination during install/uninstall is acceptable. The installer should automate known TypeDuck process cleanup rather than asking the user to manually close backend/launcher surfaces.
- **D-18:** Process cleanup should be TypeDuck-specific and installer-owned. Avoid broad legacy process kills except for documented, allowlisted migration cleanup that is necessary to prevent duplicate/broken scaffold residues.
- **D-19:** Installer cleanup should remove TypeDuck-owned system DLL registrations, startup entries, scheduled tasks, install files, and conflicting scaffold residue without purging unrelated user state.
- **D-20:** The installer first page should show `Installer.bmp` and the exact same bilingual text used in the Phase 5 About dialog.
- **D-21:** The Start Menu folder should be `TypeDuckIME`.
- **D-22:** Start Menu entries should include TypeDuck Settings worded `輸入法設定 IME Settings`, About worded `關於 About TypeDuck…`, and Uninstall worded `解除安裝 Uninstall`.

### Guard Coverage
- **D-23:** Add automated or scripted checks that fail on visible Moqi, fcitx, WebDAV/cloud clipboard, AI, Simplified-only installer strings, Simplified Chinese log/debug/printf strings, and legacy installer language resources in user-facing/product surfaces.
- **D-24:** Guard scripts should understand the v1 allowance for source identifiers. They should not fail merely because internal source paths, class names, variables, or target names still contain Moqi where those names are not user-visible.
- **D-25:** Phase 6 may include static/code tests for logs, banned strings, frame bounds, config-tool removal, cloud/AI/fcitx handler removal, installer language resource removal, and safe process cleanup.
- **D-26:** Do not treat the number of files modified as a warning or blocker. Bulk modification is expected in Phase 6, especially when changing log lines and product-owned paths.

### the agent's Discretion
The planner may choose the exact implementation split and guard script structure. It should keep user-visible cleanup, installer-owned localization, diagnostics/privacy, and IPC hardening in Phase 6; leave release-matrix execution to Phase 7; and avoid broad internal renames that do not change shipped behavior.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Planning
- `.planning/PROJECT.md` - Product truth, active cleanup requirements, Phase 5 closeout, and TypeDuck/Moqi boundary.
- `.planning/REQUIREMENTS.md` - Phase 6 requirement mapping for IDEN-01, IDEN-03, IDEN-04, LANG-01, LANG-02, SEC-01 through SEC-05, and VER-02.
- `.planning/ROADMAP.md` - Phase 6 goal, success criteria, and verification expectations.
- `.planning/STATE.md` - Current position and deferred Phase 6 cleanup risks.
- `.planning/phases/03-zh-hk-tsf-registration-and-installer-skeleton/03-CONTEXT.md` - Installer identity, process cleanup, TSF registration, and TypeDuck install path decisions.
- `.planning/phases/04-typeduck-protocol-and-typing-mvp/04-CONTEXT.md` - Protocol/framing bounds, transport boundary, and deferred scaffold cleanup.
- `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-CONTEXT.md` - Settings/About/candidate UI decisions, fixed settings executable, icon/resource decisions, and retired screenshot acceptance path.

### Product Contracts
- `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` - TypeDuck-owned names, paths, registry/profile identities, log/data directories, and artifact names.
- `.planning/product/TYPEDUCK-BANNED-SURFACES.md` - Banned Moqi/fcitx/cloud/AI/Simplified/excess-customization surfaces.
- `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` - Web alpha source of truth for visible user-facing settings/candidate language and scope.

### Codebase Maps
- `.planning/codebase/CONCERNS.md` - Named-pipe, config-tool, cloud clipboard, logging, installer, and scaffold-leakage concerns.
- `.planning/codebase/ARCHITECTURE.md` - TSF DLL, launcher, backend bridge, protocol, installer, and logging architecture.
- `.planning/codebase/INTEGRATIONS.md` - Windows APIs, storage paths, pipe/security, clipboard, installer, and CI integration points.
- `.planning/codebase/TESTING.md` - Test patterns and TypeDuck guard guidance.
- `.planning/codebase/CONVENTIONS.md` - Bilingual string policy, `pwsh` use, and TypeDuck scaffold treatment.

### Implementation Hotspots
- `installer/MoqiTsf.iss` - Installer strings, language resources, process cleanup, setup helper invocation, startup entries, and uninstall cleanup.
- `.gitmodules` - Current Simplified Chinese Inno translation submodule source to remove.
- `SetupHelper/SetupHelper.cpp` - Elevated TSF DLL copy/registration/reboot cleanup and user-visible setup messages.
- `MoqLauncher/PipeServer.cpp` - Tray/menu text, pipe server, cloud clipboard listener, logging, backend mapping, and process behavior.
- `MoqLauncher/PipeSecurity.cpp` - Named-pipe security descriptor.
- `MoqLauncher/PipeClient.cpp` - Client request routing, timeout/restart behavior, and frame parsing.
- `MoqLauncher/BackendServer.cpp` - Backend process bridge, stdout/stderr logging, restart behavior, and protocol routing.
- `MoqiTextService/MoqiClient.cpp` - Client/server identity check, synchronous RPC, technical error handling, candidate/protocol application, and log output.
- `MoqiTextService/MoqiImeModule.cpp` - Fixed settings launch replacement for backend-declared config tools.
- `MoqiTextService/TsfLog.cpp` and `libIME2/src/DebugLogConfig.cpp` - TSF/debug log path and opt-in behavior.
- `proto/ProtoFraming.h` and `proto/moqi.proto` - Frame-size limits, malformed-frame behavior, and unused scaffold methods.
- `scripts/install.ps1`, `scripts/_all_in_package.ps1`, `.github/workflows/release.yml`, `.github/workflows/nightly.yml` - Staging, artifact names, and package-time guard integration.
- `D:\VSProjects\moqi-ime` - Sibling runtime repo whose packaged payload must stop exposing stale/off-scope paths and files listed in Specific Ideas.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `SetupHelper/SetupHelper.cpp`: Existing admin elevation, dual-bitness DLL copy/registration, locked-DLL replacement, scheduled task, and uninstall mechanics should be updated rather than replaced wholesale.
- `installer/MoqiTsf.iss`: Existing Inno install/uninstall skeleton can remain the implementation base while replacing strings, language resources, process pages, and cleanup operations.
- `proto/ProtoFraming.h`: Existing shared frame helper is the correct place to enforce common max-frame behavior.
- `MoqiTextService/MoqiImeModule.cpp`: Existing configuration entry point should be narrowed to fixed first-party TypeDuck settings launch.
- `MoqLauncher/PipeSecurity.cpp` and `MoqLauncher/PipeServer.cpp`: Existing pipe ACL and namespace logic can be tightened without strict version rejection.
- Existing Phase 5 guard scripts and package checks show the right pattern for focused PowerShell verification.

### Established Patterns
- TSF host processes load the text service DLL in-process, so failures and technical diagnostics must be conservative and should avoid user-visible detail unless typing is unrecoverable.
- Launcher/backend isolation stays in place for v1. Phase 6 hardens the boundary rather than moving the engine in-process.
- Product-owned strings should be bilingual Traditional Hong Kong Chinese and English, while logs/debug/printf output should be English only.
- TypeDuck source/product contracts allow internal scaffold names to remain temporarily when they are not visible.

### Integration Points
- Installer cleanup connects `installer/MoqiTsf.iss`, `SetupHelper/SetupHelper.cpp`, system DLL names, startup entries, scheduled tasks, and process termination.
- IPC hardening connects `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeClient.cpp`, `MoqLauncher/BackendServer.cpp`, `MoqLauncher/PipeSecurity.cpp`, and `proto/ProtoFraming.h`.
- Log/privacy cleanup connects `MoqLauncher/PipeServer.cpp`, `MoqiTextService/TsfLog.cpp`, `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`, and `libIME2/src/DebugLogConfig.cpp`.
- Banned-surface guards connect source/resource scans, installer script scans, protocol method scans, and package/stage artifact scans.

</code_context>

<specifics>
## Specific Ideas

- No Simplified Chinese log/debug/printf lines; logs should be English only.
- Remove the Simplified Chinese Inno translation submodule/resource immediately and make installer text fully product-owned.
- Eliminate the installer page/surface that says TypeDuck and the backend engine are currently running; automate known TypeDuck process cleanup instead.
- Do not implement executable/service identity checks that reject past or future TypeDuck versions.
- Default user-facing failures should be non-technical; technical details go to logs unless typing cannot recover or tampering is suspected.
- Phase 6 owns non-test product code changes for cleanup/security. Phase 7 should mostly verify unless it uncovers a release-blocking failure.
- Rename the installed backend payload folder from `%PROGRAMFILES(x86)%\TypeDuckIME\moqi-ime` to a TypeDuck-owned name.
- Move `%LOCALAPPDATA%\MoqiIM` to `%LOCALAPPDATA%\TypeDuckIME`, same location family as `TypeDuckPreferences.json`.
- Rename `%APPDATA%\Moqi` to `%APPDATA%\TypeDuckIME`.
- Rename `%LOCALAPPDATA%\MoqiIM\MoqiLauncher.json` to `TypeDuckLauncher.json`.
- Rename log files under `%LOCALAPPDATA%\MoqiIM\Log`; changing the `APP` variable may be sufficient if the current logger derives names from it.
- Delete from the shipped TypeDuck runtime: `%PROGRAMFILES(x86)%\TypeDuckIME\moqi-ime\input_methods\rime\android`, `cloudclipboard`, `templates`, `test`, `ai_config.json`, and `ime.json`.
- Delete `%PROGRAMFILES(x86)%\TypeDuckIME\moqi-ime\input_methods\rime\data\appearance_themes.json`; use `%PROGRAMFILES(x86)%\TypeDuckIME\moqi-ime\input_methods\rime\appearance_themes.json` instead.
- Delete `%PROGRAMFILES(x86)%\TypeDuckIME\backends.json` and inline the needed backend content in code.
- Delete legacy roaming files `%APPDATA%\Moqi\ai_config.json`, `appearance_config.json`, `moqi_auto_pair_symbols.json`, and `moqi_custom_phrase.json` from the TypeDuck v1 path/migration surface.
- Combine or remove installed icon folders `%PROGRAMFILES(x86)%\TypeDuckIME\moqi-ime\icons` and `%PROGRAMFILES(x86)%\TypeDuckIME\moqi-ime\input_methods\rime\icons`; if they are not exposed except through app icons, removal may be preferable.
- Installer first page should use `Installer.bmp` and the exact About text from Phase 5 context.
- Start Menu folder and entries should be: `TypeDuckIME`, `輸入法設定 IME Settings`, `關於 About TypeDuck…`, and `解除安裝 Uninstall`.

</specifics>

<deferred>
## Deferred Ideas

- Screenshot-based release evidence is not part of Phase 6 and should not be automated in Phase 7 either; DPI and visual judgement are interactive.
- Integration-test setup outside the current v1 scope should not be pulled into Phase 6.

</deferred>

---

*Phase: 6-Privacy, Security, and Scaffold Cleanup*
*Context gathered: 2026-06-27T06:55:00Z*
