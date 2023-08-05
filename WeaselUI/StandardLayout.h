#pragma once

#include "Layout.h"
#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#include "MultiHintPanel.h"

namespace weasel
{
	const int MAX_CANDIDATES_COUNT = 10;
	const int STATUS_ICON_SIZE = GetSystemMetrics(SM_CXICON);

	class StandardLayout: public Layout
	{
	public:
		StandardLayout(const UIStyle &style, const Context &context, const Status &status) : Layout(style, context, status), _multiHintPanel(MultiHintPanel::GetInstance()) {}

		/* Layout */

		virtual void DoLayout(CDCHandle dc, DirectWriteResources* pDWR = NULL) = 0;
		virtual CSize GetContentSize() const { return _contentSize; }
		virtual CRect GetPreeditRect() const { return _preeditRect; }
		virtual CRect GetAuxiliaryRect() const { return _auxiliaryRect; }
		virtual CRect GetHighlightRect() const { return _highlightRect; }
		virtual CRect GetDictionaryRect() const { return _dictionaryRect; }
		virtual std::vector<DictionaryPanelRects> GetDictionaryPanelRects() const { return _dictionaryPanelRects; }
		virtual std::vector<CandidateFieldRects> GetCandidateFieldRects(int id) const { return _candidateFieldRects[id]; }
		virtual CRect GetCandidateRect(int id) const { return _candidateRects[id]; }
		virtual CRect GetStatusIconRect() const { return _statusIconRect; }
		virtual std::wstring GetLabelText(const std::vector<Text> &labels, int id, const wchar_t *format) const;
		virtual bool IsInlinePreedit() const;
		virtual bool ShouldDisplayStatusIcon() const;
		virtual IsToRoundStruct GetRoundInfo(int id) { return _roundInfo[id]; } 
		virtual IsToRoundStruct GetTextRoundInfo() { return _textRoundInfo; }
		virtual CRect GetContentRect() { return _contentRect; }
#ifdef USE_PAGER_MARK
		virtual CRect GetPrepageRect() { return _prePageRect; }
		virtual CRect GetNextpageRect() { return _nextPageRect; }
#endif /*  USE_PAGER_MARK */

		void GetTextSizeDW(const std::wstring text, IDWriteTextFormat1* pTextFormat, DirectWriteResources* pDWR, LPSIZE lpSize, int characterSpacing = 0) const;

	protected:
		/* Utility functions */
		CSize GetPreeditSize(CDCHandle dc, const weasel::Text& text, IDWriteTextFormat1* pTextFormat = NULL, DirectWriteResources* pDWR = NULL) const;
		bool _IsHighlightOverCandidateWindow(CRect& rc, CDCHandle& dc);
		void _PrepareRoundInfo(CDCHandle& dc);

		void UpdateStatusIconLayout(int* width, int* height);

		CSize _contentSize;
		CRect _preeditRect, _auxiliaryRect, _highlightRect;
		CRect _candidateRects[MAX_CANDIDATES_COUNT];
		[[deprecated]] CRect _candidateLabelRects[MAX_CANDIDATES_COUNT];
		[[deprecated]] CRect _candidateTextRects[MAX_CANDIDATES_COUNT];
		[[deprecated]] CRect _candidateCommentRects[MAX_CANDIDATES_COUNT];
		CRect _statusIconRect;
		CRect _bgRect;
		CRect _contentRect;
		CRect _dictionaryRect;
		std::vector<DictionaryPanelRects> _dictionaryPanelRects;
		std::vector<CandidateFieldRects> _candidateFieldRects[MAX_CANDIDATES_COUNT];
		IsToRoundStruct _roundInfo[MAX_CANDIDATES_COUNT];
		IsToRoundStruct _textRoundInfo;
		MultiHintPanel* _multiHintPanel;
#ifdef USE_PAGER_MARK
		CRect _prePageRect;
		CRect _nextPageRect;
		const std::wstring pre = L"‹";
		const std::wstring next = L"›";
#endif /* USE_PAGER_MARK */
	};
};
