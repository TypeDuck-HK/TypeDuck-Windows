# Phase 1: Identity and Web Parity Contract - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md - this log preserves the alternatives considered.

**Date:** 2026-06-23
**Phase:** 1-Identity and Web Parity Contract
**Areas discussed:** identity contract shape, TypeDuck-owned identifier strategy, Web alpha fixture capture, banned scaffold surfaces, phase boundary

---

## Identity Contract Shape

| Option | Description | Selected |
|--------|-------------|----------|
| Product contract under `.planning/product/` | Durable cross-phase source of truth, not buried only in the phase directory. | yes |
| Inline only in Phase 1 context | Simpler, but later agents may miss it. | |
| Implementation constants first | Useful later, but too early before values are reviewed. | |

**User's choice:** Auto-selected recommended default.
**Notes:** Contract must include current scaffold values, target TypeDuck values, affected files, and verification notes.

---

## TypeDuck-Owned Identifier Strategy

| Option | Description | Selected |
|--------|-------------|----------|
| New deterministic TypeDuck-owned IDs | Clean product identity and avoids Moqi registration leakage. | yes |
| Preserve Moqi IDs for compatibility | Lower churn but keeps wrong product identity. | |
| Decide per subsystem later | Flexible but risks drift and conflicting renames. | |

**User's choice:** Auto-selected recommended default.
**Notes:** Migration compatibility can be documented as an exception later, but the default target is TypeDuck-owned identity.

---

## Web Alpha Fixture Capture

| Option | Description | Selected |
|--------|-------------|----------|
| Dated source-backed fixture set | Captures settings, candidate, dictionary, and visual references with source/date provenance. | yes |
| Narrative summary only | Human-readable but too weak for parity testing. | |
| Screenshot-only fixture set | Useful visually but misses settings/data semantics. | |

**User's choice:** Auto-selected recommended default.
**Notes:** If the local alpha or source is unavailable, execution must mark fixtures partial/blocked rather than inventing data.

---

## Banned Scaffold Surfaces

| Option | Description | Selected |
|--------|-------------|----------|
| Negative contract plus audit patterns | Makes off-scope Moqi/fcitx/cloud/AI/Simplified leakage explicit for later verification. | yes |
| Rely on requirements only | Requirements mention the ban, but not enough file/pattern detail for cleanup. | |
| Clean everything now | Too broad for Phase 1 and belongs to later implementation phases. | |

**User's choice:** Auto-selected recommended default.
**Notes:** Phase 1 documents banned surfaces; Phase 6 performs the comprehensive cleanup/audit.

---

## Phase Boundary

| Option | Description | Selected |
|--------|-------------|----------|
| Documentation and fixtures only | Matches the roadmap goal and avoids premature product rewrites. | yes |
| Start renaming production code immediately | Tempting, but risks spreading identity changes before the contract is reviewed. | |
| Build fixture tools and rename code together | Too much scope for the first phase. | |

**User's choice:** Auto-selected recommended default.
**Notes:** Small reproducibility helpers are allowed only if they directly support the contract/fixture artifacts.

---

## the agent's Discretion

- Exact markdown table structures for the product artifacts.
- Supporting fixture asset filenames under `.planning/product/web-alpha-fixtures/2026-06-23/`.
- Whether fixture snapshots use JSON, markdown tables, screenshots, or a combination, as long as they are source-backed and easy to consume.

## Deferred Ideas

- Production TypeDuck renames and registry/installer changes.
- Candidate/settings/About UI implementation.
- Full banned-string automated enforcement.
