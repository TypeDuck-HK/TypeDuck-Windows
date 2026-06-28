---
status: complete
phase: 06-privacy-security-and-scaffold-cleanup
source:
  - 06-01-SUMMARY.md
  - 06-02-SUMMARY.md
  - 06-03-SUMMARY.md
  - 06-04-SUMMARY.md
  - 06-05-SUMMARY.md
  - 06-06-SUMMARY.md
  - 06-07-SUMMARY.md
started: 2026-06-27T11:59:01Z
updated: 2026-06-28T08:33:34Z
---

# Phase 6 UAT: Privacy, Security, and Scaffold Cleanup

## Current Test

[testing complete]

## Tests

### 1. TypeDuck Installer Identity
expected: Installer title, publisher, destination folder, Start Menu folder, shortcuts, and final messages use TypeDuck identity and bilingual Traditional Chinese/English wording.
result: pass
evidence:
  - `installer/MoqiTsf.iss`
  - `scripts/Test-TypeDuckInstallerSkeleton.ps1 -Strict`

### 2. No Simplified Inno Translation Dependency
expected: The installer does not depend on the removed Simplified Chinese Inno translation submodule, and installer-controlled user-facing strings are supplied directly by TypeDuck.
result: pass
evidence:
  - `.gitmodules`
  - `scripts/Test-TypeDuckInstallerSkeleton.ps1 -Strict`

### 3. Legacy Moqi Coexistence
expected: Installing or uninstalling TypeDuck does not delete or modify Legacy Moqi registry keys, startup entries, installed files, or processes.
result: pass
evidence:
  - TypeDuck-only installer cleanup guard
  - Phase 7 VM uninstall iteration

### 4. TypeDuckRuntime Package Shape
expected: The shipped runtime folder is `TypeDuckRuntime`, not `moqi-ime`, and it excludes Android, cloud clipboard, templates, tests, duplicate Rime icon folders, AI config, old `ime.json`, top-level `backends.json`, and the duplicate data-path `appearance_themes.json`.
result: pass
evidence:
  - `scripts/Test-TypeDuckRuntimePackagePruning.ps1 -Strict`
  - `D:\VSProjects\moqi-ime\scripts\build\TypeDuckRuntime`

### 5. Manifest-Free Runtime Bridge
expected: Launcher/backend startup no longer depends on packaged `backends.json`; the launcher constructs the fixed TypeDuck runtime bridge to `TypeDuckRuntime\server.exe`.
result: pass
evidence:
  - `MoqLauncher/PipeServer.cpp`
  - `scripts\Test-TypeDuckLauncherProtocol.ps1 -Strict`

### 6. Fixed First-Party Settings Surface
expected: TSF Configure, installer flow, tray settings, and launcher settings route to first-party `TypeDuckSettings.exe`, not backend-declared config tools.
result: pass
evidence:
  - `MoqiTextService/MoqiImeModule.cpp`
  - `scripts\Test-TypeDuckSettingsAboutUi.ps1 -Strict`

### 7. TypeDuck-Owned Log and Data Paths
expected: Runtime logs are under `%LOCALAPPDATA%\TypeDuckIME\Log`, launcher config is `TypeDuckLauncher.json`, preferences/custom Rime YAML are under `%APPDATA%\TypeDuckIME`, and old `%LOCALAPPDATA%\MoqiIM` / `%APPDATA%\Moqi` paths are not the product paths.
result: pass
evidence:
  - `scripts\Test-TypeDuckBackendDiagnostics.ps1 -Strict`
  - Phase 7 VM logs: `%LOCALAPPDATA%\TypeDuckIME\Log\TypeDuckBackend-2026-06-28.log`

### 8. Diagnostic Privacy
expected: Routine diagnostics are English, TypeDuck-owned, and avoid raw typed content by default; quote-pair/commit diagnostics are redacted to lengths or metadata where needed.
result: pass
evidence:
  - `scripts\Test-TypeDuckBackendDiagnostics.ps1 -Strict`
  - `.planning/product/privacy-security/phase-06-guard-results.json`

### 9. Removed Cloud/WebDAV/AI/Fcitx Surfaces
expected: User-facing v1 surfaces expose no cloud clipboard, WebDAV, AI, fcitx, scheme download/update, auto-pair, or custom phrase menus/commands.
result: pass
evidence:
  - `D:\VSProjects\moqi-ime\input_methods\rime\removed_surfaces_test.go`
  - `scripts\Test-TypeDuckBackendDiagnostics.ps1 -Strict`

### 10. Backend Menu Unreachable
expected: The stale backend `buildMenu` helper cannot reach the TypeDuck v1 menu surface; active `onMenu` returns no v1-visible backend menu items.
result: pass
evidence:
  - Backend commit `6841759 Make TypeDuck backend menu unreachable`
  - Frontend guard `cc39e64 test(06): guard backend menu payload localization`

### 11. Named Pipe and Frame Hardening
expected: The TypeDuck launcher pipe uses TypeDuck-owned namespace and explicit access rights, checks inspectable launcher identity, preserves denied-inspection compatibility, and rejects oversized frames.
result: pass
evidence:
  - `scripts\Test-TypeDuckLauncherProtocol.ps1 -Strict`
  - `ProtocolRecovery_test`

### 12. Release and Workflow Naming
expected: CI/release workflows use `TypeDuck-Windows`, `TypeDuck-Windows-backend`, `${{ github.repository_owner }}/schema`, `aap2-alpha`, and TypeDuck installer artifact names with no standalone schema artifact.
result: pass
evidence:
  - `.github/workflows/release.yml`
  - `.github/workflows/nightly.yml`
  - `scripts\Test-TypeDuckReleaseArtifacts.ps1 -Strict`

### 13. Appearance Theme Metadata Cleanup
expected: Packaged appearance themes contain product data only; development-only `source` metadata is removed and guarded.
result: pass
evidence:
  - Backend commit `c0686de Remove source metadata from appearance themes`
  - Frontend commit `0f591a6 Guard appearance themes against source metadata`

### 14. Runtime Icon Pruning Boundary
expected: Legacy Moqi image files and duplicate runtime icon folders remain pruned, while the only raw icon restored for the Windows system IME picker is `resources\TypeDuck_Small.ico`.
result: pass
evidence:
  - Commit `96f3453 Fix system IME picker icon packaging`
  - `scripts\Test-TypeDuckIconPackaging.ps1 -Strict`

## Summary

total: 14
passed: 14
issues: 0
pending: 0
skipped: 0
blocked: 0

## Gaps

[]
