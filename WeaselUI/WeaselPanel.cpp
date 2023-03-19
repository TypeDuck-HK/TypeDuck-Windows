#include "stdafx.h"
#include "WeaselPanel.h"
#include <WeaselCommon.h>
#include <Usp10.h>

#include "VerticalLayout.h"
#include "utils.h"

// for IDI_ZH, IDI_EN
#include <resource.h>

using namespace weasel;

WeaselPanel::WeaselPanel(weasel::UI &ui)
	: m_layout(NULL), 
	  m_ctx(ui.ctx()), 
	  m_status(ui.status()), 
	  m_style(ui.style())
{
	m_iconDisabled.LoadIconW(IDI_RELOAD, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
	m_iconEnabled.LoadIconW(IDI_ZH, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
	m_iconAlpha.LoadIconW(IDI_EN, STATUS_ICON_SIZE, STATUS_ICON_SIZE, LR_DEFAULTCOLOR);
}

WeaselPanel::~WeaselPanel()
{
	if (m_layout != NULL)
		delete m_layout;
}

void WeaselPanel::_SetFont(CDCHandle dc) {
	const long otherFontHeight = -MulDiv(m_style.font_point, dc.GetDeviceCaps(LOGPIXELSY), 72);
	const long romanFontHeight = MulDiv(m_style.font_point, 4, 5);
	const wchar_t* fontFace = m_style.font_face.c_str();
	m_romanFont.CreateFontW(romanFontHeight, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, fontFace);
	m_otherFont.CreateFontW(otherFontHeight, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, fontFace);
}

void WeaselPanel::_ResizeWindow()
{
	CDCHandle dc = GetDC();
	_SetFont(dc);

	CSize size = m_layout->GetContentSize();
	SetWindowPos(NULL, 0, 0, size.cx, size.cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	ReleaseDC(dc);
}

void WeaselPanel::_CreateLayout()
{
	if (m_layout != NULL)
		delete m_layout;

	m_layout = new VerticalLayout(m_style, m_ctx, m_status);
}

//更新界面
void WeaselPanel::Refresh()
{
	_CreateLayout();

	CDCHandle dc = GetDC();
	_SetFont(dc);
	m_layout->DoLayout(dc, m_romanFont, m_otherFont);
	ReleaseDC(dc);

	_ResizeWindow();
	_RepositionWindow();
	RedrawWindow();
}

void WeaselPanel::_HighlightText(CDCHandle dc, CRect rc, COLORREF color)
{
	rc.InflateRect(m_style.hilite_padding, m_style.hilite_padding);
	CBrush brush;
	brush.CreateSolidBrush(color);
	CBrush oldBrush = dc.SelectBrush(brush);
	CPen pen;
	pen.CreatePen(PS_SOLID, 0, color);
	CPen oldPen = dc.SelectPen(pen);
	CPoint ptRoundCorner(m_style.round_corner, m_style.round_corner);
	dc.RoundRect(rc, ptRoundCorner);
	dc.SelectBrush(oldBrush);
	dc.SelectPen(oldPen);
}

bool WeaselPanel::_DrawPreedit(Text const& text, CDCHandle dc, CRect const& rc)
{
	bool drawn = false;
	std::wstring const& t = text.str;
	if (!t.empty())
	{
		weasel::TextRange range;
		std::vector<weasel::TextAttribute> const& attrs = text.attributes;
		for (size_t j = 0; j < attrs.size(); ++j)
			if (attrs[j].type == weasel::HIGHLIGHTED)
				range = attrs[j].range;

		if (range.start < range.end)
		{
			CSize selStart, selEnd;
			dc.GetTextExtent(t.c_str(), range.start, &selStart);
			dc.GetTextExtent(t.c_str(), range.end, &selEnd);
			int x = rc.left;
			if (range.start > 0)
			{
				// zzz
				std::wstring str_before(t.substr(0, range.start));
				CRect rc_before(x, rc.top, rc.left + selStart.cx, rc.bottom);
				_TextOut(dc, x, rc.top, rc_before, str_before.c_str(), str_before.length());
				x += selStart.cx + m_style.hilite_spacing;
			}
			{
				// zzz[yyy]
				std::wstring str_highlight(t.substr(range.start, range.end - range.start));
				CRect rc_hi(x, rc.top, x + (selEnd.cx - selStart.cx), rc.bottom);
				_HighlightText(dc, rc_hi, m_style.hilited_back_color);
				dc.SetTextColor(m_style.hilited_text_color);
				dc.SetBkColor(m_style.hilited_back_color);
				_TextOut(dc, x, rc.top, rc_hi, str_highlight.c_str(), str_highlight.length());
				dc.SetTextColor(m_style.text_color);
				dc.SetBkColor(m_style.back_color);
				x += (selEnd.cx - selStart.cx);
			}
			if (range.end < static_cast<int>(t.length()))
			{
				// zzz[yyy]xxx
				x += m_style.hilite_spacing;
				std::wstring str_after(t.substr(range.end));
				CRect rc_after(x, rc.top, rc.right, rc.bottom);
				_TextOut(dc, x, rc.top, rc_after, str_after.c_str(), str_after.length());
			}
		}
		else
		{
			CRect rcText(rc.left, rc.top, rc.right, rc.bottom);
			_TextOut(dc, rc.left, rc.top, rcText, t.c_str(), t.length());
		}
		drawn = true;
	}
	return drawn;
}

bool WeaselPanel::_DrawCandidates(CDCHandle dc)
{
	bool drawn = false;
	CSize size, romanSize, charSize;
	CPoint point, romanPoint, charPoint;

	const std::vector<Text> &candidates(m_ctx.cinfo.candies);
	const std::vector<Text> &comments(m_ctx.cinfo.comments);
	const std::vector<Text> &labels(m_ctx.cinfo.labels);

	const int space = m_style.hilite_spacing;
	const int CHAR_MARGIN = m_style.hilite_spacing / 2;
	const int RUBY_MARGIN = CHAR_MARGIN;

	int y = m_style.margin_y;

	for (size_t i = 0; i < candidates.size() && i < MAX_CANDIDATES_COUNT; ++i)
	{
		if (i == m_ctx.cinfo.highlighted)
		{
			_HighlightText(dc, m_layout->GetHighlightRect(), m_style.hilited_candidate_back_color);
			dc.SetTextColor(m_style.hilited_label_text_color);
		}
		else
			dc.SetTextColor(m_style.label_text_color);

		std::wstring label = m_layout->GetLabelText(labels, i, m_style.label_text_format.c_str());
		std::u32string text = conv16to32(candidates.at(i).str);

		const std::unordered_set<char32_t>& charWithTwoSyllables = text.length() == 1 ? charWithTwoSyllablesIndividual : charWithTwoSyllablesInPhrase;
		
		bool firstComment = true;
		columns lines(conv16to32(comments.at(i).str), lineSep);
		for (const std::u32string& comment : lines) {
			columns tokens(comment, tabSep);
			auto tokens_it = tokens.begin();

			std::u32string roman = *tokens_it++;
			columns romans(roman, spaceSep);
			auto romans_it = romans.begin();

			std::wstring roman16 = conv32to16(roman);
			dc.SelectFont(m_romanFont);
			dc.GetTextExtent(roman16.c_str(), roman16.length(), &size);
			const long romanHeight = size.cy;

			std::wstring other = label + conv32to16(U'\t' + text + U'\t' + comment.substr(comment.find(U'\t')));
			dc.SelectFont(m_otherFont);
			dc.GetTextExtent(other.c_str(), other.length(), &size);
			const long otherHeight = size.cy;

			int x = m_style.margin_x;
			if (firstComment) {
				dc.GetTextExtent(label.c_str(), label.length(), &size);
				point.SetPoint(x, y + romanHeight);
				_TextOut(dc, point.x, point.y, CRect(point, size), label.c_str(), label.length());
				firstComment = false;
			}
			x += m_layout->labelWidth + space;

			int textX = x;
			for (const char32_t& ch : text) {
				std::wstring ch16 = conv32to16(std::u32string(&ch));
				std::u32string roman = charWithZeroSyllables.count(ch) || romans_it == romans.end() ? U"" : (*romans_it++ + (charWithTwoSyllables.count(ch) && romans_it != romans.end() ? U" " + *romans_it++ : U""));
				std::wstring roman16 = conv32to16(roman);

				dc.SelectFont(m_romanFont);
				dc.GetTextExtent(roman16.c_str(), roman16.length(), &romanSize);
				const long romanWidth = romanSize.cx;
				dc.SelectFont(m_otherFont);
				dc.GetTextExtent(ch16.c_str(), ch16.length(), &charSize);
				const long charWidth = charSize.cx;
				const long width = max(romanWidth, charWidth);

				dc.SelectFont(m_romanFont);
				romanPoint.SetPoint(textX + (width - romanWidth) / 2, y);
				_TextOut(dc, romanPoint.x, romanPoint.y, CRect(romanPoint, romanSize), roman16.c_str(), roman16.length());

				dc.SelectFont(m_otherFont);
				charPoint.SetPoint(textX + (width - charWidth) / 2, y + romanHeight);
				_TextOut(dc, charPoint.x, charPoint.y, CRect(charPoint, charSize), ch16.c_str(), ch16.length());

				textX += width + CHAR_MARGIN;
			}
			x += m_layout->textWidth + space;

			dc.SelectFont(m_otherFont);
			for (size_t j = 0; tokens_it != tokens.end(); tokens_it++, j++) {
				std::wstring token16 = conv32to16(*tokens_it);
				dc.GetTextExtent(token16.c_str(), token16.length(), &size);
				point.SetPoint(x, y + romanHeight);
				_TextOut(dc, point.x, point.y, CRect(point, size), token16.c_str(), token16.length());
				x += m_layout->restColumnWidths[j] + space;
			}

			y += romanHeight + RUBY_MARGIN + otherHeight + m_style.candidate_spacing;
		}
		drawn = true;
	}
	dc.SetTextColor(m_style.text_color);
	return drawn;
}

//draw client area
void WeaselPanel::DoPaint(CDCHandle dc)
{
	CRect rc;
	GetClientRect(&rc);

	// background
	{
		CBrush brush;
		brush.CreateSolidBrush(m_style.back_color);
		CRgn rgn;
		rgn.CreateRectRgnIndirect(&rc);
		dc.FillRgn(rgn, brush);

		CPen pen;
		pen.CreatePen(PS_SOLID | PS_INSIDEFRAME, m_style.border, m_style.border_color);
		CPenHandle oldPen = dc.SelectPen(pen);
		CBrushHandle oldBrush = dc.SelectBrush(brush);
		dc.Rectangle(&rc);
		dc.SelectPen(oldPen);
		dc.SelectBrush(oldBrush);
	}

	_SetFont(dc);
	CFontHandle oldFont = dc.SelectFont(m_otherFont);

	dc.SetTextColor(m_style.text_color);
	dc.SetBkColor(m_style.back_color);
	dc.SetBkMode(TRANSPARENT);
	
	bool drawn = false;

	// draw preedit string
	if (!m_layout->IsInlinePreedit())
		drawn |= _DrawPreedit(m_ctx.preedit, dc, m_layout->GetPreeditRect());
	
	// draw auxiliary string
	drawn |= _DrawPreedit(m_ctx.aux, dc, m_layout->GetAuxiliaryRect());

	// status icon (I guess Metro IME stole my idea :)
	if (m_layout->ShouldDisplayStatusIcon())
	{
		const CRect iconRect(m_layout->GetStatusIconRect());
		CIcon& icon(m_status.disabled ? m_iconDisabled : m_status.ascii_mode ? m_iconAlpha : m_iconEnabled);
		dc.DrawIconEx(iconRect.left, iconRect.top, icon, 0, 0);
		drawn = true;
	}

	// draw candidates
	drawn |= _DrawCandidates(dc);

	/* Nothing drawn, hide candidate window */
	if (!drawn)
		ShowWindow(SW_HIDE);

	dc.SelectFont(oldFont);	
}

LRESULT WeaselPanel::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Refresh();
	//CenterWindow();
	GetWindowRect(&m_inputPos);
	return TRUE;
}

LRESULT WeaselPanel::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}

void WeaselPanel::CloseDialog(int nVal)
{
	
}

void WeaselPanel::MoveTo(RECT const& rc)
{
	const int distance = 6;
	m_inputPos = rc;
	m_inputPos.OffsetRect(0, distance);
	_RepositionWindow();
}

void WeaselPanel::_RepositionWindow()
{
	RECT rcWorkArea;
	//SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
	memset(&rcWorkArea, 0, sizeof(rcWorkArea));
	HMONITOR hMonitor = MonitorFromRect(m_inputPos, MONITOR_DEFAULTTONEAREST);
	if (hMonitor)
	{
		MONITORINFO info;
		info.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo(hMonitor, &info))
		{
			rcWorkArea = info.rcWork;
		}
	}
	RECT rcWindow;
	GetWindowRect(&rcWindow);
	int width = (rcWindow.right - rcWindow.left);
	int height = (rcWindow.bottom - rcWindow.top);
	// keep panel visible
	rcWorkArea.right -= width;
	rcWorkArea.bottom -= height;
	int x = m_inputPos.left;
	int y = m_inputPos.bottom;
	if (x > rcWorkArea.right)
		x = rcWorkArea.right;
	if (x < rcWorkArea.left)
		x = rcWorkArea.left;
	// show panel above the input focus if we're around the bottom
	if (y > rcWorkArea.bottom)
		y = m_inputPos.top - height;
	if (y > rcWorkArea.bottom)
		y = rcWorkArea.bottom;
	if (y < rcWorkArea.top)
		y = rcWorkArea.top;
	// memorize adjusted position (to avoid window bouncing on height change)
	m_inputPos.bottom = y;
	SetWindowPos(HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE);
}

static HRESULT _TextOutWithFallback(CDCHandle dc, int x, int y, CRect const& rc, LPCWSTR psz, int cch)
{
    SCRIPT_STRING_ANALYSIS ssa;
    HRESULT hr;

    hr = ScriptStringAnalyse(
        dc,
        psz, cch,
        2 * cch + 16,
        -1,
        SSA_GLYPHS|SSA_FALLBACK|SSA_LINK,
        0,
        NULL, // control
        NULL, // state
        NULL, // piDx
        NULL,
        NULL, // pbInClass
        &ssa);

    if (SUCCEEDED(hr))
    {
        hr = ScriptStringOut(
            ssa, x, y, 0,
            &rc,
            0, 0, FALSE);
    }

	ScriptStringFree(&ssa);
	return hr;
}

void WeaselPanel::_TextOut(CDCHandle dc, int x, int y, CRect const& rc, LPCWSTR psz, int cch)
{
	if (FAILED(_TextOutWithFallback(dc, x, y, rc, psz, cch))) {
		dc.TextOutW(x, y, psz, cch);
	}
}
