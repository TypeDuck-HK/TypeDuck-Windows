# Phase 7: Compatibility and Release Verification - Context

**Gathered:** 2026-06-27T06:55:00Z
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase proves the TypeDuck v1 installer and IME are shippable after Phase 6 cleanup. It should gather repeatable install, uninstall, typing, host-app, protocol, recovery, and release artifact evidence without adding new product behavior. No VM automation that requires screenshot capture should be done. Visual/DPI verification is interactive: the user controls the VM and judges the result. Phase 7 may add tests, scripts, documentation, and release evidence; non-test product code changes belong in Phase 6 unless Phase 7 detects a release-blocking failure.

</domain>

<decisions>
## Implementation Decisions

### Interactive VM and Screenshot Boundary
- **D-01:** Do not create VM automation that requires capturing screenshots. Screenshot-driven verification should be interactive, not automated.
- **D-02:** "Interactive screenshot-driven verification" means the user will judge visually without capturing screenshots. Do not require screenshot files, screenshot manifests, screenshot capture scripts, or automated screenshot comparison for v1 release verification.
- **D-03:** DPI verification is user-controlled in the VM. The user will try 100%, 140% if the VM supports fractional scaling, 175%, and 200%.
- **D-04:** The user will judge DPI/visual behavior directly.
- **D-05:** Multi-monitor behavior was already human verified in Phase 5. Do not reintroduce multi-monitor screenshot automation as a Phase 7 acceptance gate.

### Release Evidence Matrix
- **D-06:** Phase 7 should include the important release verification bits that are repeatable without screenshots: clean install, reinstall/upgrade where applicable, uninstall, reboot-required registration behavior, typing smoke, host-app coverage, protocol recovery, and final artifact naming/hash evidence.
- **D-07:** Host-app verification should remain interactive where Windows UI behavior is the subject of judgement. Useful targets include Notepad, browsers, Office or Office-like apps, terminal/console contexts, elevated apps, and any awkward TSF hosts available during release testing.
- **D-08:** Windows 10/11 VM evidence is useful where available, but do not pull broader integration-test infrastructure outside v1 scope.
- **D-09:** Bitness verification remains important because the installer deploys both 32-bit and 64-bit TSF DLLs.
- **D-10:** Release evidence should be documented with commands, logs, hashes, registry/file checks, and human-verification notes instead of requiring captured screenshots.

### Protocol and Recovery Verification
- **D-11:** Include repeatable tests for the important protocol/recovery scenarios that do not require screenshots: normal Cantonese input frames, dictionary lookup payload preservation/parsing expectations, reverse lookup where supported, malformed frames, oversized frames, invalid protobuf payloads, timeout behavior, backend restart/crash behavior, stale/mismatched sequence responses, settings update/redeploy failure, and bounded degraded states.
- **D-12:** The repeatedly testable parts should be automated or scripted where practical. Visual host-app judgement remains interactive.
- **D-13:** Technical recovery failures should preserve the Phase 6 failure policy: user-facing technical detail stays out of routine UI, with detail in logs unless typing is unrecoverable or tampering is suspected.

### Product Change Boundary
- **D-14:** All non-test product code changes should be done in Phase 6 unless a failure is detected in Phase 7.
- **D-15:** If Phase 7 finds a product bug, planning should route the smallest necessary fix through a focused failure repair path and then rerun the affected release verification. Do not use Phase 7 to add new capabilities.
- **D-16:** Phase 7 may add or update tests, scripts, verification docs, release evidence manifests, and packaging checks as part of release readiness.

### GitHub Workflow and Schema Source
- **D-17:** GitHub workflow references should use the frontend repository name `TypeDuck-Windows`, replacing `moqi-im-windows`.
- **D-18:** GitHub workflow references should use the backend repository name `TypeDuck-Windows-backend`, replacing `moqi-ime`.
- **D-19:** Release/nightly workflow packaging must not use `rime-frost`. Use `https://github.com/TypeDuck-HK/schema` on the temporary `aap2-alpha` branch.
- **D-20:** The workflow/package path should run the schema files through the Rime deployer to generate the runtime `build` folder as described by the engine-runtime contract. Do not upload or publish the schema checkout as its own artifact; only the final installer/release evidence should be artifacts.

### the agent's Discretion
The planner may choose the exact release checklist and test/script split, but must keep screenshot capture out of automation, treat DPI as user-judged interactive VM work, and include only repeatable non-screenshot protocol/recovery checks in automated release verification.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Planning
- `.planning/PROJECT.md` - Verification environment expectations and Phase 5 screenshot retirement decision.
- `.planning/REQUIREMENTS.md` - Phase 7 requirement mapping for VER-03 through VER-06.
- `.planning/ROADMAP.md` - Phase 7 goal and success criteria.
- `.planning/STATE.md` - Current handoff after Phase 5 and Phase 6 planning.
- `.planning/phases/03-zh-hk-tsf-registration-and-installer-skeleton/03-CONTEXT.md` - VM-backed install/register/uninstall verification approach.
- `.planning/phases/04-typeduck-protocol-and-typing-mvp/04-CONTEXT.md` - Protocol golden path, frame bounds, timeout, and restart/degraded-state decisions.
- `.planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-CONTEXT.md` - Human-verification closeout, retired screenshot fixtures, candidate/dictionary/settings/About behavior, and release evidence handoff.
- `.planning/phases/06-privacy-security-and-scaffold-cleanup/06-CONTEXT.md` - Phase 6 cleanup/security decisions that Phase 7 verifies.

### Product Contracts and Evidence
- `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` - Release artifact names, install paths, registry/profile identifiers, and TypeDuck-owned resources.
- `.planning/product/TYPEDUCK-BANNED-SURFACES.md` - Banned-surface expectations to verify after install.
- `.planning/product/TYPEDUCK-LOOKUP-FILTER-RAW-CONTRACT.md` - Raw lookup/dictionary payload behavior for protocol recovery tests.
- `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` - Product behavior references; do not recreate retired Phase 5 screenshot gates.

### Codebase Maps
- `.planning/codebase/TESTING.md` - Current test patterns, product test guidance, and verification gaps.
- `.planning/codebase/CONCERNS.md` - Release-critical risks around TSF registration, dual-bitness deployment, candidate placement, protocol recovery, and installer cleanup.
- `.planning/codebase/ARCHITECTURE.md` - End-to-end TSF, launcher, backend, protocol, and installer architecture.
- `.planning/codebase/INTEGRATIONS.md` - Windows integration points, storage paths, logs, CI/release workflows, and pipe/backend integration.

### Verification Hotspots
- `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`, and `installer/build-installer.ps1` - Build, staging, packaging, and release artifact creation.
- `installer/MoqiTsf.iss` and `SetupHelper/SetupHelper.cpp` - Clean install, uninstall, reinstall, reboot-required registration, process cleanup, and TypeDuck path checks.
- `proto/ProtoFraming.h`, `proto/moqi.proto`, `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/PipeClient.cpp`, and `MoqLauncher/BackendServer.cpp` - Protocol, frame, timeout, stale response, and backend recovery verification.
- `MoqiTextService/MoqiCandidateWindow.cpp` and `MoqiTextService/MoqiTextService.cpp` - Interactive host-app candidate/UI verification and DPI judgement.
- `.github/workflows/release.yml` and `.github/workflows/nightly.yml` - Release artifact naming and CI packaging behavior.
- `.planning/product/TYPEDUCK-ENGINE-RUNTIME-CONTRACT.md` - TypeDuck schema source, `aap2-alpha` branch, and Rime deployer build-folder expectations.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- Existing PowerShell build/package scripts can produce installer artifacts and can host repeatable non-screenshot checks.
- Phase 3 VM verification patterns provide a precedent for install/register/uninstall evidence without relying on screenshot automation.
- Phase 4 protocol/static guards and `Proto::FrameBuffer` tests are the natural base for repeatable protocol recovery evidence.
- Phase 5 human verification records establish that candidate/settings/About UI can be accepted by hands-on judgement rather than screenshot tests.

### Established Patterns
- Use `pwsh -NoProfile -ExecutionPolicy Bypass -File ...` for project scripts and evidence commands.
- Documentation/evidence timestamps should use invariant UTC formatting.
- Release verification should mix automated checks for deterministic behavior with interactive VM checks for visual/host-app behavior.
- Screenshot tests and `.planning/product/ui-fixtures` were explicitly retired for Phase 5 and should not quietly return as a Phase 7 automation requirement.

### Integration Points
- Install verification connects installer, setup helper, system DLL registration, language profile state, startup entry, scheduled task, uninstall cleanup, and artifact naming.
- Typing verification connects TSF host apps, launcher/backend runtime, protocol frames, settings, candidate UI, and logs.
- Recovery verification connects frame parsing, request sequencing, timeout handling, backend lifecycle, user-visible degraded states, and log evidence.
- Release artifact verification connects packaging scripts, installer output, hashes, staged resources, banned-surface guards, and GitHub workflow artifact names.

</code_context>

<specifics>
## Specific Ideas

- DPI checks to be controlled by the user in the VM at 100%, 140% if available, 175%, and 200%.
- No screenshot-capture automation for VM verification.
- User judgement means no screenshots need to be captured.
- Multi-monitor does not need new screenshot automation because it was already human verified in Phase 5.
- Include important protocol recovery scenarios only when they are repeatedly testable without screenshots.
- Other broad integration-test setup is outside v1 scope.
- Non-test product code changes belong in Phase 6 unless Phase 7 discovers a real failure.
- GitHub workflow checks should use `TypeDuck-Windows` and `TypeDuck-Windows-backend`, fetch `TypeDuck-HK/schema` on `aap2-alpha`, run the schema through the Rime deployer to create a runtime `build` folder, avoid `rime-frost`, and avoid producing a separate schema artifact.

</specifics>

<deferred>
## Deferred Ideas

- Automated screenshot capture/comparison and screenshot manifests remain out of v1 verification.
- Broader integration-test infrastructure beyond the current v1 release needs is deferred.

</deferred>

---

*Phase: 7-Compatibility and Release Verification*
*Context gathered: 2026-06-27T06:55:00Z*
