# Phase 7: Compatibility and Release Verification - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-06-27T06:55:00Z
**Phase:** 7-Compatibility and Release Verification
**Areas discussed:** Release evidence matrix, protocol recovery verification, VM screenshot/DPI boundary, product change boundary

---

## Release Evidence Matrix

| Option | Description | Selected |
|--------|-------------|----------|
| Repeatable non-screenshot evidence | Use commands, logs, hashes, registry/file checks, and human notes; no screenshot-capture automation. | yes |
| Full automated screenshot matrix | Automate VM screenshots and compare/capture visual evidence. | |
| Minimal smoke only | Run only a final manual smoke and produce artifact hash. | |

**User's choice:** No VM automation that requires capturing screenshots. Visual verification is interactive, and the user will judge without capturing screenshots.
**Notes:** DPI is user-controlled in the VM. Broader integration-test setup outside v1 scope should not be pulled in.

---

## DPI And Visual Verification

| Option | Description | Selected |
|--------|-------------|----------|
| User-controlled DPI judgement | User controls VM and judges 100%, 140% if supported, 175%, and 200%. | yes |
| Automated screenshot capture | Script DPI changes and capture screenshots. | |
| Skip DPI | Rely only on Phase 5 visual acceptance. | |

**User's choice:** User will test 100%, 140% if the VM supports it, 175%, and 200%. The user will judge the result instead of captured screenshots.
**Notes:** Monitors were already human verified in Phase 5.

---

## Protocol Recovery Verification

| Option | Description | Selected |
|--------|-------------|----------|
| Repeatable non-screenshot recovery tests | Include important malformed frame, timeout, backend restart, stale response, settings failure, and normal path checks that can be repeated without screenshots. | yes |
| Exhaustive UI-driven recovery matrix | Drive recovery through every host app with screenshots. | |
| Defer recovery | Leave recovery coverage to future integration test setup. | |

**User's choice:** Include important protocol/recovery bits, but only the repeatedly testable parts that do not require screenshots.
**Notes:** Visual host-app judgement remains interactive.

---

## Product Change Boundary

| Option | Description | Selected |
|--------|-------------|----------|
| Phase 6 product changes, Phase 7 verification | Do all non-test product changes in Phase 6 unless Phase 7 detects a failure. | yes |
| Phase 7 product polish | Allow Phase 7 to make product cleanup changes freely. | |
| Verification only, no fixes | If Phase 7 finds issues, stop without fixing. | |

**User's choice:** All non-test product code changes should be done in Phase 6 unless a failure is detected in Phase 7.
**Notes:** Phase 7 may add tests/scripts/docs/evidence and rerun affected checks after a focused fix.

---

## the agent's Discretion

- Choose exact release checklist and verification script split.
- Decide which recovery cases are automated versus documented/manual, while respecting the no-screenshot-automation boundary.

## Deferred Ideas

- Automated screenshot capture/comparison.
- Broad integration-test infrastructure outside v1 scope.
