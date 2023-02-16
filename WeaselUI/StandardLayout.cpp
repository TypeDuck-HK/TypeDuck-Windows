#include "stdafx.h"
#include "StandardLayout.h"

using namespace weasel;

StandardLayout::StandardLayout(const UIStyle &style, const Context &context, const Status &status)
	: Layout(style, context, status)
{
}

std::wstring StandardLayout::GetLabelText(const std::vector<Text> &labels, int id, const wchar_t *format) const
{
	wchar_t buffer[128];
	swprintf_s<128>(buffer, format, labels.at(id).str.c_str());
	return std::wstring(buffer);
}

CSize StandardLayout::GetPreeditSize(CDCHandle dc) const
{
	const std::wstring &preedit = _context.preedit.str;
	const std::vector<weasel::TextAttribute> &attrs = _context.preedit.attributes;
	CSize size(0, 0);
	if (!preedit.empty())
	{
		dc.GetTextExtent(preedit.c_str(), preedit.length(), &size);
		for (size_t i = 0; i < attrs.size(); i++)
		{
			if (attrs[i].type == weasel::HIGHLIGHTED)
			{
				const weasel::TextRange &range = attrs[i].range;
				if (range.start < range.end)
				{
					if (range.start > 0)
						size.cx += _style.hilite_spacing;
					else
						size.cx += _style.hilite_padding;
					if (range.end < static_cast<int>(preedit.length()))
						size.cx += _style.hilite_spacing;
					else
						size.cx += _style.hilite_padding;
				}
			}
		}
	}
	return size;
}

CSize StandardLayout::GetTextWithNewLineSize(CDCHandle dc,const std::wstring& text) const{
	CSize size(0, 0);
	const auto endOfPos = std::wstring::npos;
	const auto strLen = text.length();
	dc.GetTextExtent(L"t", 1, &size);
	const int height = size.cy;
	// dc.GetTextExtent(text.c_str(), text.length(), &size);
	bool hasNewLine = text.find('\n') != endOfPos;
	if (!hasNewLine) {
		dc.GetTextExtent(text.c_str(), text.length(), &size);
		return size;
	}
	CSize tempSize(0, 0);
	LONG maxWidth = 0;
	for (int i = 0; i < text.length(); ++i) {
		auto newLinePos = text.find('\n', i);
		if (newLinePos == endOfPos) {
			break;
		}
		//ab\nbcd\n\nhello;
		dc.GetTextExtent(text.c_str() + i, newLinePos - i, &tempSize);
		size.cy += tempSize.cy;
		maxWidth = max(maxWidth, tempSize.cx);
		i = newLinePos + 1;
	}
	size.cx = maxWidth;
	return size;
}

void StandardLayout::UpdateStatusIconLayout(int* width, int* height)
{
	// rule 1. status icon is middle-aligned with preedit text or auxiliary text, whichever comes first
	// rule 2. there is a spacing between preedit/aux text and the status icon
	// rule 3. status icon is right aligned in WeaselPanel, when [margin_x + width(preedit/aux) + spacing + width(icon) + margin_x] < style.min_width
	if (ShouldDisplayStatusIcon())
	{
		int left = 0, middle = 0;
		if (!_preeditRect.IsRectNull())
		{
			left = _preeditRect.right + _style.spacing;
			middle = (_preeditRect.top + _preeditRect.bottom) / 2;
		}
		else if (!_auxiliaryRect.IsRectNull())
		{
			left = _auxiliaryRect.right + _style.spacing;
			middle = (_auxiliaryRect.top + _auxiliaryRect.bottom) / 2;
		}
		if (left && middle)
		{
			int right_alignment = *width - _style.margin_x - STATUS_ICON_SIZE;
			if (left > right_alignment)
			{
				*width = left + STATUS_ICON_SIZE + _style.margin_x;
			}
			else
			{
				left = right_alignment;
			}
			_statusIconRect.SetRect(left, middle - STATUS_ICON_SIZE / 2, left + STATUS_ICON_SIZE, middle + STATUS_ICON_SIZE / 2);
		}
		else
		{
			_statusIconRect.SetRect(0, 0, STATUS_ICON_SIZE, STATUS_ICON_SIZE);
			*width = *height = STATUS_ICON_SIZE;
		}
	}
}

bool StandardLayout::IsInlinePreedit() const
{
	return _style.inline_preedit && (_style.client_caps & weasel::INLINE_PREEDIT_CAPABLE) != 0 &&
		_style.layout_type != UIStyle::LAYOUT_VERTICAL_FULLSCREEN && _style.layout_type != UIStyle::LAYOUT_HORIZONTAL_FULLSCREEN;
}

bool StandardLayout::ShouldDisplayStatusIcon() const
{
	// rule 1. emphasis ascii mode
	// rule 2. show status icon when switching mode
	// rule 3. always show status icon with tips 
	return _status.ascii_mode || !_status.composing || !_context.aux.empty();
}
