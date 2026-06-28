# TypeDuck Phase 7 Interactive VM Checklist

This checklist is for direct human judgement in the disposable Windows VM. No screenshots are required, and this checklist must not be used as an automated visual-comparison gate.

## Preflight

- Tester: User, direct interactive VM judgement
- UTC date: 2026-06-28
- VM name/checkpoint: SHIOYA-INOBE / My Virtual Machine; checkpoint name not recorded in this checklist
- Windows version: Windows 10 guest observed in VM; exact build not recorded in this checklist
- Installer path: `installer\dist\typeduck-windows-ime-setup.exe`
- Installer SHA-256: `B5EFBCFC8620E83B2DD9E83B0D8D647F685B3882B4D793510A80BA3610C378CE`
- Install evidence reference: `.planning/product/release-fixtures/phase-07/install-verification-notes.md`
- Protocol recovery evidence reference: `.planning/product/release-fixtures/phase-07/protocol-recovery-results.json`
- Relevant command/log evidence: `.planning/product/release-fixtures/phase-07/verification-notes.md`; `%LOCALAPPDATA%\TypeDuckIME\Log\TypeDuckBackend-2026-06-28.log`; `%LOCALAPPDATA%\TypeDuckIME\Log\TypeDuckLauncher-2026-06-28.log`

## Host Apps

For each available host, type ordinary Cantonese input, open candidates, reveal dictionary detail where applicable, try reverse lookup where supported, update settings/redeploy once, observe backend restart/degraded recovery, and record the log path used.

| Host target | Available | Pass/Fail | Notes |
|-------------|-----------|-----------|-------|
| Notepad | yes | pass | N/A |
| Browser text field | yes | pass | N/A |
| Office or Office-like app | yes | pass | N/A |
| Terminal/console context | yes | pass | N/A |
| Elevated app | yes | pass | N/A |
| Awkward TSF host available in VM | yes | pass | N/A |

## Typing Smoke

| Check | Pass/Fail | Notes |
|-------|-----------|-------|
| Ordinary Cantonese input starts composition | pass | N/A |
| Candidate list appears and can be selected | pass | N/A |
| Dictionary detail appears when deliberately revealed | pass | N/A |
| Reverse lookup works where supported, or unsupported state is recorded | pass | N/A |
| Settings update/redeploy failure remains bounded and understandable | pass | N/A |
| Backend restart/degraded recovery leaves the host app responsive | pass | N/A |
| TypeDuck logs are under the TypeDuckIME log path | pass | N/A |

## DPI Judgement

The user controls DPI scaling and judges directly in the VM.

| DPI scale | Available | Pass/Fail | Notes |
|-----------|-----------|-----------|-------|
| 100% | yes | pass | N/A |
| 140% if available | yes | pass | N/A |
| 175% | yes | pass | N/A |
| 200% | yes | pass | N/A |

## Multi-Monitor

Multi-monitor behavior was already human verified in Phase 5. Phase 7 does not reintroduce a multi-monitor automated visual matrix.

## Completion

Observations have been recorded in `host-app-dpi-notes.md`. Rerun guard when release evidence is refreshed:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckInteractiveReleaseChecklist.ps1 -RepoRoot . -Strict
```
