---
status: complete
phase: 05-candidate-dictionary-settings-and-about-ui-parity
source:
  - 05-01-SUMMARY.md
  - 05-02-SUMMARY.md
  - 05-03-SUMMARY.md
  - 05-04-SUMMARY.md
  - 05-05-SUMMARY.md
  - 05-07-SUMMARY.md
  - 05-08-SUMMARY.md
  - 05-09-SUMMARY.md
  - 05-10-SUMMARY.md
  - 05-11-SUMMARY.md
  - 05-12-SUMMARY.md
  - 05-13-SUMMARY.md
  - 05-06-SUMMARY.md
  - 05-14-SUMMARY.md
  - 05-15-SUMMARY.md
started: 2026-06-24T06:50:53.0216361Z
updated: 2026-06-26T17:30:00+08:00
verification_basis: user-human-verification
---

## Current Test

[testing complete]

## Important Verification Note

All Phase 5 screenshot-based tests and screenshot fixture artifacts were explicitly retired by user direction on 2026-06-26. The entire `.planning/product/ui-fixtures` tree was removed. Phase 5 visual acceptance is now recorded as human verification from iterative VM/manual testing plus source-backed static guards. Screenshots remain useful for future exploratory debugging only, but they are no longer the acceptance mechanism for Phase 5.

## Tests

### 1. Candidate Panel Visual Tone and Sizing
expected: Native candidate panel uses TypeDuck visual tone, warm highlight, compact layout, correct fonts, measured text widths, and dynamic sizing without clipped long candidates or over-wide short candidate windows.
result: pass
closure: "Fixed through the candidate/dictionary iteration: panel colors landed, widths are content-measured without imposed max width, `measureWithFont` gained safety slack, long pronunciation/body strings stop clipping, Chinese/English fonts no longer fall back to monospace, input-buffer left edge aligns with selected candidate background, and zero-width active input-buffer segments hide their background."
regression_notes:
  - Do not return to fixed-width candidate panels or char-count text estimates.
  - Do not cap candidate rows to three rows.
  - Do not let the input buffer clip or render a one-pixel background for empty selected text.

### 2. Candidate Dictionary Row Structure
expected: Candidate entries support multi-row dictionary details, including one candidate spanning readings/translations or component rows such as `hou` and `housam`.
result: pass
closure: "Fixed by parsing TypeDuck lookup payloads into structured `CandidateEntry` rows, rendering multi-line candidate rows, preserving component rows such as 好心 and 你, removing the accidental fallback translation for rows without a translation, and splitting each pipe-delimited `otherData` field into one displayed line."
regression_notes:
  - `hou` must allow 好 to show hou2 and hou3 information.
  - `housam`/好心你 must support component rows and dictionary panel details.
  - `CandidateEntry::otherData` names align to the first value line.

### 3. Candidate Data Source and Runtime Parity
expected: Native candidates and dictionary comments come from the intended TypeDuck runtime and lookup filter, not stale Rime/Moqi paths or intercepted candidate substitutions.
result: pass
closure: "Diagnosed stale runtime evidence, upgraded the bundled runtime to TypeDuck-HK librime fork v1.1.4 x64, removed the abandoned Go-side lookup workaround, preserved candidate transport pass-through, and relied on the C++ lookup filter for sentence-component dictionary data."
regression_notes:
  - Do not reintroduce Go-side dictionary lookup rewrites.
  - Keep lookup parsing at the native renderer boundary.
  - Runtime bitness must match the backend host process; x86 DLLs fail in the current x64 backend path.

### 4. Candidate Baselines, Numbering, and Icons
expected: Candidate numbering aligns with the first-row candidate baseline, comments/definitions align cleanly, and dictionary info icons are centered and remain clickable/hoverable without gaps.
result: pass
closure: "Adjusted row metrics so candidate height follows the tallest visible row, not the tallest multi-row candidate block. Number labels were shrunk and first-row-baseline aligned; info icons were centered; hit boxes were expanded so hover does not drop in the gaps around icons or highlight backgrounds."
regression_notes:
  - The visual highlight and dictionary hover area must be larger than the painted content where needed.
  - Do not allow a one-pixel gap to clear hover and close the dictionary panel.

### 5. Part-of-Speech and Multilingual Rendering
expected: POS values render as dimmed rounded bordered pills, multilingual rows are supported, and labels/values align without bracket-string fallbacks.
result: pass
closure: "Replaced literal bracketed POS text with rounded GDI pills, aligned pill baselines near the related text baseline, split multi-language/other-data pipe values into lines, and guarded multilingual candidate data paths."
regression_notes:
  - POS must remain structured pills, not `[adjective]` text.
  - Multilingual rows must remain data-backed, not screenshot-only.

### 6. Dictionary Panel Layout and Scrolling
expected: Dictionary panel dynamically sizes to content, avoids unnecessary minimum width, handles long content without clipping, and supports scrolling for tall content without repaint artifacts.
result: pass
closure: "Adjusted dictionary panel width measurement to consider headword, pronunciation, pronunciation type, body text, other-data, and other-language tables; removed temporary max-width caps; reduced side padding; kept non-scrolling minimum height at 150; set scrolling viewport height to `max(320, left candidate panel height)` when needed; widened the scrollbar by one pixel; added mouse-wheel scrolling; removed black/white repaint artifacts; and positioned the dictionary panel bottom at least at the hovered candidate bottom while preserving current height."
regression_notes:
  - Do not fallback to another candidate's dictionary info when the highlighted candidate has none.
  - The dictionary panel must disappear rather than show a black rectangle when moving to a candidate without dictionary data.
  - Gap between candidate and dictionary panels is zero because Win32 hit testing cannot cover outside the candidate window reliably.

### 7. Hover, Highlight, and Pointer Stability
expected: Mouse hover updates highlight only after real movement, never because a static pointer happens to sit over a candidate while typing.
result: pass
closure: "Reset hover index to `-1` on candidate-list refresh, ignored static pointer presence until movement crossed the threshold, preserved engine-selected highlight while typing, and made debug mode optionally keep dictionary details visible for capture."
regression_notes:
  - Do not recompute hover solely from cursor position during candidate refresh.
  - Keep the movement threshold behavior; it prevents flicker during typing.
  - Mouse hover still overrides engine selection visually after actual movement.

### 8. Page Navigation
expected: Page navigation buttons reflect enabled/disabled state, hover with a rounded background, are vertically centered, and click through to refresh the displayed candidate page.
result: pass
closure: "Fixed stale frontend refresh after backend page updates, reverted the PageUp/PageDown fallback, adjusted nav glyph size/positioning, and added rounded hover background behavior."
regression_notes:
  - Page buttons must call the backend command path and refresh frontend state.
  - Do not rely on keyboard fallback for mouse page navigation.

### 9. Settings Dialog Layout and Copy
expected: Settings dialog is white, uses distinct radio groups, has Display Languages first, correct page-size tick labels, no technical persistence copy, no unsupported-state placeholder copy, and no clipped headings/buttons.
result: pass
closure: "Made controls/background consistently white, grouped radio sets correctly, swapped display-language radio/checkbox positions, widened Confirm/Cancel and Sung labels, fixed fieldset and heading clipping, padded page-size tick labels, kept tick labels 4-10 in UI defaults, and removed technical `TypeDuckPreferences.json`, apply-status, and unsupported-state labels."
regression_notes:
  - Win32 radio groups must start with `WS_GROUP`; all radios cannot share one set.
  - UI defaults remain 4-10, while candidate rendering must not cap externally customized schema page sizes.
  - Closing by X follows the Confirm/OK path.

### 10. Settings Apply and Rime Customization
expected: Settings save locally, apply through the launcher, write the correct Rime customization levers, and trigger fast deploy/reload behavior without noisy success notifications.
result: pass
closure: "Routed Settings confirmation through launcher IPC, wrote `default.custom.yaml` and `common.custom.yaml` via Rime customization/file fallback where needed, added fast incremental reload matching legacy TypeDuck behavior, suppressed normal apply-complete notifications, retained error notifications, and separated manual full redeploy from settings application."
regression_notes:
  - `showRomanization` must not suppress `candidateInfo.note`; TypeDuck Web logic is `!index && (!info.isReverseLookup || prefs.showReverseCode) && info.note`.
  - `isHeiTypeface` must not affect the input buffer, which remains Hei.
  - Manual redeploy must directly call the deploy path and keep start/success/error notification semantics.

### 11. Installer and Post-Install Settings Entry Points
expected: Installer-first-run and post-install entry points launch the first-party settings dialog and seed defaults without backend-declared config tool metadata.
result: pass
closure: "Staged `TypeDuckSettings.exe`, launched it during setup/post-install, preserved `/apply-defaults` for default preference seeding, and rejected generic backend `configTool` launch paths."
regression_notes:
  - Settings entry point must remain fixed and first-party.
  - Default preferences are seeded during installation if missing.

### 12. About Surface
expected: About is a separate executable with one visible instance, product banner/logos/text/links/version/attribution, and tray/menu/start access; Settings no longer owns a broken About button.
result: pass
closure: "Moved About to `TypeDuckAbout.exe`, removed Settings-panel About coupling, redesigned the About surface with larger text, product/credit bitmaps, SCOLAR/LANGUAGE FUND acknowledgement wording from current attribution copy, concise runtime/schema attribution, four links, separate Start Menu shortcuts, tray menu access under settings, and duplicate-instance foregrounding."
regression_notes:
  - Do not re-add the broken Settings-panel About button.
  - About must remain a separate executable and single-instance.
  - LearnDuck link label is `LearnDuck 粵拼打字入門 Introduction to Jyutping Typing`.

### 13. Icon and Resource Packaging
expected: TypeDuck icons are stamped into executables/installer/TSF profile surfaces, legacy Moqi icons are removed, and About BMPs are frontend-owned resources.
result: pass
closure: "Stamped TypeDuck icons into launcher/settings/About/setup-helper/backend server, restored the system IME menu profile icon using packaged `TypeDuck_Small.ico` with DLL fallback, removed legacy raw app-root TypeDuck icon leakage, removed legacy Rime `icon.ico`, removed backend `mo.ico`, `mo.png`, `moqi.ico`, and `moqi.png`, committed the remaining TypeDuck `.ico` files in the backend, and moved About/Installer BMP files into `TypeDuckSettings/resources` staged under `{app}/resources`."
regression_notes:
  - The installed app root must not contain raw standalone product icon files.
  - Backend `icons` should contain TypeDuck `.ico` assets but not Moqi image assets or About BMP resources.
  - About/Installer BMPs are frontend resources.

### 14. Launcher Tray and Duplicate Windows
expected: Tray right-click exposes settings and About entries, and repeated Settings/About launches foreground the existing window instead of opening duplicates.
result: pass
closure: "Added `關於 / About TypeDuck...` under `輸入法設定 / IME Settings` in the launcher tray menu, wired it to `TypeDuckAbout.exe`, and added mutex/foreground handling for Settings and About."
regression_notes:
  - Tray menu wording must remain bilingual.
  - Duplicate interactive Settings/About windows are rejected and existing windows are foregrounded.

### 15. Evidence Model and Screenshot Fixture Retirement
expected: Phase 5 verification no longer depends on stale screenshot manifests; all screenshot tests are explicitly replaced by human verification.
result: skipped
reason: "User explicitly directed that all screenshot tests are replaced by human verification and requested removal of `.planning/product/ui-fixtures`."
closure: "Removed `.planning/product/ui-fixtures` entirely and recorded this UAT/verification divergence. Plans 05-06, 05-14, and 05-15 are closed as explicit user-approved divergence from the earlier screenshot-manifest validator path."
regression_notes:
  - Do not recreate `.planning/product/ui-fixtures` as Phase 5 acceptance evidence.
  - Future release verification may collect screenshots again, but Phase 5 acceptance is human-verified.

### 16. Automated Guard Coverage
expected: Static/build/package guards prevent the fixed Phase 5 regressions from silently returning.
result: pass
closure: "Updated and ran focused guards for settings/About UI, icon packaging, TSF identity, launcher protocol, candidate data, candidate window rendering, settings persistence, appearance theme, and package/resource staging during the Phase 5 iterations."
regression_notes:
  - Keep `scripts/Test-TypeDuckSettingsAboutUi.ps1` and `scripts/Test-TypeDuckIconPackaging.ps1` aligned with the final product contract.
  - Screenshot-manifest aggregate validation is retired for Phase 5 closeout.

## Summary

total: 16
passed: 15
issues: 0
pending: 0
skipped: 1
blocked: 0

## Closure Record

Original UAT gaps 1-14 were fixed through Phase 5 implementation and iteration. Original evidence gaps 11-12 and the screenshot-manifest portions of gaps 10, 13, and 15 were explicitly resolved by user-approved divergence: screenshot tests and `.planning/product/ui-fixtures` are retired and replaced by human verification.

## Gaps

[none open]
