# TypeDuck Phase 7 Host-App and DPI Notes

No screenshots required. Record direct observations only.

## Session

- Tester: User, direct interactive VM judgement
- UTC date: 2026-06-28
- VM name/checkpoint: SHIOYA-INOBE / My Virtual Machine; checkpoint name not recorded
- Windows version: Windows 10 guest observed in VM; exact build not recorded
- Installer SHA-256: `B5EFBCFC8620E83B2DD9E83B0D8D647F685B3882B4D793510A80BA3610C378CE`
- Install evidence: `.planning/product/release-fixtures/phase-07/install-verification-notes.md`
- Protocol recovery evidence: `.planning/product/release-fixtures/phase-07/protocol-recovery-results.json`
- Command/log evidence: `.planning/product/release-fixtures/phase-07/verification-notes.md`; `%LOCALAPPDATA%\TypeDuckIME\Log\TypeDuckBackend-2026-06-28.log`; `%LOCALAPPDATA%\TypeDuckIME\Log\TypeDuckLauncher-2026-06-28.log`

## Host-App Results

| Host target | Pass/Fail/Skipped | Notes |
|-------------|-------------------|-------|
| Notepad | pass | Ordinary Cantonese typing, candidate list, candidate click/space commit, dictionary detail, and settings refresh remained usable. |
| Browser text field | pass | Typing and candidate UI behaved normally in a browser text field. |
| Office or Office-like app | pass | Office-like host accepted composition and committed text without focus loss. |
| Terminal/console context | pass | Console-style host remained responsive; TypeDuck did not hang the host when typing or recovering. |
| Elevated app | pass | Elevated host scenario passed without broad legacy cleanup or launcher breakage. |
| Awkward TSF host available in VM | pass | Awkward host scenario remained usable after candidate popup sizing and font fallback repairs. |

## DPI Results

| DPI scale | Pass/Fail/Skipped | Notes |
|-----------|-------------------|-------|
| 100% | pass | Candidate and dictionary popup sizes, text, and controls remained coherent. |
| 140% if available | pass | Fractional scaling was judged interactively by the user; no screen capture was used. |
| 175% | pass | Popup sizing stayed stable after the Explorer DPI sizing fix. |
| 200% | pass | Candidate rows and dictionary text remained readable without overlap. |

## Typing and Recovery Notes

| Check | Pass/Fail/Skipped | Notes |
|-------|-------------------|-------|
| Ordinary Cantonese typing | pass | TypeDuck starts composition and commits Cantonese text. |
| Candidate list | pass | Candidate list appears; space/numeric and mouse-click candidate commit both work after the click-commit repair. |
| Dictionary detail | pass | Dictionary detail appears when deliberately revealed and uses the accepted Chinese font fallback. |
| Reverse lookup where supported | pass | Supported reverse lookup behavior was accepted; unsupported/conditional cases remain recorded in protocol evidence rather than fabricated. |
| Settings update/redeploy failure | pass | Settings application now starts the launcher through the installer flow; failures remain bounded and understandable. |
| Backend restart/degraded recovery | pass | Host apps stayed responsive through restart/degraded recovery checks; technical details remained in logs/evidence. |
| TypeDuck log path capture | pass | Logs were created under `%LOCALAPPDATA%\TypeDuckIME\Log`, including TypeDuck backend and launcher logs. |

## Multi-Monitor Disposition

Phase 5 already human verified multi-monitor behavior. Phase 7 does not require new multi-monitor screenshot automation or visual comparison.

## Issues

No remaining host-app or DPI issues were reported after the final interactive VM check.
