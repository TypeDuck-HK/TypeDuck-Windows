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
		height += szy + 2 * yoffset + _style.spacing - 1;
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
		height += size.cy + 2 * yoffset + _style.spacing - 1;
		width = max(width, real_margin_x * 2 + size.cx);
		_auxiliaryRect.OffsetRect(offsetX, offsetY);
	}
	/*  preedit and auxiliary rectangle calc end */

	/* Candidates */
	const bool showHint = _multiHintPanel->isHintEnabled(StatusHintColumn::Jyutping);

	int label_width = 0, ruby_width = 0, comment_group_1_width = 0, comment_group_2_width = 0, comment_group_3_width = 0;

	CSize labelSize[MAX_CANDIDATES_COUNT];
	CSize hintSize[MAX_CANDIDATES_COUNT];
	CSize textSize[MAX_CANDIDATES_COUNT];
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
		GetTextSizeDW(text, pDWR->pTextFormat, pDWR, &textSize[i], _style.character_spacing);

		const std::wstring& comment = comments.at(i).str;
		if (_multiHintPanel->isEnabled() && !comment.empty()) {
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
			comment_group_1_height = engSize[i].cy + gap * _multiHintPanel->isHintEnabled(StatusHintColumn::Eng) * _multiHintPanel->isHintEnabled(StatusHintColumn::Ind) + indSize[i].cy,
			comment_group_2_height = hinSize[i].cy + gap * _multiHintPanel->isHintEnabled(StatusHintColumn::Hin) * _multiHintPanel->isHintEnabled(StatusHintColumn::Nep) + nepSize[i].cy,
			comment_group_3_height = urdSize[i].cy;
		height += max(label_height, max(ruby_height, max(comment_group_1_height, max(comment_group_2_height, comment_group_3_height)))) - gap;

		_candidateLabelRects[i].SetRect(w, height - label_height, w + labelSize[i].cx, height - label_height + labelSize[i].cy);
		_candidateLabelRects[i].OffsetRect(offsetX, offsetY);

		w += label_width + space;

		if (showHint) {
			_candidateHintRects[i].SetRect(w, height - ruby_height, w + hintSize[i].cx, height - ruby_height + hintSize[i].cy);
			_candidateHintRects[i].OffsetRect(offsetX, offsetY);
		}

		_candidateTextRects[i].SetRect(w, height - textSize[i].cy, w + textSize[i].cx, height);
		_candidateTextRects[i].OffsetRect(offsetX, offsetY);

		w += ruby_width + space * 2 * _multiHintPanel->isHintEnabled((int)StatusHintColumn::Eng | (int)StatusHintColumn::Ind);

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

		w += comment_group_2_width + space * 2 * _multiHintPanel->isHintEnabled(StatusHintColumn::Urd);

		if (_multiHintPanel->isHintEnabled(StatusHintColumn::Urd)) {
			_candidateUrdRects[i].SetRect(w, height - comment_group_3_height, w + urdSize[i].cx, height);
			_candidateUrdRects[i].OffsetRect(offsetX, offsetY);
		}

		w += comment_group_3_width + _style.hilite_padding + real_margin_x;

		width = max(width, w);
		height += _style.hilite_padding;
		_candidateRects[i].SetRect(real_margin_x + offsetX, top, width - real_margin_x + offsetX, height);
	}

	/* Trim the last spacing if no candidates */
	if(candidates_count == 0) height -= _style.spacing;

	height += real_margin_y;

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
		int _prey = (_preeditRect.top + _preeditRect.bottom) / 2 - pgszl.cy / 2;
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
