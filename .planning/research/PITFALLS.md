# Pitfalls Research

**Domain:** Windows TSF Cantonese IME rewrite from Moqi scaffold to TypeDuck
**Researched:** 2026-06-23
**Confidence:** MEDIUM

This file is intentionally roadmap-facing. Each pitfall names the phase that should prevent it and the evidence that phase should collect before it is considered done.

## Critical Pitfalls

### Pitfall 1: Deferring the TypeDuck Engine Boundary

**What goes wrong:**
The roadmap starts with UI, branding, or installer work while the core engine question remains unresolved: whether TypeDuck-HK librime and `rime-dictionary-lookup-filter` run in-process, behind the existing launcher/backend bridge, or behind a new TypeDuck backend package. Later phases then discover that candidate data, lookup metadata, schema assets, deployment, threading, or bitness assumptions do not fit the chosen architecture.

**Why it happens:**
The Moqi scaffold already has a working launcher, protobuf protocol, and backend process path, so it is tempting to reuse that path without first proving that the TypeDuck librime fork and lookup filter can produce all data the Windows UI needs.

**How to avoid:**
Make an engine/runtime spike the first implementation phase. Decide the boundary in writing, then prove a minimal key-to-candidate loop using TypeDuck-HK librime, TypeDuck schemas, and dictionary lookup filter output. Do not let candidate UI or settings phases start until the data contract includes candidate text, Jyutping, translations, part-of-speech, reading notes, more-language rows, paging state, and reverse lookup state.

**Warning signs:**
- `backends.json` still points at `moqi-ime\server.exe`.
- New TypeDuck UI code consumes only `CandidateEntry.text` and generic `comment`.
- Dictionary details are mocked in the frontend instead of coming from the engine/plugin path.
- The roadmap has a "pretty candidate window" phase before an engine/plugin proof phase.

**Phase to address:**
Phase 1: Engine/runtime contract spike.

**Verification evidence to collect:**
- A documented decision: in-process librime, existing launcher bridge, or new TypeDuck backend.
- Build artifact or smoke executable that loads the TypeDuck-HK librime fork and lookup filter.
- Golden engine output fixtures for at least 10 Cantonese inputs, including dictionary lookup fields.
- Failure-mode notes for missing schema, missing plugin, plugin ABI mismatch, and deploy failure.

---

### Pitfall 2: Treating Moqi-to-TypeDuck as a Cosmetic Rename

**What goes wrong:**
The product still leaks Moqi identity through executable names, CLSIDs/AppIds/profile GUID coordination, install directories, log paths, registry cleanup, launcher mutexes, pipe names, tray text, installer labels, release artifacts, generated protocol package names, and docs.

**Why it happens:**
Branding is distributed across CMake, resources, installer scripts, TSF registration code, launcher code, protocol names, CI, and runtime payload metadata. One-off string replacement misses stateful Windows identity surfaces.

**How to avoid:**
Create a TypeDuck identity contract before editing names. It should list product name, binary names, DLL names, CLSID, profile GUID, install path, app data paths, registry keys, mutex/pipe names, startup entry, log paths, icon resources, installer artifact names, and protocol package naming. Apply it through constants or configured definitions, then audit for banned strings.

**Warning signs:**
- A branch changes `README.md` and installer title but leaves `MoqiTextService.dll`, `%LOCALAPPDATA%\MoqiIM`, or `\\.\pipe\<user>\MoqiIM\Launcher`.
- Old Moqi registry cleanup remains but TypeDuck cleanup is not defined.
- Side-by-side Moqi/TypeDuck installs would share a mutex, pipe, log path, or startup entry.

**Phase to address:**
Phase 0: Product identity and scaffold cleanup contract.

**Verification evidence to collect:**
- A checked-in identity contract document or constants module.
- `rg` audit results proving user-visible Moqi/fcitx/WebDAV/AI strings are removed or intentionally internal.
- Install/uninstall registry and filesystem snapshots showing TypeDuck-specific paths.
- Release artifact names and CI outputs using TypeDuck naming.

---

### Pitfall 3: Letting Backend `ime.json` Own zh-HK TSF Registration

**What goes wrong:**
TypeDuck fails to install under Chinese (Traditional, Hong Kong), registers under Simplified Chinese, registers no profile when backend metadata is missing, or leaves orphaned language profiles after uninstall/reinstall.

**Why it happens:**
The scaffold discovers language profile metadata from installed backend `input_methods/*/ime.json` at `DllRegisterServer` time. That makes a core product requirement depend on runtime payload shape and JSON correctness instead of first-party TSF registration code.

**How to avoid:**
Make the TypeDuck Hong Kong Traditional profile a first-party deterministic registration resource. Backend metadata may configure schemas, but TSF profile identity must be compiled or staged as part of the TypeDuck product contract: text service CLSID, profile GUID, locale/LANGID mapping for zh-HK, bilingual display names, icon, and uninstall cleanup.

**Warning signs:**
- Registration tests pass only when backend folders are present.
- `ime.json` is the only source of `zh-HK`, display name, or profile GUID.
- Installer copy changes happen without a profile registration verification script.
- Windows language settings show Chinese (Simplified) or a Moqi display name.

**Phase to address:**
Phase 2: TSF profile registration and installer skeleton.

**Verification evidence to collect:**
- Clean Windows 10/11 install screenshot or scripted output showing TypeDuck under Chinese (Traditional, Hong Kong).
- `regsvr32`/SetupHelper logs for both x86 and x64 DLL registration.
- Registry/export snapshots before install, after install, after uninstall, and after reinstall.
- Negative test proving missing backend runtime does not prevent profile registration diagnostics.

---

### Pitfall 4: Building Candidate UI Without TSF Host Compatibility Tests

**What goes wrong:**
The candidate/dictionary window looks correct in a preview tool but is misplaced, clipped, behind the host window, wrong on high DPI, broken in UI-less contexts, or unstable in Office, browsers, terminal, UWP/immersive apps, elevated apps, or multi-monitor setups.

**Why it happens:**
Candidate placement depends on TSF context rectangles, owner HWND discovery, focus/foreground windows, topmost/no-activate behavior, and `ITfCandidateListUIElement` state. The scaffold already has FIXME-style fallback placement; TypeDuck's richer panel increases size and positioning pressure.

**How to avoid:**
Split candidate data from candidate rendering, then verify the actual TSF window in host apps. Keep a standalone preview for fast iteration, but do not accept UI phases without in-host screenshots and `ITfCandidateListUIElement` behavior checks.

**Warning signs:**
- Candidate UI is only tested in `Preview/main.cpp`.
- Rich dictionary rows are added inside `MoqiCandidateWindow.cpp` without a view model.
- No test matrix includes DPI scaling, multiple monitors, UI-less hosts, or browser/Office targets.
- Candidate count/page-size assumptions remain capped at the old `1234567890` model without TypeDuck parity review.

**Phase to address:**
Phase 4: Candidate and settings UI parity.

**Verification evidence to collect:**
- Screenshot matrix for Notepad, Word/Office, Chrome/Edge, terminal/console, UWP/immersive host, elevated app, and multi-monitor DPI.
- Automated or scripted checks for candidate selection, paging, hide/show, composition movement, and focus changes.
- Visual parity comparison against TypeDuck Web alpha screenshots.
- Accessibility/TSF UI element checks for selection/count consistency.

---

### Pitfall 5: Web Alpha Parity Is Remembered, Not Encoded

**What goes wrong:**
The Windows IME ships with settings or candidate behavior that "feels close" but diverges from TypeDuck Web alpha: missing Display Languages priority, wrong page size behavior, wrong Jyutping visibility, missing Cangjie v3/v5, missing input memory, different auto-completion/correction/composition semantics, wrong Chinese typeface, or incomplete reverse lookup.

**Why it happens:**
Web alpha is treated as a reference someone looked at once, not as an acceptance fixture with versioned screenshots, setting inventory, expected behaviors, copy, and candidate examples.

**How to avoid:**
Create a Web parity contract early. Capture settings, defaults, bilingual labels, candidate panel states, dictionary examples, visual tokens, and interaction notes from the alpha. Each UI/engine phase should update or consume the contract. Re-check the alpha before locking release behavior.

**Warning signs:**
- Requirements say "match Web alpha" but no fixture names or screenshots are referenced in phase plans.
- Settings dialog implementation starts before defaults and labels are enumerated.
- Native UI adds extra scaffold customization not present in Web alpha.
- Candidate tests do not include translation, part-of-speech, reading notes, and More Languages examples.

**Phase to address:**
Phase 0 for parity contract, then Phase 4 for UI implementation.

**Verification evidence to collect:**
- Versioned Web alpha screenshots and setting inventory dated with capture time.
- Default settings table with expected storage keys and UI labels.
- Candidate/dictionary fixture pack with rendered Windows screenshots next to Web alpha examples.
- Final pre-release parity audit confirming the alpha has not changed materially.

---

### Pitfall 6: Treating Installer and Registration as End-of-Project Packaging

**What goes wrong:**
The IME works for developers but fails for users: wrong DLL bitness registered, old DLL locked until reboot, orphaned registry entries, broken upgrade/uninstall, missing startup launcher, forced process kill affects the wrong app, or settings dialog does not appear during installation.

**Why it happens:**
Windows IME installation is product functionality, not packaging polish. The scaffold copies TSF DLLs into system directories, registers both x86/x64 components, uses reboot fallback, writes startup entries, and currently force-kills the Moqi launcher.

**How to avoid:**
Make installer verification a vertical phase soon after profile registration. Keep SetupHelper behavior isolated, TypeDuck-named, logged, and testable. Define clean install, upgrade, reinstall, uninstall, reboot-required replacement, and non-admin failure cases as acceptance tests.

**Warning signs:**
- Installer work is scheduled after all engine/UI work.
- `taskkill /F /IM MoqiLauncher.exe` survives TypeDuck rename.
- Only a developer machine install is tested.
- No logs are captured from failed `regsvr32` or SetupHelper runs.

**Phase to address:**
Phase 2 for installer skeleton, Phase 6 for installer hardening and release verification.

**Verification evidence to collect:**
- Clean VM install/upgrade/uninstall transcripts.
- x86 and x64 `regsvr32` registration evidence.
- Reboot-required replacement test.
- Settings dialog launch evidence during install with language picker first.
- Uninstall cleanup diff proving no TypeDuck or Moqi residue from the install remains except intentional user data.

---

### Pitfall 7: Shipping Hidden Scaffold Features

**What goes wrong:**
Users see or trigger fcitx references, cloud clipboard/WebDAV, AI controls, Moqi menus, Simplified Chinese installer/profile defaults, or excessive customization that TypeDuck did not ask for. Worse, cloud clipboard may still read clipboard text when config says enabled.

**Why it happens:**
Legacy scaffold behavior exists in docs, launcher code, protocol fields, backend routing, and configuration paths. Hiding menu items is not enough if background code or config readers remain active.

**How to avoid:**
Remove or compile-gate non-TypeDuck features at the source boundary. For privacy-sensitive features such as cloud clipboard, delete the listener/protocol path unless explicitly re-scoped with consent and length limits. Add banned-surface audits to every phase.

**Warning signs:**
- `METHOD_CLOUD_CLIPBOARD_UPLOAD` remains reachable.
- `%APPDATA%\Moqi` or `%LOCALAPPDATA%\MoqiIM` is still read for TypeDuck runtime behavior.
- README or installer text still mentions fcitx, WebDAV, AI, or Moqi.
- Protocol cleanup is deferred because "the UI does not expose it."

**Phase to address:**
Phase 0 for source cleanup policy, Phase 5 for privacy/security hardening.

**Verification evidence to collect:**
- Static audit for `Moqi`, `fcitx`, `WebDAV`, `cloud_clipboard`, `AI`, Simplified-only strings, and legacy config paths.
- Runtime test proving clipboard listener is absent or disabled unconditionally for TypeDuck.
- UI walkthrough showing no unused scaffold menu/settings/tray surfaces.
- Protocol review showing removed or unreachable legacy methods.

---

### Pitfall 8: Leaving IPC Framing and Key-Path Blocking Unbounded

**What goes wrong:**
Malformed or oversized frames cause memory growth, stale async responses stall synchronous calls, backend restart disconnects all clients, or per-key named-pipe round trips make typing laggy. Cantonese input then feels unreliable even if the engine is correct.

**Why it happens:**
The scaffold synchronously calls the backend from key handling and `FrameBuffer` trusts a 32-bit frame length without a maximum payload. The current protocol also mixes product features, tray notifications, cloud clipboard, Rime deployment, and key handling in one surface.

**How to avoid:**
Add frame-size caps, protocol versioning, bounded per-key timeouts, sequencing tests, backend crash/restart tests, and local fast-path handling where possible. Keep TypeDuck dictionary payloads bounded and paged.

**Warning signs:**
- `Proto::FrameBuffer` still accepts arbitrary declared lengths.
- Dictionary payloads can include unbounded translations or language rows.
- Key handlers call blocking RPC without strict timeout and degraded-mode behavior.
- Backend crash tests close every client without preserving or clearly canceling composition.

**Phase to address:**
Phase 3 for protocol hardening, Phase 6 for stress and release verification.

**Verification evidence to collect:**
- Unit tests for max frame size, malformed length prefix, stale sequence numbers, and async response ordering.
- Typing latency measurements for printable keys and Backspace under normal and backend-restarting conditions.
- Crash/restart test logs showing bounded recovery behavior.
- Payload-size limits for candidate/dictionary responses.

---

### Pitfall 9: Keeping Tests at the Existing COM Helper Level

**What goes wrong:**
The rewrite appears complete but has no regression coverage for TypeDuck behavior: engine integration, dictionary lookup fields, zh-HK registration, bilingual UI strings, candidate placement, installer cleanup, protocol compatibility, and scaffold leakage are only manually checked.

**Why it happens:**
The repository already has GoogleTest wired for low-level COM helpers, so CI can appear to have tests while product behavior remains uncovered.

**How to avoid:**
Add TypeDuck-owned test layers as soon as the engine/protocol boundaries are defined. Unit-test non-UI mapping logic first, then add installer/registration scripts and host-app smoke tests. CI should run CTest plus packaging smoke tests; manual UAT should be explicit for true TSF host coverage.

**Warning signs:**
- CI builds installers but never runs CTest or install smoke tests.
- TypeDuck candidate behavior is tested only by ad hoc typing.
- No tests assert absence of Moqi/fcitx/WebDAV/AI surfaces.
- Phase plans treat tests as a final cleanup task.

**Phase to address:**
Every phase, with a dedicated Phase 6 compatibility/release verification pass.

**Verification evidence to collect:**
- New product-level test CMake subtree or equivalent.
- CI output showing TypeDuck unit/protocol tests and CTest execution.
- Installer/registration verification scripts checked in.
- Manual UAT matrix with screenshots/logs for TSF host compatibility.

---

### Pitfall 10: Underestimating the Older TypeDuck-HK librime Fork

**What goes wrong:**
The project assumes modern librime APIs, build flags, plugin ABI, or dependency versions that the TypeDuck-HK fork does not support. The Windows build then fails late, or the lookup filter compiles but is not loaded at runtime.

**Why it happens:**
The scaffold already has modern-looking CMake/protobuf/vendor assumptions, while the required TypeDuck dependencies may be older and forked. Rime plugin integration depends on binary compatibility, deployment paths, module loading, schema configuration, and runtime data directories.

**How to avoid:**
Pin exact TypeDuck-HK dependency commits and build them in CI before product UI work. Document the supported librime ABI, plugin build method, dynamic library names, deployment directories, and schema configuration required for the lookup filter.

**Warning signs:**
- Dependency versions are described as "latest" instead of commit SHAs.
- The plugin is added as a submodule but no runtime load proof exists.
- Windows CI does not build the fork and plugin from clean checkout.
- The UI is built against mocked dictionary fields before plugin output is verified.

**Phase to address:**
Phase 1: Engine/runtime contract spike.

**Verification evidence to collect:**
- Submodule or dependency lockfile with exact commits.
- CI build logs for librime fork and dictionary lookup filter.
- Runtime log proving plugin load and schema deployment.
- Golden output proving lookup metadata came from the plugin, not fixtures.

---

### Pitfall 11: Logging and Diagnostics Leak Typed Content

**What goes wrong:**
Debug logs, crash traces, or installer/runtime diagnostics capture candidate content, composition text, process paths, clipboard text, or other sensitive user context under TypeDuck-branded logs.

**Why it happens:**
IME code runs inside arbitrary host processes and handles text as the product's core data. The scaffold already logs process paths, focus/composition/candidate state, and has a cloud clipboard path that can read arbitrary clipboard text.

**How to avoid:**
Make logging opt-in, bounded, rotated, and scrubbed by default. Separate operational diagnostics from typed content. Remove cloud clipboard. Ensure any debug mode warns users and uses TypeDuck-specific paths.

**Warning signs:**
- Candidate strings or preedit text appear in default logs.
- Logs remain under Moqi paths or mixed TypeDuck/Moqi paths.
- Clipboard listener code remains compiled.
- Debug logging can be enabled by backend config without visible user consent.

**Phase to address:**
Phase 5: Privacy and security hardening.

**Verification evidence to collect:**
- Log review from a typing session showing no raw typed content in default mode.
- Static audit of logging calls around key/composition/candidate paths.
- Config review proving debug logging is opt-in and TypeDuck-named.
- Runtime proof that clipboard upload/listener code is absent or inert.

## Technical Debt Patterns

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Reusing Moqi names internally | Fewer rename edits | Broken side-by-side installs, wrong registry cleanup, confusing logs, product leakage | Only in untouched third-party history, never in product-owned runtime surfaces |
| Keeping backend `ime.json` as profile source of truth | Faster registration reuse | zh-HK install depends on runtime payload and can silently fail | Never for TypeDuck profile identity |
| Building UI directly in large legacy files | Fast first visual output | Hard-to-test rendering, protocol, and TSF state entanglement | Accept only for small adapter glue; new logic needs view models/helpers |
| Deferring installer tests | More time for engine/UI | Users cannot install, upgrade, or uninstall reliably | Never after Phase 2 |
| Keeping old protocol fields "unused" | Avoids backend/client coordination | Privacy risk and accidental feature resurrection | Only if compile-gated and verified unreachable |

## Integration Gotchas

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| TypeDuck-HK librime fork | Assume upstream librime docs/API/build behavior applies exactly | Pin fork commit, build on Windows CI, document ABI and deployment paths |
| `rime-dictionary-lookup-filter` | Package DLL but never prove plugin load or schema wiring | Add runtime plugin-load smoke test and golden lookup metadata output |
| TSF profile registration | Let backend JSON define profile identity | First-party TypeDuck zh-HK profile constants with installer verification |
| Existing launcher/backend bridge | Keep Moqi backend names and methods while adding TypeDuck data | Version the TypeDuck protocol and remove or gate legacy methods |
| Inno Setup/SetupHelper | Rename installer text but leave Moqi process kills/system DLL names | TypeDuck-named setup flow with clean install/upgrade/uninstall tests |
| TypeDuck Web alpha | Treat screenshots as inspiration | Treat alpha as source-of-truth fixtures with dated setting and candidate captures |

## Performance Traps

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| Synchronous per-key backend RPC | Laggy Backspace, dropped keys, frozen composition | Local fast paths, strict timeouts, circuit breaker, latency tests | Any slow/crashed backend or high-frequency deletion |
| Rich dictionary panel measured entirely in GDI on every update | Flicker, large candidate window jank, bad DPI behavior | View model, text measurement cache, bounded rows, visual smoke tests | Long translations, many languages, high DPI |
| Unbounded protobuf frame buffering | Memory growth or process instability | Max frame size, per-client quotas, malformed-frame tests | Broken or malicious backend/client |
| Backend restart closes all clients | Composition loss across apps | Backoff, health state, clear cancellation/replay behavior | Engine crash, deploy, schema reload |
| Logging too much on key path | Typing latency and privacy exposure | Default low-volume logs, scrubbed diagnostics, opt-in debug | Debug builds or trace mode in real host apps |

## Security Mistakes

| Mistake | Risk | Prevention |
|---------|------|------------|
| Broad named-pipe ACLs and no server identity validation | Same-user spoofing or unintended app-container access | Minimum pipe rights, bounded pipe names, client/server identity checks where practical |
| No frame-size limit | Memory exhaustion from malformed frames | Reject oversized frames before buffering |
| Backend-provided config tool launch | Arbitrary executable launch from payload metadata | Restrict to signed first-party TypeDuck executable under install dir or remove |
| Force-killing by legacy image name | Kills wrong process or misses TypeDuck process | TypeDuck-specific graceful shutdown path, then scoped fallback |
| Cloud clipboard retained | Uploads arbitrary clipboard text outside TypeDuck scope | Remove feature; if ever reintroduced, require explicit consent and limits |
| Default logs include typed content | Sensitive text leaks to disk | Scrub composition/candidate text by default and rotate logs |

## UX Pitfalls

| Pitfall | User Impact | Better Approach |
|---------|-------------|-----------------|
| Installing under Simplified Chinese | Hong Kong users cannot find/trust the IME profile | Register as Chinese (Traditional, Hong Kong) / zh-HK |
| English-only or Simplified-only UI | Product misses bilingual HK requirement | Every user-facing string has Traditional HK Chinese and English coverage |
| Extra Moqi/fcitx/WebDAV/AI controls | Confusing, untrusted product surface | Remove unused scaffold controls, keep Web alpha settings only |
| Candidate panel lacks dictionary structure | TypeDuck feels less capable than Web alpha | Render structured lookup fields with translations, POS, notes, and more-language rows |
| Settings order differs from Web alpha | Users cannot map Web behavior to Windows | Put Display Languages first and preserve Web alpha setting set/defaults |
| Candidate window optimized only for compact lists | Large dictionary panel overlaps text or disappears | Host-app placement matrix, DPI checks, bounded panel layout |

## "Looks Done But Isn't" Checklist

- [ ] **Engine integration:** A mocked candidate list is not enough; verify real TypeDuck-HK librime plus lookup filter output.
- [ ] **Dictionary UI:** Rendered translations are not enough; verify structured data fields and More Languages rows.
- [ ] **zh-HK registration:** Installer success is not enough; verify Windows language profile placement and display names.
- [ ] **Brand migration:** Installer title is not enough; audit binaries, paths, registry, pipes, mutexes, logs, CI artifacts, and docs.
- [ ] **Candidate window:** Preview app is not enough; verify real TSF host apps, DPI, focus, and UI-less contexts.
- [ ] **Settings dialog:** Controls are not enough; verify Web alpha defaults, order, bilingual copy, and persistence.
- [ ] **Installer:** A local developer install is not enough; verify clean install, upgrade, uninstall, reinstall, and reboot replacement.
- [ ] **Security cleanup:** Hidden UI is not enough; verify cloud clipboard, AI, WebDAV, and arbitrary config-tool paths are unreachable.
- [ ] **Testing:** Existing COM helper tests are not enough; add TypeDuck protocol, engine, installer, and parity tests.
- [ ] **Release:** A built EXE is not enough; verify signed/channel-ready artifacts, dependency pins, and fresh VM smoke tests.

## Recovery Strategies

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Wrong engine boundary chosen late | HIGH | Freeze UI work, define adapter boundary, preserve golden candidate fixtures, migrate protocol/view model before adding features |
| Moqi identity leaked after partial rename | MEDIUM | Build identity inventory, run static audit, update installer/registry/pipe/log surfaces together, test side-by-side behavior |
| zh-HK registration missing or wrong | HIGH | Move profile metadata to first-party constants, regenerate GUID/locale contract if needed, rebuild installer, clean orphaned registrations |
| Lookup-filter payload treated as opaque display text | HIGH | Preserve the documented CSV payload, parse it into fields, and update UI/tests to consume those fields |
| Candidate window fails in host apps | MEDIUM | Add host matrix, isolate placement code, add fallback positioning and screenshot evidence |
| Installer fails upgrade/uninstall | HIGH | Reproduce in clean VM, capture registry/filesystem diff, fix SetupHelper idempotency, retest reboot path |
| Legacy cloud/AI/WebDAV remains reachable | MEDIUM | Remove protocol handling and runtime listeners, audit config paths, add regression tests |
| IPC hangs or memory growth appears | MEDIUM | Add frame caps/timeouts, fuzz malformed frames, add backend health/degraded state |

## Pitfall-to-Phase Mapping

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Cosmetic Moqi rename | Phase 0: Identity and parity contract | Identity contract plus banned-string/path audit |
| Web alpha parity remembered, not encoded | Phase 0: Identity and parity contract | Dated Web alpha fixture pack and settings inventory |
| Deferred engine boundary | Phase 1: Engine/runtime contract spike | Real librime/plugin smoke output and boundary decision |
| Older librime fork underestimated | Phase 1: Engine/runtime contract spike | Pinned commits, CI build, runtime plugin-load proof |
| Backend JSON owns zh-HK registration | Phase 2: TSF registration and installer skeleton | Clean VM zh-HK profile registration evidence |
| Installer treated as final packaging | Phase 2 and Phase 6 | Install/upgrade/uninstall/reboot transcripts |
| Lookup-filter payload mishandled | Phase 3: Protocol and candidate model | Payload preservation fixtures, parser tests, and view-model mapper tests |
| IPC/key path remains fragile | Phase 3 and Phase 6 | Frame/sequence tests and typing latency measurements |
| Candidate UI lacks TSF host testing | Phase 4: Candidate/settings UI parity | Host app screenshot matrix and UI element checks |
| Hidden scaffold features ship | Phase 5: Privacy/security cleanup | Static/runtime audits for removed legacy surfaces |
| Logs leak typed content | Phase 5: Privacy/security cleanup | Log review showing no raw typed content by default |
| Product behavior untested | All phases, final Phase 6 | CI/test artifacts and manual UAT evidence |

## Roadmap Prevention Summary

Recommended phase order:

1. **Identity and Web parity contract** - lock TypeDuck identity, banned scaffold surfaces, Web alpha settings, candidate examples, and bilingual copy rules before code churn.
2. **Engine/runtime contract spike** - prove TypeDuck-HK librime and dictionary lookup filter output on Windows before UI depends on it.
3. **TSF zh-HK registration and installer skeleton** - make the IME installable under the correct Windows language profile early.
4. **Protocol and candidate/dictionary model** - create typed data contracts before rendering the rich dictionary UI.
5. **Candidate/settings UI parity** - build visible UX against fixtures and host-app compatibility evidence.
6. **Privacy/security cleanup** - remove legacy cloud/AI/WebDAV/fcitx surfaces and scrub logs/config paths.
7. **Compatibility and release verification** - run installer, TSF host, protocol, latency, DPI, and cleanup checks on fresh Windows environments.

Do not reorder engine/runtime and zh-HK registration behind visual polish. The rewrite can look complete while still being unshippable if those foundations are unresolved.

## Sources

- `.planning/PROJECT.md` - TypeDuck product target, Web alpha parity, zh-HK requirement, TypeDuck-HK librime/plugin constraints.
- `.planning/codebase/CONCERNS.md` - existing scaffold risks, missing TypeDuck features, security/performance/test gaps.
- `.planning/codebase/ARCHITECTURE.md` - TSF/launcher/backend boundaries, registration path, candidate UI architecture.
- `.planning/codebase/TESTING.md` - current test coverage and TypeDuck-specific testing guidance.
- `.planning/codebase/INTEGRATIONS.md` - external integrations, launcher/backend, installer, cloud/AI legacy surfaces.
- Microsoft Learn: `ITfInputProcessorProfileMgr::RegisterProfile` and TSF input processor profile registration.
- Microsoft Learn: `ITfCandidateListUIElement` candidate UI element contract.
- Microsoft Learn: `regsvr32` command and Windows COM DLL registration behavior.
- Rime/librime upstream repository and TypeDuck-HK dependency context supplied by project requirements.
- Research cache entries from 2026-06-23 using the GSD research-plan seam, provider `brave`, verified confidence `MEDIUM`.

---
*Pitfalls research for: TypeDuck Windows IME*
*Researched: 2026-06-23*
