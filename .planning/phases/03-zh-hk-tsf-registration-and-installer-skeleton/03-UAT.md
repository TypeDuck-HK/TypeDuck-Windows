---
status: complete
phase: 03-zh-hk-tsf-registration-and-installer-skeleton
source: [03-VERIFICATION.md]
started: 2026-06-23T17:10:19Z
updated: 2026-06-23T17:29:51.062Z
---

# Phase 03 UAT: zh-HK TSF Registration and Installer Skeleton

## Current Test

number: none
name: [complete]
expected: |
  All UAT checkpoints for this phase have completed.
awaiting: [complete]

## Tests

### 1. Windows Settings Selectability

expected: TypeDuck 粵語輸入法 / TypeDuck Cantonese IME is selectable under Chinese (Traditional, Hong Kong).
result: pass
notes: User confirmed TypeDuck 粵語輸入法 / TypeDuck Cantonese IME is selectable under Chinese (Traditional, Hong Kong) in the VM Settings language/input UI.

### 2. Dual-Bitness Host Activation

expected: One 32-bit host and one 64-bit host can load/select the TypeDuck TSF profile without missing-COM or missing-DLL errors.
result: pass
notes: User confirmed one 32-bit host and one 64-bit host can load/select TypeDuck without missing-COM or missing-DLL errors. ASCII text output was observed, but engine behavior is outside this phase's TSF registration/installer skeleton scope.

## Summary

total: 2
passed: 2
issues: 0
pending: 0
skipped: 0
blocked: 0

## Gaps

none

## Observations

- Installer language picker showed duplicate English entries. Treat as non-blocking follow-up for installer localization rather than a Phase 03 acceptance failure.
- Installer still raises the known non-blocking Start Menu folder error for the bilingual shortcut folder path; this is already expected to be corrected in a later phase.
