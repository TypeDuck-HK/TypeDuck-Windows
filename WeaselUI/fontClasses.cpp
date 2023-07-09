#include "stdafx.h"
#include <string>
#include "fontClasses.h"

std::vector<std::wstring> ws_split(const std::wstring& in, const std::wstring& delim) 
{
	std::wregex re{ delim };
	return std::vector<std::wstring> {
		std::wsregex_token_iterator(in.begin(), in.end(), re, -1),
			std::wsregex_token_iterator()
	};
}

std::vector<std::string> c_split(const char* in, const char* delim) 
{
	std::regex re{ delim };
	return std::vector<std::string> {
		std::cregex_token_iterator(in, in + strlen(in),re, -1),
			std::cregex_token_iterator()
	};
}

std::vector<std::wstring> wc_split(const wchar_t* in, const wchar_t* delim) 
{
	std::wregex re{ delim };
	return std::vector<std::wstring> {
		std::wcregex_token_iterator(in, in + wcslen(in),re, -1),
			std::wcregex_token_iterator()
	};
}

DirectWriteResources::DirectWriteResources(weasel::UIStyle& style, UINT dpi = 0) :
	_style(style),
	dpiScaleX_(0),
	dpiScaleY_(0),
	pD2d1Factory(NULL),
	pDWFactory(NULL),
	pRenderTarget(NULL),
	pPreeditTextFormat(NULL),
	pTextFormat(NULL),
	pLabelTextFormat(NULL),
	pCommentTextFormat(NULL),
	pHintTextFormat(NULL),
	pEngTextFormat(NULL),
	pHinTextFormat(NULL),
	pUrdTextFormat(NULL),
	pNepTextFormat(NULL),
	pIndTextFormat(NULL),
	pPageMarkFormat(NULL),
	pEntryTextFormat(NULL),
	pPronTextFormat(NULL)
{
	// prepare d2d1 resources
	HRESULT hResult = S_OK;
	// create factory
	if (pD2d1Factory == NULL)
		hResult = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pD2d1Factory);
	// create IDWriteFactory
	if (pDWFactory == NULL)
		hResult = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWFactory));
	/* ID2D1HwndRenderTarget */
	if (pRenderTarget == NULL)
	{
		const D2D1_PIXEL_FORMAT format = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
		const D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, format);
		pD2d1Factory->CreateDCRenderTarget(&properties, &pRenderTarget);
		pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
		pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
	}
	//get the dpi information
	if (dpi == 0)
	{
		pD2d1Factory->GetDesktopDpi(&dpiScaleX_, &dpiScaleY_);
		dpiScaleX_ /= 72.0f;
		dpiScaleY_ /= 72.0f;
	}
	else
	{
		dpiScaleX_ = dpi / 72.0f;
		dpiScaleY_ = dpi / 72.0f;
	}

	InitResources(style, dpi);
}

DirectWriteResources::~DirectWriteResources()
{
	SafeRelease(&pPreeditTextFormat);
	SafeRelease(&pTextFormat);
	SafeRelease(&pLabelTextFormat);
	SafeRelease(&pCommentTextFormat);
	SafeRelease(&pHintTextFormat);
	SafeRelease(&pEngTextFormat);
	SafeRelease(&pHinTextFormat);
	SafeRelease(&pUrdTextFormat);
	SafeRelease(&pNepTextFormat);
	SafeRelease(&pIndTextFormat);
	SafeRelease(&pPageMarkFormat);
	SafeRelease(&pEntryTextFormat);
	SafeRelease(&pPronTextFormat);
	SafeRelease(&pRenderTarget);
	SafeRelease(&pDWFactory);
	SafeRelease(&pD2d1Factory);
	SafeRelease(&pTextLayout);
}

HRESULT DirectWriteResources::_SetupTextFormat(std::wstring font_face, int font_point, IDWriteTextFormat1** ppTextFormat,
											   DWRITE_READING_DIRECTION reading_direction) {
	// split the font face and set up weight and style
	std::vector<std::wstring> fontFaceStrVector = ws_split(font_face, L",");
	DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL;
	DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL;
	_ParseFontFace(fontFaceStrVector[0], fontWeight, fontStyle);
	fontFaceStrVector[0] = std::regex_replace(fontFaceStrVector[0], std::wregex(L":[a-zA-Z_]+", std::wregex::icase), L"");

	// create the text format
	const std::wstring _mainFontFace = L"_InvalidFontName_";
	HRESULT hResult = pDWFactory->CreateTextFormat(_mainFontFace.c_str(), NULL,
			fontWeight, fontStyle, DWRITE_FONT_STRETCH_NORMAL,
			font_point * dpiScaleX_, L"", reinterpret_cast<IDWriteTextFormat**>(ppTextFormat));

	// set up the text format
	if (*ppTextFormat != NULL) {
		(*ppTextFormat)->SetReadingDirection(reading_direction);
		(*ppTextFormat)->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		(*ppTextFormat)->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		(*ppTextFormat)->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		_SetFontFallback(*ppTextFormat, fontFaceStrVector);
	}

	return hResult;
}

HRESULT DirectWriteResources::InitResources(UIStyle& style, UINT dpi, bool vertical_text)
{
	_style = style;

	if (dpi) {
		dpiScaleX_ = dpi / 72.0f;
		dpiScaleY_ = dpi / 72.0f;
	}

	SafeRelease(&pPreeditTextFormat);
	SafeRelease(&pTextFormat);
	SafeRelease(&pLabelTextFormat);
	SafeRelease(&pCommentTextFormat);
	SafeRelease(&pHintTextFormat);
	SafeRelease(&pEngTextFormat);
	SafeRelease(&pHinTextFormat);
	SafeRelease(&pUrdTextFormat);
	SafeRelease(&pNepTextFormat);
	SafeRelease(&pIndTextFormat);
	SafeRelease(&pPageMarkFormat);
	SafeRelease(&pEntryTextFormat);
	SafeRelease(&pPronTextFormat);

	return S_OK
		| _SetupTextFormat(style.preedit_font_face, style.preedit_font_point, &pPreeditTextFormat)
		| _SetupTextFormat(style.font_face, style.font_point, &pTextFormat)
		| _SetupTextFormat(style.label_font_face, style.label_font_point, &pLabelTextFormat)
		| _SetupTextFormat(style.comment_font_face, style.comment_font_point, &pCommentTextFormat)
		| _SetupTextFormat(style.hint_font_face, style.hint_font_point, &pHintTextFormat)
		| _SetupTextFormat(style.eng_font_face, style.eng_font_point, &pEngTextFormat)
		| _SetupTextFormat(style.hin_font_face, style.hin_font_point, &pHinTextFormat)
		| _SetupTextFormat(style.urd_font_face, style.urd_font_point, &pUrdTextFormat, DWRITE_READING_DIRECTION_RIGHT_TO_LEFT)
		| _SetupTextFormat(style.nep_font_face, style.nep_font_point, &pNepTextFormat)
		| _SetupTextFormat(style.ind_font_face, style.ind_font_point, &pIndTextFormat)
		| _SetupTextFormat(style.font_face, style.font_point * 7 / 4, &pPageMarkFormat)
		| _SetupTextFormat(style.font_face, style.font_point * 4 / 3, &pEntryTextFormat)
		| _SetupTextFormat(style.hint_font_face, style.hint_font_point * 4 / 3, &pPronTextFormat);
}

void weasel::DirectWriteResources::SetDpi(UINT dpi)
{
	InitResources(_style, dpi);
}

void DirectWriteResources::_ParseFontFace(const std::wstring fontFaceStr, DWRITE_FONT_WEIGHT& fontWeight, DWRITE_FONT_STYLE& fontStyle)
{
	if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":thin", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_THIN;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":extra_light", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_EXTRA_LIGHT;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":ultra_light", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_ULTRA_LIGHT;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":light", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_LIGHT;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":semi_light", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_SEMI_LIGHT;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":medium", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_MEDIUM;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":demi_bold", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_DEMI_BOLD;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":semi_bold", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_SEMI_BOLD;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":bold", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_BOLD;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":extra_bold", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_EXTRA_BOLD;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":ultra_bold", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_ULTRA_BOLD;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":black", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_BLACK;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":heavy", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_HEAVY;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":extra_black", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_EXTRA_BLACK;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":ultra_black", std::wregex::icase)))
		fontWeight = DWRITE_FONT_WEIGHT_ULTRA_BLACK;
	else
		fontWeight = DWRITE_FONT_WEIGHT_NORMAL;

	if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":italic", std::wregex::icase)))
		fontStyle = DWRITE_FONT_STYLE_ITALIC;
	else if (std::regex_search(fontFaceStr, std::wsmatch(), std::wregex(L":oblique", std::wregex::icase)))
		fontStyle = DWRITE_FONT_STYLE_OBLIQUE;
	else
		fontStyle = DWRITE_FONT_STYLE_NORMAL;
}

void DirectWriteResources::_SetFontFallback(IDWriteTextFormat1* textFormat, std::vector<std::wstring> fontVector)
{
	IDWriteFontFallback* pSysFallback;
	pDWFactory->GetSystemFontFallback(&pSysFallback);
	IDWriteFontFallback* pFontFallback = NULL;
	IDWriteFontFallbackBuilder* pFontFallbackBuilder = NULL;
	pDWFactory->CreateFontFallbackBuilder(&pFontFallbackBuilder);
	std::vector<std::wstring> fallbackFontsVector;
	for (UINT32 i = 0; i < fontVector.size(); i++)
	{
		fallbackFontsVector = ws_split(fontVector[i], L":");
		std::wstring _fontFaceWstr, firstWstr, lastWstr;
		if (fallbackFontsVector.size() == 3)
		{
			_fontFaceWstr = fallbackFontsVector[0];
			firstWstr = fallbackFontsVector[1];
			lastWstr = fallbackFontsVector[2];
			if (lastWstr.empty())
				lastWstr = L"10ffff";
			if (firstWstr.empty())
				firstWstr = L"0";
		}
		else if (fallbackFontsVector.size() == 2)	// fontName : codepoint
		{
			_fontFaceWstr = fallbackFontsVector[0];
			firstWstr = fallbackFontsVector[1];
			if (firstWstr.empty())
				firstWstr = L"0";
			lastWstr = L"10ffff";
		}
		else if (fallbackFontsVector.size() == 1)	// if only font defined, use all range
		{
			_fontFaceWstr = fallbackFontsVector[0];
			firstWstr = L"0";
			lastWstr = L"10ffff";
		}
		UINT first = 0, last = 0x10ffff;
		try {
			first = std::stoi(firstWstr.c_str(), 0, 16);
		}
		catch(...){
			first = 0;
		}
		try {
			last = std::stoi(lastWstr.c_str(), 0, 16);
		}
		catch(...){
			last = 0x10ffff;
		}
		DWRITE_UNICODE_RANGE range = { first, last };
		const  WCHAR* familys = { _fontFaceWstr.c_str() };
		pFontFallbackBuilder->AddMapping(&range, 1, &familys, 1);
		fallbackFontsVector.swap(std::vector<std::wstring>());
	}
	// add system defalt font fallback
	pFontFallbackBuilder->AddMappings(pSysFallback);
	pFontFallbackBuilder->CreateFontFallback(&pFontFallback);
	textFormat->SetFontFallback(pFontFallback);
	fallbackFontsVector.swap(std::vector<std::wstring>());
	SafeRelease(&pFontFallback);
	SafeRelease(&pSysFallback);
	SafeRelease(&pFontFallbackBuilder);
}

