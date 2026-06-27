---
phase: 07-compatibility-and-release-verification
plan: 04
subsystem: aggregate-release-verification
tags: [powershell, release-evidence, artifact-guard, manifest, human-checkpoint]

requires:
  - plan: 07-01
    provides: Install lifecycle evidence schema and manual VM install notes.
  - plan: 07-02
    provides: Protocol recovery cases, executed probe results, and guard.
  - plan: 07-03
    provides: Interactive host-app/DPI checklist and notes template.
provides:
  - Aggregate release verification runner.
  - Release artifact/workflow/schema-source guard.
  - Aggregate release manifest validator.
  - Phase 7 release evidence manifest and human-readable notes.
affects: [phase-07-release-verification, VER-03, VER-04, VER-05, VER-06]

key-files:
  created:
    - scripts/Test-TypeDuckReleaseArtifacts.ps1
    - scripts/Invoke-TypeDuckReleaseVerification.ps1
    - scripts/Test-TypeDuckReleaseVerification.ps1
    - .planning/product/release-fixtures/phase-07/release-verification.json
    - .planning/product/release-fixtures/phase-07/verification-notes.md
  modified:
    - .planning/product/release-fixtures/phase-07/protocol-recovery-results.json
    - .planning/product/privacy-security/phase-06-guard-results.json

requirements-completed: ["VER-03", "VER-05", "VER-06"]
requirements-human-pending: ["VER-04"]

completed: 2026-06-27
status: human_checkpoint_pending
---

# Phase 07 Plan 04: Aggregate Release Verification Summary

Created the final non-interactive release evidence packet. The aggregate now validates TypeDuck release workflow names, schema source, Rime deployer runtime build generation, installer artifact name/hash, Phase 6 cleanup/security guards, install evidence schema, executed protocol recovery results, and the interactive host-app/DPI checklist contract.

## Outputs

- `scripts/Test-TypeDuckReleaseArtifacts.ps1` validates `TypeDuck-Windows`, `TypeDuck-Windows-backend`, `https://github.com/TypeDuck-HK/schema`, branch `aap2-alpha`, Rime deployer `build` output, TypeDuck artifact names, and no standalone schema artifact.
- `scripts/Invoke-TypeDuckReleaseVerification.ps1` runs the deterministic release guards and writes `.planning/product/release-fixtures/phase-07/release-verification.json` plus `verification-notes.md`.
- `scripts/Test-TypeDuckReleaseVerification.ps1` validates the aggregate manifest, required case IDs, executed protocol recovery results, artifact hash evidence, and the no-screenshot-capture boundary.

## Evidence

- Installer artifact: `installer\dist\typeduck-windows-ime-setup.exe`
- SHA-256: `CDD1028B630A74AE21D095A76843581FA8D794A5250FF00CD69F78D3F447AF0E`
- Byte size: `21075640`
- Aggregate status: `automated-ready-human-pending`

## Verification

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckReleaseArtifacts.ps1 -RepoRoot . -Strict` - passed.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Invoke-TypeDuckReleaseVerification.ps1 -RepoRoot . -EvidenceRoot .planning\product\release-fixtures\phase-07 -BackendRoot D:\VSProjects\moqi-ime` - passed.
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckReleaseVerification.ps1 -RepoRoot . -Strict` - passed.

## Remaining Human Checkpoint

VER-04 remains intentionally pending. The final step is direct human VM judgement using `.planning/product/release-fixtures/phase-07/interactive-vm-checklist.md` and recording results in `.planning/product/release-fixtures/phase-07/host-app-dpi-notes.md`. No screenshots are required or requested.

---
*Phase: 07-compatibility-and-release-verification*
