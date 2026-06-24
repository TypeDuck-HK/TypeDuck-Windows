---
phase: 04-typeduck-protocol-and-typing-mvp
status: accepted
verified: 2026-06-24
human_uat: completed
strict_machine_proof: partial
---

# Phase 4 Verification

## Verdict

Phase 4 is accepted.

The acceptance gate is the combination of Phase 4 static/protocol guards, successful release packaging, and live VM user confirmation that the installed IME types through the TypeDuck runtime instead of falling back to ASCII. The user also confirmed that replacing the packaged schema payload with the TypeDuck schema source worked.

## Evidence

| Criterion | Status | Evidence |
|-----------|--------|----------|
| Cantonese typing works in a normal Windows host | Accepted | User installed the rebuilt installer in the VM and confirmed typing worked. |
| Packaged runtime uses TypeDuck schema data | Accepted | Final rebuild used `moqi-ime -RimeDataSource I:\GitHub\TypeDuck-Web\schema`; user confirmed schema drop-in worked. |
| Protocol/framing remains bounded | Passed | Phase 4 proof guards and `ProtoFraming_test` artifacts under `.planning\product\protocol-fixtures\phase-04`. |
| Launcher/client static recovery guards exist | Passed with debt | Existing Wave 4 proof artifacts cover static guard behavior; executable launcher fault-injection probes remain Phase 7 debt. |
| Reverse lookup and Cangjie frontend behavior | Accepted for Phase 4 | Frontend forwards keypresses; backend/librime owns reverse lookup and Cangjie behavior. Rich UX verification moves to Phase 5/7. |
| Raw lookup payload handling | Passed for Phase 4 | Phase 4 leaves raw payload display at the current candidate comment position; parsing belongs to Phase 5. |

## Commands

Key commands from the final accepted build:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File D:\VSProjects\moqi-ime\scripts\build.ps1 -RepoRoot D:\VSProjects\moqi-ime -RimeDataSource I:\GitHub\TypeDuck-Web\schema
pwsh -NoProfile -ExecutionPolicy Bypass -File scripts\install.ps1 -RepoRoot . -MoqiImeSource D:\VSProjects\moqi-ime\scripts\build\moqi-ime
Get-FileHash D:\VSProjects\moqi-im-windows\installer\dist\typeduck-windows-ime-setup.exe -Algorithm SHA256
```

Final installer hash:

```text
3F09C9BD16A62B09CB44F0A356DC23A120D71FF5466FD80C3492F54185773A4B
```

## UAT Notes

- The VM install was performed manually by the user after PowerShell Direct proved insufficient for interactive TSF typing.
- The original failure mode was ASCII output despite selecting TypeDuck; this was fixed and confirmed by user install.
- The first successful installer still used Mandarin defaults; the later TypeDuck schema drop-in replacement was confirmed to work.

## Carried Forward

- Phase 5 should generate and update `common.custom.yaml` from settings instead of treating it as static packaged data.
- Phase 5 should parse dictionary/comment payloads into native UI fields.
- Phase 6 should remove remaining Moqi-branded paths/log names and off-scope scaffold surfaces.
- Phase 7 should turn the remaining manual/static recovery obligations into executable runtime probes.
