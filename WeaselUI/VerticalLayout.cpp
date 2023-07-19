#include "stdafx.h"
#include "VerticalLayout.h"

using namespace weasel;

void weasel::VerticalLayout::DoLayout(CDCHandle dc, DirectWriteResources* pDWR)
{
	const int space = _style.hilite_spacing, gap = _style.text_gap;
	int width = 0, height = real_margin_y;

#ifdef USE_HILITE_MARK
	if (!_style.mark_text.empty() && (_style.hilited_mark_color & 0xff000000))
	{
		CSize sg;
		GetTextSizeDW(_style.mark_text, pDWR->pTextFormat, pDWR, &sg);
		MARK_WIDTH = sg.cx;
		MARK_HEIGHT = sg.cy;
		MARK_GAP = MARK_WIDTH + 4;
	}
	int base_offset =  ((_style.hilited_mark_color & 0xff000000) && !_style.mark_text.empty()) ? MARK_GAP : 0;
#else
	int base_offset = 0;
#endif	/* USE_HILITE_MARK */

#ifdef USE_PAGER_MARK
	// calc page indicator 
	CSize pgszl, pgszr;
	GetTextSizeDW(pre, pDWR->pPreeditTextFormat, pDWR, &pgszl);
	GetTextSizeDW(next, pDWR->pPreeditTextFormat, pDWR, &pgszr);
	int pgw = pgszl.cx + pgszr.cx + _style.hilite_spacing + _style.hilite_padding * 2;
	int pgh = max(pgszl.cy, pgszr.cy);
#endif /* USE_PAGER_MARK */

	/*  preedit and auxiliary rectangle calc start */
	CSize size;
	/* Preedit */
	if (!IsInlinePreedit() && !_context.preedit.str.empty())
	{
		size = GetPreeditSize(dc, _context.preedit, pDWR->pPreeditTextFormat, pDWR);
#ifdef USE_PAGER_MARK
		int szx = pgw, szy = max(size.cy, pgh);
#else
		int szx = 0, szy = size.cy;
#endif /*  USE_PAGER_MARK */
		// icon size higher then preedit text
		int yoffset = (STATUS_ICON_SIZE >= szy && ShouldDisplayStatusIcon()) ? (STATUS_ICON_SIZE - szy) / 2 : 0;
		_preeditRect.SetRect(real_margin_x, height + yoffset, real_margin_x + size.cx, height + yoffset + size.cy);
		height += szy + 2 * (yoffset + _style.spacing);
		width = max(width, real_margin_x * 2 + size.cx + szx);
		if(ShouldDisplayStatusIcon()) width += STATUS_ICON_SIZE;
		_preeditRect.OffsetRect(offsetX, offsetY);
	}

	/* Auxiliary */
	if (!_context.aux.str.empty())
	{
		size = GetPreeditSize(dc, _context.aux, pDWR->pPreeditTextFormat, pDWR);
		// icon size higher then auxiliary text
		int yoffset = (STATUS_ICON_SIZE >= size.cy && ShouldDisplayStatusIcon()) ? (STATUS_ICON_SIZE - size.cy) / 2 : 0;
		_auxiliaryRect.SetRect(real_margin_x, height + yoffset, real_margin_x + size.cx, height + yoffset + size.cy);
		height += size.cy + 2 * (yoffset + _style.spacing);
		width = max(width, real_margin_x * 2 + size.cx);
		_auxiliaryRect.OffsetRect(offsetX, offsetY);
	}
	/*  preedit and auxiliary rectangle calc end */

	/* Candidates */
	std::wstring dictionary_entry;
	InfoMultiHint dictionary_info;
	const bool isSingleComment = _context.preedit.str.rfind(L"[Schema Menu]", 1) != std::wstring::npos;
	const bool showHint = _multiHintPanel->isHintEnabled(StatusHintColumn::Jyutping);

	int label_width = 0, ruby_width = 0, comment_group_0_width = 0, comment_group_1_width = 0, comment_group_2_width = 0, comment_group_3_width = 0;

	CSize labelSize[MAX_CANDIDATES_COUNT];
	CSize hintSize[MAX_CANDIDATES_COUNT];
	CSize textSize[MAX_CANDIDATES_COUNT];
	CSize commentSize[MAX_CANDIDATES_COUNT];
	CSize engSize[MAX_CANDIDATES_COUNT];
	CSize hinSize[MAX_CANDIDATES_COUNT];
	CSize urdSize[MAX_CANDIDATES_COUNT];
	CSize nepSize[MAX_CANDIDATES_COUNT];
	CSize indSize[MAX_CANDIDATES_COUNT];

	for (int i = 0; i < candidates_count && i < MAX_CANDIDATES_COUNT; ++i) {
		const std::wstring& label = GetLabelText(labels, i, _style.label_text_format.c_str());
		GetTextSizeDW(label, pDWR->pLabelTextFormat, pDWR, &labelSize[i]);
		label_width = max(label_width, labelSize[i].cx);

		const std::wstring& text = candidates.at(i).str;
		GetTextSizeDW(text, pDWR->pTextFormat, pDWR, &textSize[i], _style.character_spacing * !isSingleComment);

		const std::wstring& comment = comments.at(i).str;
		if (isSingleComment) {
			GetTextSizeDW(comment, pDWR->pCommentTextFormat, pDWR, &commentSize[i]);
			comment_group_0_width = max(comment_group_0_width, commentSize[i].cx);
		} else if (_multiHintPanel->isEnabled() && !comment.empty()) {
			if (_multiHintPanel->containsCSV(comment)) {
				InfoMultiHint info(comment);
				if (showHint) GetTextSizeDW(info.Jyutping, pDWR->pHintTextFormat, pDWR, &hintSize[i]);
				if (_multiHintPanel->isHintEnabled(StatusHintColumn::Eng)) GetTextSizeDW(info.Properties.Definition.Eng, pDWR->pEngTextFormat, pDWR, &engSize[i]);
				if (_multiHintPanel->isHintEnabled(StatusHintColumn::Hin)) GetTextSizeDW(info.Properties.Definition.Hin, pDWR->pHinTextFormat, pDWR, &hinSize[i]);
				if (_multiHintPanel->isHintEnabled(StatusHintColumn::Urd)) GetTextSizeDW(info.Properties.Definition.Urd, pDWR->pUrdTextFormat, pDWR, &urdSize[i]);
				if (_multiHintPanel->isHintEnabled(StatusHintColumn::Nep)) GetTextSizeDW(info.Properties.Definition.Nep, pDWR->pNepTextFormat, pDWR, &nepSize[i]);
				if (_multiHintPanel->isHintEnabled(StatusHintColumn::Ind)) GetTextSizeDW(info.Properties.Definition.Ind, pDWR->pIndTextFormat, pDWR, &indSize[i]);
				comment_group_1_width = max(comment_group_1_width, max(engSize[i].cx, indSize[i].cx));
				comment_group_2_width = max(comment_group_2_width, max(hinSize[i].cx, nepSize[i].cx));
				comment_group_3_width = max(comment_group_3_width, urdSize[i].cx);
				if (i == _context.cinfo.highlighted) {
					dictionary_entry = text;
					dictionary_info = info;
				}
			} else if (showHint) {
				GetTextSizeDW(comment, pDWR->pHintTextFormat, pDWR, &hintSize[i]);
			}
		}
		ruby_width = max(ruby_width, max(hintSize[i].cx, textSize[i].cx));
	}

	for (int i = 0; i < candidates_count && i < MAX_CANDIDATES_COUNT; ++i) {
		if (i > 0) height += _style.candidate_spacing;

		int w = real_margin_x + _style.hilite_padding + base_offset;
		const int top = height,
			label_height = gap + labelSize[i].cy,
			ruby_height = hintSize[i].cy + gap * showHint + textSize[i].cy,
			comment_group_0_height = commentSize[i].cy,
			comment_group_1_height = engSize[i].cy + gap * _multiHintPanel->isHintEnabled(StatusHintColumn::Eng) * _multiHintPanel->isHintEnabled(StatusHintColumn::Ind) + indSize[i].cy,
			comment_group_2_height = hinSize[i].cy + gap * _multiHintPanel->isHintEnabled(StatusHintColumn::Hin) * _multiHintPanel->isHintEnabled(StatusHintColumn::Nep) + nepSize[i].cy,
			comment_group_3_height = urdSize[i].cy;
		height += max(label_height, max(ruby_height, max(comment_group_0_height, max(comment_group_1_height, max(comment_group_2_height, comment_group_3_height))))) - _style.hilite_padding;

		_candidateLabelRects[i].SetRect(w, height - label_height, w + labelSize[i].cx, height - label_height + labelSize[i].cy);
		_candidateLabelRects[i].OffsetRect(offsetX, offsetY);

		w += label_width + space;

		if (showHint) {
			_candidateHintRects[i].SetRect(w, height - ruby_height, w + hintSize[i].cx, height - ruby_height + hintSize[i].cy);
			_candidateHintRects[i].OffsetRect(offsetX, offsetY);
		}

		_candidateTextRects[i].SetRect(w, height - textSize[i].cy, w + textSize[i].cx, height);
		_candidateTextRects[i].OffsetRect(offsetX, offsetY);

		w += ruby_width + space * 2 * isSingleComment;

		if (isSingleComment) {
			_candidateCommentRects[i].SetRect(w, height - commentSize[i].cy, w + commentSize[i].cx, height);
			_candidateCommentRects[i].OffsetRect(offsetX, offsetY);
		}

		w += comment_group_0_width + space * 2 * _multiHintPanel->isHintEnabled((int)StatusHintColumn::Eng | (int)StatusHintColumn::Ind);

		if (_multiHintPanel->isHintEnabled(StatusHintColumn::Eng)) {
			_candidateEngRects[i].SetRect(w, height - comment_group_1_height, w + engSize[i].cx, height - comment_group_1_height + engSize[i].cy);
			_candidateEngRects[i].OffsetRect(offsetX, offsetY);
		}

		if (_multiHintPanel->isHintEnabled(StatusHintColumn::Ind)) {
			_candidateIndRects[i].SetRect(w, height - indSize[i].cy, w + indSize[i].cx, height);
			_candidateIndRects[i].OffsetRect(offsetX, offsetY);
		}

		w += comment_group_1_width + space * 2 * _multiHintPanel->isHintEnabled((int)StatusHintColumn::Hin | (int)StatusHintColumn::Nep);

		if (_multiHintPanel->isHintEnabled(StatusHintColumn::Hin)) {
			_candidateHinRects[i].SetRect(w, height - comment_group_2_height, w + hinSize[i].cx, height - comment_group_2_height + hinSize[i].cy);
			_candidateHinRects[i].OffsetRect(offsetX, offsetY);
		}

		if (_multiHintPanel->isHintEnabled(StatusHintColumn::Nep)) {
			_candidateNepRects[i].SetRect(w, height - nepSize[i].cy, w + nepSize[i].cx, height);
			_candidateNepRects[i].OffsetRect(offsetX, offsetY);
		}

		// Urdu right align
		w += comment_group_2_width + space * 2 * _multiHintPanel->isHintEnabled(StatusHintColumn::Urd) + comment_group_3_width;

		if (_multiHintPanel->isHintEnabled(StatusHintColumn::Urd)) {
			_candidateUrdRects[i].SetRect(w - urdSize[i].cx, height - comment_group_3_height, w, height);
			_candidateUrdRects[i].OffsetRect(offsetX, offsetY);
		}

		w += _style.hilite_padding * 2 + real_margin_x;

		width = max(width, w);
		height += _style.hilite_padding;
		_candidateRects[i].SetRect(real_margin_x + offsetX, top, width - real_margin_x + offsetX, height);
	}

	/* Trim the last spacing if no candidates */
	height += real_margin_y - (candidates_count ? _style.border : _style.spacing * 2);

	if (!_context.preedit.str.empty() && !candidates.empty())
	{
		width = max(width, _style.min_width);
		height = max(height, _style.min_height);
	}
	UpdateStatusIconLayout(&width, &height);

	_contentSize.SetSize(width + offsetX * 2, height + offsetY * 2);

	/* Highlighted Candidate */

	_highlightRect = _candidateRects[id];
#ifdef USE_PAGER_MARK
	// calc page indicator 
	if(candidates_count && !_style.inline_preedit)
	{
		int _prex = _contentSize.cx - offsetX - real_margin_x + _style.hilite_padding - pgw;
		int _prey = (_preeditRect.top + _preeditRect.bottom) / 2 - pgszl.cy / 2 - _style.hilite_padding;
		_prePageRect.SetRect(_prex, _prey, _prex + pgszl.cx, _prey + pgszl.cy);
		_nextPageRect.SetRect(_prePageRect.right + _style.hilite_spacing, 
				_prey, _prePageRect.right + _style.hilite_spacing + pgszr.cx, _prey + pgszr.cy);
		if (ShouldDisplayStatusIcon())
		{
			_prePageRect.OffsetRect(-STATUS_ICON_SIZE, 0);
			_nextPageRect.OffsetRect(-STATUS_ICON_SIZE, 0);
		}
	}
#endif /*  USE_PAGER_MARK */

	if (dictionary_entry.empty()) {
		_dictionaryRect.SetRectEmpty();
	} else {
		_dictionaryPanelRects.clear();
		DictionaryPanelRects rects;
		const int left = _contentSize.cx - real_margin_x;
		const int minW = left + _style.dictionary_panel_style.padding;

		// | -padding- [entry] -title_gap- [pron] -title_gap- [pron_type] -padding- |
		std::wstring pronType = dictionary_info.GetPronType();
		CSize entrySize, pronSize, pronTypeSize;
		GetTextSizeDW(dictionary_entry, pDWR->pEntryTextFormat, pDWR, &entrySize);
		if (!dictionary_info.Jyutping.empty()) GetTextSizeDW(dictionary_info.Jyutping, pDWR->pPronTextFormat, pDWR, &pronSize);
		if (!pronType.empty()) GetTextSizeDW(pronType, pDWR->pPronTypeTextFormat, pDWR, &pronTypeSize);
		int h = /* real_margin_y + */ _style.dictionary_panel_style.padding + max(entrySize.cy, max(pronSize.cy, pronTypeSize.cy));

		int w = minW;
		rects.entryLabel.SetRect(w, h - entrySize.cy, w + entrySize.cx, h);
		w += entrySize.cx + _style.dictionary_panel_style.title_gap * !dictionary_info.Jyutping.empty();
		rects.pronLabel.SetRect(w, h - pronSize.cy, w + pronSize.cx, h);
		w += pronSize.cx + _style.dictionary_panel_style.title_gap * !pronType.empty();
		rects.pronTypeLabel.SetRect(w, h - pronTypeSize.cy, w + pronTypeSize.cx, h);
		w += pronTypeSize.cx + _style.dictionary_panel_style.padding;
		width = w;

		// | -padding- [pos] -pos_gap- [pos] -definition_gap- [register] -definition_gap- [lbl] -lbl_gap- [lbl] -definition_gap- [definition] -padding- |
		std::vector<std::wstring> partsOfSpeech = dictionary_info.Properties.GetPartsOfSpeech();
		std::wstring registerLabel = dictionary_info.Properties.GetRegister();
		std::vector<std::wstring> lbls = dictionary_info.Properties.GetLabels();
		std::vector<InfoLanguage> definitions = dictionary_info.Properties.Definition.Get(_multiHintPanel, pDWR);
		if (!partsOfSpeech.empty() || !registerLabel.empty() || !lbls.empty() || !definitions.empty()) {
			int maxH = 0;
			std::vector<CSize> posSizes, lblSizes;
			CSize registerSize, definitionSize;
			for (const std::wstring pos : partsOfSpeech) {
				CSize posSize;
				GetTextSizeDW(pos, pDWR->pPOSTextFormat, pDWR, &posSize);
				posSizes.push_back(posSize);
				maxH = max(maxH, posSize.cy + _style.dictionary_panel_style.pos_padding * 2);
			}
			if (!registerLabel.empty()) {
				GetTextSizeDW(registerLabel, pDWR->pRegisterTextFormat, pDWR, &registerSize);
				maxH = max(maxH, registerSize.cy);
			}
			for (const std::wstring lbl : lbls) {
				CSize lblSize;
				GetTextSizeDW(lbl, pDWR->pLblTextFormat, pDWR, &lblSize);
				lblSizes.push_back(lblSize);
				maxH = max(maxH, lblSize.cy);
			}
			if (!definitions.empty()) {
				GetTextSizeDW(definitions[0].Value, definitions[0].TextFormat, pDWR, &definitionSize);
				maxH = max(maxH, definitionSize.cy);
			}
			h += _style.dictionary_panel_style.spacing + maxH - _style.dictionary_panel_style.pos_padding;

			w = minW;
			std::vector<CRect> posRects, lblRects;
			for (const CSize posSize : posSizes) {
				w += _style.dictionary_panel_style.pos_padding;
				posRects.push_back({ w, h - posSize.cy, w + posSize.cx, h });
				w += posSize.cx + _style.dictionary_panel_style.pos_padding + _style.dictionary_panel_style.pos_gap;
			}
			rects.posLabels = posRects;
			if (!posSizes.empty()) w += _style.dictionary_panel_style.definition_gap - _style.dictionary_panel_style.pos_gap;

			h += _style.dictionary_panel_style.pos_padding;
			rects.registerLabel.SetRect(w, h - registerSize.cy, w + registerSize.cx, h);
			w += registerSize.cx + _style.dictionary_panel_style.definition_gap * !registerLabel.empty();

			for (const CSize lblSize : lblSizes) {
				lblRects.push_back({ w, h - lblSize.cy, w + lblSize.cx, h });
				w += lblSize.cx + _style.dictionary_panel_style.lbl_gap;
			}
			rects.lblLabels = lblRects;
			if (!lblSizes.empty()) w += _style.dictionary_panel_style.definition_gap - _style.dictionary_panel_style.lbl_gap;

			rects.definitionLabel.SetRect(w, h - definitionSize.cy, w + definitionSize.cx, h);
			w += definitionSize.cx - _style.dictionary_panel_style.definition_gap * definitions.empty() + _style.dictionary_panel_style.padding;
			width = max(width, w);
		}

		// | -padding- [field_key] -field_spacing- [field_value] -padding- |
		int maxKeyWidth = 0;
		std::vector<std::vector<std::wstring> > otherData = dictionary_info.Properties.GetOtherData();
		std::vector<std::vector<CSize> > otherDataSizes;
		for (const std::vector<std::wstring>& row : otherData) {
			std::vector<CSize> rowSizes;
			bool first = true;
			for (const std::wstring item : row) {
				CSize itemSize;
				GetTextSizeDW(item, first ? pDWR->pFieldKeyTextFormat : pDWR->pFieldValueTextFormat, pDWR, &itemSize);
				rowSizes.push_back(itemSize);
				if (first) {
					maxKeyWidth = max(maxKeyWidth, itemSize.cx);
					first = false;
				}
			}
			otherDataSizes.push_back(rowSizes);
		}

		bool first = true;
		std::vector<std::vector<CRect> > otherDataRects;
		for (const std::vector<CSize>& rowSizes : otherDataSizes) {
			h += (first ? _style.dictionary_panel_style.spacing : _style.dictionary_panel_style.field_spacing) + max(rowSizes[0].cy, rowSizes[1].cy);
			first = false;
			w = minW + maxKeyWidth;
			std::vector<CRect> rowRects = {{ w - rowSizes[0].cx, h - rowSizes[0].cy, w, h }};
			w += _style.dictionary_panel_style.field_gap;
			rowRects.push_back({ w, h - rowSizes[1].cy, w + rowSizes[1].cx, h });
			int maxValueWidth = rowSizes[1].cx;
			for (size_t i = 2; i < rowSizes.size(); i++) {
				rowRects.push_back({ w, h, w + rowSizes[i].cx, h + rowSizes[i].cy });
				h += rowSizes[i].cy;
				maxValueWidth = max(maxValueWidth, rowSizes[i].cx);
			}
			otherDataRects.push_back(rowRects);
			w += maxValueWidth + _style.dictionary_panel_style.padding;
			width = max(width, w);
		}
		rects.fieldLabels = otherDataRects;

		if (definitions.size() > 1) {
			w = minW;
			h += _style.dictionary_panel_style.spacing;
			GetTextSizeDW(L"More Languages", pDWR->pMoreLanguagesHeadTextFormat, pDWR, &size);
			rects.moreLanguagesHeadLabel.SetRect(w, h, w + size.cx, h + size.cy);
			h += size.cy;
			w += size.cx + _style.dictionary_panel_style.padding;
			width = max(width, w);

			int maxKeyWidth = 0;
			std::vector<std::pair<CSize, CSize> > languageSizes;
			for (size_t i = 1; i < definitions.size(); i++) {
				CSize keySize, valueSize;
				GetTextSizeDW(definitions[i].Key, pDWR->pFieldKeyTextFormat, pDWR, &keySize);
				maxKeyWidth = max(maxKeyWidth, keySize.cx);
				GetTextSizeDW(definitions[i].Value, definitions[i].TextFormat, pDWR, &valueSize);
				languageSizes.push_back({ keySize, valueSize });
			}

			w = minW;
			bool first = true;
			std::vector<std::pair<CRect, CRect> > languageRects;
			for (const std::pair<CSize, CSize>& languageSize : languageSizes) {
				h += (first ? _style.dictionary_panel_style.more_languages_spacing : _style.dictionary_panel_style.field_spacing) + max(languageSize.first.cy, languageSize.second.cy);
				first = false;
				w = minW + maxKeyWidth;
				const CRect keyRect = { w - languageSize.first.cx, h - languageSize.first.cy, w, h };
				w += _style.dictionary_panel_style.field_gap;
				languageRects.push_back({ keyRect, { w, h - languageSize.second.cy, w + languageSize.second.cx, h } });
				w += languageSize.second.cx + _style.dictionary_panel_style.padding;
				width = max(width, w);
			}
			rects.moreLanguageLabels = languageRects;
		}

		h += _style.dictionary_panel_style.padding + real_margin_y;
		_contentSize.cy = max(_contentSize.cy, h);

		_dictionaryPanelRects = { rects };
		_dictionaryRect.SetRect(left, real_margin_y, width, _contentSize.cy - real_margin_y - 1);
		_contentSize.cx = width + real_margin_x;
	}

	// calc roundings start
	_contentRect.SetRect(0, 0, _contentSize.cx, _contentSize.cy);
	// background rect prepare for Hemispherical calculation
	CopyRect(_bgRect, _contentRect);
	_bgRect.DeflateRect(offsetX + 1, offsetY + 1);
	_PrepareRoundInfo(dc);

	// truely draw content size calculation
	int deflatex = offsetX - _style.border / 2;
	int deflatey = offsetY - _style.border / 2;
	_contentRect.DeflateRect(deflatex, deflatey);
	// eliminate the 1 pixel gap when border width odd and padding equal to margin
	if (_style.border % 2 == 0)	_contentRect.DeflateRect(1, 1);
}
