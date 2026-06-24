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
  - partial Phase 05 VM evidence recorded in bcea3db
started: 2026-06-24T06:50:53.0216361Z
updated: 2026-06-24T06:50:53.0216361Z
---

## Current Test

[testing complete - user rejection recorded; diagnosis and retry pending]

## Tests

### 1. Candidate Panel Visual Fidelity
expected: Candidate panel screenshots match TypeDuck Web alpha spacing, padding, margins, sizing units, and preview/VM behavior.
result: issue
reported: "Candidate-* screenshots look eye-balled or affected by DPI/unit mismatch, and the VM screenshots differ from the candidate preview screenshots."
severity: cosmetic

### 2. Candidate Dictionary Row Structure
expected: Candidate entries support TypeDuck Web alpha multi-row dictionary detail, including one candidate spanning multiple readings/translations or component rows.
result: issue
reported: "The panel appears to assume one row per candidate. For `hou`, 好 should span hou2 and hou3 rows; for `housam`, 好心你 can span 3 rows including component rows 好心 and 你."
severity: major

### 3. Candidate Input Buffer and Baseline Alignment
expected: Input buffer background/text colors match TypeDuck Web alpha, text is not clipped, and candidate numbering/comment text baselines align with the main candidate text.
result: issue
reported: "Input buffer background and text colors are incorrect and clipped in reverse-lookup-cangjie-onf.bmp. The numbering label and translation/comment text should align to the candidate main-text baseline."
severity: cosmetic

### 4. Part-of-Speech and Multilingual Rendering
expected: Parts of speech render as dimmed rounded bordered pills, not literal bracketed strings, and multilingual candidate detail is covered by evidence.
result: issue
reported: "Parts of speech should be in dimmed rounded bordered boxes instead of literal `[...]` strings. Multilingual is not tested."
severity: major

### 5. Candidate Data Source Parity
expected: Native candidates are identical to TypeDuck Web alpha for the same input, using the intended TypeDuck backend/runtime and dictionary lookup data.
result: issue
reported: "Candidates shown are not identical to TypeDuck-Web; possible old rime.dll or Moqi middle-end candidate interception needs investigation."
severity: major

### 6. Settings Dialog Visual Hierarchy and Layout
expected: Settings dialog controls blend with the dialog background, section headings are slightly larger and bold, labels stay inside their groups, and all text is readable without clipping.
result: issue
reported: "Control backgrounds are grey while the whole dialog is not; section headings need larger bold styling; some text is clipped; the 主要語言 label goes outside its rectangle and does not point to radio buttons correctly."
severity: cosmetic

### 7. Settings Dialog Controls and Copy
expected: Page-size slider shows tick labels 4 through 10, user-facing copy avoids implementation details, and unsupported-state text is removed unless a real user-facing unsupported case exists.
result: issue
reported: "The slider should show 4 5 6 7 8 9 10 under each tick instead of current value; remove the TypeDuckPreferences.json technical label; clarify/remove unsupported copy because the product should not expose unsupported cases."
severity: major

### 8. About Surface
expected: About is accessible and visibly captured, or is moved to a separate executable with the Settings Panel button removed.
result: issue
reported: "The About button does not work at all; vm-about-dialog.bmp is the wrong screenshot. Prefer a separate About executable and remove the Settings Panel About button."
severity: major

### 9. Icon Packaging and Executable Branding
expected: Installation does not copy standalone icon files into the product directory, removes legacy rime icon.ico, and stamps TypeDuck icons into TypeDuckLauncher.exe and bundled server.exe.
result: issue
reported: "Icon files should not be copied into the installation directory; moqi-ime/input_methods/rime/icon.ico should be removed; TypeDuckLauncher.exe and moqi-ime/server.exe still use stale Moqi icons."
severity: major

### 10. Evidence Screenshot Hygiene
expected: Evidence screenshots and manifest slots only reference accurate, non-cropped, current proof images.
result: issue
reported: "Remove cropped settings-two-column-layout.bmp because vm-settings-apply-persistence.bmp replaces it, and remove the incorrect vm-about-dialog.bmp screenshot."
severity: major

### 11. Phase 05 Evidence Completion
expected: Plan 05-06 has a real completion summary and the aggregate Phase 5 evidence validator passes without missing VM/manual evidence slots.
result: issue
reported: "Verifier found 05-06-SUMMARY.md is still absent and the aggregate evidence plan remains unexecuted for required VM/manual evidence."
severity: blocker

### 12. Required VM Evidence Coverage
expected: VM evidence covers visible installer-first-run settings, restart persistence, browser host-app candidates, movement reveal, high DPI, multi-monitor or extended desktop placement, UI-less TSF host behavior, imperfect composition-rectangle fallback, About replacement, and input picker/executable icons.
result: issue
reported: "Verifier found several required VM evidence gaps were only in the manifest/manual notes and not structured UAT gaps."
severity: blocker

### 13. Guard Alignment With Rejected UAT
expected: Automated Phase 5 guards encode the accepted product contract, not rejected behavior.
result: issue
reported: "Verifier found existing guards still pass or expect rejected behavior, including TypeDuckPreferences.json technical copy, unsupported-state copy, raw staged .ico files, and removed/cropped screenshot slots."
severity: blocker

### 14. Candidate Rendering Test Depth
expected: Candidate rendering tests validate `hou`, `housam`, POS pills, multilingual rows, baselines, input buffer clipping, and native/Web-alpha visual parity.
result: issue
reported: "Verifier found candidate window tests are mostly regex anchors and do not catch the rejected visual/data behavior."
severity: major

### 15. Runtime Source Provenance
expected: Retry evidence records exact backend/runtime and TypeDuck Web fixture provenance and does not silently rely on unrecorded dirty external state.
result: issue
reported: "Verifier found D:\\VSProjects\\moqi-ime and I:\\GitHub\\TypeDuck-Web are dirty enough to invalidate candidate/icon/runtime evidence if provenance is not recorded."
severity: major

## Summary

total: 15
passed: 0
issues: 15
pending: 0
skipped: 0
blocked: 0

## Gaps

- truth: "Candidate panel screenshots match TypeDuck Web alpha spacing, padding, margins, sizing units, and preview/VM behavior."
  status: failed
  reason: "User reported candidate-* screenshots look eye-balled or affected by DPI/unit mismatch, and VM screenshots differ from candidate preview screenshots."
  severity: cosmetic
  test: 1
  root_cause: ""
  artifacts:
    - path: ".planning/product/ui-fixtures/phase-05/screenshots"
      issue: "Candidate preview and VM screenshots need visual comparison against TypeDuck Web alpha."
  missing:
    - "Calibrated native candidate panel layout against TypeDuck Web fixtures and VM captures."
  debug_session: ""

- truth: "Candidate entries support TypeDuck Web alpha multi-row dictionary detail, including one candidate spanning multiple readings/translations or component rows."
  status: failed
  reason: "User reported apparent one-row-per-candidate rendering; `hou` should show 好 across hou2 and hou3, while `housam` should show 好心你 with component rows 好心 and 你."
  severity: major
  test: 2
  root_cause: ""
  artifacts:
    - path: ".planning/product/web-alpha-fixtures/2026-06-23/screenshots/dictionary-detail-housam-second-candidate-multilingual-indonesian-main-desktop-1280x720.png"
      issue: "Reference fixture demonstrates multi-row/component dictionary details."
  missing:
    - "Native data model and renderer support for multi-row candidate dictionary details."
  debug_session: ""

- truth: "Input buffer background/text colors match TypeDuck Web alpha, text is not clipped, and candidate numbering/comment text baselines align with the main candidate text."
  status: failed
  reason: "User reported incorrect/clipped input buffer colors and baseline misalignment between numbering, translation/comment text, and main candidate text."
  severity: cosmetic
  test: 3
  root_cause: ""
  artifacts:
    - path: ".planning/product/ui-fixtures/phase-05/screenshots/reverse-lookup-cangjie-onf.bmp"
      issue: "Input buffer color and clipping regression evidence."
  missing:
    - "Native renderer text metrics and baseline alignment matching TypeDuck Web."
  debug_session: ""

- truth: "Parts of speech render as dimmed rounded bordered pills, not literal bracketed strings, and multilingual candidate detail is covered by evidence."
  status: failed
  reason: "User reported literal `[...]` part-of-speech text and no multilingual testing."
  severity: major
  test: 4
  root_cause: ""
  artifacts:
    - path: ".planning/product/web-alpha-fixtures/2026-06-23/screenshots"
      issue: "Need multilingual and part-of-speech visual fixtures in native evidence."
  missing:
    - "Part-of-speech pill renderer and multilingual candidate evidence."
  debug_session: ""

- truth: "Native candidates are identical to TypeDuck Web alpha for the same input, using the intended TypeDuck backend/runtime and dictionary lookup data."
  status: failed
  reason: "User reported native candidates are not identical to TypeDuck-Web and suspects old rime.dll or Moqi middle-end interception."
  severity: major
  test: 5
  root_cause: ""
  artifacts:
    - path: "MoqLauncher"
      issue: "Launcher/backend candidate path may still transform or source candidates incorrectly."
    - path: "moqi-ime"
      issue: "Bundled runtime may include stale Rime or legacy data."
  missing:
    - "End-to-end parity proof for native candidates versus TypeDuck Web alpha."
  debug_session: ""

- truth: "Settings dialog controls blend with the dialog background, section headings are slightly larger and bold, labels stay inside their groups, and all text is readable without clipping."
  status: failed
  reason: "User reported grey controls on non-grey background, insufficient section heading emphasis, clipped text, and 主要語言 label escaping/mispointing."
  severity: cosmetic
  test: 6
  root_cause: ""
  artifacts:
    - path: ".planning/product/ui-fixtures/phase-05/screenshots/vm-settings-apply-persistence.bmp"
      issue: "Current replacement evidence for settings layout."
  missing:
    - "Settings layout pass for visual hierarchy, radio grouping, and clipped text."
  debug_session: ""

- truth: "Page-size slider shows tick labels 4 through 10, user-facing copy avoids implementation details, and unsupported-state text is removed unless a real user-facing unsupported case exists."
  status: failed
  reason: "User requested tick labels 4-10, removal of TypeDuckPreferences.json implementation copy, and removal/justification of unsupported-state copy."
  severity: major
  test: 7
  root_cause: ""
  artifacts:
    - path: "MoqiTextService"
      issue: "Settings UI copy and controls need user-facing refinement."
  missing:
    - "Slider tick labels and final product copy."
  debug_session: ""

- truth: "About is accessible and visibly captured, or is moved to a separate executable with the Settings Panel button removed."
  status: failed
  reason: "User reported About button does nothing; vm-about-dialog.bmp is wrong; user prefers a separate About executable and no Settings Panel About button."
  severity: major
  test: 8
  root_cause: ""
  artifacts:
    - path: ".planning/product/ui-fixtures/phase-05/screenshots/vm-about-dialog.bmp"
      issue: "Incorrect evidence screenshot should be removed."
  missing:
    - "Working About entry point and evidence."
  debug_session: ""

- truth: "Installation does not copy standalone icon files into the product directory, removes legacy rime icon.ico, and stamps TypeDuck icons into TypeDuckLauncher.exe and bundled server.exe."
  status: failed
  reason: "User reported stray icon files in install directory, legacy moqi-ime/input_methods/rime/icon.ico, stale TypeDuckLauncher.exe icon, and Moqi icon on bundled server.exe."
  severity: major
  test: 9
  root_cause: ""
  artifacts:
    - path: "scripts/install.ps1"
      issue: "Packaging may stage raw icon files."
    - path: "installer"
      issue: "Installer may include raw icon assets or stale executable resources."
    - path: "D:/VSProjects/moqi-ime"
      issue: "Bundled backend server icon/resource may need stamping or replacement."
  missing:
    - "Install tree icon cleanup and executable icon verification."
  debug_session: ""

- truth: "Evidence screenshots and manifest slots only reference accurate, non-cropped, current proof images."
  status: failed
  reason: "User requested removal of cropped settings-two-column-layout.bmp and incorrect vm-about-dialog.bmp."
  severity: major
  test: 10
  root_cause: ""
  artifacts:
    - path: ".planning/product/ui-fixtures/phase-05/screenshots/settings-two-column-layout.bmp"
      issue: "Cropped screenshot should be removed."
    - path: ".planning/product/ui-fixtures/phase-05/screenshots/vm-about-dialog.bmp"
      issue: "Incorrect screenshot should be removed."
  missing:
    - "Evidence manifest cleanup after screenshot removal."
  debug_session: ""

- truth: "Plan 05-06 has a real completion summary and the aggregate Phase 5 evidence validator passes without missing VM/manual evidence slots."
  status: failed
  reason: "Verifier reported 05-06-SUMMARY.md is absent and the evidence validator still has missing VM/manual slots."
  severity: blocker
  test: 11
  root_cause: ""
  artifacts:
    - path: ".planning/phases/05-candidate-dictionary-settings-and-about-ui-parity/05-06-PLAN.md"
      issue: "Evidence plan remains incomplete."
    - path: "scripts/Test-TypeDuckPhase05UiEvidence.ps1"
      issue: "Strict validator still fails because final evidence is missing or stale."
  missing:
    - "Complete 05-06 only after accepted evidence is captured and strict validator passes."
    - "Create 05-06-SUMMARY.md after the evidence plan is genuinely complete."
  debug_session: ""

- truth: "VM evidence covers visible installer-first-run settings, restart persistence, browser host-app candidates, movement reveal, high DPI, multi-monitor or extended desktop placement, UI-less TSF host behavior, imperfect composition-rectangle fallback, About replacement, and input picker/executable icons."
  status: failed
  reason: "Verifier reported required VM evidence gaps were only in the manifest/manual notes and not structured UAT gaps."
  severity: blocker
  test: 12
  root_cause: ""
  artifacts:
    - path: ".planning/product/ui-fixtures/phase-05/phase05-ui-evidence.json"
      issue: "Manifest marks multiple required VM evidence slots missing."
    - path: ".planning/product/ui-fixtures/phase-05/manual-uat-notes.md"
      issue: "Manual notes list missing evidence, including restart persistence and browser host evidence."
  missing:
    - "Visible installer-first-run settings capture."
    - "Restart persistence capture."
    - "Browser host-app candidate capture."
    - "Movement reveal capture."
    - "High-DPI, multi-monitor, UI-less host, and imperfect composition rectangle captures."
    - "Visible input picker and executable icon capture."
  debug_session: ""

- truth: "Automated Phase 5 guards encode the accepted product contract, not rejected behavior."
  status: failed
  reason: "Verifier reported guards still accept rejected TypeDuckPreferences.json copy, unsupported-state copy, raw staged .ico files, and removed/cropped screenshot requirements."
  severity: blocker
  test: 13
  root_cause: ""
  artifacts:
    - path: "scripts/Test-TypeDuckSettingsAboutUi.ps1"
      issue: "Guard expects rejected settings copy."
    - path: "scripts/Test-TypeDuckIconPackaging.ps1"
      issue: "Guard expects raw app-root icon files."
    - path: "scripts/Test-TypeDuckPhase05UiEvidence.ps1"
      issue: "Guard references removed/cropped evidence slots."
  missing:
    - "Update tests before retry so old rejected behavior cannot pass."
  debug_session: ""

- truth: "Candidate rendering tests validate `hou`, `housam`, POS pills, multilingual rows, baselines, input buffer clipping, and native/Web-alpha visual parity."
  status: failed
  reason: "Verifier reported candidate tests are too shallow and still miss the rejected C++ POS bracket rendering and layout problems."
  severity: major
  test: 14
  root_cause: ""
  artifacts:
    - path: "scripts/Test-TypeDuckCandidateWindow.ps1"
      issue: "Regex-only guard does not validate rejected rendering behavior."
    - path: "MoqiTextService/MoqiCandidateWindow.cpp"
      issue: "Part-of-speech still renders as bracketed text."
  missing:
    - "Rendering-level or screenshot guard for multi-row details, POS pills, baselines, and input buffer clipping."
  debug_session: ""

- truth: "Retry evidence records exact backend/runtime and TypeDuck Web fixture provenance and does not silently rely on unrecorded dirty external state."
  status: failed
  reason: "Verifier reported D:\\VSProjects\\moqi-ime and I:\\GitHub\\TypeDuck-Web are dirty enough to invalidate evidence unless provenance is recorded."
  severity: major
  test: 15
  root_cause: ""
  artifacts:
    - path: "D:/VSProjects/moqi-ime"
      issue: "Sibling backend dirty state affects runtime and icon evidence."
    - path: "I:/GitHub/TypeDuck-Web"
      issue: "Fixture authority is dirty relative to pinned fixture commit."
  missing:
    - "Record exact commits and dirty state before rebuild/retry evidence."
    - "Ensure candidate parity is tested against the intended TypeDuck Web alpha fixture/source."
  debug_session: ""
