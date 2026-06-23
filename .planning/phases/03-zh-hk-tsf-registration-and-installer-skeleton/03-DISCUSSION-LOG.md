# Phase 3: zh-HK TSF Registration and Installer Skeleton - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-06-23
**Phase:** 3-zh-HK TSF Registration and Installer Skeleton
**Areas discussed:** Profile registration authority, Installer identity scope, DLL and bitness strategy, Uninstall and migration cleanup

---

## Profile Registration Authority

| Option | Description | Selected |
|--------|-------------|----------|
| First-party TypeDuck profile | Register deterministic TypeDuck `zh-HK` profile metadata from this repo and identity contract. Recommended because backend `ime.json` is scaffold/runtime metadata and can be missing or malformed. | ✓ |
| Backend-declared profile | Continue trusting backend `ime.json` for GUID, locale, name, and icon. Faster but fails the deterministic first-party registration requirement. | |
| Hybrid with backend override | Seed TypeDuck metadata but allow backend metadata to override it. Rejected because it weakens INST-03. | |

**User's choice:** Selected all areas with the agent's recommendations.
**Notes:** Lock the recommended path: first-party `zh-HK` profile metadata is authoritative; backend metadata can remain optional during transition.

---

## Installer Identity Scope

| Option | Description | Selected |
|--------|-------------|----------|
| Registration-facing rename now | Rename/reword installer and install/register/uninstall surfaces now, while deferring broad docs/CI/UI cleanup. Recommended because it satisfies Phase 3 without becoming Phase 6. | ✓ |
| Minimal internal-only change | Change only enough to get TSF registration under `zh-HK`, leaving most installer chrome Moqi-branded. Rejected because INST-01 requires TypeDuck-branded bilingual installer wording. | |
| Full product cleanup | Rename every Moqi surface across docs, CI, protocol, tray, logs, and UI in this phase. Rejected as scope creep into Phase 6/7. | |

**User's choice:** Selected all areas with the agent's recommendations.
**Notes:** Phase 3 owns the installer skeleton and registration-facing identity only. Install-time settings flow remains Phase 5.

---

## DLL and Bitness Strategy

| Option | Description | Selected |
|--------|-------------|----------|
| Rename deployed TSF DLL now | Deploy/register `TypeDuckTextService.dll` for both bitnesses while keeping source directories/internal targets if useful. Recommended because the OS-visible COM/TSF identity should be TypeDuck. | ✓ |
| Keep Moqi DLL filename for now | Keep `MoqiTextService.dll` and only change profile metadata. Lower churn but leaves visible/system-level scaffold residue. | |
| Rename source tree and all targets now | Rename production source directories and CMake targets too. Rejected for Phase 3 because external identity matters more than internal path churn. | |

**User's choice:** Selected all areas with the agent's recommendations.
**Notes:** Preserve the existing Win32 `SysWOW64` and x64 `System32` registration model. Keep DLL naming synchronized across installer, setup helper, resources, and registration.

---

## Uninstall and Migration Cleanup

| Option | Description | Selected |
|--------|-------------|----------|
| TypeDuck-owned cleanup plus narrow migration sweep | Remove TypeDuck-owned files/registry/profile state and explicitly allowlist any legacy Moqi residues that conflict with migration. Recommended because it avoids broken duplicate profiles without broad deletion. | ✓ |
| TypeDuck-only cleanup | Remove only TypeDuck-owned state. Safer for unrelated Moqi installs but may leave broken scaffold registrations when migrating this repo. | |
| Broad Moqi cleanup | Delete broad Moqi-like files/registry/process state. Rejected because it risks removing unrelated user data and belongs outside a careful installer skeleton. | |

**User's choice:** Selected all areas with the agent's recommendations.
**Notes:** Scheduled task should become `TypeDuckIME-ReRegisterTSF`; process-stop logic should prefer TypeDuck-specific names and leave deeper hardening to Phase 6.

---

## the agent's Discretion

- The user explicitly selected all gray areas with the agent's recommendations.
- The planner may choose plan boundaries and exact code organization, but should keep Phase 3 limited to install/register/uninstall identity and verification.

## Deferred Ideas

- Install-time settings flow and settings dialog.
- Final TypeDuck protocol and typing behavior.
- Candidate/dictionary UI, About dialog, tray/settings cleanup, and broad banned-surface automation.
- CI release artifact naming and full release verification unless needed for the Phase 3 installer skeleton.
