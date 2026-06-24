# Phase 5: Candidate, Dictionary, Settings, and About UI Parity - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md - this log preserves the alternatives considered.

**Date:** 2026-06-24T00:46:20Z
**Phase:** 5-Candidate, Dictionary, Settings, and About UI Parity
**Areas discussed:** Semantic theme tokens, Qt versus native rendering, bundled appearance theme file ownership, theme JSON shape, settings/dictionary gray areas

---

## Semantic Theme Tokens

| Option | Description | Selected |
|--------|-------------|----------|
| Copy Web/Tailwind names | Reuse `primary`, `base-100`, `base-300`, and similar names directly from `tailwind.config.ts`. | |
| Semantic Windows names | Use role names such as `panel_background`, `dictionary_background`, `selection_background`, `panel_border`, and `pronunciation_text`. | ✓ |

**User's choice:** Name colors semantically; no need to follow the color names in TypeDuck Web `tailwind.config.ts`.
**Notes:** Web alpha remains the value/source authority, but Windows/runtime token names should describe UI roles.

---

## Qt Versus Native Rendering

| Option | Description | Selected |
|--------|-------------|----------|
| Integrate Qt for candidate/dictionary UI | Rich table/layout widgets, but heavy deployment and risky inside TSF host processes. | |
| Keep native Win32/custom drawing | Reuse the existing non-activating candidate popup and implement table-like layout with measured native rows. | ✓ |
| Consider Qt only for separate settings/About app | Less risky than the popup, but still adds build/package complexity and should require a concrete blocker. | |

**User's choice:** Asked for advice.
**Notes:** Advice: the Web alpha table layout is not complicated enough to justify Qt for the TSF popup. Keep candidate/dictionary native and focus-safe.

---

## Bundled Theme File Ownership

| Option | Description | Selected |
|--------|-------------|----------|
| Keep both files as permanent peers | Avoids loader churn but leaves duplicate source-of-truth ambiguity. | |
| Canonicalize root runtime file | Keep `input_methods/rime/appearance_themes.json` as canonical because loader searches it first and Go embeds that source. | ✓ |
| Canonicalize `data` copy | Aligns with shared data directory idea, but current source/embed path and loader priority do not make it the natural owner. | |

**User's choice:** Asked which of the two `appearance_themes.json` files to keep.
**Notes:** Advice: keep `%PROGRAMFILES(x86)%\TypeDuckIME\moqi-ime\input_methods\rime\appearance_themes.json` as the canonical packaged runtime file. Keep the `data` copy only temporarily if needed for compatibility tests, then remove it. The inspected staged copies are byte-identical and are copied to both paths by `D:\VSProjects\moqi-ime\scripts\build.ps1`.

---

## Theme JSON Shape

| Option | Description | Selected |
|--------|-------------|----------|
| Colors and fonts inside each theme | Simple per-theme bundle, but duplicates font stacks and mixes palette with typography. | |
| Top-level fonts plus light/dark palettes | Keeps font contract outside `themes`; `themes` remains palette-only. | ✓ |

**User's choice:** Include not only the two light/dark color palettes but also font data, and keep fonts outside the `themes` key.
**Notes:** Suggested shape: top-level `version`, top-level `fonts`, and `themes` with exactly `light` and `dark` palette entries for Phase 5.

---

## Additional Gray Areas Found

| Area | Finding | Captured Decision |
|------|---------|-------------------|
| Dictionary reveal behavior | Web alpha shows dictionary detail only after actual pointer movement, not passive hover-rest. | Preserve movement-triggered reveal to avoid typing flicker. |
| Settings ownership | Phase 4 left settings-backed schema customization as deferred work. | Phase 5 settings should persist TypeDuck-owned settings and generate/update runtime config such as `common.custom.yaml` where needed. |
| Capability-gated settings | Some settings may depend on backend support. | Keep Web-alpha settings order, but disable/explain unsupported engine-backed controls with bilingual text. |
| Parser trust boundary | Lookup-filter comments are backend text with CSV and control separators. | Parse into structured UI fields, preserve semantics, and treat display text as untrusted. |
| Verification | Preview harness cannot prove TSF placement/focus behavior. | Use preview for iteration and VM/host-app evidence for acceptance. |

## the agent's Discretion

- Exact C++ view-model names, drawing helpers, JSON field names, and plan split are left to the planner.
- The planner may decide how long to keep the compatibility `data\appearance_themes.json` copy, as long as tests prove the eventual canonical-path behavior.

## Deferred Ideas

- Additional visual themes beyond light/dark.
- Full Phase 6 scaffold/privacy/banned-surface cleanup.
- Heavy UI toolkit adoption unless a concrete native settings/About blocker appears.
