#pragma once
#include <WeaselCommon.h>
#include <WeaselUI.h>
#include <vector>
#include <regex>
#include <iterator>
#include <d2d1.h>
#include <dwrite_2.h>

using namespace weasel;

namespace weasel
{
	template <class T> void SafeRelease(T** ppT)
	{
		if (*ppT)
		{
			(*ppT)->Release();
			*ppT = NULL;
		}
	}
	class DirectWriteResources
	{
	public:
		DirectWriteResources(weasel::UIStyle& style, UINT dpi);
		~DirectWriteResources();

		HRESULT InitResources(UIStyle& style, UINT dpi = 0, bool vertical_text = false);
		void SetDpi(UINT dpi);
		float dpiScaleX_, dpiScaleY_;
		ID2D1Factory* pD2d1Factory;
		IDWriteFactory2* pDWFactory;
		ID2D1DCRenderTarget* pRenderTarget;
		IDWriteTextFormat1* pPreeditTextFormat;
		IDWriteTextFormat1* pTextFormat;
		IDWriteTextFormat1* pLabelTextFormat;
		IDWriteTextFormat1* pCommentTextFormat;
		IDWriteTextFormat1* pHintTextFormat;
		IDWriteTextFormat1* pEngTextFormat;
		IDWriteTextFormat1* pHinTextFormat;
		IDWriteTextFormat1* pUrdTextFormat;
		IDWriteTextFormat1* pNepTextFormat;
		IDWriteTextFormat1* pIndTextFormat;
		IDWriteTextFormat1* pPageMarkFormat;

		IDWriteTextFormat1* pEntryTextFormat;
		IDWriteTextFormat1* pPronTextFormat;
		IDWriteTextFormat1* pPronTypeTextFormat;
		IDWriteTextFormat1* pPOSTextFormat;
		IDWriteTextFormat1* pRegisterTextFormat;
		IDWriteTextFormat1* pLblTextFormat;
		IDWriteTextFormat1* pFieldKeyTextFormat;
		IDWriteTextFormat1* pFieldValueTextFormat;
		IDWriteTextFormat1* pMoreLanguagesHeadTextFormat;

		IDWriteTextLayout2* pTextLayout;
	private:
		UIStyle& _style;
		HRESULT _SetupTextFormat(std::wstring font_face, int font_point, IDWriteTextFormat1** ppTextFormat,
								 DWRITE_READING_DIRECTION reading_direction = DWRITE_READING_DIRECTION_LEFT_TO_RIGHT);
		void _ParseFontFace(const std::wstring fontFaceStr, DWRITE_FONT_WEIGHT& fontWeight, DWRITE_FONT_STYLE& fontStyle);
		void _SetFontFallback(IDWriteTextFormat1* pTextFormat, std::vector<std::wstring> fontVector);
	};
};
