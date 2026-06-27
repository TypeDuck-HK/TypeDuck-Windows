# Phase 6: Privacy, Security, and Scaffold Cleanup - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-06-27T06:55:00Z
**Phase:** 6-Privacy, Security, and Scaffold Cleanup
**Areas discussed:** Scaffold leakage audit, diagnostics privacy, IPC and launch hardening, installer/process cleanup, bilingual string policy

---

## Scaffold Leakage Audit

| Option | Description | Selected |
|--------|-------------|----------|
| User-visible only | Clean visible product surfaces and shipped paths while allowing internal source identifiers to remain. | yes |
| Whole-source rename | Rename source folders/files/classes/variables as well. | |
| Agent discretion | Let planner decide case by case. | |

**User's choice:** Source code folder/file/class/variable names are fine for v1. Logs, user-facing installation paths, log folders/files, and visible product surfaces must be cleaned up.
**Notes:** The guard strategy should not fail on internal implementation names that are not user-facing.

---

## Diagnostics Privacy

| Option | Description | Selected |
|--------|-------------|----------|
| English-only logs | Translate/remove Simplified Chinese log/debug/printf text and keep logs in English. | yes |
| Full diagnostic rewrite | Redesign logging broadly in Phase 6. | |
| Minimal path rename only | Rename log paths but leave message text mostly untouched. | |

**User's choice:** Logs must be entirely in English with no Simplified Chinese log/debug/printf lines. Be conservative while translating/auditing because current logs appear mostly fine.
**Notes:** Runtime diagnostics should use TypeDuck-owned paths and avoid raw typed content by default.

---

## IPC And Launch Hardening

| Option | Description | Selected |
|--------|-------------|----------|
| Compatibility-tolerant hardening | Add bounds and sanity checks without rejecting past/future TypeDuck executables or services. | yes |
| Strict identity enforcement | Reject executables/services that do not match exact current version/signature expectations. | |
| Minimal cleanup | Only remove arbitrary config tool launching. | |

**User's choice:** Agree with hardening, but do not check anything that causes executables/services from past or future versions to be rejected. Technical failure details belong in logs except unrecoverable typing failure or suspected explicit tampering.
**Notes:** Fixed first-party settings launch is required; arbitrary backend-declared config tools should be removed.

---

## Installer And Process Cleanup

| Option | Description | Selected |
|--------|-------------|----------|
| Automated TypeDuck cleanup | Installer kills known TypeDuck processes automatically and removes the running-engine page. | yes |
| Manual user prompt/page | Keep a page that shows TypeDuck/backend is running and asks the user to intervene. | |
| No process killing | Avoid process termination and require restart/reboot. | |

**User's choice:** The page showing TypeDuck and backend engine currently running should be eliminated. Killing is fine; the installer already knows what to kill and should automate it.
**Notes:** Cleanup should stay TypeDuck-specific and avoid broad unrelated process kills.

---

## Bilingual String Policy

| Option | Description | Selected |
|--------|-------------|----------|
| Own installer strings | Remove Simplified Chinese Inno translation and supply all installer strings ourselves. | yes |
| Official translation pack | Use an official Inno translation where available. | |
| Source inspection first | Inspect source before deciding. | |

**User's choice:** All v1-visible strings appear bilingual except installer. Remove the Simplified Chinese submodule immediately and supply all installer strings instead of relying on official translations.
**Notes:** Phase 6 may still inspect source/resources and add guards for visible strings.

---

## the agent's Discretion

- Choose the exact implementation split across product cleanup, log/path changes, IPC/frame hardening, installer localization, and guards.
- Keep non-test product code changes in Phase 6.

## Deferred Ideas

- Screenshot-capture automation is excluded from release verification.
- Broader integration-test setup outside v1 is not in scope.
