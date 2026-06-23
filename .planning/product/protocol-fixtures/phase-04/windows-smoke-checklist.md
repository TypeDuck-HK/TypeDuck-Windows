# TypeDuck Phase 4 Windows Smoke Checklist

Use a disposable Windows VM or equivalent checkpointed machine. Do not install the IME on the developer host as the first verification target.

## Target

- VM user: `typeduckverify`
- Product profile: TypeDuck under Chinese (Traditional, Hong Kong)
- Host app: Notepad or another ordinary Windows text field
- Evidence file: `.planning/product/protocol-fixtures/phase-04/typing-mvp-evidence.json`

## Preflight

- [ ] Build or stage the current TypeDuck installer/profile payload from this repository.
- [ ] Confirm the VM is disposable or checkpointed before TSF registration.
- [ ] Install TypeDuck in the VM.
- [ ] Select TypeDuck under Chinese (Traditional, Hong Kong).

## Typing Smoke

- [ ] Type ordinary Cantonese input such as `nei`.
- [ ] Composition text appears while typing.
- [ ] Candidate list appears.
- [ ] Candidate comments appear in the existing comment position when returned by backend.
- [ ] Candidate paging works.
- [ ] Numeric selection commits a candidate.
- [ ] Commit text appears in the host text field.

## Recovery Smoke

- [ ] Stop or restart the backend/launcher during typing.
- [ ] Host app remains responsive; no multi-second hang is observed.
- [ ] TypeDuck clears composition/candidates or enters degraded state.
- [ ] TypeDuck can recover after backend/launcher restart.

## Raw Comment Smoke

- [ ] Raw lookup-filter payload containing U+000B, U+000C, and U+000D is recorded as escaped evidence.
- [ ] No Phase 5 dictionary parsing is accepted as Phase 4 completion evidence.

## Result

- [ ] Update `windows_smoke.status` in `typing-mvp-evidence.json` to `passed`, `failed`, or `blocked`.
- [ ] Record command/manual step, timestamp, artifact path, and observed outcome for each checklist section.
