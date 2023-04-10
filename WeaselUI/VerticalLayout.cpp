#include "stdafx.h"
#include "VerticalLayout.h"
#include "utils.h"

using namespace weasel;

VerticalLayout::VerticalLayout(const UIStyle &style, const Context &context, const Status &status)
	: StandardLayout(style, context, status)
{
}

void VerticalLayout::DoLayout(CDCHandle dc, CFont romanFont, CFont otherFont)
{
	const std::vector<Text> &candidates(_context.cinfo.candies);
	const std::vector<Text> &comments(_context.cinfo.comments);
	const std::vector<Text> &labels(_context.cinfo.labels);

	CSize size;
	int width = 0, height = _style.margin_y;

	/* Preedit */
	if (!IsInlinePreedit() && !_context.preedit.str.empty())
	{
		size = GetPreeditSize(dc);
		_preeditRect.SetRect(_style.margin_x, height, _style.margin_x + size.cx, height + size.cy);
		width = max(width, _style.margin_x + size.cx + _style.margin_x);
		height += size.cy + _style.spacing;
	}

	/* Auxiliary */
	if (!_context.aux.str.empty())
	{
		dc.GetTextExtent(_context.aux.str.c_str(), _context.aux.str.length(), &size);
		_auxiliaryRect.SetRect(_style.margin_x, height, _style.margin_x + size.cx, height + size.cy);
		width = max(width, _style.margin_x + size.cx + _style.margin_x);
		height += size.cy + _style.spacing;
	}

	labelWidth = 0;
	textWidth = 0;
	restColumnWidths.clear();

	const int space = _style.hilite_spacing;
	const int CHAR_MARGIN = _style.hilite_spacing / 2;
	const int RUBY_MARGIN = CHAR_MARGIN;

	long highlightTop = 0, highlightBottom = 0;
	for (size_t i = 0; i < candidates.size() && i < MAX_CANDIDATES_COUNT; ++i) {
		if (i == _context.cinfo.highlighted) highlightTop = height;

		const std::wstring label = GetLabelText(labels, i, _style.label_text_format.c_str());
		const std::u32string text = conv16to32(candidates.at(i).str);

		dc.SelectFont(otherFont);
		dc.GetTextExtent(label.c_str(), label.size(), &size);
		labelWidth = max(labelWidth, size.cx);

		const std::unordered_set<char32_t>& charWithTwoSyllables = text.size() == 1 ? charWithTwoSyllablesIndividual : charWithTwoSyllablesInPhrase;

		columns lines(conv16to32(comments.at(i).str), lineSep);
		for (const std::u32string& comment : lines) {
			columns tokens(comment, tabSep);
			auto tokens_it = tokens.begin();

			std::u32string roman = *tokens_it++;
			columns romans(roman, spaceSep);
			auto romans_it = romans.begin();

			std::wstring roman16 = conv32to16(roman);
			dc.SelectFont(romanFont);
			dc.GetTextExtent(roman16.c_str(), roman16.length(), &size);
			const long romanHeight = size.cy;

			std::wstring other = label + conv32to16(U'\t' + text + U'\t' + comment.substr(comment.find(U'\t')));
			dc.SelectFont(otherFont);
			dc.GetTextExtent(other.c_str(), other.length(), &size);
			const long otherHeight = size.cy;

			long textX = 0;
			for (const char32_t& ch : text) {
				std::wstring ch16 = conv32to16(std::u32string(&ch));
				std::u32string roman = charWithZeroSyllables.count(ch) || romans_it == romans.end() ? U"" : (*romans_it++ + (charWithTwoSyllables.count(ch) && romans_it != romans.end() ? U" " + *romans_it++ : U""));
				std::wstring roman16 = conv32to16(roman);

				dc.SelectFont(romanFont);
				dc.GetTextExtent(roman16.c_str(), roman16.length(), &size);
				const long romanWidth = size.cx;
				dc.SelectFont(otherFont);
				dc.GetTextExtent(ch16.c_str(), ch16.length(), &size);
				const long charWidth = size.cx;
				const long width = max(romanWidth, charWidth);

				textX += width + CHAR_MARGIN;
			}
			textWidth = max(textWidth, textX - CHAR_MARGIN);

			dc.SelectFont(otherFont);
			for (size_t j = 0; tokens_it != tokens.end(); tokens_it++, j++) {
				std::wstring token16 = conv32to16(*tokens_it);
				dc.GetTextExtent(token16.c_str(), token16.length(), &size);
				if (restColumnWidths.size() < j) restColumnWidths.push_back(size.cx);
				else restColumnWidths[j] = max(restColumnWidths[j], size.cx);
			}

			height += romanHeight + RUBY_MARGIN + otherHeight + _style.candidate_spacing;
		}

		if (i == _context.cinfo.highlighted) highlightBottom = height - _style.candidate_spacing;
	}

	long commentWidth = 0;
	for (const long& restColumnWidth : restColumnWidths) commentWidth += restColumnWidth + space;
	long max_content_width = labelWidth + textWidth + commentWidth + space;
	width = max(width, max_content_width + 2 * _style.margin_x);

	if (candidates.size())
		height += _style.spacing - _style.candidate_spacing;

	/* Trim the last spacing */
	if (height > 0)
		height -= _style.spacing;
	height += _style.margin_y;

	if (!_context.preedit.str.empty() && !candidates.empty())
	{
		width = max(width, _style.min_width);
		height = max(height, _style.min_height);
	}
	UpdateStatusIconLayout(&width, &height);
	_contentSize.SetSize(width, height);

	_highlightRect.SetRect(_style.margin_x, highlightTop, width - _style.margin_x, highlightBottom);
}