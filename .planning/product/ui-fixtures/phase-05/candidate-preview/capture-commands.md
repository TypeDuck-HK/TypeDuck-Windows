# TypeDuck Candidate Preview Capture Commands

These preview scenarios are source-backed by the 2026-06-23 TypeDuck Web alpha fixtures and by `MoqiTextService/TypeDuckCandidateInfo.*`. They are iteration aids only; Plan 05-06 owns real TSF host-app proof. Runtime acceptance evidence must cite `runtime-provenance.json`; captures from stale TypeDuck 1.1.2 external runtime paths are not accepted as current package evidence.

Run after building `MoqiCandidatePreview`:

```powershell
build-vs32\Preview\Debug\MoqiCandidatePreview.exe --sample nei --capture .planning\product\ui-fixtures\phase-05\candidate-preview\nei-light.bmp
build-vs32\Preview\Debug\MoqiCandidatePreview.exe --sample multilingual --capture .planning\product\ui-fixtures\phase-05\candidate-preview\multilingual-indonesian-main.bmp
build-vs32\Preview\Debug\MoqiCandidatePreview.exe --sample housam --capture .planning\product\ui-fixtures\phase-05\candidate-preview\housam-compound.bmp
build-vs32\Preview\Debug\MoqiCandidatePreview.exe --sample reverse --capture .planning\product\ui-fixtures\phase-05\candidate-preview\reverse-cangjie-onf.bmp
build-vs32\Preview\Debug\MoqiCandidatePreview.exe --sample edge --capture .planning\product\ui-fixtures\phase-05\candidate-preview\edge-clamp.bmp
build-vs32\Preview\Debug\MoqiCandidatePreview.exe --sample dpi --capture .planning\product\ui-fixtures\phase-05\candidate-preview\high-dpi.bmp
build-vs32\Preview\Debug\MoqiCandidatePreview.exe --sample fallback --capture .planning\product\ui-fixtures\phase-05\candidate-preview\fallback-anchor.bmp
```

Keyboard shortcuts in the interactive preview:

- `1` `2` `3` `4`: switch between `nei`, multilingual Indonesian-main, `housam`, and reverse-lookup/Cangjie.
- `E` `D` `F`: switch to edge-clamp, high-DPI, and fallback-anchor placement scenarios.
- Arrow keys: change highlighted candidate.
- `H`: toggle highlighted row.
