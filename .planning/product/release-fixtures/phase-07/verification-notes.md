# TypeDuck Phase 7 Release Verification Notes

Generated UTC: 2026-06-28T08:33:34Z
Status: complete

## Installer Artifact

- Path: installer\dist\typeduck-windows-ime-setup.exe
- SHA-256: B5EFBCFC8620E83B2DD9E83B0D8D647F685B3882B4D793510A80BA3610C378CE
- Byte size: 20942974

## Automated Evidence

- phase6-privacy-security-cleanup: passed (.planning\product\privacy-security\phase-06-guard-results.json)
- phase6-backend-diagnostics: passed (.planning\product\privacy-security\phase-06-guard-results.json)
- release-install-evidence: passed (.planning\product\release-fixtures\phase-07\install-verification-template.json)
- protocol-recovery-probe: passed (.planning\product\release-fixtures\phase-07\protocol-recovery-results.json)
- protocol-recovery-guard: passed (.planning\product\release-fixtures\phase-07\protocol-recovery-cases.json)
- interactive-checklist-guard: passed (.planning\product\release-fixtures\phase-07\host-app-dpi-notes.md)
- release-artifact-guard: passed (installer\dist\typeduck-windows-ime-setup.exe)

## Interactive VM Evidence

- Install lifecycle notes: .planning\product\release-fixtures\phase-07\install-verification-notes.md
- Host-app and DPI notes: .planning\product\release-fixtures\phase-07\host-app-dpi-notes.md
- Current state: complete by direct user judgement in the guest VM.
- Screenshot capture: not required.

## Source Audit

- ROADMAP Phase 7 goal: covered by install evidence, protocol recovery, artifact guard, and completed human host-app/DPI notes.
- VER-03: covered by the install evidence schema and VM checklist.
- VER-04: complete by direct user host-app/DPI judgement without screenshot capture.
- VER-05: covered by protocol recovery tests and executed probe results.
- VER-06: covered by TypeDuck installer artifact name, hash, workflow, and schema-source guards.
- D-01 through D-05: release evidence excludes screenshot automation and keeps DPI human judged.
- D-06 through D-10: command, log, hash, registry/file, and human-note evidence are represented.
- D-11 through D-13: protocol/recovery cases are non-visual, redacted, and failure detail stays in logs/evidence.
- D-14 through D-16: no product capability changes are made by this aggregate verification.
- D-17 through D-20: workflows use TypeDuck-Windows, TypeDuck-Windows-backend, the TypeDuck schema release artifact, and no standalone schema upload artifact.
- Research: excluded because existing GSD documentation was sufficient and skip-research was requested before planning.

## Rerun Rule

If a release-blocking product bug is found, route the smallest focused repair, rerun the affected verification, then rerun scripts\Invoke-TypeDuckReleaseVerification.ps1 and scripts\Test-TypeDuckReleaseVerification.ps1.

## Regression Repairs Included in Final Evidence

- TypeDuck runtime registration no longer depends on pruned `ime.json`; backend registers the fixed TypeDuck profile GUID directly.
- Installer setup helper failure no longer results in a success-only final state; the final wording avoids technical terms and does not claim success on failure.
- TypeDuck Settings can apply during installation because the launcher is started through an explicit install-settings launcher flag before `/apply-settings` runs.
- Settings and Rime deploy behavior returned to the accepted first-run/full-check path; no prebuilt Rime build-copy shortcut remains in the final release evidence.
- `%APPDATA%\TypeDuckIME` and `%LOCALAPPDATA%\TypeDuckIME` behavior is aligned with preferences/custom YAML in Roaming and logs in Local.
- Candidate mouse-click commit now outputs the selected candidate instead of only resetting the backend state.
- Candidate popup sizing is stable in Explorer and high-DPI contexts, and candidate/definition Chinese text preserves the accepted HK Chinese font fallback.
- Installer/uninstaller UI uses bilingual custom strings, a working optional user-data deletion prompt, centered controls, and no Legacy Moqi cleanup.
- Appearance themes no longer ship development-only `source` metadata.
- The Windows system IME menu icon uses the packaged `resources\TypeDuck_Small.ico`; no `icons` folder is restored.
