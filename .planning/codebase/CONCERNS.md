# Codebase Concerns

**Analysis Date:** 2026-06-28

**Scope:** This audit treats `https://github.com/TypeDuck-HK/TypeDuck-Windows` and `https://github.com/TypeDuck-HK/TypeDuck-Windows-backend` as equivalent parts of the TypeDuck Windows v1 product codebase. Paths below are repo-relative and prefixed with `TypeDuck-Windows:` or `TypeDuck-Windows-backend:`.

## Tech Debt

**Moqi-named product identifiers remain in compiled code and build contracts:**
- Issue: Current namespaces, CMake targets, protocol package names, generated protobuf names, header guards, script parameters, installer filename, and some command IDs still use `Moqi`/`moqi` identifiers while the public product and binaries are TypeDuck.
- Files: `TypeDuck-Windows:CMakeLists.txt`, `TypeDuck-Windows:MoqiTextService/CMakeLists.txt`, `TypeDuck-Windows:MoqiTextService/MoqiClient.h`, `TypeDuck-Windows:MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows:MoqiTextService/MoqiImeModule.cpp`, `TypeDuck-Windows:MoqLauncher/PipeServer.cpp`, `TypeDuck-Windows:installer/MoqiTsf.iss`, `TypeDuck-Windows:scripts/_all_in_package.ps1`, `TypeDuck-Windows:proto/moqi.proto`, `TypeDuck-Windows-backend:go.mod`, `TypeDuck-Windows-backend:proto/moqi.proto`, `TypeDuck-Windows-backend:imecore/protocol.go`
- Impact: Future changes can update visible TypeDuck behavior while leaving internal release, diagnostics, protocol, or installer assumptions attached to the wrong product vocabulary. This also makes cross-repo search and ownership ambiguous.
- Fix approach: Define a TypeDuck identity matrix for module names, executable names, DLL names, protocol package names, CMake cache variables, script parameters, and installer filenames. Rename in thin compatibility layers first, then remove stale aliases after release tooling and tests use TypeDuck names only.

**Backend still registers non-TypeDuck services from source input method folders:**
- Issue: The backend always registers the fixed TypeDuck Rime profile, then scans `input_methods` and can register `moqi`, `fcitx5`, and default Moqi-backed services if their `ime.json` files are present.
- Files: `TypeDuck-Windows-backend:server.go`, `TypeDuck-Windows-backend:input_methods/moqi/ime.json`, `TypeDuck-Windows-backend:input_methods/fcitx5/ime.json`, `TypeDuck-Windows-backend:input_methods/moqi/moqi.go`, `TypeDuck-Windows-backend:input_methods/fcitx5/fcitx5.go`
- Impact: Source builds and ad hoc runtime packages can expose Simplified Chinese or incomplete input methods outside the TypeDuck v1 scope. This increases accidental registration and support surface.
- Fix approach: Gate service registration behind a TypeDuck runtime mode, or remove non-TypeDuck input method folders from v1 source builds. Keep test-only implementations behind explicit build tags or test helpers.

**Cross-repo protobuf schemas are not identical:**
- Issue: The frontend schema includes TypeDuck protocol info, settings snapshots, engine capabilities, health, error details, richer `CandidateEntry` fields, and reserved cloud clipboard fields. The backend schema still includes `METHOD_CLOUD_CLIPBOARD_UPLOAD` and only emits `CandidateEntry.text` and `CandidateEntry.comment`.
- Files: `TypeDuck-Windows:proto/moqi.proto`, `TypeDuck-Windows-backend:proto/moqi.proto`, `TypeDuck-Windows:MoqLauncher/PipeClient.cpp`, `TypeDuck-Windows:MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows-backend:imecore/protocol.go`
- Impact: Product behavior depends on a shared wire contract that is maintained in two repos without a single source of truth. Dictionary metadata, health diagnostics, and error recovery can drift silently.
- Fix approach: Make one protobuf schema canonical, generate C++ and Go bindings from it in CI, and add a cross-repo schema compatibility check that fails on unreviewed field, enum, or package drift.

**Backend source contains AI and cloud clipboard surfaces outside the TypeDuck v1 runtime package:**
- Issue: The backend build prunes `ai_config.json`, `cloudclipboard`, and related runtime directories from packaged output, but source code still includes AI hotkeys, AI HTTP calls, cloud clipboard menus, WebDAV dialogs, and preserved keys.
- Files: `TypeDuck-Windows-backend:input_methods/rime/rime.go`, `TypeDuck-Windows-backend:input_methods/rime/ai_config.go`, `TypeDuck-Windows-backend:input_methods/rime/ai_client.go`, `TypeDuck-Windows-backend:input_methods/rime/cloud_clipboard.go`, `TypeDuck-Windows-backend:input_methods/rime/cloudclipboard/client.go`, `TypeDuck-Windows-backend:scripts/build.ps1`, `TypeDuck-Windows-backend:scripts/Test-TypeDuckCandidateParity.ps1`
- Impact: Source-built runtimes, tests, and future changes can re-enable network or clipboard features that are not part of the v1 product surface.
- Fix approach: Move non-v1 surfaces behind build tags or remove them from the product runtime package. Keep packaging-pruning tests, but do not rely on packaging alone to define product behavior.

**Large core files concentrate unrelated responsibilities:**
- Issue: Key path files combine TSF handling, IPC, UI, rendering, settings, Rime calls, product menus, and async state machines.
- Files: `TypeDuck-Windows:MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows:MoqiTextService/MoqiTextService.cpp`, `TypeDuck-Windows:MoqiTextService/MoqiCandidateWindow.cpp`, `TypeDuck-Windows:MoqLauncher/PipeServer.cpp`, `TypeDuck-Windows-backend:input_methods/rime/rime.go`, `TypeDuck-Windows-backend:input_methods/rime/librime.go`, `TypeDuck-Windows-backend:imecore/protocol.go`
- Impact: Behavior changes for candidate display, key handling, settings, or runtime health are high-risk because unrelated flows share mutable state and helper functions.
- Fix approach: Split by stable responsibility: protocol adapter, settings bridge, candidate view model, dictionary payload parser, Rime native bridge, menu model, and installer/registration operations. Keep TSF and Rime boundary code thin.

## Known Bugs

**Backend frame reads can allocate unbounded payloads:**
- Symptoms: `readFrame` reads a 32-bit length and allocates `make([]byte, size)` without a maximum, while the frontend enforces 1 MiB frame caps.
- Files: `TypeDuck-Windows-backend:protocol_io.go`, `TypeDuck-Windows:proto/ProtoFraming.h`, `TypeDuck-Windows:MoqLauncher/BackendServer.cpp`, `TypeDuck-Windows:MoqLauncher/PipeClient.cpp`
- Trigger: A malformed or hostile stdin frame declares a very large payload length.
- Workaround: The frontend launcher limits frames it sends, but the backend process has no equivalent guard if invoked directly or fed malformed input.

**Frontend can accept a same-user named-pipe endpoint without strong server identity validation:**
- Symptoms: The TSF client checks pipe metadata in `isPipeCreatedByMoqiServer`, but the pipe name and same-user ACL are the main trust boundary.
- Files: `TypeDuck-Windows:MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows:MoqLauncher/PipeServer.cpp`, `TypeDuck-Windows:MoqLauncher/PipeSecurity.cpp`
- Trigger: Another same-user process creates or proxies the expected TypeDuck named pipe before the launcher.
- Workaround: Pipe ACLs deny network access and restrict to local app/container/logon principals; no signer or executable-path validation is enforced.

**Dictionary metadata can be reduced to comments at the backend boundary:**
- Symptoms: Backend `CandidateEntry` only has `Text` and `Comment`; frontend dictionary parsing depends on raw lookup comments surviving inside `comment` or `raw_lookup_comment`.
- Files: `TypeDuck-Windows-backend:imecore/protocol.go`, `TypeDuck-Windows-backend:input_methods/rime/rime.go`, `TypeDuck-Windows:MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows:MoqiTextService/TypeDuckCandidateInfo.cpp`, `TypeDuck-Windows:Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp`
- Trigger: Backend changes to candidate formatting, comment normalization, protobuf generation, or Rime lookup filter output.
- Workaround: Frontend tests preserve raw separator parsing; backend tests assert dictionary lookup modules exist, but no single end-to-end test proves rich dictionary fields across both generated schemas.

**Backend workflow Go version does not match `go.mod`:**
- Symptoms: `go.mod` declares `go 1.25.0`, while GitHub Actions in both repos set up Go `1.24.6`.
- Files: `TypeDuck-Windows-backend:go.mod`, `TypeDuck-Windows-backend:.github/workflows/nightly.yml`, `TypeDuck-Windows-backend:.github/workflows/release.yml`, `TypeDuck-Windows:.github/workflows/nightly.yml`, `TypeDuck-Windows:.github/workflows/release.yml`
- Trigger: CI or local builds with `GOTOOLCHAIN` disabled, unavailable toolchain downloads, or stricter Go version checks.
- Workaround: Go's automatic toolchain download can hide this when network access is available.

## Security Considerations

**Named-pipe authorization is broad for local app containers:**
- Risk: The launcher pipe grants read/write/synchronize access to LocalSystem, Administrators, all application packages, and the logon SID.
- Files: `TypeDuck-Windows:MoqLauncher/PipeSecurity.cpp`, `TypeDuck-Windows:MoqLauncher/PipeServer.cpp`
- Current mitigation: Network access is denied, rights are narrower than generic all, and the pipe is per-user under `TypeDuckIME`.
- Recommendations: Add a documented threat model for TSF host processes, verify minimum pipe rights, and add optional server/client process identity validation where Windows permits it.

**Backend process inherits most launcher environment variables:**
- Risk: The launcher copies the current environment into `TypeDuckRuntime/server.exe`, adding only `PYTHONIOENCODING`. Unexpected variables can influence Go, Rime, logging, or optional AI code paths.
- Files: `TypeDuck-Windows:MoqLauncher/BackendServer.cpp`, `TypeDuck-Windows-backend:input_methods/rime/ai_config.go`, `TypeDuck-Windows-backend:input_methods/rime/debug_logging.go`
- Current mitigation: The packaged runtime prunes AI config and the frontend owns the fixed backend command.
- Recommendations: Construct an allowlisted runtime environment for `server.exe`, explicitly remove non-v1 feature env vars, and record the runtime environment contract in tests.

**Runtime staging downloads and extracts binary dependencies:**
- Risk: Runtime staging pulls a pinned librime archive, extracts `rime.dll`, copies schema data, and records hashes. A compromised or unavailable release source blocks packaging or ships a bad engine if verification is incomplete.
- Files: `TypeDuck-Windows:scripts/Stage-TypeDuckRuntime.ps1`, `TypeDuck-Windows:scripts/Test-TypeDuckRuntimeContract.ps1`, `TypeDuck-Windows-backend:scripts/build.ps1`, `TypeDuck-Windows:.github/workflows/release.yml`, `TypeDuck-Windows:.github/workflows/nightly.yml`
- Current mitigation: The staging script records SHA-256 evidence and lookup-filter provenance in a manifest; release workflows checkout the schema source separately.
- Recommendations: Pin expected archive and DLL hashes in source-controlled verification data, verify code signing where possible, and fail release builds if manifest provenance is absent or mismatched.

**Installer performs elevated system DLL replacement and scheduled re-registration:**
- Risk: The setup helper copies 32-bit and 64-bit TSF DLLs into Windows system directories, invokes both `regsvr32` bitnesses, and can schedule a SYSTEM re-registration task after reboot.
- Files: `TypeDuck-Windows:SetupHelper/SetupHelper.cpp`, `TypeDuck-Windows:installer/MoqiTsf.iss`, `TypeDuck-Windows:scripts/install.ps1`
- Current mitigation: Admin elevation is required, source paths come from the installed app directory, and bilingual failure messages surface common failure paths.
- Recommendations: Validate source DLL hashes before copy/register, log all elevated operations to TypeDuck-specific logs, and run clean install/upgrade/uninstall/reboot verification on Windows 10 and Windows 11.

**Debug and runtime logs can contain sensitive typing context:**
- Risk: TSF, launcher, and backend logs include process paths, key states, candidate counts, composition lengths, Rime paths, and error details; backend AI/cloud clipboard source paths can log snippets or network errors when enabled in source builds.
- Files: `TypeDuck-Windows:MoqiTextService/DllEntry.cpp`, `TypeDuck-Windows:MoqiTextService/TsfLog.cpp`, `TypeDuck-Windows:MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows:MoqLauncher/PipeServer.cpp`, `TypeDuck-Windows-backend:server.go`, `TypeDuck-Windows-backend:input_methods/rime/debug_logging.go`, `TypeDuck-Windows-backend:input_methods/rime/ai_client.go`, `TypeDuck-Windows-backend:input_methods/rime/cloud_clipboard.go`
- Current mitigation: Routine backend response payload logging is intentionally suppressed, and frontend debug logging is gated.
- Recommendations: Keep debug logging opt-in, cap logged text, scrub typed content and clipboard values, and include privacy assertions in release verification.

## Performance Bottlenecks

**Synchronous backend round-trips sit on TSF key handling paths:**
- Problem: The TSF client sends key requests through a named pipe to the launcher, which forwards protobuf frames to backend stdin/stdout. Pending requests use bounded waits but still depend on the out-of-process runtime.
- Files: `TypeDuck-Windows:MoqiTextService/MoqiClient.cpp`, `TypeDuck-Windows:MoqLauncher/PipeClient.cpp`, `TypeDuck-Windows:MoqLauncher/BackendServer.cpp`, `TypeDuck-Windows-backend:server.go`, `TypeDuck-Windows-backend:input_methods/rime/rime.go`
- Cause: Input decisions are delegated to a separate process and Rime session state instead of an in-process fast path.
- Improvement path: Maintain strict per-key timeouts, keep first-printable-key fast failure behavior, add telemetry for slow key paths, and preserve local recovery when backend health is degraded.

**Rime initialization and redeploy hold global backend state:**
- Problem: Native Rime initialization is guarded by `sync.Once`; redeploy uses global `nativeRuntimeState` locks and can make operations unavailable during reload.
- Files: `TypeDuck-Windows-backend:input_methods/rime/native_cgo.go`, `TypeDuck-Windows-backend:input_methods/rime/librime.go`, `TypeDuck-Windows-backend:input_methods/rime/appearance_config.go`, `TypeDuck-Windows-backend:input_methods/rime/rime.go`
- Cause: Librime runtime state is process-global while TypeDuck sessions are per TSF client.
- Improvement path: Surface redeploy health through the shared protocol, test concurrent settings updates and key events, and avoid destroying active sessions unless the settings change requires it.

**Candidate window layout and dictionary rendering are heavyweight UI paths:**
- Problem: The candidate window measures text, computes dictionary panels, paints rich rows, and manages candidate details inside a single Win32 window class.
- Files: `TypeDuck-Windows:MoqiTextService/MoqiCandidateWindow.cpp`, `TypeDuck-Windows:MoqiTextService/TypeDuckCandidateInfo.cpp`, `TypeDuck-Windows:MoqiTextService/MoqiTextService.cpp`
- Cause: Rendering, layout, dictionary parsing, host window ownership, and TSF UI element behavior are tightly coupled.
- Improvement path: Cache parsed candidate info and text measurements, separate a renderer-independent view model, and add screenshot/host coverage for dense dictionary rows.

**Release workflows depend on a schema release artifact:**
- Problem: The schema repository now owns the built schema zip, while frontend workflows download that artifact and backend packaging consumes its extracted directory through `-RimeDataSource`.
- Files: `TypeDuck-Windows:.github/workflows/nightly.yml`, `TypeDuck-Windows:.github/workflows/release.yml`, `TypeDuck-Windows-backend:scripts/build.ps1`, `TypeDuck-Windows:scripts/_all_in_package.ps1`
- Cause: Schema build ownership moved out of the frontend workflow, but the frontend installer still depends on the release artifact shape and availability.
- Improvement path: Keep schema artifact ownership and release URL explicit in workflow tests, and avoid duplicating schema pruning/build logic in frontend packaging.

## Fragile Areas

**TSF/COM profile registration depends on exact GUID, locale, and bitness coordination:**
- Files: `TypeDuck-Windows:MoqiTextService/TypeDuckProfile.cpp`, `TypeDuck-Windows:MoqiTextService/DllEntry.cpp`, `TypeDuck-Windows:MoqiTextService/MoqiImeModule.cpp`, `TypeDuck-Windows:SetupHelper/SetupHelper.cpp`, `TypeDuck-Windows:installer/MoqiTsf.iss`
- Why fragile: The profile GUID, CLSID, `zh-HK` locale, service name, deployed DLL name, Inno cleanup keys, and both `regsvr32` bitnesses must stay synchronized.
- Safe modification: Centralize identity constants generated into C++, Inno, and tests. Any GUID/DLL rename requires installer, setup helper, TSF registration, launcher mapping, and backend profile tests in the same change.
- Test coverage: `TypeDuck-Windows:scripts/Test-TypeDuckTsfIdentity.ps1` covers identity expectations, but automated admin install/uninstall registry verification remains limited.

**Backend runtime packaging is split across two repos:**
- Files: `TypeDuck-Windows:scripts/_all_in_package.ps1`, `TypeDuck-Windows:scripts/install.ps1`, `TypeDuck-Windows-backend:scripts/build.ps1`, `TypeDuck-Windows-backend:.github/workflows/release.yml`, `TypeDuck-Windows:.github/workflows/release.yml`
- Why fragile: The frontend installer expects `TypeDuckRuntime/server.exe` and `input_methods/rime/rime.dll`, while backend scripts and workflows still emit or test some older package-output names.
- Safe modification: Treat frontend installer packaging as the release authority for v1. Keep backend artifacts internal unless they are renamed and verified as TypeDuck runtime packages.
- Test coverage: Runtime package pruning and release artifact scripts exist, but a full cross-repo release artifact contract is not enforced by the backend repo's own workflow names and output paths.

**Candidate details depend on lookup-filter control characters and CSV header order:**
- Files: `TypeDuck-Windows:MoqiTextService/TypeDuckCandidateInfo.cpp`, `TypeDuck-Windows:MoqiTextService/MoqiCandidateWindow.cpp`, `TypeDuck-Windows-backend:input_methods/rime/librime.go`, `TypeDuck-Windows-backend:input_methods/rime/rime_runtime_test.go`, `TypeDuck-Windows-backend:input_methods/rime/rime.go`
- Why fragile: `rime-dictionary-lookup-filter` output is encoded inside comments with separator characters and CSV rows. Any plugin output, schema, or comment formatting change can break dictionary display without breaking basic candidate selection.
- Safe modification: Preserve raw lookup comments as opaque wire payloads, parse in one frontend module, and add a cross-repo golden fixture that starts from a real backend response and ends at rendered candidate info.
- Test coverage: Frontend parser tests and backend module tests exist; cross-repo rich dictionary rendering coverage is incomplete.

**Generated protobuf artifacts can diverge from schemas:**
- Files: `TypeDuck-Windows:proto/moqi.proto`, `TypeDuck-Windows:proto/moqi.pb.cc`, `TypeDuck-Windows:proto/moqi.pb.h`, `TypeDuck-Windows-backend:proto/moqi.proto`, `TypeDuck-Windows-backend:proto/moqi.pb.go`, `TypeDuck-Windows:CMakeLists.txt`
- Why fragile: The frontend can generate C++ protobuf files during build or fall back to checked-in generated sources; the backend has checked-in Go output. Without a shared generation check, stale generated code can compile.
- Safe modification: Add a `protoc` verification job in both repos that regenerates and diffs generated files against source, or stop committing generated files and require generated build outputs only.
- Test coverage: Protocol tests exercise selected fields and framing, not full generated schema freshness.

**Settings are persisted and applied in both frontend and backend:**
- Files: `TypeDuck-Windows:MoqLauncher/TypeDuckPreferences.cpp`, `TypeDuck-Windows:TypeDuckSettings/TypeDuckSettingsWindow.cpp`, `TypeDuck-Windows:MoqLauncher/PipeClient.cpp`, `TypeDuck-Windows-backend:input_methods/rime/appearance_config.go`
- Why fragile: Frontend preferences include display-only fields and Rime side effects; backend preferences only receive a subset. Defaults such as page size and Cangjie behavior must match both sides.
- Safe modification: Maintain one documented settings schema and separate "interface-only" from "Rime-affecting" fields in both generated protocol and tests.
- Test coverage: `TypeDuck-Windows:Tests/TypeDuckSettings/TypeDuckPreferences_test.cpp` and `TypeDuck-Windows-backend:input_methods/rime/rime_test.go` cover pieces, but no single integration test validates settings UI to backend Rime config on an installed system.

## Scaling Limits

**Single per-user launcher, pipe, and runtime bridge:**
- Current capacity: One `TypeDuckLauncherMutex`, one per-user `TypeDuckIME` launcher pipe, and one fixed `typeduck-runtime-bridge` backend.
- Limit: Side-by-side stable/beta/dev channels, per-user test installs, or concurrent runtime versions collide.
- Scaling path: Namespace mutexes, pipes, log directories, registry values, runtime directories, and installer AppIds by channel.

**Runtime package assumes one x64 backend and two TSF DLL bitnesses:**
- Current capacity: x64 installer with Win32 and x64 TSF DLLs, plus one x64 Go backend/runtime DLL package.
- Limit: ARM64 Windows, per-architecture backend packages, or multi-engine runtimes are not modeled.
- Scaling path: Add architecture metadata to packaging scripts and runtime manifests, then verify each TSF/launcher/backend combination separately.

**Candidate and dictionary display are bounded by selection-key assumptions:**
- Current capacity: Backend defaults to numeric selection keys and frontend candidate UI commonly assumes up to 10 candidates.
- Limit: Dictionary-rich results, paging, reverse lookup, and multilingual detail panes can exceed compact IME assumptions.
- Scaling path: Keep selection keys, page metadata, and dictionary detail presentation explicit in protocol fixtures and UI tests.

## Dependencies at Risk

**Pinned TypeDuck librime and lookup filter are binary-runtime dependencies:**
- Risk: Product dictionary behavior depends on a specific `rime.dll` build and statically included `rime-dictionary-lookup-filter` evidence.
- Impact: Replacing the DLL or schema data can remove lookup comments, break Rime ABI expectations, or change candidate ordering.
- Next action: Keep runtime manifest hashes, plugin commit evidence, and schema commit evidence required for release. Add a binary compatibility smoke test that loads `rime.dll`, confirms `dictionary_lookup`, and types a lookup-filter case.

**Backend Go module identity remains external to public TypeDuck repo naming:**
- Risk: `go.mod` and imports use an older external module identity, while public repo naming is `TypeDuck-HK/TypeDuck-Windows-backend`.
- Impact: Fork/module resolution, vulnerability scanning, generated protobuf package names, and developer documentation point to a different module identity.
- Next action: Plan a Go module rename with import updates, generated protobuf update, and compatibility notes for any external consumers.

**Frontend C++ dependencies are old or vendored:**
- Risk: `libIME2`, `libuv`, `jsoncpp`, `googletest`, `spdlog`, and protobuf are vendored, fetched, or submodule-based with Windows-specific patches.
- Impact: Security updates, MSVC updates, and Windows API changes can require patching dependencies that are part of the IME runtime path.
- Next action: Track dependency versions in `TypeDuck-Windows:.planning/codebase/STACK.md`, keep product-specific submodule deltas in `TypeDuck-Windows:patches/`, add update tests around TSF registration and IPC, and avoid modifying vendored code outside documented patches.

**Backend workflows package stale standalone runtime assets:**
- Risk: Backend release/nightly workflows still package zip artifacts with older runtime naming, while the TypeDuck installer path consumes `TypeDuckRuntime`.
- Impact: A backend-only release can publish artifacts that do not match the TypeDuck Windows v1 installer contract.
- Next action: Retire standalone backend release artifacts or rename them to TypeDuck runtime artifacts with the same pruning and manifest checks used by the frontend installer build.

## Missing Critical Features

**No single cross-repo release contract file:**
- Problem: The installer, launcher, backend build, schema release artifact, runtime DLL, protobuf schema, and settings fields are verified by separate scripts and tests.
- Blocks: Automated confidence that a given pair of frontend/backend commits produces a compatible TypeDuck Windows v1 installer.

**No authoritative IPC/protocol version negotiation at backend runtime:**
- Problem: The frontend schema has `TypeDuckProtocolInfo`, health, and capability messages, but the backend does not expose matching protocol negotiation.
- Blocks: Safe rolling upgrades, helpful mismatch diagnostics, and explicit refusal of incompatible backend runtimes.

**No installed-system UAT automation for TSF host compatibility:**
- Problem: Script guards inspect source and artifacts, but TSF behavior in real host processes depends on Windows profile registration, COM loading, focus windows, DPI, UI-less mode, and app bitness.
- Blocks: Confident release signoff for Windows 10/11 apps, browsers, Office, console/terminal, elevated apps, and 32-bit hosts.

## Test Coverage Gaps

**Installer and TSF registration lack automated admin integration tests:**
- What's not tested: Clean install, upgrade, uninstall, reboot-required replacement, scheduled re-registration, HKCU/HKLM TSF registry state, and both `System32`/`SysWOW64` registrations.
- Files: `TypeDuck-Windows:SetupHelper/SetupHelper.cpp`, `TypeDuck-Windows:installer/MoqiTsf.iss`, `TypeDuck-Windows:scripts/Test-TypeDuckInstallerSkeleton.ps1`, `TypeDuck-Windows:scripts/Invoke-TypeDuckVmInstallerVerification.ps1`
- Risk: The installer can build while real Windows typing profile registration fails or leaves stale profile entries.
- Priority: High

**Cross-repo protocol generation and compatibility are not one command:**
- What's not tested: Regenerating both C++ and Go protobuf bindings from one schema, diffing generated output, and validating all TypeDuck enum/field support in both runtimes.
- Files: `TypeDuck-Windows:proto/moqi.proto`, `TypeDuck-Windows-backend:proto/moqi.proto`, `TypeDuck-Windows:Tests/TypeDuckProtocol/ProtoFraming_test.cpp`, `TypeDuck-Windows-backend:imecore/protocol_test.go`
- Risk: Candidate metadata, settings updates, and runtime health can regress through schema drift.
- Priority: High

**Real candidate window rendering coverage is incomplete:**
- What's not tested: Visual placement and text fit for dictionary-rich candidate rows across DPI, multi-monitor layouts, vertical/horizontal layouts, UI-less hosts, and long multilingual definitions.
- Files: `TypeDuck-Windows:MoqiTextService/MoqiCandidateWindow.cpp`, `TypeDuck-Windows:Preview/main.cpp`, `TypeDuck-Windows:scripts/Test-TypeDuckCandidateWindow.ps1`, `TypeDuck-Windows:Tests/TypeDuckCandidateData/TypeDuckCandidateInfo_test.cpp`
- Risk: Candidate parsing can pass while actual Windows UI clips, overlaps, or appears in the wrong location.
- Priority: High

**Backend runtime source has more surfaces than packaged runtime tests exercise:**
- What's not tested: Source-built activation of AI, cloud clipboard, `moqi`, `fcitx5`, mobile bridge, Android paths, and standalone backend workflows against the v1 product contract.
- Files: `TypeDuck-Windows-backend:server.go`, `TypeDuck-Windows-backend:mobilebridge/bridge.go`, `TypeDuck-Windows-backend:input_methods/rime/ai_config.go`, `TypeDuck-Windows-backend:input_methods/rime/cloud_clipboard.go`, `TypeDuck-Windows-backend:.github/workflows/release.yml`
- Risk: Non-v1 features can re-enter builds through source or workflow changes without failing installer-focused tests.
- Priority: Medium

**Runtime provenance tests do not fully cover release workflow outputs:**
- What's not tested: Final installer contains exactly the hashed runtime files, schema files, and lookup-filter-enabled built schema declared by the runtime manifest.
- Files: `TypeDuck-Windows:scripts/Stage-TypeDuckRuntime.ps1`, `TypeDuck-Windows:scripts/Test-TypeDuckRuntimeContract.ps1`, `TypeDuck-Windows:scripts/Test-TypeDuckReleaseArtifacts.ps1`, `TypeDuck-Windows-backend:scripts/build.ps1`
- Risk: The runtime manifest can be correct for a staging path while CI packages a different runtime directory.
- Priority: High

---

*Concerns audit: 2026-06-28*
