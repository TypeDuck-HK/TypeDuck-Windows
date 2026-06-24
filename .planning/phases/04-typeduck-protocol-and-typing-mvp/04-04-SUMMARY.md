---
phase: 04-typeduck-protocol-and-typing-mvp
plan: 04
subsystem: typing-mvp-verification
status: complete
completed: 2026-06-24
requires:
  - 04-01-SUMMARY.md
  - 04-02-SUMMARY.md
  - 04-03-SUMMARY.md
provides:
  - Live Windows VM acceptance evidence for the typing MVP.
  - Latest installer artifact with TypeDuck schema payload.
  - Packaging rule that TypeDuck schemas belong in the sibling moqi-ime runtime, not this repo.
affects:
  - Phase 5 candidate/settings UI
  - Phase 6 scaffold cleanup
  - Phase 7 release verification
---

# Phase 4 Plan 04 Summary

## Result

Wave 4 is complete and Phase 4 is accepted on live VM evidence.

The original Wave 4 proof runner produced useful static guards and evidence artifacts, but the live debugging session became the real acceptance path. The user installed the rebuilt installer in the VM, confirmed TypeDuck no longer outputs ASCII, then confirmed the TypeDuck schema drop-in replacement works. That satisfies the Phase 4 requirement for a targeted Windows host-process typing smoke test.

The strict proof artifact is intentionally not represented as a full machine pass. Several recovery obligations are still static guards or future fault-injection probes rather than executable end-to-end runtime probes.

## What Changed

- Diagnosed the VM failure where TypeDuck selected correctly but typed ASCII.
- Fixed the installed runtime path where the TSF DLL attempted to launch `MoqiLauncher.exe` instead of the deployed `TypeDuckLauncher.exe`.
- Added tray icon failure logging and changed the tray tooltip/menu/window strings to TypeDuck bilingual copy.
- Fixed the release build blockers around protobuf/zlib and MSVC runtime propagation.
- Rebuilt the sibling `moqi-ime` runtime and the Windows installer.
- Replaced the previous Mandarin `rime-frost` schema payload with the TypeDuck Web schema source via the sibling `moqi-ime` build script.
- Confirmed the packaged TypeDuck Rime data includes built artifacts such as `jyut6ping3.table.bin`, `jyut6ping3.prism.bin`, and `jyut6ping3.reverse.bin`.

## Installer Artifact

- Path: `D:\VSProjects\moqi-im-windows\installer\dist\typeduck-windows-ime-setup.exe`
- SHA-256: `3F09C9BD16A62B09CB44F0A356DC23A120D71FF5466FD80C3492F54185773A4B`
- Size: `22302964` bytes
- Payload source used for the final rebuild: `D:\VSProjects\moqi-ime\scripts\build\moqi-ime`

## Validation

Commands and checks used during closeout:

- `pwsh -NoProfile -ExecutionPolicy Bypass -File D:\VSProjects\moqi-ime\scripts\build.ps1 -RepoRoot D:\VSProjects\moqi-ime -RimeDataSource I:\GitHub\TypeDuck-Web\schema`
- `pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -MoqiImeSource D:\VSProjects\moqi-ime\scripts\build\moqi-ime`
- `Get-FileHash D:\VSProjects\moqi-im-windows\installer\dist\typeduck-windows-ime-setup.exe -Algorithm SHA256`
- User VM UAT: latest installer installed successfully; TypeDuck typing worked; TypeDuck schema drop-in worked.
- Reviewer sidecar checked Wave 4 closeout and confirmed Phase 4 can close if the live VM evidence is recorded as the human UAT result.

## Schema Packaging Decision

TypeDuck schema packaging happens in the sibling `moqi-ime` repo. No `third_party/typeduck-schema` submodule is needed in `moqi-im-windows`.

Future runtime builds should use `https://github.com/TypeDuck-HK/schema/tree/aap2-alpha` as the source, after removing:

- `.gitignore`
- `default.custom.yaml`
- `jyut6ping3_mobile_10keys.schema.yaml`
- `jyut6ping3_mobile_initial_final.schema.yaml`
- `jyut6ping3_mobile_longpress.schema.yaml`
- `jyut6ping3_mobile.schema.yaml`
- `loengfan_longpress.schema.yaml`
- `trime.yaml`
- `weasel.yaml`

`common.custom.yaml` is a frontend-generated settings template and may optionally be removed from the packaged static source. Phase 5 should own generation and mutation of that file when settings change.

Schemas can be prebuilt independently with:

```powershell
rime_deployer.exe --build path\to\schema-folder
```

Use the `rime_deployer.exe` from `dist\bin\rime_deployer.exe` in the same `.7z` release that supplies `rime.dll`, and remove the excluded files before building. The packaged runtime should include the resulting `build` folder.

## Residual Debt

- `scripts\Test-TypeDuckTypingMvpProof.ps1 -Strict` is still stricter than the accepted closeout evidence because some launcher fault-injection obligations remain manual/static rather than executable runtime probes.
- The evidence JSON now has a separate user UAT artifact, but the deeper runtime recovery probes should move to Phase 7.
- Reverse lookup and Cangjie remain backend/librime responsibilities; the frontend requirement for Phase 4 is key forwarding. Rich UX confirmation belongs to Phase 5 and Phase 7.
- Log and data directories still use legacy Moqi paths such as `%LOCALAPPDATA%\MoqiIM`; full naming cleanup belongs to Phase 6.
