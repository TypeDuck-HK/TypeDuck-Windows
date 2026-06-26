---
status: complete
phase: 05-candidate-dictionary-settings-and-about-ui-parity
verified_at: 2026-06-26T17:30:00+08:00
verification_basis: human-verification-and-static-guards
---

# Phase 05 Verification Closeout

## Verdict

Phase 5 is accepted as complete.

The final acceptance basis is:

- Human verification during iterative VM/manual testing of native candidate, dictionary, settings, About, tray, icon, and packaging behavior.
- Focused static/build/package guards that encode the final product contract.
- Explicit user-approved divergence from the earlier screenshot-manifest evidence plan.

## Important Note: Screenshot Evidence Retired

All Phase 5 screenshot tests are explicitly replaced by human verification. The `.planning/product/ui-fixtures` tree was removed by user direction and must not be recreated as a Phase 5 acceptance gate. Future phases may capture fresh screenshots for release evidence, but Phase 5 closeout does not depend on screenshot fixtures, screenshot manifests, or the retired aggregate screenshot validator path.

## Human-Verified Feature Coverage

### Candidate and Dictionary Panel

- Native candidate popup uses the TypeDuck visual direction: light surface, warm amber highlight, compact layout, readable HK Chinese/English fonts, measured text widths, and dynamic panel sizing.
- Candidate text, Jyutping/pronunciation, Chinese terms, comments/definitions, dictionary indicators, and compound/component rows render from structured candidate data.
- Lookup-filter payloads are parsed at the native renderer boundary; candidate transport remains pass-through and no Go-side lookup rewrite remains.
- TypeDuck-HK librime fork v1.1.4 x64 is the runtime basis for the current backend path; the earlier stale runtime mismatch was diagnosed and fixed.
- Multi-row entries work for cases such as `hou` and `housam`; rows are not capped to three lines.
- POS values render as dimmed rounded pills rather than bracket strings.
- Multilingual and `otherData` pipe-delimited values split into separate display lines with names aligned to first value lines.
- Input-buffer selection background aligns with selected candidate background; empty active input-buffer segments do not draw a one-pixel background.
- Candidate row height follows visible row content and avoids clipping long candidates, pronunciations, definitions, and body text.
- Number labels align to the first row baseline; info icons are centered.
- Page navigation buttons refresh the displayed candidate page and show hover/enabled/disabled state.

### Dictionary Panel Layout and Interaction

- Dictionary panel width adapts to content, including headword, pronunciation, pronunciation type, body, other-data, and other-language tables.
- Dictionary side padding and minimum height were tuned; non-scrolling minimum height remains 150 and scrolling viewport height is `max(320, left candidate panel height)`.
- Mouse-wheel scrolling works with a slightly wider scrollbar.
- Hover hit boxes cover visual gaps around candidates and info icons.
- Moving from a candidate with dictionary info to one without no longer leaves a black stale panel.
- Dictionary panel does not fallback to a different candidate's dictionary data.
- Static mouse position no longer changes hover while typing; hover resets to engine-selected state on candidate-list refresh until actual pointer movement crosses threshold.
- Gap between candidate panel and dictionary panel is zero because Win32 hit testing cannot reliably treat outside-window space as candidate hover.

### Settings

- Native Win32 settings dialog is the first-party settings surface and does not use Qt or backend-declared `configTool` metadata.
- Display Languages appears first, with separate main-language radio groups and display-language checkboxes.
- Page-size control shows 4-10 tick labels with correct padding; UI defaults remain 4-10 while candidate rendering does not cap externally customized schema page size.
- Dialog/control backgrounds are consistently white, section headings are bold/larger, fieldsets avoid clipping, and Confirm/Cancel are widened.
- User-facing copy removes technical `TypeDuckPreferences.json`, apply-status, and unsupported-state labels.
- Confirm saves/applies and closes; X follows the same confirm path; errors surface in an error dialog.
- UI preferences persist locally and load into the native candidate renderer.
- Rime-affecting settings write customization files and trigger fast incremental reload.
- Normal settings apply success notifications are suppressed; error notifications remain.

### About

- About is a separate `TypeDuckAbout.exe` executable.
- Settings no longer owns or exposes the broken About button.
- About launches from Start Menu entries and from the launcher tray menu.
- About is single-instance and foregrounds the existing window on duplicate launch.
- About displays product banner, bilingual intro/contact text, credit logos, SCOLAR/LANGUAGE FUND acknowledgement wording from the accepted attribution text, product version, TypeDuck-HK librime fork attribution, TypeDuck-HK schema attribution, and four product links.
- LearnDuck link label is `LearnDuck 粵拼打字入門 Introduction to Jyutping Typing`.

### Installer, Tray, Icons, and Resources

- Installer-first-run and post-install settings entry points launch fixed first-party `TypeDuckSettings.exe`.
- Settings/About duplicate launches foreground the existing window.
- Tray menu exposes `輸入法設定 / IME Settings` and `關於 / About TypeDuck...` in the expected order.
- TypeDuck executable icons are stamped into launcher, settings, About, setup helper, backend server, TSF profile/input picker, installer, and uninstaller surfaces.
- Legacy raw product-root icon staging is removed.
- Backend legacy images `mo.ico`, `mo.png`, `moqi.ico`, and `moqi.png` are removed; backend `icons` now carries the three TypeDuck `.ico` files.
- About and installer BMP files are frontend-owned under `TypeDuckSettings/resources` and staged under the product `resources` folder.

## Guard Coverage

The following guards were run during closeout or during the final implementation iterations:

- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckSettingsAboutUi.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckIconPackaging.ps1 -RepoRoot . -BackendRoot D:\VSProjects\moqi-ime -Strict`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckTsfIdentity.ps1 -RepoRoot . -Strict`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\Test-TypeDuckLauncherProtocol.ps1 -RepoRoot . -Strict`
- Candidate/theme/settings/data/window guards from Plans 05-01 through 05-13 remain part of the Phase 5 regression net.

The retired screenshot-manifest validator is not a Phase 5 acceptance gate.

## Closed Plans

| Plan | Status | Closeout |
|------|--------|----------|
| 05-01 | Complete | Theme/font contract implemented and guarded. |
| 05-02 | Complete | CandidateInfo/CandidateEntry parser and display data implemented. |
| 05-03 | Complete | Preferences persistence and Rime customization implemented. |
| 05-04 | Complete | Native candidate/dictionary popup implemented. |
| 05-05 | Complete | Native Settings and About foundations implemented. |
| 05-06 | Complete by explicit user divergence | Earlier screenshot/VM manifest evidence path retired; acceptance replaced by human verification. |
| 05-07 | Complete | Installer/post-install settings launch wired. |
| 05-08 | Complete | TypeDuck icons and resources packaged. |
| 05-09 | Complete | Rejected-UAT guards/provenance aligned. |
| 05-10 | Complete | Candidate parity and dictionary rendering repaired. |
| 05-11 | Complete | Settings layout/copy and separate About executable repaired. |
| 05-12 | Complete | Icon packaging/stamping cleaned. |
| 05-13 | Complete | Package/provenance/guard rebuild completed. |
| 05-14 | Skipped by explicit user direction | Screenshot/manual manifest plan retired; human verification accepted instead. |
| 05-15 | Complete by explicit user direction | Final closeout recorded in this file, UAT, summaries, and roadmap/state docs. |

## Residual Work Deferred Beyond Phase 5

- Broad scaffold cleanup, remaining Moqi/fcitx/cloud/AI/string/path audit, and privacy/logging work remain Phase 6 scope.
- Release-grade compatibility matrix, fresh screenshot capture if desired, multi-VM install/upgrade/uninstall, host-app matrix, and final artifact evidence remain Phase 7 scope.
- Phase 5 does not claim release-complete coverage; it closes native candidate/dictionary/settings/About parity by human verification and static guards.
