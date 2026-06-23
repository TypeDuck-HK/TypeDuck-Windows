---
status: testing
phase: 03-zh-hk-tsf-registration-and-installer-skeleton
source: [03-VERIFICATION.md]
started: 2026-06-23T17:10:19Z
updated: 2026-06-23T17:10:19Z
---

# Phase 03 UAT: zh-HK TSF Registration and Installer Skeleton

## Current Test

number: 1
name: Windows Settings selectability
expected: |
  TypeDuck 粵語輸入法 / TypeDuck Cantonese IME is selectable under Chinese (Traditional, Hong Kong) in the VM Settings language/input UI.
awaiting: user response

## Tests

### 1. Windows Settings Selectability

expected: TypeDuck 粵語輸入法 / TypeDuck Cantonese IME is selectable under Chinese (Traditional, Hong Kong).
result: pending
notes: Get-WinUserLanguageList did not report zh-HK, so this needs a visual/user-flow check in the VM after install.

### 2. Dual-Bitness Host Activation

expected: One 32-bit host and one 64-bit host can load/select the TypeDuck TSF profile without missing-COM or missing-DLL errors.
result: pending
notes: The harness verified SysWOW64/System32 DLL registration and registry state, but not real host activation.

## Summary

total: 2
passed: 0
issues: 0
pending: 2
skipped: 0
blocked: 0

## Gaps
