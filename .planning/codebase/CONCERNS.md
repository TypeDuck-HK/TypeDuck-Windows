# Codebase Concerns

**Analysis Date:** 2026-06-23

## Tech Debt

**Legacy Moqi product identity is embedded across code, installer, protocol, CI, and runtime paths:**
- Issue: Current binaries, namespaces, directories, registry cleanup, log paths, tray text, installer labels, release artifacts, and documentation are Moqi-specific. Treat these as scaffold-only for the TypeDuck Cantonese IME target.
- Files: `CMakeLists.txt`, `README.md`, `TODO.md`, `MoqiTextService/MoqiImeModule.cpp`, `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiTextService.rc.in`, `MoqiTextService/MoqiTextService.def`, `MoqiTextService/TsfLog.cpp`, `MoqLauncher/PipeServer.cpp`, `SetupHelper/SetupHelper.cpp`, `installer/MoqiTsf.iss`, `installer/README.txt`, `scripts/build.ps1`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`, `.github/workflows/release.yml`, `.github/workflows/nightly.yml`
- Impact: TypeDuck deliverables inherit wrong product names, wrong install directories, wrong AppId/CLSID coordination, wrong log locations, wrong tray/menu strings, wrong release asset names, and user-facing Simplified Chinese/Moqi references.
- Fix approach: Create a single TypeDuck product identity contract with executable names, DLL names, AppId, TSF CLSID, profile GUIDs, install directories, log directories, release artifact names, installer text, tray strings, and protocol package names. Apply it through CMake configure definitions and installer/script constants instead of editing one-off string literals.

**Backend architecture still depends on an out-of-process TypeDuck runtime bridge:**
- Issue: The Windows frontend now launches one fixed `TypeDuckRuntime\server.exe` bridge and speaks protobuf over stdio through `MoqLauncher`; the sibling backend source still carries legacy Moqi implementation history.
- Files: `MoqLauncher/BackendServer.cpp`, `MoqLauncher/PipeServer.cpp`, `MoqiTextService/MoqiClient.cpp`, `proto/moqi.proto`, `proto/ProtoFraming.h`, `scripts/install.ps1`, `scripts/_all_in_package.ps1`, `.github/workflows/release.yml`, `.github/workflows/nightly.yml`
- Impact: The process boundary is now TypeDuck-owned, but later cleanup still needs diagnostics/path/IPC hardening and release verification around the packaged runtime.
- Fix approach: Keep the fixed `TypeDuckRuntime` bridge for v1, harden IPC and diagnostics in Phase 6, and verify runtime behavior in Phase 7.

**Runtime language profile metadata is discovered from backend `ime.json` files:**
- Issue: Registration scans `{programDir}\{backend}\input_methods\*\ime.json` at `DllRegisterServer` time and trusts those files for profile name, GUID, locale, fallback locale, and icon.
- Files: `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiImeModule.cpp`, `libIME2/src/ImeModule.cpp`, `MoqLauncher/PipeServer.cpp`
- Impact: A missing or malformed backend payload can register no language profiles. TypeDuck's required installation under Chinese (Traditional, Hong Kong) is not guaranteed by first-party TSF code.
- Fix approach: Make the TypeDuck Hong Kong Traditional profile a first-party build-time resource with deterministic GUID, locale `zh-HK`, display names in Traditional Chinese Hong Kong and English, and icon metadata. Use backend metadata only for optional engine configuration.

**Generated protobuf sources are committed while CMake also regenerates them:**
- Issue: `proto/moqi.pb.cc` and `proto/moqi.pb.h` are checked in, but `CMakeLists.txt` generates protobuf output into `${CMAKE_BINARY_DIR}/generated/proto` and the targets compile `${MOQI_PROTO_CPP}` from the build directory.
- Files: `proto/moqi.pb.cc`, `proto/moqi.pb.h`, `proto/moqi.proto`, `CMakeLists.txt`, `MoqiTextService/CMakeLists.txt`, `MoqLauncher/CMakeLists.txt`
- Impact: Developers can read or edit stale generated sources that are not compiled. Protocol changes can appear committed while local builds use different generated files.
- Fix approach: Either remove checked-in generated protobuf files or make them the canonical source with a regeneration check in CI. Prefer removing generated files and adding an explicit `protoc` verification step.

**Large first-party source files mix TSF, UI, RPC, state management, and product behavior:**
- Issue: Several core files are oversized and multi-responsibility: `MoqiTextService/MoqiClient.cpp` (1584 lines), `MoqiTextService/MoqiTextService.cpp` (1155 lines), `MoqiTextService/MoqiCandidateWindow.cpp` (914 lines), `SetupHelper/SetupHelper.cpp` (767 lines), and `MoqLauncher/PipeServer.cpp` (713 lines).
- Files: `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`, `SetupHelper/SetupHelper.cpp`, `MoqLauncher/PipeServer.cpp`
- Impact: TypeDuck behavior changes are high-risk because input processing, candidate UI, async response handling, logging, and compatibility workarounds are interleaved.
- Fix approach: Split along stable boundaries: `TypeduckEngineClient`, `CandidateViewModel`, `CandidateWindowRenderer`, `ProfileRegistration`, `InstallerSystemDllOps`, and `LauncherIpcServer`. Keep TSF callback glue thin.

**User-facing fcitx, AI, WebDAV, and Moqi feature clutter exists in docs and launcher behavior:**
- Issue: README and launcher code describe or implement features outside the TypeDuck target, including fcitx-in-progress text, WebDAV cloud clipboard, AI config, multiple Moqi schemes, and Moqi tray controls.
- Files: `README.md`, `TODO.md`, `MoqLauncher/PipeServer.cpp`, `MoqLauncher/BackendServer.cpp`, `proto/moqi.proto`, `MoqiTextService/MoqiClient.cpp`
- Impact: Shipping these paths creates unused user-facing clutter and product confusion for a TypeDuck Cantonese IME.
- Fix approach: Remove or compile-gate non-TypeDuck feature surfaces. Keep only controls that match TypeDuck Web alpha behavior and expose text in bilingual Traditional Hong Kong Chinese and English.

## Known Bugs

**Backspace while editing is noted as sluggish:**
- Symptoms: Holding Backspace while deleting composing text is reported as laggy.
- Files: `TODO.md`, `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/BackendServer.cpp`
- Trigger: Hold Backspace during active composition with backend round-trips enabled.
- Workaround: Not detected.

**Mixed Chinese/English input and Shift case switching are noted as unreliable:**
- Symptoms: Mixed Chinese/English input has bugs and Shift case switching sometimes fails even when keyboard events are observed.
- Files: `TODO.md`, `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiClient.cpp`, `proto/moqi.proto`
- Trigger: Switch between Chinese and English input with Shift while composing or entering printable keys.
- Workaround: Not detected.

**Client-side server identity check always succeeds:**
- Symptoms: `Client::isPipeCreatedByMoqiServer` obtains the named-pipe server PID but returns `true` without validating executable path, signer, owner, or command line.
- Files: `MoqiTextService/MoqiClient.cpp`
- Trigger: A same-user process creates the expected named pipe before or instead of the launcher.
- Workaround: Pipe name includes the current username, and launcher pipe ACLs restrict some access on Windows 8+.

**Backend responses without expected sequencing can stall or reset the client path:**
- Symptoms: `Client::callRpcMethod` loops until it receives a response with the matching `seq_num`; mismatched frames are queued as async responses and the method then tries to read more pending data.
- Files: `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/BackendServer.cpp`, `proto/moqi.proto`
- Trigger: Backend emits async responses, stale responses, malformed responses, or no matching response during a synchronous key request.
- Workaround: Failed calls close the RPC connection and reset text-service state.

**Message and candidate window placement uses TODO/FIXME fallback positioning:**
- Symptoms: Candidate and message windows move to `textRect.left, textRect.bottom` with explicit FIXME comments about placement; message windows default to `(0, 0)` when there is no composition rect.
- Files: `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`
- Trigger: Host applications without reliable `ITfContextView` rectangles, UI-less contexts, immersive apps, console apps, or composition rect failures.
- Workaround: Candidate owner/window recovery logic recreates or repositions the candidate window when possible.

## Security Considerations

**Named-pipe framing has no maximum payload length:**
- Risk: `Proto::FrameBuffer::nextFrame` trusts a 32-bit frame length and keeps appending until that length arrives. A malicious or broken backend/client can force unbounded memory growth.
- Files: `proto/ProtoFraming.h`, `MoqLauncher/BackendServer.cpp`, `MoqLauncher/PipeClient.cpp`, `MoqiTextService/MoqiClient.cpp`
- Current mitigation: Protobuf parsing rejects invalid complete payloads; no size limit is enforced before buffering.
- Recommendations: Define a strict maximum frame size for requests and responses, reject oversized frames, clear buffers on violation, and log a bounded diagnostic.

**Pipe access control is broad for app containers and uses a process-style access mask:**
- Risk: `PipeSecurityAttributes` grants `GENERIC_ALL` to LocalSystem, Built-in Administrators, and all application packages, then grants the logon SID a broad `PROCESS_ALL_ACCESS_HEX` value for a pipe security descriptor.
- Files: `MoqLauncher/PipeSecurity.cpp`, `MoqLauncher/PipeServer.cpp`
- Current mitigation: Remote network access is denied and the logon SID is included; pipe names include the username.
- Recommendations: Replace broad rights with explicit named-pipe read/write/connect rights, validate the TSF client process identity where practical, and document the minimum access required for packaged/Store app compatibility.

**Runtime profile and configuration tools are trusted from install payload JSON:**
- Risk: `MoqiImeModule::onConfigure` reads `configTool`, `configToolParams`, and `configToolDir` from backend `ime.json` and launches the tool with `ShellExecuteW`.
- Files: `MoqiTextService/MoqiImeModule.cpp`, `MoqiTextService/DllEntry.cpp`
- Current mitigation: Relative paths are resolved under the input method directory; no signer or directory allowlist is enforced.
- Recommendations: For TypeDuck, remove arbitrary backend-provided config tool launching or restrict it to a signed first-party executable under the TypeDuck install directory.

**Installer force-kills launcher processes and writes system DLLs:**
- Risk: Inno Setup runs `taskkill /F /T /IM MoqiLauncher.exe`, and `SetupHelper` copies TSF DLLs into system directories and schedules reboot replacements/tasks as admin.
- Files: `installer/MoqiTsf.iss`, `SetupHelper/SetupHelper.cpp`
- Current mitigation: Installer requires admin, uses quoted command-line arguments, and handles reboot-required cases.
- Recommendations: Use TypeDuck-specific process names, avoid broad image-name kills where possible, validate source DLL paths and signatures before copying, and log setup operations to a TypeDuck-specific location.

**Cloud clipboard uploads arbitrary clipboard text when enabled:**
- Risk: The launcher registers a clipboard listener, reads `CF_UNICODETEXT`, and forwards it to the backend when `cloud_clipboard.json` has `enabled=true`.
- Files: `MoqLauncher/PipeServer.cpp`, `MoqLauncher/BackendServer.cpp`, `proto/moqi.proto`
- Current mitigation: Feature is gated by config and returns early when disabled.
- Recommendations: Remove this feature for TypeDuck unless explicitly required. If retained, add clear consent, maximum text length, secret redaction guidance, and visible status controls.

**Debug logs can record process names, executable paths, key handling state, and candidate content behavior:**
- Risk: Trace/debug logs write to `%LOCALAPPDATA%\MoqiIM\Log` and include process paths, focus state, composition/candidate state, and internal events.
- Files: `MoqiTextService/DllEntry.cpp`, `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/TsfLog.cpp`, `MoqLauncher/PipeServer.cpp`
- Current mitigation: Trace/debug logging is gated by `Ime::isTraceLoggingEnabled()` and `Ime::isDebugLoggingEnabled()` in several paths; launcher log level defaults to error.
- Recommendations: Keep logs opt-in, scrub typed text where possible, rotate all logs consistently, and rename paths to TypeDuck-specific storage.

## Performance Bottlenecks

**Synchronous named-pipe RPC is on the key handling path:**
- Problem: `Client::callRpcMethod` uses `TransactNamedPipe` during key filters and key handling, with reconnect loops that can wait up to 10 attempts of 3000 ms when establishing the launcher pipe.
- Files: `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiTextService.cpp`
- Cause: The TSF layer waits on backend IPC for key decisions instead of having a non-blocking local engine path or a bounded fast-fail cache.
- Improvement path: Keep key filtering local wherever possible. For backend calls, use strict per-key timeouts, async result delivery, circuit-breaker state, and a visible degraded mode when the engine is unavailable.

**Backend process restart is immediate on read error and disconnects all clients:**
- Problem: `BackendServer::onReadError` calls `restartProcess`, and `terminateProcess` calls `PipeServer::onBackendClosed`, which closes every client using that backend.
- Files: `MoqLauncher/BackendServer.cpp`, `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeClient.cpp`
- Cause: Backend lifecycle and client state are tightly coupled.
- Improvement path: Add backoff, health state, and reconnect semantics. Preserve TSF state when the engine restarts if the active composition can be safely canceled or replayed.

**Candidate window recalculates GDI text metrics and redraws frequently:**
- Problem: `CandidateWindow::recalculateSize` measures each candidate and comment with `GetTextExtentPoint32W`, rebuilds width arrays, resizes the HWND, and reapplies shape on content changes.
- Files: `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/MoqiTextService.cpp`
- Cause: UI layout is directly computed inside the window class without a reusable view model or measured text cache.
- Improvement path: Cache measurements by font/text/comment, separate candidate view data from HWND painting, and add visual regression checks for TypeDuck Web alpha parity.

**Clipboard listener reads and uploads whole clipboard text after a short debounce:**
- Problem: `flushDebouncedClipboardUpload` reads the whole Unicode clipboard and schedules upload after 400 ms when cloud clipboard is enabled.
- Files: `MoqLauncher/PipeServer.cpp`
- Cause: There is no size bound or content classification before backend upload.
- Improvement path: Remove cloud clipboard for TypeDuck or impose maximum payload length, debounce cancellation, and explicit user-triggered sync.

## Fragile Areas

**TSF registration and uninstall write undocumented per-user language profile registry values:**
- Files: `libIME2/src/ImeModule.cpp`, `SetupHelper/SetupHelper.cpp`, `installer/MoqiTsf.iss`
- Why fragile: Registration writes under `HKEY_USERS\<sid>\Control Panel\International\User Profile\<locale>` and loads the default user hive. This is undocumented and can vary by Windows version, user profile state, and admin context.
- Safe modification: Add tests or scripted verification for Windows 10/11 clean install, upgrade, uninstall, and reinstall. Keep TypeDuck profile locale/GUID constants centralized.
- Test coverage: No automated install or registry tests detected.

**Dual-architecture TSF DLL deployment is manual and system-directory dependent:**
- Files: `SetupHelper/SetupHelper.cpp`, `scripts/build.ps1`, `scripts/install.ps1`, `installer/MoqiTsf.iss`
- Why fragile: Win32 and x64 DLLs are copied into `SysWOW64` and `System32`, registered with separate `regsvr32.exe` paths, and sometimes replaced on reboot.
- Safe modification: Preserve separate Win32/x64 build outputs, verify both `regsvr32` registrations, and avoid renaming DLLs without updating setup, installer cleanup, resource metadata, and COM registration together.
- Test coverage: No automated setup-helper tests detected.

**Candidate window ownership/topmost behavior depends on host-specific HWND discovery:**
- Files: `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/MoqiTextService.cpp`
- Why fragile: Owner resolution falls back from TSF context view to `GetFocus` and `GetForegroundWindow`; the window is `WS_EX_TOPMOST | WS_EX_NOACTIVATE`, and ownership is adjusted dynamically.
- Safe modification: Test Notepad, Office, browsers, terminal/console, UWP/immersive hosts, games, elevated apps, multi-monitor DPI, and UI-less TSF hosts after each UI change.
- Test coverage: No UI automation or screenshot-based candidate window tests detected.

**Protocol compatibility crosses C++, generated protobuf, Go package naming, and backend behavior:**
- Files: `proto/moqi.proto`, `proto/moqi.pb.cc`, `proto/moqi.pb.h`, `CMakeLists.txt`, `MoqiTextService/MoqiClient.cpp`, `MoqLauncher/BackendServer.cpp`
- Why fragile: The schema includes Moqi-specific methods and features such as cloud clipboard, tray notifications, UI customization, preserved keys, and Rime deploy command behavior.
- Safe modification: Version the TypeDuck protocol explicitly. Remove unused methods only with coordinated backend/client changes and golden request/response tests.
- Test coverage: No protocol compatibility tests detected.

**Submodules and third-party trees are part of the build surface:**
- Files: `.gitmodules`, `libIME2/`, `libuv/`, `jsoncpp/`, `libIME2/lib/googletest-release-1.10.0/`, `CMakeLists.txt`
- Why fragile: The repo depends on forked or vendored dependencies (`gaboolic/libIME2.git`, `EasyIME/libuv`, bundled jsoncpp and googletest). Local submodule tooling failed in this environment because Git's shell helpers could not find Unix tools.
- Safe modification: Pin submodule commits in CI, document bootstrap requirements, and avoid editing vendored code except behind clearly named patches.
- Test coverage: CI builds release artifacts, but dependency update checks and vulnerability scans are not detected.

**Detached helper thread sends synthetic left-arrow input for quote pairing:**
- Files: `MoqiTextService/MoqiClient.cpp`
- Why fragile: `sendDelayedLeftArrow` detaches a thread, polls foreground window and Shift state, then calls `SendInput` to move the caret. This can race focus changes or conflict with host shortcuts.
- Safe modification: Replace synthetic keyboard input with TSF edit-session cursor manipulation where possible. If retained, guard it behind TypeDuck-specific pairing settings and add host compatibility tests.
- Test coverage: No tests cover quote pairing or caret movement.

## Scaling Limits

**Single launcher and backend instance per user:**
- Current capacity: One `MoqiLauncherMutex` and one `\\.\pipe\<username>\MoqiIM\Launcher` endpoint per user.
- Limit: Multiple TypeDuck channels, side-by-side Moqi/TypeDuck installs, beta/stable installs, or per-profile engines collide unless names are changed.
- Scaling path: Namespace mutexes, pipes, data directories, registry keys, and process names by TypeDuck product/channel.

**Candidate list UI is capped by selection keys and UI element reporting:**
- Current capacity: Default `selKeys_` is `1234567890`; `CandidateWindow::GetCount` reports at most 10 candidates.
- Limit: TypeDuck Web alpha behavior with richer lookup/filter candidates or dictionary entries can exceed 10 visible items.
- Scaling path: Define paging and dictionary lookup UX explicitly, including selection keys, comments, paging commands, and TSF UI element reporting.

**Backend response buffering is unbounded:**
- Current capacity: `FrameBuffer` stores all partial data in a single `std::string`.
- Limit: A declared frame length up to `uint32_t` can retain gigabytes in memory before a complete frame exists.
- Scaling path: Add frame-size caps, incremental parse limits, and per-client/backend buffer quotas.

**Release automation assumes one Windows installer artifact:**
- Current capacity: `.github/workflows/release.yml` and `.github/workflows/nightly.yml` build and upload `installer/dist/moqi-im-windows-setup.exe`.
- Limit: Separate TypeDuck alpha/beta/stable channels, signed installers, symbol artifacts, and architecture-specific packages are not modeled.
- Scaling path: Introduce channel-aware artifact names, code signing, SBOM/dependency manifests, and separate smoke-test jobs before release upload.

## Dependencies at Risk

**`spdlog` is pinned to v1.2.1 via FetchContent:**
- Risk: Very old logging dependency with potential compatibility and security maintenance gaps.
- Impact: Build or runtime logging issues can surface when modernizing CMake/MSVC or enabling stricter warnings.
- Migration plan: Upgrade to a current pinned spdlog release or use a small first-party logging wrapper around Windows ETW/file logging.

**`protobuf` is downloaded during configure/build unless local roots are supplied:**
- Risk: `CMakeLists.txt` fetches protobuf 33.5 from GitHub and CI separately downloads `protoc`; builds depend on network availability and exact upstream artifact availability.
- Impact: Offline development and reproducible builds are fragile.
- Migration plan: Pin a verified protobuf/protoc package in the build environment or vendor via a package manager with checksums.

**`libIME2` is a forked TSF foundation with local behavior changes:**
- Risk: Core TSF registration, COM, text-service, and candidate UI behavior live under `libIME2/`, including undocumented registry writes and Simplified conversion helper code.
- Impact: TypeDuck changes may need TSF framework fixes that are easy to confuse with app-level behavior.
- Migration plan: Keep local patches explicit, add regression tests around modified TSF framework behavior, and document which `libIME2` APIs are stable for TypeDuck.

**`libuv` is sourced from `EasyIME/libuv`:**
- Risk: Launcher IPC and backend process management depend on a forked libuv tree plus a custom named-pipe wrapper.
- Impact: Process/pipe bugs may require maintaining old libuv code rather than using supported Windows APIs directly.
- Migration plan: Reassess whether TypeDuck needs libuv. If the engine boundary is in-process, remove launcher/libuv. If out-of-process, consider modern libuv or first-party Windows overlapped pipe code with tests.

**Vendored Inno Setup Simplified Chinese translation conflicts with TypeDuck locale requirements:**
- Risk: Installer language file is Simplified Chinese and the setup script registers only `chinesesimplified`.
- Impact: TypeDuck must present Traditional Hong Kong Chinese and English, not Simplified Chinese installer chrome.
- Migration plan: Replace with Traditional Chinese Hong Kong and English installer resources, and review every string in `installer/MoqiTsf.iss`.

## Missing Critical Features

**TypeDuck Cantonese engine integration is absent:**
- Problem: No TypeDuck librime fork integration, dictionary lookup filter plugin packaging, or TypeDuck schema assets are present.
- Blocks: The product cannot mirror TypeDuck Web alpha behavior or produce TypeDuck Cantonese candidates.

**Hong Kong Traditional profile installation is not defined:**
- Problem: Profile locale comes from backend `ime.json`; no first-party TypeDuck `zh-HK` profile metadata is present in the repo.
- Blocks: Installation under Chinese (Traditional, Hong Kong) cannot be guaranteed.

**Bilingual Traditional Hong Kong Chinese and English UI copy is absent:**
- Problem: User-facing strings are mostly Moqi Simplified Chinese or English scaffold text.
- Blocks: TypeDuck UI, installer, tray, logs, error messages, and configuration affordances cannot meet the bilingual product requirement.

**TypeDuck visual parity is not encoded:**
- Problem: Candidate window defaults are generic white/blue GDI styling and do not reference TypeDuck Web alpha tokens, spacing, typography, or interaction behavior.
- Blocks: Windows candidate UI cannot be judged against TypeDuck Web alpha without a design contract and visual regression fixtures.

**Dictionary lookup filter UX is not modeled in the protocol or UI:**
- Problem: Current protocol has candidate text/comment fields and generic menu/buttons but no explicit dictionary lookup filter lifecycle, metadata, or interaction contract.
- Blocks: The documented lookup-filter CSV payload has no first-class parser/view-model path, making parity and testing fragile.

## Test Coverage Gaps

**First-party TSF text service behavior is untested:**
- What's not tested: Activation, deactivation, key filtering, composition updates, preserved keys, candidate show/hide, UI-less hosts, async response flushing, and connection recovery.
- Files: `MoqiTextService/MoqiTextService.cpp`, `MoqiTextService/MoqiClient.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`
- Risk: Input regressions ship unnoticed because only manual Windows testing catches them.
- Priority: High

**Launcher/backend IPC is untested:**
- What's not tested: Named-pipe security, frame parsing, request sequencing, malformed frames, backend crash/restart, stderr handling, cloud clipboard uploads, and multi-client routing.
- Files: `MoqLauncher/PipeServer.cpp`, `MoqLauncher/PipeClient.cpp`, `MoqLauncher/BackendServer.cpp`, `proto/ProtoFraming.h`
- Risk: Hangs, spoofing, memory growth, and lost responses ship unnoticed.
- Priority: High

**Installer and registration are untested:**
- What's not tested: Clean install, upgrade, reboot-required replacement, uninstall cleanup, system DLL copy, scheduled re-registration task, and Windows language profile placement.
- Files: `SetupHelper/SetupHelper.cpp`, `installer/MoqiTsf.iss`, `libIME2/src/ImeModule.cpp`, `scripts/install.ps1`
- Risk: Users can end up with broken or orphaned TSF registrations.
- Priority: High

**TypeDuck migration requirements have no automated acceptance tests:**
- What's not tested: `zh-HK` profile registration, Traditional Hong Kong and English strings, absence of Moqi/fcitx/WebDAV clutter, TypeDuck candidate styling, and dictionary lookup filter behavior.
- Files: `README.md`, `installer/MoqiTsf.iss`, `MoqLauncher/PipeServer.cpp`, `MoqiTextService/MoqiCandidateWindow.cpp`, `MoqiTextService/MoqiTextService.rc.in`, `proto/moqi.proto`
- Risk: Legacy scaffold behavior persists under the TypeDuck product.
- Priority: High

**Only low-level COM helper tests are detected:**
- What's not tested: Application code outside `libIME2/test/ComPtr_test.cpp` and `libIME2/test/ComObject_test.cpp`.
- Files: `libIME2/test/ComPtr_test.cpp`, `libIME2/test/ComObject_test.cpp`, `libIME2/test/CMakeLists.txt`
- Risk: Most product behavior lacks automated regression coverage.
- Priority: Medium

---

*Concerns audit: 2026-06-23*
