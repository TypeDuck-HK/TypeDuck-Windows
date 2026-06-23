---
phase: 01
slug: identity-and-web-parity-contract
status: draft
nyquist_compliant: true
wave_0_complete: true
created: 2026-06-23
---

# Phase 01 - Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | PowerShell source assertions over planning artifacts |
| **Config file** | none - documentation-only phase |
| **Quick run command** | `powershell -NoProfile -Command 'Test-Path -LiteralPath ".planning/phases/01-identity-and-web-parity-contract/01-CONTEXT.md"'` |
| **Full suite command** | Run the `<verify><automated>` commands in `01-01-PLAN.md` and `01-02-PLAN.md` |
| **Estimated runtime** | ~10 seconds |

---

## Sampling Rate

- **After every task commit:** Run the task-level `<verify><automated>` command from the active plan.
- **After every plan wave:** Run both plan-level verification command blocks from `01-01-PLAN.md` and `01-02-PLAN.md`.
- **Before `$gsd-verify-work`:** Confirm all Phase 1 product artifacts exist and parse/check as documented.
- **Max feedback latency:** 30 seconds.

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 01-01-01 | 01 | 1 | IDEN-02 | T-01-01-02 | Identity values are source-backed and do not mutate production files | source assertion | `01-01-PLAN.md` Task 1 verify command | yes | pending |
| 01-01-02 | 01 | 1 | IDEN-02 | T-01-01-03 / T-01-01-04 | Banned privacy/security surfaces are explicitly documented for later cleanup | source assertion | `01-01-PLAN.md` Task 2 verify command | yes | pending |
| 01-02-01 | 02 | 1 | VER-01 | T-01-02-01 / T-01-02-02 | Fixture evidence records provenance and partial/blocked state instead of fabricating data | JSON parse + source assertion | `01-02-PLAN.md` Task 1 verify command | yes | pending |
| 01-02-02 | 02 | 1 | VER-01 | T-01-02-01 / T-01-02-03 | Dated fixture markdown links supporting evidence and limitations | source assertion | `01-02-PLAN.md` Task 2 verify command | yes | pending |

*Status: pending, green, red, flaky*

---

## Wave 0 Requirements

Existing infrastructure covers all phase requirements. This phase is documentation-only and does not require new test framework installation.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Review exact TypeDuck-owned AppId/CLSID/profile GUID values before later implementation locks them into binaries and installer metadata | IDEN-02 | Product identity values may need human approval before implementation, even if Phase 1 can document proposed values | Inspect `.planning/product/TYPEDUCK-IDENTITY-CONTRACT.md` and confirm each proposed identifier has status and verification notes |
| Confirm screenshot sufficiency for Web alpha visual fixtures | VER-01 | Runtime/browser availability may vary, and screenshots can be partial/blocked | Inspect `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md` plus metadata/screenshot entries and confirm blocked/partial notes are explicit |

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify commands or documentation-only equivalents.
- [x] Sampling continuity: no 3 consecutive tasks without automated verify.
- [x] Wave 0 covers all missing references.
- [x] No watch-mode flags.
- [x] Feedback latency < 30s.
- [x] `nyquist_compliant: true` set in frontmatter.

**Approval:** pending execution
