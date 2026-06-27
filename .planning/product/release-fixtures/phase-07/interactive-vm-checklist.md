# TypeDuck Phase 7 Interactive VM Checklist

This checklist is for direct human judgement in the disposable Windows VM. No screenshots are required, and this checklist must not be used as an automated visual-comparison gate.

## Preflight

- Tester:
- UTC date:
- VM name/checkpoint:
- Windows version:
- Installer path:
- Installer SHA-256:
- Install evidence reference:
- Protocol recovery evidence reference:
- Relevant command/log evidence:

## Host Apps

For each available host, type ordinary Cantonese input, open candidates, reveal dictionary detail where applicable, try reverse lookup where supported, update settings/redeploy once, observe backend restart/degraded recovery, and record the log path used.

| Host target | Available | Pass/Fail | Notes |
|-------------|-----------|-----------|-------|
| Notepad | | | |
| Browser text field | | | |
| Office or Office-like app | | | |
| Terminal/console context | | | |
| Elevated app | | | |
| Awkward TSF host available in VM | | | |

## Typing Smoke

| Check | Pass/Fail | Notes |
|-------|-----------|-------|
| Ordinary Cantonese input starts composition | | |
| Candidate list appears and can be selected | | |
| Dictionary detail appears when deliberately revealed | | |
| Reverse lookup works where supported, or unsupported state is recorded | | |
| Settings update/redeploy failure remains bounded and understandable | | |
| Backend restart/degraded recovery leaves the host app responsive | | |
| TypeDuck logs are under the TypeDuckIME log path | | |

## DPI Judgement

The user controls DPI scaling and judges directly in the VM.

| DPI scale | Available | Pass/Fail | Notes |
|-----------|-----------|-----------|-------|
| 100% | | | |
| 140% if available | | | |
| 175% | | | |
| 200% | | | |

## Multi-Monitor

Multi-monitor behavior was already human verified in Phase 5. Phase 7 does not reintroduce a multi-monitor automated visual matrix.

## Completion

Record observations in `host-app-dpi-notes.md`, then rerun:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckInteractiveReleaseChecklist.ps1 -RepoRoot . -Strict
```
