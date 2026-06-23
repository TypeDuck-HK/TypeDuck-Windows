# Phase 2: Engine Runtime Contract Spike - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md - this log preserves the alternatives considered.

**Date:** 2026-06-23
**Phase:** 2-Engine Runtime Contract Spike
**Areas discussed:** Spike Boundary, Dictionary Payload Shape, Runtime Asset Packaging, Legacy Backend Replacement Line

---

## Spike Boundary

| Option | Description | Selected |
|--------|-------------|----------|
| Host + staged runtime | Prove a minimal engine host and produce a Windows-shaped TypeDuck runtime tree for later packaging. | |
| Host only | Prove librime/plugin behavior without deciding much packaging shape yet. | |
| Packaging too | Also wire a temporary backend/runtime package path. | |
| Minimal moqi-ime compatibility adapter | Use the current backend shape as an internal bridge, modify as little as possible, and focus on TypeDuck-HK librime compatibility. | yes |

**User's choice:** Minimal `moqi-ime` compatibility adapter + staged runtime.
**Notes:** User questioned whether a middle-end is necessary because TypeDuck-Web works with librime directly. The accepted answer was that a middle-end is not intrinsically necessary, but the existing Windows process boundary can ship faster and protect TSF host applications. User wants the least possible `moqi-ime` changes and only the compatibility work needed for `rime_api` differences.

---

## Dictionary Payload Shape

| Option | Description | Selected |
|--------|-------------|----------|
| Strict typed sample contract | Capture exact fields as named planning fixture fields. | |
| Raw comment first | Accept the lookup-filter CSV/comment output as the first proof. | yes |
| Both raw + typed | Store raw plugin output plus a best-effort structured JSON shape. | |

**User's choice:** Raw comment/CSV contract first.
**Notes:** User clarified that the CSV is already clearly specified with columns `match_input_buffer,honzi,jyutping,canonical_honzi,canonical_jyutping,components_honzi,components_jyutping,pron_label,lit_col_reading,pos,register,label,written_form,vernacular_form,collocation,eng,hin,urd,nep,ind`. Phase 2 should prove these are emitted; later phases map them into protocol/UI fields.

---

## Runtime Asset Packaging

| Option | Description | Selected |
|--------|-------------|----------|
| Windows-shaped staged tree | Produce a local TypeDuck runtime folder containing DLL, plugin support, schemas, dictionaries, OpenCC assets, and built schema artifacts. | yes |
| Build outputs only | Prove DLL/plugin/schema build outputs exist and defer final layout. | |
| Installer-like stage | Put assets directly into near-final installer staging now. | |

**User's choice:** Windows-shaped staged tree.
**Notes:** User specified that a replacement `rime.dll` is available and the out-of-scope process used to obtain it should not be documented. Project documentation should point to `https://github.com/TypeDuck-HK/librime/releases/download/v1.1.3/rime-TypeDuck-v1.1.3-Windows-msvc-x86.7z`. User also specified adding `https://github.com/TypeDuck-HK/schema` branch `aap2-alpha` as a submodule for schemas, dictionaries, and OpenCC assets.

---

## Legacy Backend Replacement Line

| Option | Description | Selected |
|--------|-------------|----------|
| No launcher/backend wiring yet | Prove the TypeDuck runtime through a standalone spike host and staged runtime tree. | |
| Temporary TypeDuck backend entry | Add a non-shipping temporary backend manifest entry for local testing. | |
| Replace Moqi backend path now | Start changing packaging/manifest behavior away from `moqi-ime` during Phase 2. | |
| Minimal moqi-ime compatibility adapter | Use the existing backend path as the fastest internal bridge while keeping later cleanup separate. | yes |

**User's choice:** Minimal `moqi-ime` compatibility adapter.
**Notes:** Using `moqi-ime` as a bridge is acceptable only as internal compatibility work. It should not preserve Moqi product behavior, legacy fallback behavior, or banned user-visible surfaces as TypeDuck truth.

---

## the agent's Discretion

- Planner may choose exact staging directory and verification command shapes.
- Direct standalone librime probing may be used as fallback/debug evidence if the `moqi-ime` compatibility route blocks proof.

## Deferred Ideas

- Native candidate/dictionary UI belongs to Phase 5.
- Final TypeDuck protocol/data model belongs to Phase 4.
- Broad TypeDuck cleanup, installer identity, and release verification belong to later mapped phases.
