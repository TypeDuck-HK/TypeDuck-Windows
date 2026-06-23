# Phase 1: Identity and Web Parity Contract - Research

**Researched:** 2026-06-23  
**Domain:** TypeDuck product identity contract, Windows TSF identity metadata, and Web alpha parity fixtures  
**Confidence:** HIGH for project/codebase scope, MEDIUM for external Windows TSF references

<user_constraints>
## User Constraints (from CONTEXT.md)

All content in this section is copied from `.planning/phases/01-identity-and-web-parity-contract/01-CONTEXT.md`. [VERIFIED: 01-CONTEXT.md]

### Locked Decisions

## Implementation Decisions

### Identity Contract Shape
- **D-01:** Create a durable product contract at `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md`.
- **D-02:** The contract must cover executable names, DLL names, AppId, CLSID, profile GUIDs, install directories, log/data directories, pipe and mutex names, registry keys, resource strings, installer/release artifact names, and user-visible display names.
- **D-03:** Every identity entry must include current Moqi scaffold value when discoverable, target TypeDuck value, ownership/status (`locked`, `proposed`, or `deferred`), affected files, and verification notes.
- **D-04:** The contract is the authority for later implementation phases; downstream agents should not invent new product names, paths, GUIDs, or registry locations outside this contract without updating it.

### TypeDuck-Owned Identifier Strategy
- **D-05:** Prefer new deterministic TypeDuck-owned CLSID/profile GUID/AppId values instead of preserving Moqi identifiers. Moqi identifiers are scaffold residue unless explicitly retained for migration compatibility.
- **D-06:** Keep TypeDuck identity names channel-ready where cheap, but do not expand phase scope into beta/stable side-by-side support. Future channel support can extend the contract later.
- **D-07:** The TSF profile target must be Chinese (Traditional, Hong Kong) / `zh-HK`, with bilingual Traditional Hong Kong Chinese and English display text.

### Web Alpha Parity Fixtures
- **D-08:** Create a dated fixture set at `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` plus any small supporting assets under `.planning/product/web-alpha-fixtures/2026-06-23/`.
- **D-09:** Fixtures must be source-backed, not memory-backed. Capture the alpha URL, local TypeDuck Web source path, source commit/hash when available, capture date, browser/viewport details for screenshots, and any access limitations.
- **D-10:** The fixture set must cover settings order, settings labels, defaults/options, display-language behavior, candidate list presentation, dictionary/detail panel data, visual references, and known Web alpha gaps or unsupported Windows mappings.
- **D-11:** If `http://localhost:5173/TypeDuck-Web/aap2-alpha/` or `I:\GitHub\TypeDuck-Web` is unavailable during execution, the fixture artifact must record that as blocked/partial instead of fabricating parity data.

### Banned Scaffold Surfaces
- **D-12:** Create a negative contract at `.planning/product/TYPEDUCK-BANNED-SURFACES.md`.
- **D-13:** The banned list must include visible Moqi branding, fcitx references, WebDAV/cloud clipboard, AI controls, Simplified-only Chinese copy, arbitrary backend-declared config tools, excessive customization beyond Web alpha, and legacy Moqi backend fallback behavior.
- **D-14:** The banned-surface artifact should include audit patterns and likely files for later verification, but Phase 1 should not attempt the full cleanup. Cleanup belongs mainly to Phase 6.

### Phase Boundary and Handoff
- **D-15:** Phase 1 should be documentation and fixture capture only, with small helper scripts allowed only if they make the contract or fixture capture reproducible.
- **D-16:** Do not rename binaries, registry keys, protocol packages, installer files, or runtime paths in production code during Phase 1 unless the plan explicitly proves it is only creating documentation or fixture support. Implementation happens in later mapped phases.
- **D-17:** Downstream planning must treat IDEN-02 and VER-01 as the only requirements this phase completes.

### the agent's Discretion
Auto mode selected the recommended defaults without user prompts. The agent may choose exact markdown table layouts, screenshot filenames, and fixture JSON shapes, provided the artifacts remain human-readable, source-backed, and easy for later phases to reference.

### Deferred Ideas (OUT OF SCOPE)

- Applying TypeDuck identity across binaries, registry keys, installer strings, paths, CI artifact names, and visible UI belongs to later implementation phases.
- Implementing Web alpha candidate/settings/About UI parity belongs to Phase 5.
- Automated banned-string verification belongs mainly to Phase 6, though Phase 1 may document the audit patterns.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| IDEN-02 | Developer has a single TypeDuck identity contract covering executable names, DLL names, AppId/CLSID/profile GUIDs, install directories, log/data directories, pipe/mutex names, registry keys, resource strings, and release artifact names. [VERIFIED: .planning/REQUIREMENTS.md] | Use a central markdown table with current scaffold value, target TypeDuck value, status, affected files, and verification notes; populate from codebase grep and codebase maps. [VERIFIED: 01-CONTEXT.md; VERIFIED: codebase grep] |
| VER-01 | Developer has a dated TypeDuck Web alpha parity fixture set covering settings defaults, labels, candidate list, dictionary panel, and visual references. [VERIFIED: .planning/REQUIREMENTS.md] | Capture runtime URL availability, source path, source commit/hash, dirty status, settings labels/defaults from Web source, and screenshots with browser/viewport metadata. [VERIFIED: 01-CONTEXT.md; VERIFIED: TypeDuck-Web source grep] |
</phase_requirements>

## Summary

Phase 1 should create three durable planning/product artifacts and no broad production renames: `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md`, `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md`, and `.planning/product/TYPEDUCK-BANNED-SURFACES.md`. [VERIFIED: 01-CONTEXT.md] The identity contract must inventory current Moqi scaffold values and proposed or locked TypeDuck replacements across binaries, TSF/COM identifiers, installer metadata, runtime storage, IPC names, registry keys, resources, and release artifacts. [VERIFIED: 01-CONTEXT.md; VERIFIED: codebase grep]

The Web alpha is currently reachable at `http://localhost:5173/TypeDuck-Web/aap2-alpha/` with HTTP 200, and the local source exists at `I:\GitHub\TypeDuck-Web` at commit `db21054`; the source worktree is dirty and includes modified files plus submodule changes, so fixture evidence must record that provenance. [VERIFIED: Invoke-WebRequest; VERIFIED: TypeDuck-Web git status] The Web source confirms settings labels/order in `src/Preferences.tsx`, defaults in `src/consts.ts`, candidate list presentation in `src/Candidate.tsx`, dictionary panel structure in `src/DictionaryPanel.tsx`, and visual tokens/font stacks in `tailwind.config.ts` and `src/index.css`. [VERIFIED: TypeDuck-Web source grep]

Microsoft TSF registration planning must preserve the distinction between COM in-process server identity, TSF text service registration, language profile GUID/display/icon metadata, and category registration. [CITED: https://learn.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfinputprocessorprofiles-register; CITED: https://learn.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfinputprocessorprofiles-addlanguageprofile; CITED: https://learn.microsoft.com/en-us/windows/win32/api/msctf/nn-msctf-itfcategorymgr] Windows locale names follow BCP 47-style language-region patterns such as `<language>-<REGION>`, and this project has locked the target profile to `zh-HK`. [CITED: https://learn.microsoft.com/en-us/windows/win32/intl/locale-names; VERIFIED: 01-CONTEXT.md]

**Primary recommendation:** Plan Phase 1 as artifact-first evidence capture: generate the identity contract, Web alpha fixture set, and banned-surface contract from verifiable sources, with optional helper scripts only for reproducible capture/checks. [VERIFIED: 01-CONTEXT.md]

## Project Constraints (from AGENTS.md)

- TypeDuck Web alpha is the product source of truth until official release; do not treat Moqi behavior as product truth. [VERIFIED: AGENTS.md]
- Every user-facing string must be bilingual Traditional Hong Kong Chinese and English. [VERIFIED: AGENTS.md]
- TSF profile registration must target Chinese (Traditional, Hong Kong) / `zh-HK`. [VERIFIED: AGENTS.md]
- Use the TypeDuck-HK librime fork and `rime-dictionary-lookup-filter` in later engine/runtime phases; Phase 1 should not invent incompatible identity or fixture assumptions. [VERIFIED: AGENTS.md]
- Divergence from Moqi is expected; preserve useful Windows TSF/COM/build architecture, not Moqi product choices. [VERIFIED: AGENTS.md]
- Installer settings must be shown during installation, with language picker prioritized; Phase 1 should encode this as future fixture/contract evidence, not implement the installer flow. [VERIFIED: AGENTS.md; VERIFIED: 01-CONTEXT.md]
- Remove unused/excessive Moqi/fcitx/cloud/AI surfaces from the TypeDuck product; Phase 1 documents banned surfaces while Phase 6 performs most cleanup. [VERIFIED: AGENTS.md; VERIFIED: 01-CONTEXT.md]
- Respect Windows TSF/COM bitness, installer registration, and host-process behavior in downstream plans. [VERIFIED: AGENTS.md]
- New TypeDuck-owned C++ should follow the newer 2-space style used by `SetupHelper/SetupHelper.cpp` and `proto/ProtoFraming.h`; generated protobuf files must not be hand-edited. [VERIFIED: AGENTS.md]
- GSD workflow says not to make direct repo edits outside a GSD workflow; this research write is the requested GSD artifact, and production code should remain untouched in Phase 1. [VERIFIED: AGENTS.md; VERIFIED: 01-CONTEXT.md]

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|--------------|----------------|-----------|
| Product identity contract | Planning/Product Docs | Windows TSF/Installer | The contract is the authority later phases consume before changing code, installer, registry, and release names. [VERIFIED: 01-CONTEXT.md] |
| TSF CLSID/profile/AppId naming | Windows TSF/Installer | Planning/Product Docs | Later implementation must coordinate `MoqiTextService`, `libIME2`, `SetupHelper`, and Inno metadata; Phase 1 only records deterministic targets. [VERIFIED: .planning/codebase/INTEGRATIONS.md; VERIFIED: codebase grep] |
| Web alpha parity fixtures | Planning/Product Docs | Local Web Runtime/Source | The fixture set must capture runtime/source evidence before native Windows UI work starts. [VERIFIED: 01-CONTEXT.md; VERIFIED: TypeDuck-Web source grep] |
| Banned scaffold surfaces | Planning/Product Docs | Launcher/Protocol/Installer/Docs | Phase 1 defines negative rules and audit patterns; Phase 6 removes or gates most surfaces. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/REQUIREMENTS.md] |
| Fixture capture helper scripts | Local tooling | Planning/Product Docs | Small scripts are allowed only when they make evidence capture/checks reproducible. [VERIFIED: 01-CONTEXT.md] |

## Standard Stack

### Core

| Library/Tool | Version | Purpose | Why Standard |
|--------------|---------|---------|--------------|
| Markdown planning artifacts | N/A | Canonical contracts and fixtures under `.planning/product/`. | The phase scope is documentation and fixture capture only. [VERIFIED: 01-CONTEXT.md] |
| PowerShell | Windows PowerShell present; PowerShell 7.5.4 also present | Reproducible local availability checks and optional artifact validation scripts. | Existing build/install automation uses PowerShell and the project is Windows-native. [VERIFIED: environment probe; VERIFIED: AGENTS.md] |
| `rg` | Available by use in this session | Surface discovery for Moqi/fcitx/WebDAV/cloud/AI/Simplified-only patterns. | Fast repo grep is the repo-recommended search path for codebase research. [VERIFIED: codebase grep] |
| Git | 2.47.1.windows.1 | Source commit/hash and dirty-worktree provenance for this repo and TypeDuck Web source. | Fixture artifacts must record source commit/hash when available. [VERIFIED: environment probe; VERIFIED: 01-CONTEXT.md] |
| TypeDuck Web local source | commit `db21054`, dirty worktree | Source-backed settings/candidate/dictionary/visual fixture evidence. | The project defines TypeDuck Web alpha/source as product truth. [VERIFIED: TypeDuck-Web git status; VERIFIED: .planning/PROJECT.md] |
| Microsoft TSF APIs | Windows SDK APIs | External constraints for CLSID/profile registration planning. | `ITfInputProcessorProfiles::Register` registers text services; `AddLanguageProfile` creates language profiles with CLSID, LANGID, profile GUID, display text, and optional icon. [CITED: https://learn.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfinputprocessorprofiles-register; CITED: https://learn.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfinputprocessorprofiles-addlanguageprofile] |

### Supporting

| Tool | Version | Purpose | When to Use |
|------|---------|---------|-------------|
| Browser capture | Use available browser tooling during execution | Screenshots and viewport metadata for Web alpha visual fixtures. | Use for VER-01 visual references; record viewport, URL, date, and limitations. [VERIFIED: 01-CONTEXT.md] |
| `Invoke-WebRequest` | PowerShell built-in | Quick runtime availability check for Web alpha URL. | Use before fixture capture to decide complete vs partial artifact status. [VERIFIED: environment probe] |
| CMake | 3.29.2 available | Not needed for Phase 1 unless helper scripts inspect build config. | Later implementation phases need builds; Phase 1 does not. [VERIFIED: environment probe; VERIFIED: 01-CONTEXT.md] |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Markdown source-of-truth artifacts | Production renames first | Rejected because Phase 1 explicitly does not rename binaries, registry keys, protocol packages, installer files, or runtime paths. [VERIFIED: 01-CONTEXT.md] |
| Source-backed Web fixtures | Memory-backed summary | Rejected because D-09 requires source-backed fixtures and blocked/partial status when sources are unavailable. [VERIFIED: 01-CONTEXT.md] |
| Deterministic TypeDuck identifiers | Preserve Moqi CLSID/profile/AppId | Rejected as default because D-05 prefers new deterministic TypeDuck-owned identifiers unless migration compatibility explicitly retains a Moqi value. [VERIFIED: 01-CONTEXT.md] |

**Installation:** No external package installation is recommended for Phase 1. [VERIFIED: 01-CONTEXT.md]

```powershell
# No npm/pip/cargo install for Phase 1.
```

## Package Legitimacy Audit

No external packages are installed or recommended by this phase, so the Package Legitimacy Gate is not applicable. [VERIFIED: 01-CONTEXT.md]

## Architecture Patterns

### System Architecture Diagram

```text
Project decisions + AGENTS.md
        |
        v
Phase 1 planner
        |
        +--> Codebase identity grep + codebase maps
        |         |
        |         v
        |   TYPEDUCK-IDENTITY-CONTRACT.md
        |
        +--> TypeDuck Web runtime + TypeDuck Web source
        |         |
        |         v
        |   TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md
        |         |
        |         v
        |   supporting screenshots/assets under web-alpha-fixtures/2026-06-23/
        |
        +--> Requirements out-of-scope + banned code grep
                  |
                  v
            TYPEDUCK-BANNED-SURFACES.md
                  |
                  v
            Later phases consume contracts before touching production code
```

### Recommended Project Structure

```text
.planning/
├── product/
│   ├── TYPEDUCK-IDENTITY-CONTRACT.md
│   ├── TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md
│   ├── TYPEDUCK-BANNED-SURFACES.md
│   └── web-alpha-fixtures/
│       └── 2026-06-23/
│           ├── README.md
│           ├── screenshots/
│           └── source-excerpts/
└── phases/
    └── 01-identity-and-web-parity-contract/
        └── 01-RESEARCH.md
```

### Pattern 1: Identity Contract as a Table, Not Narrative

**What:** Track each identity surface with current Moqi value, target TypeDuck value, status, affected files, owner phase, and verification notes. [VERIFIED: 01-CONTEXT.md]  
**When to use:** Use for AppId, CLSID, profile GUIDs, file names, install paths, log/data paths, pipe/mutex/window class names, registry keys, resources, user-visible display names, and release artifacts. [VERIFIED: 01-CONTEXT.md; VERIFIED: codebase grep]

**Example:**

```markdown
| Surface | Current Scaffold Value | Target TypeDuck Value | Status | Affected Files | Verification Notes |
|---------|------------------------|-----------------------|--------|----------------|--------------------|
| TSF CLSID | `{8F204C91-2D7A-4B3E-9E1F-6A5C0D8B2E7F}` | TBD TypeDuck-owned deterministic GUID | proposed | `MoqiTextService/MoqiImeModule.cpp`, `installer/MoqiTsf.iss` | Must stay synchronized across COM registration and installer cleanup. |
```

### Pattern 2: Fixture Evidence Captures Source, Runtime, and Limits Together

**What:** For each fixture area, store the observed behavior, source file/line evidence, runtime URL, screenshot metadata, source commit/hash, and access limitations. [VERIFIED: 01-CONTEXT.md]  
**When to use:** Use for settings order/defaults, display-language behavior, candidate list, dictionary panel, visual tokens, and Windows mapping gaps. [VERIFIED: 01-CONTEXT.md; VERIFIED: TypeDuck-Web source grep]

**Example:**

```markdown
| Fixture Area | Runtime Evidence | Source Evidence | Captured Value | Limitations |
|--------------|------------------|-----------------|----------------|-------------|
| Settings defaults | `http://localhost:5173/TypeDuck-Web/aap2-alpha/`, viewport TBD | `src/consts.ts`, commit `db21054`, dirty worktree | `pageSize=6`, `showRomanization=Always`, English enabled/main | Must refresh if Web alpha changes. |
```

### Pattern 3: Negative Contract for Banned Surfaces

**What:** Document banned terms/features with rationale, likely files, and audit patterns, without performing full cleanup in Phase 1. [VERIFIED: 01-CONTEXT.md]  
**When to use:** Use for Moqi branding, fcitx, WebDAV/cloud clipboard, AI controls, Simplified-only Chinese, arbitrary backend config tools, excessive customization, and legacy backend fallback. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/REQUIREMENTS.md]

### Anti-Patterns to Avoid

- **Single-file brand rename:** Changing only visible strings or installer constants leaves CLSID/profile/AppId/registry/pipe/log/release surfaces inconsistent. [VERIFIED: .planning/codebase/CONCERNS.md]
- **Memory-backed fixture summaries:** VER-01 requires dated, source-backed fixtures; if runtime/source is unavailable, record partial/blocked status. [VERIFIED: 01-CONTEXT.md]
- **Production rename in Phase 1:** D-16 explicitly defers production renames to later implementation phases. [VERIFIED: 01-CONTEXT.md]
- **Treating backend `ime.json` as TypeDuck profile authority:** Current registration scans backend metadata, but TypeDuck needs deterministic first-party `zh-HK` profile identity later. [VERIFIED: .planning/codebase/CONCERNS.md; VERIFIED: codebase grep]

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| TSF identity semantics | Ad hoc registry theory in the contract | Microsoft TSF `Register`, `AddLanguageProfile`, and category concepts | The API defines the separate CLSID, LANGID, profile GUID, description, icon, and category pieces later phases must preserve. [CITED: https://learn.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfinputprocessorprofiles-addlanguageprofile; CITED: https://learn.microsoft.com/en-us/windows/win32/api/msctf/nn-msctf-itfcategorymgr] |
| Locale format | Custom locale strings | Windows locale-name convention plus locked `zh-HK` project decision | Windows locale names follow BCP 47-style patterns, and the project has locked Chinese Traditional Hong Kong as `zh-HK`. [CITED: https://learn.microsoft.com/en-us/windows/win32/intl/locale-names; VERIFIED: 01-CONTEXT.md] |
| Web parity memory | Handwritten recollection of UI behavior | Runtime/source-backed fixtures from TypeDuck Web alpha | The phase explicitly requires source-backed dated fixtures. [VERIFIED: 01-CONTEXT.md] |
| Banned surface discovery | Manual file-by-file reading only | `rg` audit patterns plus curated codebase maps | Known banned surfaces are spread across docs, launcher, protocol, installer, scripts, and workflows. [VERIFIED: .planning/codebase/CONCERNS.md; VERIFIED: codebase grep] |

**Key insight:** Phase 1 is not a rename phase; it is the contract that prevents later rename, registration, UI, and release phases from making incompatible one-off choices. [VERIFIED: 01-CONTEXT.md]

## Runtime State Inventory

This phase does not mutate runtime state, but it prepares a future identity migration contract; therefore the planner should require the identity contract to list runtime-state categories explicitly. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/codebase/INTEGRATIONS.md]

| Category | Items Found | Action Required |
|----------|-------------|-----------------|
| Stored data | `%LOCALAPPDATA%\MoqiIM` stores launcher config/logs; `%APPDATA%\Moqi` stores legacy cloud clipboard/config references. [VERIFIED: .planning/codebase/INTEGRATIONS.md; VERIFIED: codebase grep] | Contract-only in Phase 1; later phases decide migration vs fresh TypeDuck-owned paths. [VERIFIED: 01-CONTEXT.md] |
| Live service config | No external hosted service config was found; local backend manifests and installed `input_methods/*/ime.json` are file-based and currently feed profile metadata. [VERIFIED: .planning/codebase/INTEGRATIONS.md; VERIFIED: codebase grep] | Contract must mark backend-declared profile/config-tool authority as scaffold residue. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/codebase/CONCERNS.md] |
| OS-registered state | COM/TSF registration uses CLSID/profile data; installer startup key uses `MoqiLauncher`; scheduled re-registration task is `MoqiIM-ReRegisterTSF`. [VERIFIED: codebase grep] | Contract-only in Phase 1; later installer/registration phases must update and clean old registrations. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/ROADMAP.md] |
| Secrets/env vars | `MOQI_PROGRAM_DIR` affects registration/runtime discovery; GitHub Actions use `GH_TOKEN`; no repository secret files detected. [VERIFIED: .planning/codebase/INTEGRATIONS.md; VERIFIED: codebase grep] | Contract should decide whether future env vars are TypeDuck-owned or retained only as compatibility aliases. [VERIFIED: 01-CONTEXT.md] |
| Build artifacts | Build/stage outputs include `MoqiLauncher.exe`, `MoqiTextService.dll`, `installer/stage/win32/MoqiIM`, `installer/stage/x64/MoqiIM`, and `moqi-im-windows-setup`. [VERIFIED: codebase grep] | Contract-only in Phase 1; later build/release phases apply artifact renames. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/ROADMAP.md] |

## Common Pitfalls

### Pitfall 1: Fixture Drift Without Provenance
**What goes wrong:** Later UI plans implement stale Web alpha behavior. [VERIFIED: .planning/STATE.md]  
**Why it happens:** The alpha can drift, and the source worktree is currently dirty. [VERIFIED: TypeDuck-Web git status]  
**How to avoid:** Record capture date, URL, commit/hash, dirty status, source files, viewport, screenshots, and limitations in the fixture artifact. [VERIFIED: 01-CONTEXT.md]  
**Warning signs:** Fixture text lacks commit/hash, screenshot viewport, or blocked/partial notes. [VERIFIED: 01-CONTEXT.md]

### Pitfall 2: Conflating AppId, CLSID, and Profile GUID
**What goes wrong:** Installer upgrade identity, COM registration, and TSF language profile identity drift apart. [VERIFIED: .planning/codebase/CONCERNS.md]  
**Why it happens:** Current Moqi identity is scattered across `installer/MoqiTsf.iss`, `MoqiTextService/MoqiImeModule.cpp`, `libIME2/src/ImeModule.cpp`, and setup cleanup paths. [VERIFIED: codebase grep]  
**How to avoid:** Contract each identifier separately and include synchronization notes. [VERIFIED: 01-CONTEXT.md]  
**Warning signs:** One GUID appears in a plan without listing all affected files and verification notes. [VERIFIED: codebase grep]

### Pitfall 3: Letting Backend Metadata Define Product Identity
**What goes wrong:** Missing or malformed installed backend metadata can prevent profile registration or expose wrong product strings. [VERIFIED: .planning/codebase/CONCERNS.md]  
**Why it happens:** Current registration scans backend `input_methods/*/ime.json` for profile fields. [VERIFIED: codebase grep]  
**How to avoid:** Phase 1 contract should mark first-party deterministic TypeDuck `zh-HK` identity as the later implementation target. [VERIFIED: 01-CONTEXT.md]  
**Warning signs:** Contract treats `ime.json` as the sole future source of profile GUID/locale/display name. [VERIFIED: .planning/codebase/CONCERNS.md]

### Pitfall 4: Banned Surfaces Listed Without Audit Patterns
**What goes wrong:** Phase 6 cannot reliably verify removal of Moqi/fcitx/WebDAV/cloud/AI/Simplified-only surfaces. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/REQUIREMENTS.md]  
**Why it happens:** Banned surfaces exist across docs, launcher, protocol, installer, scripts, and workflows. [VERIFIED: codebase grep]  
**How to avoid:** Include banned terms, rationale, likely files, and exact `rg` patterns in `TYPEDUCK-BANNED-SURFACES.md`. [VERIFIED: 01-CONTEXT.md]  
**Warning signs:** Banned contract has prose only and no file/pattern columns. [VERIFIED: 01-CONTEXT.md]

## Code Examples

### Identity Surface Audit Command

```powershell
# Source: verified by running targeted grep in this session.
rg -n "Moqi|MoqiIM|MoqLauncher|MoqiTextService|CLSID|AppId|GUID|locale|fallbackLocale|profile|pipe|mutex|chinesesimplified|fcitx|WebDAV|cloud|clipboard|AI|configTool" `
  CMakeLists.txt backends.json README.md TODO.md MoqiTextService MoqLauncher SetupHelper installer scripts proto .github `
  -g "!proto/moqi.pb.*"
```

### Fixture Source Audit Command

```powershell
# Source: verified against I:\GitHub\TypeDuck-Web in this session.
rg -n "Display Languages|Candidates Jyutping|Auto-completion|Auto-correction|Auto-composition|Input Memory|Reverse Lookup|Show Full Input Code|Cangjie|Chinese Typeface|No\. of Candidates|More Languages|Jyutping|Hindi|Indonesian|Nepali|Urdu" `
  "I:\GitHub\TypeDuck-Web\src" "I:\GitHub\TypeDuck-Web\public" "I:\GitHub\TypeDuck-Web\schema" `
  -g "!node_modules"
```

### Minimal Contract Table Shape

```markdown
| Surface | Current Scaffold Value | Target TypeDuck Value | Status | Owner Phase | Affected Files | Verification Notes |
|---------|------------------------|-----------------------|--------|-------------|----------------|--------------------|
| Launcher exe | `MoqiLauncher.exe` | TBD | proposed | Phase 6/7 | `MoqLauncher/*`, `scripts/install.ps1`, `installer/MoqiTsf.iss` | Keep startup key, process launch, artifact names, and uninstall cleanup synchronized. |
```

### Minimal Web Fixture Table Shape

```markdown
| Area | Observed Order/Default | Source Files | Runtime Evidence | Windows Mapping Notes |
|------|------------------------|--------------|------------------|-----------------------|
| Display Languages | English enabled/main by default | `src/consts.ts`, `src/Preferences.tsx` | screenshot path + viewport | Settings dialog must show this group first. |
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Moqi scaffold identity spread through code/scripts/installer | Central TypeDuck identity contract before implementation | Phase 1 decision on 2026-06-23 | Later agents must use the contract instead of inventing names/GUIDs. [VERIFIED: 01-CONTEXT.md] |
| Backend `ime.json` controls profile metadata | Deterministic TypeDuck-owned `zh-HK` identity is the target | Phase 1 decision on 2026-06-23 | Later registration work should move product identity into first-party constants/resources. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/codebase/CONCERNS.md] |
| Memory-backed UI parity | Dated source-backed Web alpha fixtures | Phase 1 decision on 2026-06-23 | Later UI and verification phases can compare against concrete evidence. [VERIFIED: 01-CONTEXT.md] |

**Deprecated/outdated:**
- Moqi/Simplified/fcitx/WebDAV/cloud clipboard/AI user-facing surfaces are scaffold residue and banned from v1. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/REQUIREMENTS.md]
- Shipping legacy Moqi backend as fallback is out of scope because it validates wrong behavior. [VERIFIED: .planning/REQUIREMENTS.md]

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | Browser screenshot capture will be available during execution through the normal local browser tooling. [ASSUMED] | Standard Stack / Environment Availability | VER-01 visual fixtures may need a manual screenshot fallback with recorded limitations. |
| A2 | The exact target TypeDuck executable/DLL/path/GUID values should be chosen during artifact creation, not in research. [ASSUMED] | Architecture Patterns | Planner may need to add a human-review checkpoint before locking deterministic identifiers. |

## Open Questions

Resolved by Phase 1 plan coverage in revision iteration 1. [VERIFIED: 01-01-PLAN.md; VERIFIED: 01-02-PLAN.md]

1. **Should generated TypeDuck GUID/AppId values be locked automatically or gated by human review?**
   - Resolution: `01-01-PLAN.md` requires the identity contract to include Channel Readiness guidance and to mark AppId/CLSID/profile GUID values as proposed/pending human review unless already approved. [VERIFIED: 01-01-PLAN.md]
   - Implementation boundary: Phase 1 remains documentation-only and explicitly does not implement beta/stable side-by-side support. [VERIFIED: 01-CONTEXT.md; VERIFIED: 01-01-PLAN.md]

2. **How much screenshot capture is enough for VER-01?**
   - Resolution: `01-02-PLAN.md` defines the Phase 1 minimum sufficient visual set as desktop 1280x720 settings, candidate-list, and dictionary-detail screenshots when runtime capture works. [VERIFIED: 01-02-PLAN.md]
   - Fallback: blocked/partial metadata is acceptable when runtime or screenshot capture is unavailable, provided the exact unavailable source or attempted capture is recorded. [VERIFIED: 01-CONTEXT.md; VERIFIED: 01-02-PLAN.md]

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|-------------|-----------|---------|----------|
| TypeDuck Web alpha URL | VER-01 runtime fixture capture | yes | HTTP 200, length 2208 | If unavailable during execution, mark fixture partial/blocked. [VERIFIED: Invoke-WebRequest; VERIFIED: 01-CONTEXT.md] |
| `I:\GitHub\TypeDuck-Web` | VER-01 source fixture capture | yes | commit `db21054`, dirty worktree | If unavailable during execution, mark fixture partial/blocked. [VERIFIED: TypeDuck-Web git status; VERIFIED: 01-CONTEXT.md] |
| Git | Source provenance | yes | 2.47.1.windows.1 | Record unavailable commit/hash as limitation. [VERIFIED: environment probe] |
| Node.js | Optional helper scripts | yes | v22.13.1 | Use PowerShell-only checks if not needed. [VERIFIED: environment probe] |
| PowerShell 7 | Optional helper scripts | yes | 7.5.4 | Windows PowerShell is also present. [VERIFIED: environment probe] |
| CMake | Not required for Phase 1 docs | yes | 3.29.2 | Skip build work in this phase. [VERIFIED: environment probe; VERIFIED: 01-CONTEXT.md] |
| `protoc` | Not required for Phase 1 docs | no | — | No fallback needed for Phase 1. [VERIFIED: environment probe] |
| Inno Setup `ISCC` | Not required for Phase 1 docs | no | — | No fallback needed for Phase 1. [VERIFIED: environment probe] |

**Missing dependencies with no fallback:**
- None for Phase 1 scope. [VERIFIED: 01-CONTEXT.md; VERIFIED: environment probe]

**Missing dependencies with fallback:**
- Browser screenshot tooling was not probed in this research; use manual or alternative screenshot capture if unavailable and record limitations. [ASSUMED]

## Validation Architecture

### Test Framework

| Property | Value |
|----------|-------|
| Framework | GoogleTest 1.10.0 exists only under `libIME2`; Phase 1 artifact validation should use PowerShell/`rg` checks unless helper tests are introduced. [VERIFIED: AGENTS.md; VERIFIED: libIME2/test/CMakeLists.txt] |
| Config file | `libIME2/test/CMakeLists.txt` for existing COM helper tests; no top-level artifact validation config exists. [VERIFIED: rg --files] |
| Quick run command | `powershell -NoProfile -ExecutionPolicy Bypass -File .planning/product/verify-phase1-artifacts.ps1` if planner adds the script; otherwise manual artifact checklist. [ASSUMED] |
| Full suite command | Not applicable for Phase 1 docs unless helper script is added. [VERIFIED: 01-CONTEXT.md] |

### Phase Requirements -> Test Map

| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|--------------|
| IDEN-02 | Identity contract exists and covers required identity categories with current value, target value, status, affected files, and verification notes. [VERIFIED: .planning/REQUIREMENTS.md; VERIFIED: 01-CONTEXT.md] | artifact validation | `Test-Path .planning/product/TYPEDUCK-IDENTITY-CONTRACT.md; rg -n "Executable|DLL|AppId|CLSID|profile GUID|install directories|log/data|pipe|mutex|registry|resource|string|release artifact" .planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` | no, Wave 0 |
| VER-01 | Web alpha fixture exists and covers settings defaults/labels, candidate list, dictionary panel, visual references, source commit, URL, viewport, and limitations. [VERIFIED: .planning/REQUIREMENTS.md; VERIFIED: 01-CONTEXT.md] | artifact validation | `Test-Path .planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md; rg -n "Display Languages|No. of Candidates|Candidates Jyutping|Dictionary|More Languages|viewport|commit|limitations" .planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` | no, Wave 0 |

### Sampling Rate

- **Per task commit:** Run the artifact validation checks for the artifact touched by that task. [ASSUMED]
- **Per wave merge:** Run all Phase 1 artifact checks and confirm no production code files were modified unless only helper support was intentionally added. [VERIFIED: 01-CONTEXT.md]
- **Phase gate:** Confirm IDEN-02 and VER-01 artifacts exist, include provenance, and mark unavailable Web evidence as partial/blocked instead of fabricated. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/REQUIREMENTS.md]

### Wave 0 Gaps

- [ ] `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` — covers IDEN-02. [VERIFIED: 01-CONTEXT.md]
- [ ] `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` — covers VER-01. [VERIFIED: 01-CONTEXT.md]
- [ ] `.planning/product/TYPEDUCK-BANNED-SURFACES.md` — supports Phase 1 success criterion 3 and later VER-02. [VERIFIED: .planning/ROADMAP.md]
- [ ] Optional `.planning/product/verify-phase1-artifacts.ps1` — repeatable artifact validation. [ASSUMED]

## Security Domain

OWASP ASVS is a verification standard for application security controls, and OWASP currently lists ASVS 5.0.0 as the latest stable version. [CITED: https://owasp.org/www-project-application-security-verification-standard/] The category labels below follow the GSD template categories as a planning checklist for this native Windows IME phase. [ASSUMED]

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|------------------|
| V2 Authentication | no for Phase 1 artifact work | No account/auth system is introduced. [VERIFIED: 01-CONTEXT.md] |
| V3 Session Management | no for Phase 1 artifact work | No user session system is introduced. [VERIFIED: 01-CONTEXT.md] |
| V4 Access Control | yes for future IPC/install planning | Banned-surface contract should flag arbitrary backend config-tool launch and same-user pipe spoofing risks for later phases. [VERIFIED: .planning/codebase/CONCERNS.md] |
| V5 Input Validation | yes for artifact provenance and future protocol planning | Fixture/artifact checks should reject fabricated or missing provenance; later protocol phases handle frame bounds. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/codebase/CONCERNS.md] |
| V6 Cryptography | no for Phase 1 artifact work | No cryptographic storage or transport is introduced. [VERIFIED: 01-CONTEXT.md] |

### Known Threat Patterns for Native Windows IME Planning

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Arbitrary backend-declared config tool execution | Elevation of Privilege / Tampering | Ban generic backend config tools from v1 and later replace with a fixed first-party TypeDuck settings entry point. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/codebase/CONCERNS.md] |
| Same-user named pipe spoofing | Spoofing | Identity contract should namespace TypeDuck pipe/mutex names; later security phase should validate server identity and pipe ACLs. [VERIFIED: .planning/codebase/CONCERNS.md] |
| Cloud clipboard/WebDAV scaffold leakage | Information Disclosure | Banned-surface contract should mark cloud clipboard/WebDAV as v1-prohibited. [VERIFIED: 01-CONTEXT.md; VERIFIED: .planning/REQUIREMENTS.md] |
| Raw typed content in diagnostics | Information Disclosure | Identity contract should reserve TypeDuck-owned log paths; later cleanup should keep logs opt-in and avoid raw typed content by default. [VERIFIED: .planning/REQUIREMENTS.md; VERIFIED: .planning/codebase/CONCERNS.md] |

## Sources

### Primary (HIGH confidence)
- `.planning/phases/01-identity-and-web-parity-contract/01-CONTEXT.md` - locked Phase 1 decisions, boundaries, artifact paths, and deferred work. [VERIFIED: local file]
- `.planning/REQUIREMENTS.md` - IDEN-02 and VER-01 definitions and out-of-scope product behavior. [VERIFIED: local file]
- `.planning/ROADMAP.md` - Phase 1 goal, success criteria, phase dependencies, and downstream phase boundaries. [VERIFIED: local file]
- `.planning/PROJECT.md` and `AGENTS.md` - product source of truth and project constraints. [VERIFIED: local file]
- `.planning/codebase/STRUCTURE.md`, `.planning/codebase/CONCERNS.md`, `.planning/codebase/INTEGRATIONS.md` - code surface map, migration risks, and integration points. [VERIFIED: local file]
- TypeDuck Web source at `I:\GitHub\TypeDuck-Web` - `src/Preferences.tsx`, `src/consts.ts`, `src/Candidate.tsx`, `src/DictionaryPanel.tsx`, `src/index.css`, `tailwind.config.ts`. [VERIFIED: source grep]

### Secondary (MEDIUM confidence)
- Microsoft `ITfInputProcessorProfiles::Register` docs - TSF text service registration. [CITED: https://learn.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfinputprocessorprofiles-register]
- Microsoft `ITfInputProcessorProfiles::AddLanguageProfile` docs - language profile CLSID/LANGID/profile GUID/display/icon fields. [CITED: https://learn.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfinputprocessorprofiles-addlanguageprofile]
- Microsoft `ITfCategoryMgr` docs - TSF GUID/category organization. [CITED: https://learn.microsoft.com/en-us/windows/win32/api/msctf/nn-msctf-itfcategorymgr]
- Microsoft locale-name docs - BCP 47-style locale names. [CITED: https://learn.microsoft.com/en-us/windows/win32/intl/locale-names]
- OWASP ASVS project page - ASVS purpose and current stable release note. [CITED: https://owasp.org/www-project-application-security-verification-standard/]

### Tertiary (LOW confidence)
- Assumptions about browser capture availability and exact screenshot count; planner should verify during execution. [ASSUMED]

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH for local tools/artifact stack because it was verified from project files and environment probes. [VERIFIED: environment probe; VERIFIED: local files]
- Architecture: HIGH for phase boundaries and local code surface ownership because it comes from CONTEXT/codebase maps and grep. [VERIFIED: 01-CONTEXT.md; VERIFIED: codebase grep]
- Pitfalls: HIGH for local scaffold risks because they are documented in codebase maps and confirmed by grep. [VERIFIED: .planning/codebase/CONCERNS.md; VERIFIED: codebase grep]
- External Windows TSF facts: MEDIUM because they were verified against Microsoft Learn during this session. [CITED: Microsoft Learn URLs above]

**Research date:** 2026-06-23  
**Valid until:** 2026-07-23 for local phase planning; refresh Web alpha fixtures immediately before Phase 5 UI implementation because the alpha can drift. [VERIFIED: .planning/STATE.md]
