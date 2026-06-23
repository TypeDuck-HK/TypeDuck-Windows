# TypeDuck Lookup-Filter Raw Payload Contract

**Capture date:** 2026-06-23  
**Requirement traceability:** ENG-05  
**Decision traceability:** D-09, D-10, D-11, D-12, D-13  
**Source fixture:** `.planning/product/TYPEDUCK-WEB-ALPHA-FIXTURES-2026-06-23.md`

## Purpose

This contract records the raw `rime-dictionary-lookup-filter` candidate comment payload evidence captured in Phase 2 Plan 03. It proves that the TypeDuck-HK runtime path emits dictionary-backed CSV/comment data for representative TypeDuck inputs without turning that data into final protocol fields or native UI fields.

Per D-09 and D-13, Phase 2 preserves raw payloads only. Phase 4 and Phase 5 will map these raw columns into structured TypeDuck protocol and candidate/dictionary UI fields.

## Evidence Files

| File | Purpose |
|------|---------|
| `.planning/product/engine-runtime-fixtures/phase-02/typing-proof.json` | Plan 02-02 source proof used as the real engine output source. |
| `.planning/product/engine-runtime-fixtures/phase-02/lookup-filter-raw-comments.ndjson` | One raw comment record per candidate, including escaped notation, UTF-8 hex, code points, control counts, and SHA-256. |
| `.planning/product/engine-runtime-fixtures/phase-02/lookup-filter-csv-rows.csv` | Extracted dictionary rows using the exact D-10 column order. |
| `.planning/product/engine-runtime-fixtures/phase-02/lookup-filter-validation.json` | Validator output showing D-10, D-11, and D-12 checks passed. |
| `scripts/Test-TypeDuckLookupPayload.ps1` | Repeatable validator for raw separator preservation and CSV parsing. |

## D-10 Raw CSV Header

The lookup-filter dictionary rows use this exact header order:

```text
match_input_buffer,honzi,jyutping,canonical_honzi,canonical_jyutping,components_honzi,components_jyutping,pron_label,lit_col_reading,pos,register,label,written_form,vernacular_form,collocation,eng,hin,urd,nep,ind
```

The validator fails if `lookup-filter-csv-rows.csv` uses any other column order or if any row parses to a different column count.

## D-11 Separator Semantics

Candidate comments are raw payload strings, not display-ready text. The control separators are part of the contract:

| Escaped marker | Raw character | Meaning |
|----------------|---------------|---------|
| `\v` | U+000B vertical tab | Optional leading reverse lookup marker. |
| `\f` | U+000C form feed | Separates the candidate note from the remaining payload. |
| `\r` | U+000D carriage return | Prefixes and separates dictionary CSV rows after the note. |

Parsing rule:

1. If the payload starts with `\v`, record `hasLeadingReverseLookupMarker=true` and parse the remaining payload.
2. Text before the first `\f` is the candidate note.
3. If the payload after the first `\f` starts with `\r`, every following `\r`-delimited segment is one dictionary CSV row.
4. If the payload after the first `\f` does not start with `\r`, treat it as `\f`-separated Jyutping-only pronunciation data, not dictionary rows.

The Plan 02-02 `typing-proof.json` evidence contains dictionary rows for `nei` and `housam`. It does not contain an observed leading `\v`; the NDJSON records preserve this as `hasLeadingReverseLookupMarker=false` rather than inventing reverse-lookup data. The validator still includes a parser self-test for optional `\v` semantics so later captured reverse-lookup payloads fail if that marker is stripped.

## D-12 CSV Parsing

Each dictionary row is parsed with standard CSV rules:

- comma-separated fields;
- double-quoted fields are accepted;
- doubled quotes inside quoted fields are accepted;
- empty fields are valid and must remain empty;
- `\v`, `\f`, and `\r` are not stripped, replaced, or normalized before raw evidence capture.

The validator includes quoted-field and doubled-quote self-tests and validates the extracted rows from `lookup-filter-csv-rows.csv`.

## Evidence Summary

Plan 02-03 captures:

| Input | Candidates with raw comments | Dictionary rows | Representative data |
|-------|------------------------------|-----------------|---------------------|
| `nei` | 6 | 18 | `你`, `尼`, `呢`, `妮`, `彌`, `妳`; Jyutping such as `nei5`; English and enabled-language meanings including `you (singular)` and `kamu`. |
| `housam` | 6 | 14 | `好心`, `好心你`, `好`, `號`, `豪`, `毫`; part-of-speech such as `adj|adv`; written form `拜托`; meanings such as `kind; come on`. |

This satisfies ENG-05 for the Phase 2 spike: the runtime emits lookup-filter-backed dictionary data with headword, Jyutping, part-of-speech, English meaning, reading notes, and enabled-language meanings when available.

## Boundary

This contract intentionally does not define final TypeDuck protocol messages, database/schema changes, native candidate UI fields, settings fields, or dictionary panel layout. Those mappings belong to Phase 4 and Phase 5.

Raw strings from backend output must be treated as untrusted display data by later consumers. Phase 2 validates preservation and parseability only.

## Source Audit

| Source | ID | Feature/Requirement | Plan | Status | Notes |
|--------|----|---------------------|------|--------|-------|
| GOAL | Phase 2 | Prove TypeDuck-HK librime, lookup-filter, schemas, dictionaries, and built assets produce candidate and dictionary data | 02-01, 02-02, 02-03 | COVERED | Runtime staging, adapter behavior proof, and raw lookup evidence together satisfy the ROADMAP goal. |
| REQ | ENG-01 | Package pinned TypeDuck-HK librime fork as engine dependency | 02-01, 02-02 | COVERED | D-06 release archive staged and then exercised. |
| REQ | ENG-02 | Package rime-dictionary-lookup-filter plugin with engine | 02-01, 02-03 | COVERED | Plugin evidence staged and lookup-filter payload validated. |
| REQ | ENG-03 | Include TypeDuck schemas, dictionaries, OpenCC assets, and built schema assets | 02-01 | COVERED | D-08 submodule and staged manifest prove assets. |
| REQ | ENG-04 | Produce Cantonese composition, candidate pages, Jyutping, and commit output | 02-02 | COVERED | Adapter proof captures `typing-proof.json`. |
| REQ | ENG-05 | Produce lookup-filter-backed dictionary data for candidates | 02-03 | COVERED | Raw comments, CSV rows, and validation JSON prove dictionary data. |
| CONTEXT | D-01 | Use current moqi-ime backend shape as internal compatibility adapter | 02-02 | COVERED | Probe and runner use adapter process boundary only. |
| CONTEXT | D-02 | Minimal changes to load TypeDuck-HK rime.dll and schema assets | 02-01, 02-02 | COVERED | Staging overlays existing Rime DLL loading path. |
| CONTEXT | D-03 | Focus code work on rime_api compatibility issues | 02-02 | COVERED | Adapter patch area records only required deltas. |
| CONTEXT | D-04 | Do not rewrite TSF frontend around direct in-process librime calls | 02-02 | COVERED | Direct probing only allowed as debug evidence after concrete adapter blockers. |
| CONTEXT | D-05 | Produce Windows-shaped staged TypeDuck runtime tree | 02-01 | COVERED | `Stage-TypeDuckRuntime.ps1` and manifest. |
| CONTEXT | D-06 | Use pinned TypeDuck-HK librime release URL | 02-01 | COVERED | Exact URL is required by staging test. |
| CONTEXT | D-07 | Do not document how replacement rime.dll was produced | 02-01 | COVERED | Runtime contract treats release artifact as source. |
| CONTEXT | D-08 | Add TypeDuck-HK/schema branch aap2-alpha as submodule | 02-01 | COVERED | Submodule path `third_party/typeduck-schema`. |
| CONTEXT | D-09 | Prove raw lookup-filter CSV/comment output first | 02-03 | COVERED | Raw evidence contract only. |
| CONTEXT | D-10 | Preserve exact CSV/comment columns | 02-03 | COVERED | Validator enforces header. |
| CONTEXT | D-11 | Preserve control-character separator semantics | 02-03 | COVERED | Validator enforces `\v`, `\f`, and `\r` behavior. |
| CONTEXT | D-12 | Standard CSV parsing with quoted fields and no separator normalization | 02-03 | COVERED | Validator enforces parsing and raw preservation. |
| CONTEXT | D-13 | Later Phase 4/5 maps raw columns into protocol/UI | 02-03 | COVERED | Contract says raw only. |
| CONTEXT | D-14 | Bridge does not permit shipping unchanged Moqi surfaces | 02-01, 02-02 | COVERED | Runtime and adapter contracts reference banned surfaces. |
| CONTEXT | D-15 | Legacy fallback behavior is not valid product behavior | 02-02 | COVERED | Proof rejects legacy-only evidence. |
| CONTEXT | D-16 | Broad protocol/product naming/installer/UI cleanup remains outside Phase 2 | 02-01, 02-02, 02-03 | COVERED | Each plan includes boundary language. |
| RESEARCH | skipped | User invoked --skip-research; no 02-RESEARCH.md source exists | NONE | EXCLUDED | Explicitly skipped by user; not a missing item. |
