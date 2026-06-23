# TypeDuck Web Alpha Fixtures - 2026-06-23

**VER-01 fixture set for D-08, D-09, D-10, D-11, D-15, D-16, and D-17.**

This artifact records the TypeDuck Web alpha behavior and source evidence captured on 2026-06-23 for downstream Windows IME parity work. It is a planning/product fixture only: no production code, installer, protocol, registry, runtime path, or release workflow changes are part of this capture.

## Metadata

| Field | Value |
|-------|-------|
| Requirement | VER-01 |
| Decisions | D-08, D-09, D-10, D-11, D-15, D-16, D-17 |
| Capture date | 2026-06-23 |
| Runtime URL | http://localhost:5173/TypeDuck-Web/aap2-alpha/ |
| Runtime status | Available, HTTP 200, content length 2208 |
| Local source path | I:\GitHub\TypeDuck-Web |
| Escaped source path for checks | I:\\GitHub\\TypeDuck-Web |
| Source branch | `aap2-alpha` |
| Source commit/hash | `db21054` |
| Source dirty status | Dirty: `.gitignore`, `README.md`, `scripts/build_schema.ts`, `librime`, and `schema` showed local modifications/submodule changes |
| Browser capture | Playwright with Google Chrome |
| Browser executable | `C:\Program Files\Google\Chrome\Application\chrome.exe` |
| Viewport | 1280x720, device scale factor 1 |
| Overall fixture status | Complete with baseline limitation: source evidence, runtime URL, baseline settings/candidate screenshots, and an additional multilingual Indonesian-main settings/candidate/dictionary screenshot set were captured |

Supporting assets:

- `web-alpha-fixtures/2026-06-23/source-metadata.json`
- `web-alpha-fixtures/2026-06-23/settings-order.json`
- `web-alpha-fixtures/2026-06-23/candidate-list-sample.json`
- `web-alpha-fixtures/2026-06-23/dictionary-detail-sample.json`
- `web-alpha-fixtures/2026-06-23/screenshots/settings-desktop-1280x720.png` - captured
- `web-alpha-fixtures/2026-06-23/screenshots/candidate-desktop-1280x720.png` - captured
- `web-alpha-fixtures/2026-06-23/screenshots/dictionary-detail-desktop-1280x720.png` - partial/blocked, no file created because the dictionary panel did not appear under controlled Playwright hover/touch attempts
- `web-alpha-fixtures/2026-06-23/screenshots/settings-multilingual-indonesian-main-desktop-1280x720.png` - captured with all Display Languages enabled and Indonesian selected as main
- `web-alpha-fixtures/2026-06-23/screenshots/candidate-multilingual-indonesian-main-desktop-1280x720.png` - captured with all Display Languages enabled and Indonesian selected as main
- `web-alpha-fixtures/2026-06-23/screenshots/dictionary-detail-multilingual-indonesian-main-desktop-1280x720.png` - captured after scrolling down and physically moving the mouse over the third candidate cell for 呢

## Settings

Source-backed by `I:\GitHub\TypeDuck-Web\src\Preferences.tsx` and `I:\GitHub\TypeDuck-Web\src\consts.ts` at commit `db21054`.

| Order | Setting | Control | Default | Options / Notes |
|-------|---------|---------|---------|-----------------|
| 1 | 顯示語言 Display Languages | Radio checkbox list | English enabled and main | English, Hindi, Indonesian, Nepali, Urdu |
| 2 | 每頁候選詞數量 No. of Candidates Per Page | Range | 6 | Integer range 4 through 10 |
| 3 | 中文字體 Chinese Typeface | Segmented control | 宋體 Sung | 宋體 Sung, 黑體 Hei |
| 4 | 候選詞粵拼 Candidates Jyutping | Radio list | 顯示 Always Show | 顯示 Always Show, 僅反查 Only in Reverse Lookup, 隱藏 Hide |
| 5 | 自動完成 Auto-completion | Toggle | On | Engine capability must confirm later Windows support |
| 6 | 自動校正 Auto-correction | Toggle | Off | Engine capability must confirm later Windows support |
| 7 | 自動組詞 Auto-composition | Toggle | On | Engine capability must confirm later Windows support |
| 8 | 輸入記憶 Input Memory | Toggle | On | Engine capability must confirm later Windows support |
| 9 | 反查設定 Reverse Lookup Settings | Section heading | N/A | Groups reverse lookup controls |
| 10 | 顯示完整輸入碼 Show Full Input Code | Toggle | On | Affects candidate note/code display in reverse lookup paths |
| 11 | 倉頡版本 Cangjie Version | Segmented control | 五代 Version 5 | 三代 Version 3, 五代 Version 5 |

Display Languages behavior:

- The group is first and must stay first in Windows settings and install-time settings surfaces.
- Selecting a main display language also enables that language.
- The main-language marker is `⬑ 主要語言 Main Language`.
- The Web alpha language set for VER-01 is English, Hindi, Indonesian, Nepali, and Urdu.
- Additional multilingual screenshot coverage enables all five languages and selects Indonesian (`印尼語 Indonesian`) as the main language.
- Full machine-readable detail is in `web-alpha-fixtures/2026-06-23/settings-order.json`.

## Candidate List

Source-backed by `I:\GitHub\TypeDuck-Web\src\CandidatePanel.tsx`, `Candidate.tsx`, and `CandidateInfo.ts`; runtime-backed by controlled Playwright input `nei`.

Visual reference:

- `web-alpha-fixtures/2026-06-23/screenshots/candidate-desktop-1280x720.png`
- `web-alpha-fixtures/2026-06-23/screenshots/candidate-multilingual-indonesian-main-desktop-1280x720.png`
- Viewport: 1280x720
- Status: captured

Observed runtime candidate panel for `nei`:

| Label | Jyutping | Candidate | Meaning / Note | Dictionary Indicator |
|-------|----------|-----------|----------------|----------------------|
| 1. | `nei5` | 你 | you (singular) | shown |
| 2. | `nei4` | 尼 | nun | shown |
| 3. | `nei1` | 呢 | `→ni1` canonical reference | shown |
| 4. | `nei4` | 妮 | little girl | shown |
| 5. | `nei4` | 彌 | full; fill | shown |
| 6. | `nei5` | 妳 | you | shown |

Candidate presentation contract:

- Candidate panel shows active input buffer text.
- Candidate rows include selection label, Jyutping when enabled, Chinese candidate text, note/full input code when relevant, enabled display-language definitions or canonical/label text, and an info indicator when dictionary data is available.
- Default display-language definitions show English only because `displayLanguages` defaults to English.
- With all Display Languages enabled and Indonesian selected as main, candidate rows show Indonesian definitions in the main position and additional enabled-language rows for English, Hindi, Nepali, and Urdu when data is present.
- Candidate page size defaults to 6 and is configurable from 4 through 10.
- Web alpha visual tone uses a light panel, base-400 border, warm amber highlight, HK-aware font stacks, compact rows, and restrained rounded corners.
- Full machine-readable detail is in `web-alpha-fixtures/2026-06-23/candidate-list-sample.json`.

## Dictionary Detail

Source-backed by `I:\GitHub\TypeDuck-Web\src\DictionaryPanel.tsx`, `CandidateInfo.ts`, `consts.ts`, and `index.css`.

Visual reference:

- `web-alpha-fixtures/2026-06-23/screenshots/dictionary-detail-desktop-1280x720.png`
- `web-alpha-fixtures/2026-06-23/screenshots/dictionary-detail-multilingual-indonesian-main-desktop-1280x720.png`
- Viewport: 1280x720
- Status: baseline partial, multilingual captured
- Limitation: The original baseline dictionary-detail screenshot remains partial because the first automation did not move across a concrete candidate cell. The multilingual Indonesian-main dictionary screenshot was captured after scrolling down and physically moving the mouse across the third candidate cell for 呢, which matches the Web alpha behavior that prevents flicker when the pointer is merely resting on candidates during typing.

Dictionary fields the Windows UI must be able to represent when engine data exists:

| Field | Web Alpha Source | Notes |
|-------|------------------|-------|
| Headword | `CandidateEntry.honzi` | Rendered as large Chinese text in the entry head |
| Jyutping | `CandidateEntry.jyutping` | Rendered beside the headword |
| Reading notes | `CandidateEntry.pronunciationType` | Includes changed tone, semantic reading, creative reading, literary/colloquial reading notes when present |
| Part-of-speech | `formattedPartsOfSpeech` | Rendered as badge-like `pos` spans |
| Register | `formattedRegister` | Rendered as italic secondary text |
| Labels | `formattedLabels` | Rendered as secondary label text |
| Main meaning | `properties.definition[prefs.mainLanguage]` | English by default |
| Other data | `written`, `vernacular`, `collocation` | Rendered in dictionary table rows |
| More Languages | `otherLanguages(preferences)` | Table caption is `More Languages`; rows use enabled non-main display languages |
| Canonical reference | `canonicalReference` | Rendered as an arrow reference instead of a direct definition |

Sample source-backed entry anchored to the runtime `nei` candidate list:

- Headword: 你
- Jyutping: `nei5`
- English meaning: `you (singular)`
- Indonesian main definition in multilingual capture: `kamu`
- More Languages runtime capture: English `you (singular)`, Hindi `आप`, Nepali `तपाईं`, Urdu `تم`.

Full machine-readable detail is in `web-alpha-fixtures/2026-06-23/dictionary-detail-sample.json`.

## Visual References

The minimum sufficient Phase 1 visual set, when runtime screenshot capture works, is exactly:

1. Settings desktop screenshot at 1280x720.
2. Candidate-list desktop screenshot at 1280x720.
3. Dictionary-detail desktop screenshot at 1280x720.

Captured status for this run:

| Screenshot | Status | Path / Limitation |
|------------|--------|-------------------|
| Settings | captured | `web-alpha-fixtures/2026-06-23/screenshots/settings-desktop-1280x720.png` |
| Candidate list | captured | `web-alpha-fixtures/2026-06-23/screenshots/candidate-desktop-1280x720.png` |
| Dictionary detail | partial/blocked baseline | No baseline screenshot file created; the first automation did not move across a concrete candidate cell. |
| Settings, multilingual Indonesian main | captured | `web-alpha-fixtures/2026-06-23/screenshots/settings-multilingual-indonesian-main-desktop-1280x720.png` |
| Candidate list, multilingual Indonesian main | captured | `web-alpha-fixtures/2026-06-23/screenshots/candidate-multilingual-indonesian-main-desktop-1280x720.png` |
| Dictionary detail, multilingual Indonesian main | captured | `web-alpha-fixtures/2026-06-23/screenshots/dictionary-detail-multilingual-indonesian-main-desktop-1280x720.png` |

Blocked/partial metadata is acceptable for Phase 1 when capture is unavailable or unreliable, provided the exact attempted source and limitation are recorded. This artifact records that limitation in `source-metadata.json` and `dictionary-detail-sample.json`.

### Pattern Established: Dictionary Hover Requires Movement

- The Web alpha intentionally requires actual mouse movement over a candidate before showing the dictionary panel. This prevents flicker while typing if the pointer happens to be resting on later candidates.
- The successful capture scrolled down, then moved the mouse across the third candidate cell for 呢 after typing `nei`.
- Downstream Windows candidate UI work should model this as movement-triggered dictionary reveal, not as passive hover-on-rest. Phase 5 should verify the native candidate panel does not flicker when a stationary pointer rests over a candidate during typing.

Visual token evidence from `tailwind.config.ts` and `index.css`:

- Light theme primary: `#dfa852`.
- Light highlighted candidate: `#fedc9c`.
- Candidate panel border: `#ded9cf`.
- Candidate panel background: `#ffffff`.
- Dictionary panel background: `#f6f3ed`.
- Body/background base: `#fefefd`.
- HK Chinese font tokens include Chiron Sung HK, Chiron Hei HK, MingLiU_HKSCS, Microsoft JhengHei, Noto Sans HK, and Noto Serif HK fallbacks.

## Unsupported or Blocked Windows Mappings

These notes are future consumer constraints only. They are not production changes in Phase 1.

| Area | Current Windows Scaffold Limitation | Future Mapping Note |
|------|-------------------------------------|---------------------|
| Candidate data model | `proto/moqi.proto` has `CandidateEntry { text, comment }` and a flattened repeated candidate list shape | Phase 4/5 need structured candidate and dictionary fields rather than one comment string |
| Native candidate visual tone | `MoqiTextService/MoqiCandidateWindow.cpp` currently uses white background, gray border, blue selected background, and Moqi log path text | Phase 5 should consume this fixture before restyling native candidate UI |
| Preview utility | `Preview/main.cpp` uses Moqi preview title, Simplified sample candidates, and blue highlight | Preview can be useful later, but it is not Web alpha parity today |
| Dictionary detail | Current native candidate window has no source-backed dictionary panel equivalent | Phase 5 needs a native dictionary panel surface or an explicitly accepted alternative |
| Display language settings | Current Windows scaffold does not expose the Web alpha Display Languages group first | Phase 5 settings and installer-first-run settings must use Display Languages first |
| Screenshot capture | Dictionary-detail screenshot is partial/blocked in this run | Refresh before Phase 5 UI implementation and before Phase 7 release verification |

## Source Evidence

| Source | Evidence Used |
|--------|---------------|
| `I:\GitHub\TypeDuck-Web\src\Preferences.tsx` | Settings labels/order, Display Languages first, main-language marker, range/segment/toggle controls |
| `I:\GitHub\TypeDuck-Web\src\consts.ts` | Language labels/codes, defaults, Jyutping visibility options, definition layout, dictionary label maps |
| `I:\GitHub\TypeDuck-Web\src\CandidatePanel.tsx` | Candidate panel layout, input buffer, paging, candidate list, dictionary panel conditional rendering |
| `I:\GitHub\TypeDuck-Web\src\Candidate.tsx` | Candidate row columns, Jyutping visibility, language definitions, canonical reference, dictionary indicator |
| `I:\GitHub\TypeDuck-Web\src\CandidateInfo.ts` | Parsing of candidate comments into dictionary fields, language definitions, labels, readings, references |
| `I:\GitHub\TypeDuck-Web\src\DictionaryPanel.tsx` | Dictionary head/body/table/More Languages rendering |
| `I:\GitHub\TypeDuck-Web\src\index.css` | Candidate and dictionary panel CSS classes, font stacks, layout tokens |
| `I:\GitHub\TypeDuck-Web\tailwind.config.ts` | DaisyUI theme colors and HK-aware font family tokens |
| `MoqiTextService/MoqiCandidateWindow.cpp` | Future consumer limitation for native candidate rendering |
| `Preview/main.cpp` | Future consumer limitation for preview visual parity |
| `proto/moqi.proto` | Future consumer limitation for structured candidate/dictionary protocol data |

## Refresh Guidance

- Refresh this fixture before Phase 5 UI implementation because the Web alpha can drift.
- Refresh again before Phase 7 release verification if user-facing Web alpha behavior or visuals changed.
- Always record capture date, source path, commit/hash, dirty status, runtime URL, viewport, browser/tool, screenshots, and limitations.
- Validate readable UTF-8 JSON with `Get-Content -Raw -Encoding UTF8 -LiteralPath <file> | ConvertFrom-Json` in Windows PowerShell. The JSON files are also saved with a UTF-8 BOM for compatibility with the original plan checks.
- Use controlled sample input only. Do not capture personal typed content or local user data.
- If runtime, source, screenshot, candidate, or dictionary evidence is unavailable, mark the affected section blocked or partial with the exact attempted source.
- Do not treat Moqi, fcitx, WebDAV/cloud clipboard, AI controls, Simplified-only copy, generic backend config tools, excessive customization, or legacy Moqi backend behavior as acceptable TypeDuck parity.

---

*Fixture: TypeDuck Web alpha 2026-06-23*
*Status: partial, source-backed*
