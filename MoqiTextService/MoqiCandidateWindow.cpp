//
//    Copyright (C) 2026
//

#include "MoqiCandidateWindow.h"
#include "MoqiTextService.h"

#include <LibIME2/src/DebugLogConfig.h>
#include <LibIME2/src/DebugLogFile.h>
#include <LibIME2/src/DrawUtils.h>
#include <LibIME2/src/EditSession.h>
#include <LibIME2/src/TextService.h>
#include <windowsx.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <sstream>

namespace {

constexpr COLORREF kWindowBackground = RGB(255, 255, 255);       // panel_background
constexpr COLORREF kDictionaryBackground = RGB(246, 243, 237);   // dictionary_background
constexpr COLORREF kInputBufferBackground = RGB(246, 234, 216);  // input_buffer_background
constexpr COLORREF kInputBufferText = RGB(70, 58, 42);           // input_buffer_text
constexpr COLORREF kWindowBorder = RGB(222, 217, 207);           // panel_border
constexpr COLORREF kDividerColor = RGB(222, 217, 207);
constexpr COLORREF kItemText = RGB(36, 34, 30);                  // text_primary
constexpr COLORREF kSecondaryText = RGB(105, 98, 88);            // text_secondary
constexpr COLORREF kPronunciationText = RGB(102, 93, 82);        // pronunciation_text
constexpr COLORREF kDefinitionText = RGB(78, 72, 63);            // definition_text
constexpr COLORREF kDisabledText = RGB(168, 160, 148);           // disabled_text
constexpr COLORREF kLinkText = RGB(151, 102, 31);                // link_text
constexpr COLORREF kSelectedBackground = RGB(254, 220, 156);     // selection_background
constexpr COLORREF kSelectedText = RGB(36, 34, 30);
constexpr COLORREF kPosPillBackground = RGB(246, 243, 237);
constexpr COLORREF kPosPillBorder = RGB(180, 171, 157);
constexpr COLORREF kPosPillText = RGB(86, 79, 69);
constexpr int kDefaultCandidateSpacing = 20;
constexpr int kTypeDuckCandidatePanelRenderer = 1;
constexpr int kMovementRevealThreshold = 2;
constexpr int kPageNavNone = -1;
constexpr int kPageNavPrevious = 0;
constexpr int kPageNavNext = 1;
constexpr int kWindowDpiBaseline = 96;
constexpr int kBorderWidth = 1;
constexpr int kInitialCompactPanelPaddingX = 7;
constexpr int kInitialCompactPanelPaddingY = 3;
constexpr int kInitialBorderRadius = 4;
constexpr int kInitialCandidateMinWidth = 200;
constexpr int kInitialPanelGap = 10;
constexpr int kInitialJyutpingColumnWidth = 84;
constexpr int kInitialHonziColumnWidth = 86;
constexpr int kInitialNoteColumnWidth = 78;
constexpr int kInitialDefinitionColumnWidth = 230;
constexpr int kInitialDictionaryPanelMinHeight = 260;
constexpr int kPanelPaddingX = 10;
constexpr int kPanelPaddingY = 6;
constexpr int kImmersivePanelPaddingX = 12;
constexpr int kImmersivePanelPaddingY = 8;
constexpr int kCandidateMinWidth = 150;
constexpr int kCandidateBorderRadius = 8;
constexpr int kCandidateLabelGap = 6;
constexpr int kCandidateCommentGap = 8;
constexpr int kCandidatePreeditGap = 8;
constexpr int kCandidateRowPaddingY = 5;
constexpr int kCandidateRowInnerGap = 6;
constexpr int kCandidateRowCornerRadius = 5;
constexpr int kCandidateSelectionInsetX = 8;
constexpr int kCandidateIndicatorInsetRight = 8;
constexpr int kCandidateJyutpingMinWidth = 42;
constexpr int kCandidateHonziMinWidth = 34;
constexpr int kCandidateNoteMinWidth = 42;
constexpr int kCandidateDefinitionMinWidth = 78;
constexpr int kCandidateColumnPad = 8;
constexpr int kCandidateIndicatorColumnWidth = 22;
constexpr int kCandidateBodyLineMinHeight = 25;
constexpr int kCandidateRowLineMinHeight = 20;
constexpr int kPageNavWidth = 64;
constexpr int kPageNavPreeditlessHeight = 30;
constexpr int kPageNavGlyphPointSize = 28;
constexpr int kPageNavGlyphYOffset = 7;
constexpr int kPageNavHoverRadius = 6;
constexpr int kPreeditExtraHeight = 8;
constexpr int kPreeditTextWidthPadding = 28;
constexpr int kPreeditPlainMargin = 4;
constexpr int kPreeditActivePaddingX = 6;
constexpr int kPreeditActiveCornerRadius = 5;
constexpr int kPreeditCursorWidth = 2;
constexpr int kPreeditCursorVerticalInset = 1;
constexpr int kDictionaryPanelMinWidth = 260;
constexpr int kDictionaryPanelMinHeight = 150;
constexpr int kDictionaryPanelTopPadding = 18;
constexpr int kDictionaryPanelHorizontalPadding = 16;
constexpr int kDictionaryPanelVerticalPadding = 16;
constexpr int kDictionaryWidthSlack = 16;
constexpr int kDictionaryMeasureWidthPadding = 32;
constexpr int kDictionaryHeaderGap = 20;
constexpr int kDictionarySectionSpacing = 16;
constexpr int kDictionaryPosPadding = 3;
constexpr int kDictionaryPosGap = 8;
constexpr int kDictionaryDefinitionGap = 18;
constexpr int kDictionaryFieldSpacing = 5;
constexpr int kDictionaryFieldGap = 12;
constexpr int kDictionaryMoreLanguagesSpacing = 5;
constexpr int kDictionaryEntrySpacing = 18;
constexpr int kDictionaryBodyMinHeight = 24;
constexpr int kDictionaryPillHeight = 20;
constexpr int kDictionaryPillCornerRadius = 4;
constexpr int kDictionaryPillBaselineOffset = 2;
constexpr int kDictionaryPillGap = 8;
constexpr int kDictionaryLabelBaselineOffset = 1;
constexpr int kDictionaryScrollViewportMinHeight = 320;
constexpr int kDictionaryPaintCullPadding = 80;
constexpr int kDictionaryScrollStep = 36;
constexpr int kDictionaryScrollTrackPadding = 8;
constexpr int kDictionaryScrollTrackWidth = 4;
constexpr int kDictionaryScrollThumbMinHeight = 24;
constexpr int kDictionaryBridgeOverlap = 6;
constexpr int kInlineBodySpacingUnit = 4;
constexpr int kUnboundedMeasureHeight = 32000;
constexpr COLORREF kDictionaryScrollTrack = RGB(224, 218, 208);
constexpr COLORREF kDictionaryScrollThumb = RGB(166, 153, 132);
constexpr const wchar_t* kInputBufferFontName = L"Microsoft JhengHei UI";

Moqi::TextService* productTextService(Ime::TextService* service) {
    return static_cast<Moqi::TextService*>(service);
}

std::wstring currentProcessPath() {
    std::wstring buffer(MAX_PATH, L'\0');
    DWORD len = ::GetModuleFileNameW(nullptr, &buffer[0], static_cast<DWORD>(buffer.size()));
    if (len == 0) {
        return L"";
    }
    while (len >= buffer.size() - 1) {
        buffer.resize(buffer.size() * 2);
        len = ::GetModuleFileNameW(nullptr, &buffer[0], static_cast<DWORD>(buffer.size()));
        if (len == 0) {
            return L"";
        }
    }
    buffer.resize(len);
    return buffer;
}

std::wstring processBaseName(const std::wstring& imagePath) {
    const size_t pos = imagePath.find_last_of(L"\\/");
    return pos == std::wstring::npos ? imagePath : imagePath.substr(pos + 1);
}

std::wstring timestampNow() {
    SYSTEMTIME st{};
    ::GetLocalTime(&st);
    wchar_t buffer[64] = {0};
    _snwprintf_s(buffer, _countof(buffer), _TRUNCATE,
        L"%04u-%02u-%02u %02u:%02u:%02u.%03u",
        st.wYear, st.wMonth, st.wDay,
        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return buffer;
}

std::wstring formatCandidateWindowLogLine(const std::wstring& message) {
    const std::wstring exeName = processBaseName(currentProcessPath());
    std::wostringstream line;
    line << L"[" << timestampNow() << L"]"
         << L"[pid=" << ::GetCurrentProcessId() << L"]"
         << L"[tid=" << ::GetCurrentThreadId() << L"]"
         << L"[exe=" << (exeName.empty() ? L"<unknown>" : exeName) << L"] "
         << message;
    return line.str();
}

int fontPointHeight(HDC hdc, int point) {
    const int dpiY = hdc ? ::GetDeviceCaps(hdc, LOGPIXELSY) : kWindowDpiBaseline;
    return -::MulDiv(point, dpiY, 72);
}

HFONT createPanelFont(HDC hdc,
                      const wchar_t* faceName,
                      int pointSize,
                      int weight = FW_NORMAL,
                      bool italic = false) {
    return ::CreateFontW(
        fontPointHeight(hdc, pointSize),
        0,
        0,
        0,
        weight,
        italic ? TRUE : FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        faceName);
}

HFONT createDerivedFont(HFONT baseFont, const wchar_t* faceName) {
    LOGFONTW lf = {};
    if (!baseFont || ::GetObjectW(baseFont, sizeof(lf), &lf) == 0) {
        HFONT defaultFont = reinterpret_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
        if (!defaultFont || ::GetObjectW(defaultFont, sizeof(lf), &lf) == 0) {
            return nullptr;
        }
    }
    wcscpy_s(lf.lfFaceName, faceName);
    return ::CreateFontIndirectW(&lf);
}

SIZE textExtent(HDC hdc, HFONT font, const std::wstring& text) {
    HGDIOBJ oldFont = ::SelectObject(hdc, font);
    SIZE size{};
    if (!text.empty()) {
        ::GetTextExtentPoint32W(hdc, text.c_str(), static_cast<int>(text.length()), &size);
        size.cx += 1;
    }
    ::SelectObject(hdc, oldFont);
    return size;
}

std::wstring joinDisplayValues(const std::vector<std::wstring>& values, const std::wstring& separator) {
    std::wstring result;
    for (const auto& value : values) {
        if (value.empty()) {
            continue;
        }
        if (!result.empty()) {
            result += separator;
        }
        result += value;
    }
    return result;
}

void appendCandidateWindowLog(const std::wstring& message) {
    if (!Ime::isTraceLoggingEnabled()) {
        return;
    }

    const wchar_t* localAppData = _wgetenv(L"LOCALAPPDATA");
    if (!localAppData || !*localAppData) {
        return;
    }

    std::wstring logDir = std::wstring(localAppData) + L"\\TypeDuckIME\\Log";
    std::wstring logPath = Ime::DebugLogFile::prepareDailyLogFilePath(
        logDir, L"candidate-window.log");
    if (logPath.empty()) {
        return;
    }

    std::wofstream stream(logPath, std::ios::app);
    if (!stream.is_open()) {
        return;
    }
    stream << formatCandidateWindowLogLine(message) << L"\n";
}

std::wstring hwndToString(HWND hwnd) {
    std::wostringstream stream;
    stream << L"0x" << std::hex << reinterpret_cast<UINT_PTR>(hwnd) << std::dec;
    return stream.str();
}

void logCandidateWindowState(const std::wstring& tag, HWND hwnd) {
    if (!hwnd) {
        appendCandidateWindowLog(tag + L" hwnd=<null>");
        return;
    }

    RECT rect{};
    ::GetWindowRect(hwnd, &rect);
    const DWORD style = static_cast<DWORD>(::GetWindowLongPtr(hwnd, GWL_STYLE));
    const DWORD exStyle = static_cast<DWORD>(::GetWindowLongPtr(hwnd, GWL_EXSTYLE));
    const HWND owner = reinterpret_cast<HWND>(::GetWindowLongPtr(hwnd, GWLP_HWNDPARENT));
    const HWND gwOwner = ::GetWindow(hwnd, GW_OWNER);
    const HWND parent = ::GetParent(hwnd);
    const HWND root = ::GetAncestor(hwnd, GA_ROOT);
    const HWND rootOwner = ::GetAncestor(hwnd, GA_ROOTOWNER);

    std::wostringstream log;
    log << tag
        << L" hwnd=" << hwndToString(hwnd)
        << L" visible=" << (::IsWindowVisible(hwnd) ? L"true" : L"false")
        << L" owner=" << hwndToString(owner)
        << L" gw_owner=" << hwndToString(gwOwner)
        << L" parent=" << hwndToString(parent)
        << L" root=" << hwndToString(root)
        << L" root_owner=" << hwndToString(rootOwner)
        << L" style=0x" << std::hex << style
        << L" exstyle=0x" << exStyle << std::dec
        << L" rect=(" << rect.left << L"," << rect.top << L"," << rect.right << L"," << rect.bottom << L")";
    appendCandidateWindowLog(log.str());
}

HWND normalizeCandidateOwnerWindow(HWND hwnd, bool immersive, const wchar_t* reason) {
    if (hwnd == nullptr) {
        return nullptr;
    }

    const HWND root = ::GetAncestor(hwnd, GA_ROOT);
    const HWND rootOwner = ::GetAncestor(hwnd, GA_ROOTOWNER);
    const HWND normalized = immersive ? hwnd : (root != nullptr ? root : hwnd);

    std::wostringstream log;
    log << L"[normalizeCandidateOwnerWindow] reason=" << reason
        << L" immersive=" << (immersive ? L"true" : L"false")
        << L" raw=" << hwndToString(hwnd)
        << L" root=" << hwndToString(root)
        << L" root_owner=" << hwndToString(rootOwner)
        << L" normalized=" << hwndToString(normalized);
    appendCandidateWindowLog(log.str());
    return normalized;
}

void enforceCandidateWindowTopmost(HWND hwnd, bool showWindow, const wchar_t* reason) {
    if (!hwnd) {
        return;
    }

    UINT flags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;
    if (showWindow) {
        flags |= SWP_SHOWWINDOW;
    }

    ::SetLastError(0);
    const BOOL ok = ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, flags);
    const DWORD error = ok ? 0 : ::GetLastError();

    std::wostringstream log;
    log << L"[enforceCandidateWindowTopmost] reason=" << reason
        << L" hwnd=" << hwndToString(hwnd)
        << L" show=" << (showWindow ? L"true" : L"false")
        << L" ok=" << (ok ? L"true" : L"false");
    if (!ok) {
        log << L" last_error=" << error;
    }
    appendCandidateWindowLog(log.str());
    logCandidateWindowState(L"[CandidateWindow::state]", hwnd);
}

HWND resolveCandidateOwnerWindow(Ime::EditSession* session) {
    HWND hwnd = nullptr;
    const wchar_t* source = L"none";
    if (session != nullptr) {
        if (ITfContext* context = session->context()) {
            ITfContextView* view = nullptr;
            if (SUCCEEDED(context->GetActiveView(&view)) && view != nullptr) {
                view->GetWnd(&hwnd);
                if (hwnd != nullptr) {
                    source = L"context-view";
                }
                view->Release();
            }
        }
    }
    if (hwnd == nullptr) {
        hwnd = ::GetFocus();
        if (hwnd != nullptr) {
            source = L"GetFocus";
        }
    }
    if (hwnd == nullptr) {
        hwnd = ::GetForegroundWindow();
        if (hwnd != nullptr) {
            source = L"GetForegroundWindow";
        }
    }
    std::wostringstream log;
    log << L"[resolveCandidateOwnerWindow] session=" << session
        << L" source=" << source << L" hwnd=" << hwnd;
    appendCandidateWindowLog(log.str());
    return hwnd;
}

} // namespace

namespace Moqi {

CandidateWindow::CandidateWindow(Ime::TextService* service, Ime::EditSession* session)
    : Ime::ImeWindow(service),
      shown_(false),
      selKeyWidth_(0),
      textWidth_(0),
      commentWidth_(0),
      itemHeight_(0),
      candPerRow_(1),
      candSpacing_(kDefaultCandidateSpacing),
      colSpacing_(0),
      rowSpacing_(0),
      padX_(service->isImmersive() ? kPanelPaddingX : kInitialCompactPanelPaddingX),
      padY_(service->isImmersive() ? kPanelPaddingY : kInitialCompactPanelPaddingY),
      labelGap_(kCandidateLabelGap),
      commentGap_(kCandidateCommentGap),
      borderWidth_(kBorderWidth),
      borderRadius_(kInitialBorderRadius),
      minWidth_(kInitialCandidateMinWidth),
      preeditHeight_(0),
      preeditGap_(kCandidatePreeditGap),
      contentTop_(0),
      panelGap_(kInitialPanelGap),
      rowPaddingY_(kCandidateRowPaddingY),
      rowInnerGap_(kCandidateRowInnerGap),
      jyutpingColumnWidth_(kInitialJyutpingColumnWidth),
      honziColumnWidth_(kInitialHonziColumnWidth),
      noteColumnWidth_(kInitialNoteColumnWidth),
      definitionColumnWidth_(kInitialDefinitionColumnWidth),
      indicatorColumnWidth_(kCandidateIndicatorColumnWidth),
      pageNavWidth_(kPageNavWidth),
      candidatePanelWidth_(0),
      candidatePanelHeight_(0),
      dictionaryPanelWidth_(0),
      dictionaryPanelTop_(0),
      dictionaryPanelHeight_(0),
      dictionaryPanelMinHeight_(kInitialDictionaryPanelMinHeight),
      dictionaryContentHeight_(0),
      dictionaryScrollOffset_(0),
      dictionaryRevealIndex_(-1),
      dictionaryHoverIndex_(-1),
      actualPointerMovementCount_(0),
      movementRevealThreshold_(kMovementRevealThreshold),
      lastMouseMovePoint_{0, 0},
      hasLastMouseMovePoint_(false),
      backgroundColor_(kWindowBackground),
      highlightColor_(kSelectedBackground),
      textColor_(kItemText),
      highlightTextColor_(kSelectedText),
      commentColor_(kItemText),
      commentHighlightColor_(kSelectedText),
      preeditCursor_(0),
      preeditSelectionStart_(0),
      preeditSelectionEnd_(0),
      currentSel_(0),
      pressedSel_(-1),
      pressedPageNavDirection_(kPageNavNone),
      hoveredPageNavDirection_(kPageNavNone),
      draggingWindow_(false),
      trackingMouse_(false),
      useCursor_(false),
      commentFont_(nullptr) {
    margin_ = 0;

    const HWND rawOwner = resolveCandidateOwnerWindow(session);
    const HWND owner = normalizeCandidateOwnerWindow(rawOwner, service->isImmersive(), L"ctor");
    create(owner, WS_POPUP | WS_CLIPCHILDREN,
           WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE);

    std::wostringstream log;
    log << L"[CandidateWindow::ctor] hwnd=" << hwnd_
        << L" raw_owner=" << rawOwner
        << L" owner=" << owner;
    appendCandidateWindowLog(log.str());
    logCandidateWindowState(L"[CandidateWindow::ctor.state]", hwnd_);
}

CandidateWindow::~CandidateWindow(void) {
}

STDMETHODIMP CandidateWindow::GetDescription(BSTR* pbstrDescription) {
    if (!pbstrDescription) {
        return E_INVALIDARG;
    }
    *pbstrDescription = SysAllocString(L"TypeDuck 候選詞視窗 Candidate window");
    return S_OK;
}

STDMETHODIMP CandidateWindow::GetGUID(GUID* pguid) {
    if (!pguid) {
        return E_INVALIDARG;
    }
    *pguid = {0x89671502, 0x43ab, 0x4939, {0x84, 0x6f, 0xe8, 0x30, 0x2b, 0x73, 0x7d, 0x3c}};
    return S_OK;
}

STDMETHODIMP CandidateWindow::Show(BOOL bShow) {
    shown_ = bShow;
    {
        std::wostringstream log;
        log << L"[CandidateWindow::Show] bShow=" << bShow
            << L" hwnd=" << hwnd_
            << L" owner=" << reinterpret_cast<HWND>(::GetWindowLongPtr(hwnd_, GWLP_HWNDPARENT))
            << L" gw_owner=" << ::GetWindow(hwnd_, GW_OWNER);
        appendCandidateWindowLog(log.str());
    }
    if (shown_) {
        show();
        enforceCandidateWindowTopmost(hwnd_, true, L"Show(TRUE)");
    } else {
        hide();
        logCandidateWindowState(L"[CandidateWindow::hide.state]", hwnd_);
    }
    return S_OK;
}

STDMETHODIMP CandidateWindow::IsShown(BOOL* pbShow) {
    if (!pbShow) {
        return E_INVALIDARG;
    }
    *pbShow = shown_;
    return S_OK;
}

STDMETHODIMP CandidateWindow::GetUpdatedFlags(DWORD* pdwFlags) {
    if (!pdwFlags) {
        return E_INVALIDARG;
    }
    *pdwFlags = TF_CLUIE_DOCUMENTMGR | TF_CLUIE_COUNT | TF_CLUIE_SELECTION |
                TF_CLUIE_STRING | TF_CLUIE_PAGEINDEX | TF_CLUIE_CURRENTPAGE;
    return S_OK;
}

STDMETHODIMP CandidateWindow::GetDocumentMgr(ITfDocumentMgr** ppdim) {
    if (!textService_ || !textService_->currentContext()) {
        return E_FAIL;
    }
    return textService_->currentContext()->GetDocumentMgr(ppdim);
}

STDMETHODIMP CandidateWindow::GetCount(UINT* puCount) {
    if (!puCount) {
        return E_INVALIDARG;
    }
    const auto* service = productTextService(textService_);
    const int totalCount = service != nullptr ? service->candidateTotalCount() : 0;
    *puCount = totalCount > 0
        ? static_cast<UINT>(totalCount)
        : static_cast<UINT>(items_.size());
    return S_OK;
}

STDMETHODIMP CandidateWindow::GetSelection(UINT* puIndex) {
    if (!puIndex) {
        return E_INVALIDARG;
    }
    assert(currentSel_ >= 0);
    const auto* service = productTextService(textService_);
    const int pageSize = service != nullptr ? service->candidatePageSize() : 0;
    const int pageIndex = service != nullptr ? service->candidatePageIndex() : 0;
    const int pageStart = pageSize > 0 ? pageIndex * pageSize : 0;
    *puIndex = static_cast<UINT>(pageStart + currentSel_);
    return S_OK;
}

STDMETHODIMP CandidateWindow::GetString(UINT uIndex, BSTR* pbstr) {
    if (!pbstr) {
        return E_INVALIDARG;
    }
    const auto* service = productTextService(textService_);
    const int pageSize = service != nullptr ? service->candidatePageSize() : 0;
    const int pageIndex = service != nullptr ? service->candidatePageIndex() : 0;
    const UINT pageStart =
        pageSize > 0 ? static_cast<UINT>(pageIndex * pageSize) : 0;
    const UINT localIndex = pageSize > 0 ? uIndex - pageStart : uIndex;
    if (pageSize > 0 && uIndex < pageStart) {
        return E_INVALIDARG;
    }
    if (localIndex >= items_.size()) {
        return E_INVALIDARG;
    }
    *pbstr = SysAllocString(items_[localIndex].combinedText().c_str());
    return S_OK;
}

STDMETHODIMP CandidateWindow::GetPageIndex(UINT* puIndex, UINT uSize, UINT* puPageCnt) {
    if (!puPageCnt) {
        return E_INVALIDARG;
    }
    const auto* service = productTextService(textService_);
    const int pageSize = service != nullptr ? service->candidatePageSize() : 0;
    const int totalCount = service != nullptr ? service->candidateTotalCount() : 0;
    const UINT effectivePageSize =
        pageSize > 0 ? static_cast<UINT>(pageSize)
                     : (std::max<UINT>)(1, static_cast<UINT>(items_.size()));
    const UINT effectiveTotal =
        totalCount > 0 ? static_cast<UINT>(totalCount)
                       : static_cast<UINT>(items_.size());
    *puPageCnt = (std::max<UINT>)(1, (effectiveTotal + effectivePageSize - 1) /
                                         effectivePageSize);
    if (puIndex) {
        if (uSize < *puPageCnt) {
            return E_INVALIDARG;
        }
        for (UINT i = 0; i < *puPageCnt; ++i) {
            puIndex[i] = i * effectivePageSize;
        }
    }
    return S_OK;
}

STDMETHODIMP CandidateWindow::SetPageIndex(UINT* puIndex, UINT uPageCnt) {
    (void)uPageCnt;
    if (!puIndex) {
        return E_INVALIDARG;
    }
    return S_OK;
}

STDMETHODIMP CandidateWindow::GetCurrentPage(UINT* puPage) {
    if (!puPage) {
        return E_INVALIDARG;
    }
    const auto* service = productTextService(textService_);
    const int pageSize = service != nullptr ? service->candidatePageSize() : 0;
    const int totalCount = service != nullptr ? service->candidateTotalCount() : 0;
    const UINT effectivePageSize =
        pageSize > 0 ? static_cast<UINT>(pageSize)
                     : (std::max<UINT>)(1, static_cast<UINT>(items_.size()));
    const UINT effectiveTotal =
        totalCount > 0 ? static_cast<UINT>(totalCount)
                       : static_cast<UINT>(items_.size());
    const UINT pageCount = (std::max<UINT>)(
        1, (effectiveTotal + effectivePageSize - 1) / effectivePageSize);
    const UINT pageIndex =
        service != nullptr ? static_cast<UINT>(service->candidatePageIndex()) : 0;
    *puPage = (std::min)(pageIndex, pageCount - 1);
    return S_OK;
}

void CandidateWindow::add(CandidateUiItem item, wchar_t selKey) {
    wchar_t label[] = L"?.";
    label[0] = selKey;
    const std::wstring rawComment = item.comment;
    item.diagnosticRawComment = rawComment;
    item.displayPreferences = displayPreferences_;
    item.candidateInfo = TypeDuck::CandidateInfo(
        label, item.text, rawComment, item.inputCode);
    items_.push_back(std::move(item));
    selKeys_.push_back(selKey);
}

void CandidateWindow::clear() {
    items_.clear();
    selKeys_.clear();
    itemTextWidths_.clear();
    itemCommentWidths_.clear();
    itemWidths_.clear();
    itemHeights_.clear();
    candidatePanelWidth_ = 0;
    candidatePanelHeight_ = 0;
    dictionaryPanelWidth_ = 0;
    dictionaryPanelTop_ = 0;
    dictionaryPanelHeight_ = 0;
    dictionaryContentHeight_ = 0;
    dictionaryScrollOffset_ = 0;
    resetDictionaryReveal(false);
    if (::GetCapture() != hwnd_) {
        pressedSel_ = -1;
    }
}

void CandidateWindow::setCandPerRow(int n) {
    n = (std::max)(1, n);
    if (candPerRow_ != n) {
        candPerRow_ = n;
        recalculateSize();
    }
}

void CandidateWindow::setCandSpacing(int spacing) {
    spacing = (std::max)(0, spacing);
    if (candSpacing_ != spacing) {
        candSpacing_ = spacing;
        recalculateSize();
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setCurrentSel(int sel) {
    if (items_.empty()) {
        currentSel_ = 0;
        return;
    }
    if (sel < 0 || sel >= static_cast<int>(items_.size())) {
        sel = 0;
    }
    if (currentSel_ != sel) {
        const bool oldDictionaryVisible = dictionaryPanelVisible();
        currentSel_ = sel;
        if (dictionaryRevealIndex_ >= static_cast<int>(items_.size())) {
            resetDictionaryReveal();
        }
        if (Ime::isDebugLoggingEnabled() && oldDictionaryVisible != dictionaryPanelVisible()) {
            recalculateSize();
        }
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setUseCursor(bool use) {
    useCursor_ = use;
    if (isVisible()) {
        ::InvalidateRect(hwnd_, NULL, TRUE);
    }
}

void CandidateWindow::setPreeditText(std::wstring text) {
    if (preedit_ != text) {
        preedit_ = std::move(text);
        if (preeditCursor_ > static_cast<int>(preedit_.length())) {
            preeditCursor_ = static_cast<int>(preedit_.length());
        }
        if (preeditSelectionStart_ > static_cast<int>(preedit_.length())) {
            preeditSelectionStart_ = static_cast<int>(preedit_.length());
        }
        if (preeditSelectionEnd_ > static_cast<int>(preedit_.length())) {
            preeditSelectionEnd_ = static_cast<int>(preedit_.length());
        }
        recalculateSize();
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setPreeditCursor(int cursor) {
    cursor = (std::max)(0, (std::min)(cursor, static_cast<int>(preedit_.length())));
    if (preeditCursor_ != cursor) {
        preeditCursor_ = cursor;
        if (isVisible() && !preedit_.empty()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setPreeditSelection(int start, int end) {
    const int length = static_cast<int>(preedit_.length());
    start = (std::max)(0, (std::min)(start, length));
    end = (std::max)(0, (std::min)(end, length));
    if (end < start) {
        std::swap(start, end);
    }
    if (preeditSelectionStart_ == start && preeditSelectionEnd_ == end) {
        return;
    }
    preeditSelectionStart_ = start;
    preeditSelectionEnd_ = end;
    if (isVisible() && !preedit_.empty()) {
        ::InvalidateRect(hwnd_, NULL, TRUE);
    }
}

void CandidateWindow::setCommentFont(HFONT font) {
    if (commentFont_ != font) {
        commentFont_ = font;
        recalculateSize();
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setBackgroundColor(COLORREF color) {
    if (backgroundColor_ != color) {
        backgroundColor_ = color;
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setHighlightColor(COLORREF color) {
    if (highlightColor_ != color) {
        highlightColor_ = color;
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setTextColor(COLORREF color) {
    if (textColor_ != color) {
        textColor_ = color;
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setHighlightTextColor(COLORREF color) {
    if (highlightTextColor_ != color) {
        highlightTextColor_ = color;
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setCommentColor(COLORREF color) {
    if (commentColor_ != color) {
        commentColor_ = color;
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setCommentHighlightColor(COLORREF color) {
    if (commentHighlightColor_ != color) {
        commentHighlightColor_ = color;
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::setDisplayPreferences(TypeDuck::DisplayPreferences preferences) {
    displayPreferences_ = std::move(preferences);
    for (auto& item : items_) {
        item.displayPreferences = displayPreferences_;
    }
    recalculateSize();
    if (isVisible()) {
        ::InvalidateRect(hwnd_, NULL, TRUE);
    }
}

void CandidateWindow::syncOwner(Ime::EditSession* session) {
    if (!hwnd_) {
        return;
    }

    const HWND rawOwner = resolveCandidateOwnerWindow(session);
    const HWND owner = normalizeCandidateOwnerWindow(rawOwner, textService_->isImmersive(), L"syncOwner");
    if (owner == nullptr) {
        std::wostringstream log;
        log << L"[CandidateWindow::syncOwner] owner unavailable hwnd=" << hwnd_
            << L" current_owner="
            << reinterpret_cast<HWND>(::GetWindowLongPtr(hwnd_, GWLP_HWNDPARENT))
            << L" current_gw_owner=" << ::GetWindow(hwnd_, GW_OWNER)
            << L" raw_owner=" << rawOwner
            << L" shown=" << shown_;
        appendCandidateWindowLog(log.str());
        logCandidateWindowState(L"[CandidateWindow::syncOwner.owner_unavailable.state]", hwnd_);
        return;
    }

    const HWND currentOwner =
        reinterpret_cast<HWND>(::GetWindowLongPtr(hwnd_, GWLP_HWNDPARENT));
    const HWND currentGwOwner = ::GetWindow(hwnd_, GW_OWNER);
    bool ownerUpdated = false;
    DWORD ownerError = 0;
    if (currentOwner != owner) {
        ::SetLastError(0);
        ::SetWindowLongPtr(hwnd_, GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(owner));
        ownerError = ::GetLastError();
        ownerUpdated = ownerError == 0;
    }

    if (shown_) {
        enforceCandidateWindowTopmost(hwnd_, true, L"syncOwner");
    }

    std::wostringstream log;
    log << L"[CandidateWindow::syncOwner] hwnd=" << hwnd_
        << L" old_owner=" << currentOwner
        << L" old_gw_owner=" << currentGwOwner
        << L" raw_owner=" << rawOwner
        << L" new_owner=" << owner
        << L" shown=" << shown_
        << L" owner_updated=" << (ownerUpdated ? L"true" : L"false");
    if (ownerError != 0) {
        log << L" last_error=" << ownerError;
    }
    appendCandidateWindowLog(log.str());
    logCandidateWindowState(L"[CandidateWindow::syncOwner.state]", hwnd_);
}

void CandidateWindow::recalculateSize() {
    if (items_.empty() && preedit_.empty()) {
        selKeyWidth_ = 0;
        textWidth_ = 0;
        commentWidth_ = 0;
        itemHeight_ = 0;
        preeditHeight_ = 0;
        contentTop_ = borderWidth_ + padY_;
        candidatePanelWidth_ = padX_ * 2 + borderWidth_ * 2;
        candidatePanelHeight_ = padY_ * 2 + borderWidth_ * 2;
        resize(candidatePanelWidth_, candidatePanelHeight_);
        applyWindowShape();
        return;
    }

    HDC hdc = ::GetWindowDC(hwnd());
    if (!hdc) {
        return;
    }
    if (dictionaryHoverIndex_ >= static_cast<int>(items_.size())) {
        dictionaryHoverIndex_ = -1;
    }
    if (dictionaryRevealIndex_ >= static_cast<int>(items_.size())) {
        dictionaryRevealIndex_ = -1;
    }

    const int scaledPadX = scalePx(textService_->isImmersive() ? kImmersivePanelPaddingX : kPanelPaddingX);
    const int scaledPadY = scalePx(textService_->isImmersive() ? kImmersivePanelPaddingY : kPanelPaddingY);
    padX_ = scaledPadX;
    padY_ = scaledPadY;
    labelGap_ = scalePx(kCandidateLabelGap);
    commentGap_ = scalePx(kCandidateCommentGap);
    borderWidth_ = (std::max)(kBorderWidth, scalePx(kBorderWidth));
    borderRadius_ = scalePx(kCandidateBorderRadius);
    preeditGap_ = scalePx(kCandidatePreeditGap);
    // Keep panels touching so pointer travel into the dictionary panel stays inside this shaped popup.
    panelGap_ = 0;
    rowPaddingY_ = scalePx(kCandidateRowPaddingY);
    rowInnerGap_ = scalePx(kCandidateRowInnerGap);
    pageNavWidth_ = scalePx(kPageNavWidth);
    indicatorColumnWidth_ = scalePx(kCandidateIndicatorColumnWidth);
    jyutpingColumnWidth_ = 0;
    honziColumnWidth_ = 0;
    noteColumnWidth_ = 0;
    definitionColumnWidth_ = 0;
    dictionaryPanelMinHeight_ = scalePx(kDictionaryPanelMinHeight);
    minWidth_ = scalePx(kCandidateMinWidth);

    selKeyWidth_ = 0;
    textWidth_ = 0;
    commentWidth_ = 0;
    itemHeight_ = 0;
    preeditHeight_ = 0;
    itemTextWidths_.assign(items_.size(), 0);
    itemCommentWidths_.assign(items_.size(), 0);
    itemWidths_.assign(items_.size(), 0);
    itemHeights_.assign(items_.size(), 0);
    int preeditWidth = 0;
    int jyutpingContentWidth = 0;
    int honziContentWidth = 0;
    int noteContentWidth = 0;
    int definitionContentWidth = 0;
    bool hasJyutpingColumn = false;
    bool hasNoteColumn = false;
    bool hasDefinitionColumn = false;
    bool hasIndicatorColumn = false;

    HGDIOBJ oldFont = ::SelectObject(hdc, font_);
    HFONT rowMetaFont = createPanelFont(hdc, L"Segoe UI", 12);
    TEXTMETRICW metrics = {};
    TEXTMETRICW commentMetrics = {};
    ::GetTextMetricsW(hdc, &metrics);
    if (commentFont_) {
        ::SelectObject(hdc, commentFont_);
        ::GetTextMetricsW(hdc, &commentMetrics);
        ::SelectObject(hdc, font_);
    }
    const int bodyLineHeight = (std::max)(
        scalePx(kCandidateBodyLineMinHeight),
        static_cast<int>((std::max)(
            metrics.tmHeight + metrics.tmExternalLeading,
            commentFont_ ? commentMetrics.tmHeight + commentMetrics.tmExternalLeading : 0)));
    auto measureWithFont = [&](HFONT font, const std::wstring& value) -> int {
        if (value.empty()) {
            return 0;
        }
        HGDIOBJ previous = ::SelectObject(hdc, font ? font : font_);
        SIZE size = {};
        ::GetTextExtentPoint32W(hdc, value.c_str(), static_cast<int>(value.length()), &size);
        ::SelectObject(hdc, previous);
        return static_cast<int>(size.cx) + 1;
    };
    auto layoutDefinition = [&](const TypeDuck::CandidateEntry& entry,
                                const CandidateUiItem&) -> std::wstring {
        std::wstring definition = entry.definition(displayPreferences_.mainLanguage);
        if (definition.empty()) {
            const std::wstring reference = entry.canonicalReference();
            definition = reference.empty() ? joinDisplayValues(entry.formattedLabels(), L" ") : L"→" + reference;
        }
        return definition;
    };
    for (int i = 0, n = static_cast<int>(items_.size()); i < n; ++i) {
        SIZE selKeySize = {};
        wchar_t selKey[] = L"?.";
        selKey[0] = selKeys_[i];
        HGDIOBJ previousFont = ::SelectObject(hdc, rowMetaFont ? rowMetaFont : font_);
        ::GetTextExtentPoint32W(hdc, selKey, 2, &selKeySize);
        ::SelectObject(hdc, previousFont);
        selKeyWidth_ = (std::max)(selKeyWidth_, static_cast<int>(selKeySize.cx));

        SIZE candidateSize = {};
        const CandidateUiItem& item = items_[i];
        const std::wstring itemText = item.displayText();
        const std::wstring itemComment = item.displayComment();
        ::GetTextExtentPoint32W(hdc, itemText.c_str(), static_cast<int>(itemText.length()), &candidateSize);
        itemTextWidths_[i] = static_cast<int>(candidateSize.cx);
        textWidth_ = (std::max)(textWidth_, static_cast<int>(candidateSize.cx));
        const int candidateEntryRows = entryRowCount(item);
        if (!itemComment.empty() && commentFont_) {
            SIZE commentSize = {};
            ::SelectObject(hdc, commentFont_);
            ::GetTextExtentPoint32W(hdc, itemComment.c_str(), static_cast<int>(itemComment.length()), &commentSize);
            ::SelectObject(hdc, font_);
            itemCommentWidths_[i] = static_cast<int>(commentSize.cx);
            commentWidth_ = (std::max)(commentWidth_, static_cast<int>(commentSize.cx));
        }
        itemHeights_[i] = rowPaddingY_ * 2 + candidateEntryRows * bodyLineHeight;
        itemHeight_ = (std::max)(itemHeight_, itemHeights_[i]);

        std::vector<TypeDuck::CandidateEntry> layoutEntries = item.candidateInfo.matchedEntries();
        if (layoutEntries.empty()) {
            layoutEntries = item.candidateInfo.entries;
        }
        if (layoutEntries.empty()) {
            TypeDuck::CandidateEntry fallbackEntry;
            fallbackEntry.honzi = item.displayText();
            layoutEntries.push_back(std::move(fallbackEntry));
        }

        const bool showJyutping = displayPreferences_.shouldShowJyutping(item.candidateInfo.isReverseLookup);
        for (int entryIndex = 0; entryIndex < static_cast<int>(layoutEntries.size()); ++entryIndex) {
            const TypeDuck::CandidateEntry& entry = layoutEntries[entryIndex];
            if (showJyutping && !entry.jyutping.empty()) {
                hasJyutpingColumn = true;
                jyutpingContentWidth = (std::max)(
                    jyutpingContentWidth,
                    measureWithFont(rowMetaFont ? rowMetaFont : (commentFont_ ? commentFont_ : font_), entry.jyutping));
            }

            const std::wstring honzi = entry.honzi.empty() ? item.displayText() : entry.honzi;
            if (!honzi.empty()) {
                honziContentWidth = (std::max)(honziContentWidth, measureWithFont(font_, honzi));
            }

            const std::wstring note = (!item.candidateInfo.isReverseLookup || displayPreferences_.showReverseCode)
                                          ? item.candidateInfo.note
                                          : L"";
            if (entryIndex == 0 && !note.empty()) {
                hasNoteColumn = true;
                noteContentWidth = (std::max)(
                    noteContentWidth,
                    measureWithFont(commentFont_ ? commentFont_ : font_, note));
            }

            const std::wstring definition = layoutDefinition(entry, item);
            if (!definition.empty()) {
                hasDefinitionColumn = true;
                definitionContentWidth = (std::max)(
                    definitionContentWidth,
                    measureWithFont(commentFont_ ? commentFont_ : font_, definition));
            }
        }
        if (item.candidateInfo.hasDictionaryEntry(displayPreferences_)) {
            hasIndicatorColumn = true;
        }
    }
    if (!preedit_.empty()) {
        HFONT inputFont = createDerivedFont(font_, kInputBufferFontName);
        HGDIOBJ previousFont = ::SelectObject(hdc, inputFont ? inputFont : font_);
        SIZE preeditSize = {};
        ::GetTextExtentPoint32W(hdc, preedit_.c_str(), static_cast<int>(preedit_.length()), &preeditSize);
        ::SelectObject(hdc, previousFont);
        if (inputFont) {
            ::DeleteObject(inputFont);
        }
        preeditWidth = static_cast<int>(preeditSize.cx) + scalePx(kPreeditTextWidthPadding);
        textWidth_ = (std::max)(textWidth_, static_cast<int>(preeditSize.cx));
        preeditHeight_ = static_cast<int>(preeditSize.cy);
    }

    int dictionaryContentWidth = 0;
    dictionaryContentHeight_ = 0;
    if (dictionaryPanelVisible()) {
        HFONT entryFont = createPanelFont(hdc, L"DFKai-SB", 32);
        HFONT pronFont = createPanelFont(hdc, L"Segoe UI", 15);
        HFONT pronTypeFont = createPanelFont(hdc, L"Segoe UI", 12);
        HFONT posFont = createPanelFont(hdc, L"Segoe UI", 10);
        HFONT bodyFont = createPanelFont(hdc, L"Segoe UI", 12);
        HFONT valueFont = createPanelFont(hdc, L"Microsoft JhengHei", 12);
        HFONT captionFont = createPanelFont(hdc, L"Segoe UI", 13, FW_SEMIBOLD);
        const int titleGap = scalePx(kDictionaryHeaderGap);
        const int posPadding = scalePx(kDictionaryPosPadding);
        const int posGap = scalePx(kDictionaryPosGap);
        const int labelGap = scalePx(kCandidateCommentGap);
        const int definitionGap = scalePx(kDictionaryDefinitionGap);
        const int fieldGap = scalePx(kDictionaryFieldGap);
        const int pillGap = scalePx(kDictionaryPillGap);
        auto addWidth = [&](int width) {
            dictionaryContentWidth = (std::max)(dictionaryContentWidth, width);
        };
        auto joinedInlineBody = [&](const TypeDuck::CandidateEntry& entry) {
            std::wstring body;
            for (const auto& reg : entry.formattedRegister()) {
                if (!body.empty()) {
                    body += L"  ";
                }
                body += reg;
            }
            for (const auto& label : entry.formattedLabels()) {
                if (!body.empty()) {
                    body += std::wstring(labelGap / (std::max)(1, scalePx(kInlineBodySpacingUnit)), L' ');
                }
                body += label;
            }
            const std::wstring canonical = entry.canonicalReference();
            if (!canonical.empty()) {
                if (!body.empty()) {
                    body += L"  ";
                }
                body += L"→" + canonical;
            } else {
                const std::wstring mainDefinition = entry.definition(displayPreferences_.mainLanguage);
                if (!mainDefinition.empty()) {
                    if (!body.empty()) {
                        body += L"  ";
                    }
                    body += mainDefinition;
                }
            }
            return body;
        };
        const TypeDuck::CandidateInfo& dictionaryInfo =
            items_[effectiveDictionaryIndex()].candidateInfo;
        for (const auto& entry : dictionaryInfo.entries) {
            if (!entry.isDictionaryEntry(displayPreferences_)) {
                continue;
            }
            int headerWidth = static_cast<int>(textExtent(hdc, entryFont, entry.honzi).cx);
            const int pronWidth = static_cast<int>(textExtent(hdc, pronFont, entry.jyutping).cx);
            const std::wstring pronType = entry.pronunciationType();
            const int pronTypeWidth = static_cast<int>(textExtent(hdc, pronTypeFont, pronType).cx);
            if (pronWidth > 0) {
                headerWidth += titleGap + pronWidth;
            }
            if (pronTypeWidth > 0) {
                headerWidth += titleGap + pronTypeWidth;
            }
            addWidth(headerWidth);

            int posWidth = 0;
            for (const auto& part : entry.formattedPartsOfSpeech()) {
                const int pillWidth = static_cast<int>(textExtent(hdc, posFont, part).cx) + posPadding * 2;
                if (posWidth > 0) {
                    posWidth += pillGap;
                }
                posWidth += pillWidth;
            }
            const int bodyWidth =
                static_cast<int>(textExtent(hdc, bodyFont, joinedInlineBody(entry)).cx) +
                scalePx(kDictionaryWidthSlack);
            addWidth(posWidth + (posWidth > 0 && bodyWidth > 0 ? definitionGap - posGap : 0) + bodyWidth);

            int keyWidth = 0;
            for (const auto& other : entry.otherData()) {
                keyWidth = (std::max)(keyWidth, static_cast<int>(textExtent(hdc, bodyFont, other.name).cx));
            }
            for (const auto& other : entry.otherData()) {
                for (const auto& value : other.values()) {
                    addWidth(keyWidth + fieldGap + static_cast<int>(textExtent(hdc, valueFont, value).cx));
                }
            }

            const auto moreLanguages = entry.otherLanguages(displayPreferences_);
            if (!moreLanguages.empty()) {
                addWidth(static_cast<int>(textExtent(hdc, captionFont, L"More Languages").cx));
                int languageKeyWidth = 0;
                for (const auto& language : moreLanguages) {
                    languageKeyWidth = (std::max)(languageKeyWidth,
                                                  static_cast<int>(textExtent(hdc, bodyFont, language.name).cx));
                }
                for (const auto& language : moreLanguages) {
                    addWidth(languageKeyWidth + fieldGap +
                             static_cast<int>(textExtent(hdc, valueFont, language.value).cx));
                }
            }
        }
        ::DeleteObject(captionFont);
        ::DeleteObject(valueFont);
        ::DeleteObject(bodyFont);
        ::DeleteObject(posFont);
        ::DeleteObject(pronTypeFont);
        ::DeleteObject(pronFont);
        ::DeleteObject(entryFont);
    }
    dictionaryPanelWidth_ = dictionaryPanelVisible()
                                ? (std::max)(dictionaryContentWidth + scalePx(kDictionaryMeasureWidthPadding),
                                             scalePx(kDictionaryPanelMinWidth))
                                : 0;
    if (dictionaryPanelVisible()) {
        RECT measureRc = {0, 0, dictionaryPanelWidth_, kUnboundedMeasureHeight};
        int y = measureRc.top + scalePx(kDictionaryPanelTopPadding);
        const TypeDuck::CandidateInfo& dictionaryInfo =
            items_[effectiveDictionaryIndex()].candidateInfo;
        for (const auto& entry : dictionaryInfo.entries) {
            if (!entry.isDictionaryEntry(displayPreferences_)) {
                continue;
            }
            paintDictionaryEntry(hdc, y, measureRc, entry, false);
        }
        dictionaryContentHeight_ = (std::max)(0, static_cast<int>(y - measureRc.top));
    }
    if (rowMetaFont) {
        ::DeleteObject(rowMetaFont);
    }
    ::SelectObject(hdc, oldFont);
    ::ReleaseDC(hwnd(), hdc);

    preeditHeight_ = preedit_.empty()
                         ? 0
                         : (std::max)(preeditHeight_, static_cast<int>(metrics.tmHeight + metrics.tmExternalLeading));

    const int columnPad = scalePx(kCandidateColumnPad);
    jyutpingColumnWidth_ = hasJyutpingColumn
                               ? (std::max)(jyutpingContentWidth + columnPad, scalePx(kCandidateJyutpingMinWidth))
                               : 0;
    honziColumnWidth_ = (std::max)(honziContentWidth + columnPad, scalePx(kCandidateHonziMinWidth));
    noteColumnWidth_ = hasNoteColumn
                           ? (std::max)(noteContentWidth + columnPad, scalePx(kCandidateNoteMinWidth))
                           : 0;
    definitionColumnWidth_ = hasDefinitionColumn
                                 ? (std::max)(definitionContentWidth + columnPad,
                                              scalePx(kCandidateDefinitionMinWidth))
                                 : 0;
    indicatorColumnWidth_ = hasIndicatorColumn ? scalePx(kCandidateIndicatorColumnWidth) : 0;

    auto rowBodyWidth = [&]() {
        int width = 0;
        auto addColumn = [&](int columnWidth) {
            if (columnWidth <= 0) {
                return;
            }
            if (width > 0) {
                width += rowInnerGap_;
            }
            width += columnWidth;
        };
        addColumn(jyutpingColumnWidth_);
        addColumn(honziColumnWidth_);
        addColumn(noteColumnWidth_);
        addColumn(definitionColumnWidth_);
        addColumn(indicatorColumnWidth_);
        return width;
    };
    const int rowContentWidth = selKeyWidth_ + labelGap_ + rowBodyWidth();
    for (int i = 0, n = static_cast<int>(items_.size()); i < n; ++i) {
        itemWidths_[i] = rowContentWidth;
    }

    const int rows = static_cast<int>(items_.size());
    const int candidateContentWidth = rowContentWidth;
    const int contentWidth = (std::max)((std::max)(candidateContentWidth, preeditWidth + pageNavWidth_), minWidth_);
    const int candidatePanelWidth = padX_ * 2 + contentWidth + borderWidth_ * 2;
    int candidatePanelHeight = (std::max)(0, rows - 1) * rowSpacing_;
    for (int height : itemHeights_) {
        candidatePanelHeight += height;
    }
    if (!preedit_.empty()) {
        contentTop_ = borderWidth_ + padY_ + preeditHeight_ + preeditGap_;
        candidatePanelHeight += preeditHeight_ + preeditGap_;
    } else {
        contentTop_ = borderWidth_ + padY_;
    }
    candidatePanelHeight += padY_ * 2 + borderWidth_ * 2;
    candidatePanelWidth_ = candidatePanelWidth;
    candidatePanelHeight_ = candidatePanelHeight;
    if (dictionaryPanelVisible()) {
        const int scrollingViewportHeight = (std::max)(scalePx(kDictionaryScrollViewportMinHeight), candidatePanelHeight_);
        dictionaryPanelHeight_ = dictionaryContentHeight_ > scrollingViewportHeight
                                     ? scrollingViewportHeight
                                     : (std::max)(dictionaryPanelMinHeight_, dictionaryContentHeight_);
        dictionaryPanelTop_ = 0;
        const int dictionaryIndex = effectiveDictionaryIndex();
        if (dictionaryIndex >= 0 && dictionaryIndex < static_cast<int>(items_.size())) {
            RECT dictionaryTargetRc = {};
            itemRect(dictionaryIndex, dictionaryTargetRc);
            const int targetBottom = static_cast<int>(dictionaryTargetRc.bottom);
            if (targetBottom > dictionaryPanelHeight_) {
                dictionaryPanelTop_ = (std::max)(
                    0,
                    (std::min)(candidatePanelHeight_ - dictionaryPanelHeight_,
                               targetBottom - dictionaryPanelHeight_));
            }
        }
    } else {
        dictionaryPanelTop_ = 0;
        dictionaryPanelHeight_ = 0;
        dictionaryScrollOffset_ = 0;
    }
    clampDictionaryScrollOffset();
    const int dictionaryHeight = dictionaryPanelVisible() ? dictionaryPanelTop_ + dictionaryPanelHeight_ : 0;
    const int width = candidatePanelWidth +
                      (dictionaryPanelVisible() ? panelGap_ + dictionaryPanelWidth_ : 0);
    const int height = (std::max)(candidatePanelHeight, dictionaryHeight);
    resize(width, height);
    applyWindowShape();

    std::wostringstream log;
    log << L"[CandidateWindow::recalculateSize] items=" << items_.size()
        << L" width=" << width << L" height=" << height
        << L" perRow=" << candPerRow_;
    appendCandidateWindowLog(log.str());
}

LRESULT CandidateWindow::wndProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_PAINT:
        onPaint();
        return 0;
    case WM_ERASEBKGND:
        return TRUE;
    case WM_LBUTTONDOWN:
        onLButtonDown(wp, lp);
        return 0;
    case WM_MOUSEMOVE:
        onMouseMove(wp, lp);
        return 0;
    case WM_LBUTTONUP:
        onLButtonUp(wp, lp);
        return 0;
    case WM_MOUSELEAVE:
        onMouseLeave();
        return 0;
    case WM_MOUSEWHEEL:
        onMouseWheel(wp, lp);
        return 0;
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;
    default:
        return Window::wndProc(msg, wp, lp);
    }
}

void CandidateWindow::onPaint() {
    PAINTSTRUCT ps = {};
    BeginPaint(hwnd_, &ps);
    HDC hdc = ps.hdc;

    RECT rc = {};
    GetClientRect(hwnd_, &rc);
    HDC memdc = ::CreateCompatibleDC(hdc);
    HBITMAP membmp = ::CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
    HGDIOBJ oldBitmap = ::SelectObject(memdc, membmp);
    HGDIOBJ oldFont = ::SelectObject(memdc, font_);
    ::SetBkMode(memdc, TRANSPARENT);

    HBRUSH backgroundBrush = ::CreateSolidBrush(backgroundColor_);
    HBRUSH borderBrush = ::CreateSolidBrush(kWindowBorder);
    const int candidatePanelWidth = candidatePanelWidth_ > 0 ? candidatePanelWidth_ : rc.right - rc.left;
    const int dictionaryRight = candidatePanelWidth + panelGap_ + dictionaryPanelWidth_;
    RECT candidatePanelRc = {rc.left, rc.top, candidatePanelWidth, rc.top + candidatePanelHeight_};
    HRGN windowRgn = ::CreateRoundRectRgn(
        candidatePanelRc.left, candidatePanelRc.top,
        candidatePanelRc.right + 1, candidatePanelRc.bottom + 1,
        borderRadius_ * 2, borderRadius_ * 2);
    if (dictionaryPanelVisible()) {
        HRGN dictionaryWindowRgn = ::CreateRoundRectRgn(
            candidatePanelRc.right + panelGap_,
            rc.top + dictionaryPanelTop_,
            dictionaryRight + 1,
            rc.top + dictionaryPanelTop_ + dictionaryPanelHeight_ + 1,
            borderRadius_ * 2,
            borderRadius_ * 2);
        ::CombineRgn(windowRgn, windowRgn, dictionaryWindowRgn, RGN_OR);
        ::DeleteObject(dictionaryWindowRgn);
    }
    ::FillRect(memdc, &rc, backgroundBrush);
    ::FillRgn(memdc, windowRgn, backgroundBrush);
    ::FrameRgn(memdc, windowRgn, borderBrush, borderWidth_, borderWidth_);

    paintInputBuffer(memdc, candidatePanelRc);
    paintPageNavigation(memdc, candidatePanelRc);

    int y = contentTop_;
    for (int i = 0, n = static_cast<int>(items_.size()); i < n; ++i) {
        const int rowHeight = itemHeight(i);
        RECT rowRc = {
            borderWidth_ + padX_ / 2,
            y,
            candidatePanelRc.right - borderWidth_ - padX_ / 2,
            y + rowHeight};
        paintCandidateRow(memdc, i, rowRc);
        y += rowHeight + rowSpacing_;
    }

    if (dictionaryPanelVisible()) {
        RECT dictionaryRc = {
            candidatePanelRc.right + panelGap_,
            rc.top + dictionaryPanelTop_,
            dictionaryRight,
            rc.top + dictionaryPanelTop_ + dictionaryPanelHeight_};
        HRGN dictionaryRgn = ::CreateRoundRectRgn(
            dictionaryRc.left, dictionaryRc.top, dictionaryRc.right + 1, dictionaryRc.bottom + 1,
            borderRadius_ * 2, borderRadius_ * 2);
        HBRUSH dictionaryBrush = ::CreateSolidBrush(kDictionaryBackground);
        ::FillRgn(memdc, dictionaryRgn, dictionaryBrush);
        ::FrameRgn(memdc, dictionaryRgn, borderBrush, borderWidth_, borderWidth_);
        ::DeleteObject(dictionaryBrush);
        ::DeleteObject(dictionaryRgn);
        paintDictionaryPanel(memdc, dictionaryRc, items_[effectiveDictionaryIndex()].candidateInfo);
        paintDictionaryScrollBar(memdc, dictionaryRc);
    }

    ::BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, memdc, 0, 0, SRCCOPY);

    ::DeleteObject(windowRgn);
    ::DeleteObject(borderBrush);
    ::DeleteObject(backgroundBrush);
    ::SelectObject(memdc, oldFont);
    ::SelectObject(memdc, oldBitmap);
    ::DeleteObject(membmp);
    ::DeleteDC(memdc);
    EndPaint(hwnd_, &ps);
}

void CandidateWindow::paintInputBuffer(HDC hdc, const RECT& panelRc) {
    if (preedit_.empty()) {
        return;
    }

    RECT preeditRc = {
        panelRc.left + borderWidth_ + padX_ / 2,
        panelRc.top + borderWidth_ + padY_,
        panelRc.right - borderWidth_ - padX_ - pageNavWidth_,
        panelRc.top + borderWidth_ + padY_ + preeditHeight_ + scalePx(kPreeditExtraHeight)};
    HFONT inputFont = createDerivedFont(font_, kInputBufferFontName);
    HGDIOBJ oldFont = ::SelectObject(hdc, inputFont ? inputFont : font_);
    ::SetBkMode(hdc, TRANSPARENT);

    const int length = static_cast<int>(preedit_.length());
    int activeStart = (std::max)(0, (std::min)(preeditSelectionStart_, length));
    int activeEnd = (std::max)(0, (std::min)(preeditSelectionEnd_, length));
    if (activeEnd < activeStart) {
        std::swap(activeStart, activeEnd);
    }
    const std::wstring before = preedit_.substr(0, activeStart);
    const std::wstring active = preedit_.substr(activeStart, activeEnd - activeStart);
    const std::wstring after = preedit_.substr(activeEnd);
    auto textWidth = [&](const std::wstring& text) {
        if (text.empty()) {
            return 0;
        }
        SIZE size = {};
        ::GetTextExtentPoint32W(hdc, text.c_str(), static_cast<int>(text.length()), &size);
        return static_cast<int>(size.cx);
    };

    const int plainMargin = scalePx(kPreeditPlainMargin);
    const int activePadX = scalePx(kPreeditActivePaddingX);
    int x = preeditRc.left;

    if (!before.empty()) {
        RECT beforeRc = {x + plainMargin, preeditRc.top, preeditRc.right, preeditRc.bottom};
        ::SetTextColor(hdc, kItemText);
        ::DrawTextW(hdc, before.c_str(), static_cast<int>(before.length()), &beforeRc,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        x += textWidth(before) + plainMargin * 2;
    }

    const int activeWidth = textWidth(active);
    RECT activeRc = {x, preeditRc.top, x, preeditRc.bottom};
    RECT activeTextRc = activeRc;
    if (!active.empty() && activeWidth > 0) {
        activeRc.right = (std::min)(static_cast<int>(preeditRc.right), x + activeWidth + activePadX * 2);
        HBRUSH inputBrush = ::CreateSolidBrush(kInputBufferBackground);
        HRGN activeRgn = ::CreateRoundRectRgn(activeRc.left, activeRc.top, activeRc.right + 1,
                                              activeRc.bottom + 1,
                                              scalePx(kPreeditActiveCornerRadius) * 2,
                                              scalePx(kPreeditActiveCornerRadius) * 2);
        ::FillRgn(hdc, activeRgn, inputBrush);
        ::DeleteObject(activeRgn);
        ::DeleteObject(inputBrush);

        activeTextRc = activeRc;
        activeTextRc.left += activePadX;
        activeTextRc.right -= activePadX;
        ::SetTextColor(hdc, kInputBufferText);
        ::DrawTextW(hdc, active.c_str(), static_cast<int>(active.length()), &activeTextRc,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        x = activeRc.right;
    }
    if (!after.empty() && x < preeditRc.right) {
        RECT afterRc = {x + plainMargin, preeditRc.top, preeditRc.right, preeditRc.bottom};
        ::SetTextColor(hdc, kItemText);
        ::DrawTextW(hdc, after.c_str(), static_cast<int>(after.length()), &afterRc,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }

    auto cursorXForIndex = [&](int index) {
        index = (std::max)(0, (std::min)(index, length));
        if (index <= activeStart) {
            return preeditRc.left + plainMargin + textWidth(preedit_.substr(0, index));
        }
        if (index <= activeEnd) {
            return activeTextRc.left + textWidth(preedit_.substr(activeStart, index - activeStart));
        }
        return activeRc.right + plainMargin + textWidth(preedit_.substr(activeEnd, index - activeEnd));
    };
    paintPreeditCursor(hdc, preeditRc, cursorXForIndex(preeditCursor_));

    const int dividerY = preeditRc.bottom + preeditGap_ / 2;
    HPEN dividerPen = ::CreatePen(PS_SOLID, 1, kDividerColor);
    HGDIOBJ oldPen = ::SelectObject(hdc, dividerPen);
    ::MoveToEx(hdc, panelRc.left + borderWidth_ + padX_, dividerY, nullptr);
    ::LineTo(hdc, panelRc.right - borderWidth_ - padX_, dividerY);
    ::SelectObject(hdc, oldPen);
    ::DeleteObject(dividerPen);
    ::SelectObject(hdc, oldFont);
    if (inputFont) {
        ::DeleteObject(inputFont);
    }
}

void CandidateWindow::paintPageNavigation(HDC hdc, const RECT& panelRc) {
    const bool hasPrev = isPageNavigationEnabled(false);
    const bool hasNext = isPageNavigationEnabled(true);
    RECT prevRc = {};
    RECT nextRc = {};
    pageNavigationButtonRect(false, prevRc);
    pageNavigationButtonRect(true, nextRc);
    ::OffsetRect(&prevRc, panelRc.left, panelRc.top);
    ::OffsetRect(&nextRc, panelRc.left, panelRc.top);

    auto paintNavBackground = [&](int direction, const RECT& buttonRc, bool enabled) {
        if (!enabled) {
            return;
        }
        if (pressedPageNavDirection_ != direction && hoveredPageNavDirection_ != direction) {
            return;
        }
        HBRUSH brush = ::CreateSolidBrush(kWindowBorder);
        HBRUSH oldBrush = static_cast<HBRUSH>(::SelectObject(hdc, brush));
        HPEN pen = ::CreatePen(PS_SOLID, 1, kWindowBorder);
        HPEN oldPen = static_cast<HPEN>(::SelectObject(hdc, pen));
        ::RoundRect(hdc, buttonRc.left, buttonRc.top, buttonRc.right, buttonRc.bottom,
                    scalePx(kPageNavHoverRadius), scalePx(kPageNavHoverRadius));
        ::SelectObject(hdc, oldPen);
        ::SelectObject(hdc, oldBrush);
        ::DeleteObject(pen);
        ::DeleteObject(brush);
    };
    paintNavBackground(kPageNavPrevious, prevRc, hasPrev);
    paintNavBackground(kPageNavNext, nextRc, hasNext);

    HFONT navFont = createPanelFont(hdc, L"Segoe UI Symbol", kPageNavGlyphPointSize);
    HGDIOBJ oldFont = ::SelectObject(hdc, navFont ? navFont : font_);
    auto drawNavGlyph = [&](const wchar_t* glyph, const RECT& glyphRc, COLORREF color) {
        SIZE glyphSize = {};
        ::GetTextExtentPoint32W(hdc, glyph, 1, &glyphSize);
        const int x = glyphRc.left + (glyphRc.right - glyphRc.left - glyphSize.cx) / 2;
        const int y = glyphRc.top + (glyphRc.bottom - glyphRc.top - glyphSize.cy) / 2 -
                      scalePx(kPageNavGlyphYOffset);
        ::SetTextColor(hdc, color);
        ::TextOutW(hdc, x, y, glyph, 1);
    };
    drawNavGlyph(L"‹", prevRc, hasPrev ? kLinkText : kDisabledText);
    drawNavGlyph(L"›", nextRc, hasNext ? kLinkText : kDisabledText);
    ::SelectObject(hdc, oldFont);
    if (navFont) {
        ::DeleteObject(navFont);
    }
}

void CandidateWindow::paintItem(HDC hdc, int index, int x, int y) {
    RECT rowRc = {x, y, x + itemWidth(index), y + itemHeight(index)};
    paintCandidateRow(hdc, index, rowRc);
}

void CandidateWindow::paintCandidateRow(HDC hdc, int index, const RECT& rowRc) {
    // Mirrors TypeDuck Web definitionLayout: enabled displayLanguages decide
    // which mainLanguage/otherLanguages definitions are visible.
    const bool selected = dictionaryHoverIndex_ >= 0
                              ? index == dictionaryHoverIndex_
                              : (useCursor_ && index == currentSel_);

    const COLORREF bgColor = selected ? highlightColor_ : backgroundColor_;
    const COLORREF selColor = selected ? highlightTextColor_ : textColor_;

    if (selected) {
        HBRUSH highlightBrush = ::CreateSolidBrush(bgColor);
        HRGN rowRgn = ::CreateRoundRectRgn(rowRc.left, rowRc.top, rowRc.right + 1, rowRc.bottom + 1,
                                           scalePx(kCandidateRowCornerRadius) * 2,
                                           scalePx(kCandidateRowCornerRadius) * 2);
        ::FillRgn(hdc, rowRgn, highlightBrush);
        ::DeleteObject(rowRgn);
        ::DeleteObject(highlightBrush);
    }

    RECT selRc = rowRc;
    selRc.left += scalePx(kCandidateSelectionInsetX);
    selRc.right = selRc.left + selKeyWidth_;
    RECT firstLineRc = selRc;
    wchar_t selKey[] = L"?.";
    selKey[0] = selKeys_[index];
    const COLORREF oldColor = ::SetTextColor(hdc, selColor);
    HFONT rowMetaFont = createPanelFont(hdc, L"Segoe UI", 12);
    HGDIOBJ oldFont = ::SelectObject(hdc, rowMetaFont ? rowMetaFont : font_);

    const CandidateUiItem& item = items_[index];
    const auto matchedEntries = item.candidateInfo.matchedEntries();
    const std::vector<TypeDuck::CandidateEntry>& allEntries = item.candidateInfo.entries;
    const bool useMatched = !matchedEntries.empty();
    const int rowCount = entryRowCount(item);
    const int lineHeight =
        (std::max)(scalePx(kCandidateRowLineMinHeight), (itemHeight(index) - rowPaddingY_ * 2) / rowCount);
    int y = rowRc.top + rowPaddingY_;
    firstLineRc.top = y;
    firstLineRc.bottom = y + lineHeight;
    // candidateBaselineAligned: labels, Jyutping, Honzi, and definitions share the first row baseline.
    ::DrawTextW(hdc, selKey, 2, &firstLineRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    ::SelectObject(hdc, font_);
    for (int entryIndex = 0; entryIndex < rowCount; ++entryIndex) {
        TypeDuck::CandidateEntry fallbackEntry;
        const TypeDuck::CandidateEntry* entry = nullptr;
        if (useMatched && entryIndex < static_cast<int>(matchedEntries.size())) {
            entry = &matchedEntries[entryIndex];
        } else if (!useMatched && entryIndex < static_cast<int>(allEntries.size())) {
            entry = &allEntries[entryIndex];
        }
        if (!entry) {
            fallbackEntry.honzi = item.displayText();
            fallbackEntry.jyutping = L"";
            entry = &fallbackEntry;
        }

        int columnX = selRc.right + labelGap_;
        auto nextColumn = [&](int width) {
            RECT rc = {columnX, y, columnX + width, y + lineHeight};
            if (width > 0) {
                columnX = rc.right + rowInnerGap_;
            }
            return rc;
        };
        RECT jyutpingRc = nextColumn(jyutpingColumnWidth_);
        RECT honziRc = nextColumn(honziColumnWidth_);
        RECT noteRc = nextColumn(noteColumnWidth_);
        RECT definitionRc = nextColumn(definitionColumnWidth_);
        RECT indicatorRc = nextColumn(indicatorColumnWidth_);
        indicatorRc.top = rowRc.top;
        indicatorRc.bottom = rowRc.bottom;
        indicatorRc.right = rowRc.right - scalePx(kCandidateIndicatorInsetRight);

        ::SelectObject(hdc, rowMetaFont ? rowMetaFont : (commentFont_ ? commentFont_ : font_));
        ::SetTextColor(hdc, selected ? commentHighlightColor_ : kPronunciationText);
        const bool showJyutping = displayPreferences_.shouldShowJyutping(item.candidateInfo.isReverseLookup);
        if (showJyutping && jyutpingColumnWidth_ > 0) {
            ::DrawTextW(hdc, entry->jyutping.c_str(), static_cast<int>(entry->jyutping.length()), &jyutpingRc,
                        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        }

        ::SelectObject(hdc, font_);
        ::SetTextColor(hdc, selected ? highlightTextColor_ : textColor_);
        const std::wstring honzi = entry->honzi.empty() ? item.displayText() : entry->honzi;
        ::DrawTextW(hdc, honzi.c_str(), static_cast<int>(honzi.length()), &honziRc,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);

        ::SelectObject(hdc, commentFont_ ? commentFont_ : font_);
        ::SetTextColor(hdc, selected ? commentHighlightColor_ : kSecondaryText);
        const std::wstring note = (!item.candidateInfo.isReverseLookup || displayPreferences_.showReverseCode)
                                      ? item.candidateInfo.note
                                      : L"";
        if (entryIndex == 0 && !note.empty() && noteColumnWidth_ > 0) {
            ::DrawTextW(hdc, note.c_str(), static_cast<int>(note.length()), &noteRc,
                        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        }

        std::wstring definition = entry->definition(displayPreferences_.mainLanguage);
        if (definition.empty()) {
            const std::wstring reference = entry->canonicalReference();
            definition = reference.empty() ? joinDisplayValues(entry->formattedLabels(), L" ") : L"→" + reference;
        }
        if (definitionColumnWidth_ > 0) {
            ::SetTextColor(hdc, selected ? commentHighlightColor_ : kDefinitionText);
            ::DrawTextW(hdc, definition.c_str(), static_cast<int>(definition.length()), &definitionRc,
                        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        }

        if (entryIndex == 0 && indicatorColumnWidth_ > 0 &&
            item.candidateInfo.hasDictionaryEntry(displayPreferences_)) {
            ::SetTextColor(hdc, selected ? highlightTextColor_ : kSecondaryText);
            ::DrawTextW(hdc, L"ⓘ", 1, &indicatorRc,
                        DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
        y += lineHeight;
    }
    ::SelectObject(hdc, oldFont);
    if (rowMetaFont) {
        ::DeleteObject(rowMetaFont);
    }
    ::SetTextColor(hdc, oldColor);
}

void CandidateWindow::paintDictionaryPanel(
    HDC hdc,
    const RECT& panelRc,
    const TypeDuck::CandidateInfo& info) {
    const int savedDc = ::SaveDC(hdc);
    ::IntersectClipRect(hdc,
                        panelRc.left + borderWidth_,
                        panelRc.top + borderWidth_,
                        panelRc.right - borderWidth_,
                        panelRc.bottom - borderWidth_);
    int y = panelRc.top + scalePx(kDictionaryPanelTopPadding) - dictionaryScrollOffset_;

    for (const auto& entry : info.entries) {
        if (!entry.isDictionaryEntry(displayPreferences_)) {
            continue;
        }
        paintDictionaryEntry(hdc, y, panelRc, entry, true);
        if (y > panelRc.bottom + dictionaryScrollOffset_ + scalePx(kDictionaryPaintCullPadding)) {
            break;
        }
    }
    ::RestoreDC(hdc, savedDc);
}

void CandidateWindow::paintDictionaryEntry(
    HDC hdc,
    int& y,
    const RECT& panelRc,
    const TypeDuck::CandidateEntry& entry,
    bool paint) {
    HFONT entryFont = createPanelFont(hdc, L"DFKai-SB", 32);
    HFONT pronFont = createPanelFont(hdc, L"Segoe UI", 15);
    HFONT pronTypeFont = createPanelFont(hdc, L"Segoe UI", 12);
    HFONT posFont = createPanelFont(hdc, L"Segoe UI", 10);
    HFONT bodyFont = createPanelFont(hdc, L"Segoe UI", 12);
    HFONT valueFont = createPanelFont(hdc, L"Microsoft JhengHei", 12);
    HFONT captionFont = createPanelFont(hdc, L"Segoe UI", 13, FW_SEMIBOLD);
    HGDIOBJ oldFont = ::SelectObject(hdc, font_);

    const int padX = scalePx(kDictionaryPanelHorizontalPadding);
    const int padY = scalePx(kDictionaryPanelVerticalPadding);
    const int titleGap = scalePx(kDictionaryHeaderGap);
    const int spacing = scalePx(kDictionarySectionSpacing);
    const int posPadding = scalePx(kDictionaryPosPadding);
    const int posGap = scalePx(kDictionaryPosGap);
    const int labelGap = scalePx(kCandidateCommentGap);
    const int definitionGap = scalePx(kDictionaryDefinitionGap);
    const int fieldSpacing = scalePx(kDictionaryFieldSpacing);
    const int fieldGap = scalePx(kDictionaryFieldGap);
    const int moreLanguagesSpacing = scalePx(kDictionaryMoreLanguagesSpacing);
    const int entrySpacing = scalePx(kDictionaryEntrySpacing);
    const int right = panelRc.right - padX;

    std::wstring pronunciation = entry.jyutping;
    const std::wstring pronType = entry.pronunciationType();
    const SIZE entrySize = textExtent(hdc, entryFont, entry.honzi);
    const SIZE pronSize = textExtent(hdc, pronFont, pronunciation);
    const SIZE pronTypeSize = textExtent(hdc, pronTypeFont, pronType);
    const int titleHeight = (std::max)(entrySize.cy, (std::max)(pronSize.cy, pronTypeSize.cy));
    int x = panelRc.left + padX;

    RECT headRc = {x, y, x + entrySize.cx, y + titleHeight};
    if (paint) {
        ::SelectObject(hdc, entryFont);
        ::SetTextColor(hdc, textColor_);
        ::DrawTextW(hdc, entry.honzi.c_str(), static_cast<int>(entry.honzi.length()), &headRc,
                    DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }

    x = headRc.right + (pronunciation.empty() ? 0 : titleGap);
    RECT pronunciationRc = {x, y, right, y + titleHeight};
    if (paint) {
        ::SelectObject(hdc, pronFont);
        ::SetTextColor(hdc, kPronunciationText);
        ::DrawTextW(hdc, pronunciation.c_str(), static_cast<int>(pronunciation.length()), &pronunciationRc,
                    DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }
    x += pronSize.cx + (pronType.empty() ? 0 : titleGap);
    RECT pronTypeRc = {x, y, right, y + titleHeight};
    if (paint) {
        ::SelectObject(hdc, pronTypeFont);
        ::SetTextColor(hdc, kSecondaryText);
        ::DrawTextW(hdc, pronType.c_str(), static_cast<int>(pronType.length()), &pronTypeRc,
                    DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }
    y += titleHeight + spacing;

    int bodyX = panelRc.left + padX;
    const int bodyTop = y;
    const int bodyHeight =
        (std::max)(scalePx(kDictionaryBodyMinHeight),
                   static_cast<int>(textExtent(hdc, bodyFont, L"Ag").cy) + posPadding * 2);
    if (paint) {
        paintPartOfSpeechPills(
            hdc, bodyX, bodyTop + scalePx(kDictionaryPillBaselineOffset), right,
            entry.formattedPartsOfSpeech(), posFont);
    }
    if (!entry.formattedPartsOfSpeech().empty()) {
        bodyX += definitionGap - posGap;
    }

    std::wstring body;
    for (const auto& reg : entry.formattedRegister()) {
        if (!body.empty()) {
            body += L"  ";
        }
        body += reg;
    }
    for (const auto& label : entry.formattedLabels()) {
        if (!body.empty()) {
            body += std::wstring(labelGap / (std::max)(1, scalePx(kInlineBodySpacingUnit)), L' ');
        }
        body += label;
    }
    const std::wstring canonical = entry.canonicalReference();
    if (!canonical.empty()) {
        if (!body.empty()) {
            body += L"  ";
        }
        body += L"→" + canonical;
    } else {
        const std::wstring mainDefinition = entry.definition(displayPreferences_.mainLanguage);
        if (!mainDefinition.empty()) {
            if (!body.empty()) {
                body += L"  ";
            }
            body += mainDefinition;
        }
    }
    RECT bodyRc = {bodyX, bodyTop, right, bodyTop + bodyHeight};
    if (paint) {
        ::SelectObject(hdc, bodyFont);
        ::SetTextColor(hdc, kDefinitionText);
        ::DrawTextW(hdc, body.c_str(), static_cast<int>(body.length()), &bodyRc,
                    DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }
    y = bodyRc.bottom + spacing;

    int keyWidth = 0;
    for (const auto& other : entry.otherData()) {
        keyWidth = (std::max)(keyWidth, static_cast<int>(textExtent(hdc, bodyFont, other.name).cx));
    }
    for (const auto& other : entry.otherData()) {
        const std::vector<std::wstring> values = other.values();
        const int labelHeight = static_cast<int>(textExtent(hdc, bodyFont, other.name).cy);
        for (int i = 0, n = static_cast<int>(values.size()); i < n; ++i) {
            const int lineHeight = (std::max)(labelHeight,
                                              static_cast<int>(textExtent(hdc, valueFont, values[i]).cy));
            RECT labelRc = {
                panelRc.left + padX,
                y - scalePx(kDictionaryLabelBaselineOffset),
                panelRc.left + padX + keyWidth,
                y + lineHeight};
            RECT valueRc = {labelRc.right + fieldGap, y, right, labelRc.bottom};
            if (paint && i == 0) {
                ::SelectObject(hdc, bodyFont);
                ::SetTextColor(hdc, kSecondaryText);
                ::DrawTextW(hdc, other.name.c_str(), static_cast<int>(other.name.length()), &labelRc,
                            DT_RIGHT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            }
            if (paint) {
                ::SelectObject(hdc, valueFont);
                ::SetTextColor(hdc, textColor_);
                ::DrawTextW(hdc, values[i].c_str(), static_cast<int>(values[i].length()), &valueRc,
                            DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            }
            y += lineHeight + fieldSpacing;
        }
    }

    const auto moreLanguages = entry.otherLanguages(displayPreferences_);
    if (!moreLanguages.empty()) {
        y += spacing - fieldSpacing;
        RECT captionRc = {panelRc.left + padX, y, right, y + textExtent(hdc, captionFont, L"More Languages").cy};
        const std::wstring caption = L"More Languages";
        if (paint) {
            ::SelectObject(hdc, captionFont);
            ::SetTextColor(hdc, textColor_);
            ::DrawTextW(hdc, caption.c_str(), static_cast<int>(caption.length()), &captionRc,
                        DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX);
        }
        y = captionRc.bottom + moreLanguagesSpacing;
        int languageKeyWidth = 0;
        for (const auto& language : moreLanguages) {
            languageKeyWidth = (std::max)(languageKeyWidth, static_cast<int>(textExtent(hdc, bodyFont, language.name).cx));
        }
        for (const auto& language : moreLanguages) {
            const int lineHeight = (std::max)(static_cast<int>(textExtent(hdc, bodyFont, language.name).cy),
                                              static_cast<int>(textExtent(hdc, valueFont, language.value).cy));
            RECT labelRc = {
                panelRc.left + padX,
                y - scalePx(kDictionaryLabelBaselineOffset),
                panelRc.left + padX + languageKeyWidth,
                y + lineHeight};
            RECT valueRc = {labelRc.right + fieldGap, y, right, labelRc.bottom};
            if (paint) {
                ::SelectObject(hdc, bodyFont);
                ::SetTextColor(hdc, kSecondaryText);
                ::DrawTextW(hdc, language.name.c_str(), static_cast<int>(language.name.length()), &labelRc,
                            DT_RIGHT | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
                ::SelectObject(hdc, valueFont);
                ::SetTextColor(hdc, kDefinitionText);
                ::DrawTextW(hdc, language.value.c_str(), static_cast<int>(language.value.length()), &valueRc,
                            DT_LEFT | DT_BOTTOM | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            }
            y += lineHeight + fieldSpacing;
        }
    }

    y += entrySpacing - fieldSpacing + padY;
    ::SelectObject(hdc, oldFont);
    ::DeleteObject(captionFont);
    ::DeleteObject(valueFont);
    ::DeleteObject(bodyFont);
    ::DeleteObject(posFont);
    ::DeleteObject(pronTypeFont);
    ::DeleteObject(pronFont);
    ::DeleteObject(entryFont);
}

void CandidateWindow::paintDictionaryScrollBar(HDC hdc, const RECT& panelRc) {
    const int maxScroll = dictionaryMaxScrollOffset();
    if (maxScroll <= 0 || dictionaryPanelHeight_ <= 0 || dictionaryContentHeight_ <= 0) {
        return;
    }

    const int trackPad = scalePx(kDictionaryScrollTrackPadding);
    const int trackWidth = (std::max)(scalePx(kDictionaryScrollTrackWidth), kDictionaryScrollTrackWidth);
    RECT trackRc = {
        panelRc.right - borderWidth_ - trackPad,
        panelRc.top + borderWidth_ + trackPad,
        panelRc.right - borderWidth_ - trackPad + trackWidth,
        panelRc.bottom - borderWidth_ - trackPad};
    const int trackHeight = trackRc.bottom - trackRc.top;
    if (trackHeight <= 0) {
        return;
    }
    const int thumbHeight = (std::max)(
        scalePx(kDictionaryScrollThumbMinHeight),
        ::MulDiv(trackHeight, dictionaryPanelHeight_, dictionaryContentHeight_));
    const int thumbTravel = (std::max)(0, trackHeight - thumbHeight);
    const int thumbTop = trackRc.top + (maxScroll > 0 ? ::MulDiv(thumbTravel, dictionaryScrollOffset_, maxScroll) : 0);
    RECT thumbRc = {trackRc.left, thumbTop, trackRc.right, thumbTop + thumbHeight};
    HBRUSH trackBrush = ::CreateSolidBrush(kDictionaryScrollTrack);
    HBRUSH thumbBrush = ::CreateSolidBrush(kDictionaryScrollThumb);
    ::FillRect(hdc, &trackRc, trackBrush);
    ::FillRect(hdc, &thumbRc, thumbBrush);
    ::DeleteObject(thumbBrush);
    ::DeleteObject(trackBrush);
}

void CandidateWindow::paintPartOfSpeechPills(
    HDC hdc,
    int& x,
    int y,
    int maxRight,
    const std::vector<std::wstring>& values,
    HFONT pillFont) {
    if (values.empty()) {
        return;
    }

    HGDIOBJ oldFont = ::SelectObject(hdc, pillFont ? pillFont : (commentFont_ ? commentFont_ : font_));
    HPEN borderPen = ::CreatePen(PS_SOLID, (std::max)(kBorderWidth, scalePx(kBorderWidth)), kPosPillBorder);
    HBRUSH fillBrush = ::CreateSolidBrush(kPosPillBackground);
    HGDIOBJ oldPen = ::SelectObject(hdc, borderPen);
    HGDIOBJ oldBrush = ::SelectObject(hdc, fillBrush);
    const int gap = scalePx(kDictionaryPillGap);
    const int padX = scalePx(kDictionaryPosPadding);
    const int pillHeight = scalePx(kDictionaryPillHeight);

    for (const auto& value : values) {
        SIZE textSize = {};
        ::GetTextExtentPoint32W(hdc, value.c_str(), static_cast<int>(value.length()), &textSize);
        const int pillWidth = static_cast<int>(textSize.cx) + padX * 2;
        if (x + pillWidth > maxRight) {
            break;
        }

        ::RoundRect(hdc, x, y, x + pillWidth, y + pillHeight,
                    scalePx(kDictionaryPillCornerRadius) * 2,
                    scalePx(kDictionaryPillCornerRadius) * 2);
        RECT textRc = {x + padX, y, x + pillWidth - padX, y + pillHeight};
        ::SetTextColor(hdc, kPosPillText);
        ::DrawTextW(hdc, value.c_str(), static_cast<int>(value.length()), &textRc,
                    DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        x += pillWidth + gap;
    }

    ::SelectObject(hdc, oldBrush);
    ::SelectObject(hdc, oldPen);
    ::SelectObject(hdc, oldFont);
    ::DeleteObject(fillBrush);
    ::DeleteObject(borderPen);
}

void CandidateWindow::paintPreeditCursor(HDC hdc, const RECT& preeditRc, int cursorX) {
    if (preedit_.empty()) {
        return;
    }

    cursorX = (std::max)(static_cast<int>(preeditRc.left),
                         (std::min)(static_cast<int>(preeditRc.right - 1), cursorX));
    const int cursorWidth = kPreeditCursorWidth;
    RECT cursorRc = {
        cursorX,
        preeditRc.top + kPreeditCursorVerticalInset,
        cursorX + cursorWidth,
        preeditRc.bottom - kPreeditCursorVerticalInset};
    HBRUSH cursorBrush = ::CreateSolidBrush(textColor_);
    ::FillRect(hdc, &cursorRc, cursorBrush);
    ::DeleteObject(cursorBrush);
}

int CandidateWindow::hitTestCandidate(POINT pt) const {
    if (items_.empty()) {
        return -1;
    }

    RECT clientRc = {};
    ::GetClientRect(hwnd_, &clientRc);
    const int candidatePanelRight = candidatePanelWidth_ > 0 ? candidatePanelWidth_ : clientRc.right;
    for (int i = 0, n = static_cast<int>(items_.size()); i < n; ++i) {
        RECT rect = {};
        itemRect(i, rect);
        if (candPerRow_ == 1) {
            rect.left = borderWidth_ + padX_;
            rect.right = candidatePanelRight;
        }
        if (::PtInRect(&rect, pt)) {
            return i;
        }
    }
    return -1;
}

void CandidateWindow::pageNavigationButtonRect(bool next, RECT& rect) const {
    const int candidatePanelRight = candidatePanelWidth_ > 0
                                        ? candidatePanelWidth_
                                        : padX_ * 2 + minWidth_ + borderWidth_ * 2;
    rect = {
        candidatePanelRight - borderWidth_ - padX_ - pageNavWidth_,
        borderWidth_ + padY_,
        candidatePanelRight - borderWidth_ - padX_,
        borderWidth_ + padY_ +
            (preedit_.empty() ? scalePx(kPageNavPreeditlessHeight)
                              : preeditHeight_ + scalePx(kPreeditExtraHeight))};
    if (next) {
        rect.left += pageNavWidth_ / 2;
    } else {
        rect.right = rect.left + pageNavWidth_ / 2;
    }
}

bool CandidateWindow::isPageNavigationEnabled(bool next) const {
    const auto* service = productTextService(textService_);
    if (!service) {
        return false;
    }
    if (next) {
        if (service->candidateHasNext()) {
            return true;
        }
        const int pageSize = service->candidatePageSize();
        const int totalCount = service->candidateTotalCount();
        return pageSize > 0 && totalCount > (service->candidatePageIndex() + 1) * pageSize;
    }
    return service->candidateHasPrevious() || service->candidatePageIndex() > 0;
}

int CandidateWindow::hitTestPageNavigation(POINT pt) const {
    if (pageNavWidth_ <= 0 || candidatePanelWidth_ <= 0) {
        return kPageNavNone;
    }
    RECT prevRc = {};
    RECT nextRc = {};
    pageNavigationButtonRect(false, prevRc);
    pageNavigationButtonRect(true, nextRc);
    if (isPageNavigationEnabled(false) && ::PtInRect(&prevRc, pt)) {
        return kPageNavPrevious;
    }
    if (isPageNavigationEnabled(true) && ::PtInRect(&nextRc, pt)) {
        return kPageNavNext;
    }
    return kPageNavNone;
}

void CandidateWindow::onLButtonDown(WPARAM wp, LPARAM lp) {
    POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
    const int navDirection = hitTestPageNavigation(pt);
    if (navDirection != kPageNavNone) {
        pressedPageNavDirection_ = navDirection;
        hoveredPageNavDirection_ = navDirection;
        pressedSel_ = -1;
        draggingWindow_ = false;
        ::InvalidateRect(hwnd_, NULL, FALSE);
        ::SetCapture(hwnd_);
        return;
    }

    const int hitIndex = hitTestCandidate(pt);
    if (hitIndex >= 0) {
        pressedSel_ = hitIndex;
        pressedPageNavDirection_ = kPageNavNone;
        draggingWindow_ = false;
        ::SetCapture(hwnd_);
        return;
    }

    pressedSel_ = -1;
    pressedPageNavDirection_ = kPageNavNone;
    draggingWindow_ = true;
    Ime::ImeWindow::onLButtonDown(wp, lp);
}

void CandidateWindow::onLButtonUp(WPARAM wp, LPARAM lp) {
    const bool hadCapture = ::GetCapture() == hwnd_;
    if (hadCapture) {
        ::ReleaseCapture();
    }

    if (draggingWindow_) {
        draggingWindow_ = false;
        Ime::ImeWindow::onLButtonUp(wp, lp);
        return;
    }

    POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
    const int navDirection = hitTestPageNavigation(pt);
    if (pressedPageNavDirection_ != kPageNavNone &&
        navDirection == pressedPageNavDirection_) {
        const int direction = pressedPageNavDirection_;
        pressedPageNavDirection_ = kPageNavNone;
        hoveredPageNavDirection_ = navDirection;
        ::InvalidateRect(hwnd_, NULL, FALSE);
        if (auto* textService = static_cast<Moqi::TextService*>(textService_)) {
            textService->changeCandidatePage(direction == kPageNavPrevious);
        }
        pressedSel_ = -1;
        return;
    }
    pressedPageNavDirection_ = kPageNavNone;

    const int hitIndex = hitTestCandidate(pt);
    if (pressedSel_ >= 0 && hitIndex >= 0 &&
        (hitIndex == currentSel_ || hitIndex == pressedSel_)) {
        if (auto* textService = static_cast<Moqi::TextService*>(textService_)) {
            textService->selectCandidate(hitIndex);
        }
    }
    pressedSel_ = -1;
    ::InvalidateRect(hwnd_, NULL, FALSE);
}

void CandidateWindow::onMouseMove(WPARAM wp, LPARAM lp) {
    if (!trackingMouse_) {
        TRACKMOUSEEVENT tme = {};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd_;
        if (::TrackMouseEvent(&tme)) {
            trackingMouse_ = true;
        }
    }

    if (draggingWindow_) {
        Ime::ImeWindow::onMouseMove(wp, lp);
        return;
    }

    POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
    const int navDirection = hitTestPageNavigation(pt);
    if (hoveredPageNavDirection_ != navDirection) {
        hoveredPageNavDirection_ = navDirection;
        ::InvalidateRect(hwnd_, NULL, FALSE);
    }
    updateDictionaryRevealFromMovement(pt);
}

void CandidateWindow::onMouseLeave() {
    trackingMouse_ = false;
    hoveredPageNavDirection_ = kPageNavNone;
    pressedPageNavDirection_ = kPageNavNone;
    resetDictionaryReveal();
    recalculateSize();
    if (isVisible()) {
        ::InvalidateRect(hwnd_, NULL, TRUE);
    }
}

void CandidateWindow::onMouseWheel(WPARAM wp, LPARAM lp) {
    const short delta = GET_WHEEL_DELTA_WPARAM(wp);
    if (delta == 0) {
        return;
    }
    POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
    ::ScreenToClient(hwnd_, &pt);
    if (dictionaryPanelVisible() && pointInDictionaryPanel(pt) && dictionaryMaxScrollOffset() > 0) {
        const int scrollLines = (std::max)(1, std::abs(delta) / WHEEL_DELTA);
        const int scrollStep = scalePx(kDictionaryScrollStep) * scrollLines;
        dictionaryScrollOffset_ += delta > 0 ? -scrollStep : scrollStep;
        clampDictionaryScrollOffset();
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
        return;
    }
    resetDictionaryReveal();
    if (auto* textService = static_cast<Moqi::TextService*>(textService_)) {
        textService->changeCandidatePage(delta > 0);
    }
}

int CandidateWindow::scalePx(int value) const {
    HDC hdc = hwnd_ ? ::GetDC(hwnd_) : ::GetDC(nullptr);
    const int dpi = hdc ? ::GetDeviceCaps(hdc, LOGPIXELSX) : kWindowDpiBaseline;
    if (hdc) {
        ::ReleaseDC(hwnd_ ? hwnd_ : nullptr, hdc);
    }
    return ::MulDiv(value, dpi, kWindowDpiBaseline);
}

int CandidateWindow::entryRowCount(const CandidateUiItem& item) const {
    const auto matched = item.candidateInfo.matchedEntries();
    if (!matched.empty()) {
        return (std::max)(1, static_cast<int>(matched.size()));
    }
    if (!item.candidateInfo.entries.empty()) {
        return (std::max)(1, static_cast<int>(item.candidateInfo.entries.size()));
    }
    return 1;
}

int CandidateWindow::effectiveDictionaryIndex() const {
    if (dictionaryRevealIndex_ >= 0 &&
        dictionaryRevealIndex_ < static_cast<int>(items_.size())) {
        return dictionaryRevealIndex_;
    }
    if (Ime::isDebugLoggingEnabled()) {
        if (dictionaryHoverIndex_ >= 0 &&
            dictionaryHoverIndex_ < static_cast<int>(items_.size())) {
            return dictionaryHoverIndex_;
        }
        if (currentSel_ >= 0 &&
            currentSel_ < static_cast<int>(items_.size())) {
            return currentSel_;
        }
    }
    return -1;
}

bool CandidateWindow::dictionaryPanelVisible() const {
    const int index = effectiveDictionaryIndex();
    return index >= 0 && items_[index].candidateInfo.hasDictionaryEntry(displayPreferences_);
}

int CandidateWindow::dictionaryMaxScrollOffset() const {
    return (std::max)(0, dictionaryContentHeight_ - dictionaryPanelHeight_);
}

bool CandidateWindow::pointInDictionaryPanel(POINT pt) const {
    if (!dictionaryPanelVisible() || dictionaryPanelWidth_ <= 0 || dictionaryPanelHeight_ <= 0) {
        return false;
    }
    RECT clientRc = {};
    ::GetClientRect(hwnd_, &clientRc);
    RECT dictionaryRc = {
        candidatePanelWidth_ + panelGap_,
        clientRc.top + dictionaryPanelTop_,
        candidatePanelWidth_ + panelGap_ + dictionaryPanelWidth_,
        clientRc.top + dictionaryPanelTop_ + dictionaryPanelHeight_};
    return ::PtInRect(&dictionaryRc, pt) != FALSE;
}

void CandidateWindow::clampDictionaryScrollOffset() {
    dictionaryScrollOffset_ = (std::max)(0, (std::min)(dictionaryScrollOffset_, dictionaryMaxScrollOffset()));
}

void CandidateWindow::resetDictionaryReveal(bool resetMouseTracking) {
    dictionaryRevealIndex_ = -1;
    dictionaryHoverIndex_ = -1;
    dictionaryScrollOffset_ = 0;
    actualPointerMovementCount_ = 0;
    if (resetMouseTracking) {
        hasLastMouseMovePoint_ = false;
        lastMouseMovePoint_ = {0, 0};
    }
}

void CandidateWindow::updateDictionaryRevealFromMovement(POINT pt) {
    POINT screenPt = pt;
    if (hwnd_) {
        ::ClientToScreen(hwnd_, &screenPt);
    }

    const int hitIndex = hitTestCandidate(pt);
    if (hitIndex < 0) {
        if (dictionaryPanelVisible()) {
            RECT clientRc = {};
            ::GetClientRect(hwnd_, &clientRc);
            const int dictionaryLeft = candidatePanelWidth_ + panelGap_;
            RECT dictionaryBridgeRc = {
                candidatePanelWidth_ - scalePx(kDictionaryBridgeOverlap),
                clientRc.top + dictionaryPanelTop_,
                dictionaryLeft + dictionaryPanelWidth_,
                clientRc.top + dictionaryPanelTop_ + dictionaryPanelHeight_};
            if (::PtInRect(&dictionaryBridgeRc, pt)) {
                return;
            }
        }
        if (dictionaryRevealIndex_ >= 0 || dictionaryHoverIndex_ >= 0) {
            resetDictionaryReveal();
            recalculateSize();
            if (isVisible()) {
                ::InvalidateRect(hwnd_, NULL, TRUE);
            }
        }
        return;
    }

    const bool actualPointerMovement =
        !hasLastMouseMovePoint_ ||
        lastMouseMovePoint_.x != screenPt.x ||
        lastMouseMovePoint_.y != screenPt.y;
    if (!actualPointerMovement) {
        return;
    }

    hasLastMouseMovePoint_ = true;
    lastMouseMovePoint_ = screenPt;
    ++actualPointerMovementCount_;

    if (actualPointerMovementCount_ < movementRevealThreshold_) {
        return;
    }

    if (dictionaryHoverIndex_ != hitIndex) {
        const int oldEffectiveDictionaryIndex = effectiveDictionaryIndex();
        const bool oldDictionaryVisible = dictionaryPanelVisible();
        dictionaryHoverIndex_ = hitIndex;
        dictionaryRevealIndex_ =
            items_[hitIndex].candidateInfo.hasDictionaryEntry(displayPreferences_) ? hitIndex : -1;
        dictionaryScrollOffset_ = 0;
        actualPointerMovementCount_ = movementRevealThreshold_;
        const bool dictionaryTargetChanged = oldEffectiveDictionaryIndex != effectiveDictionaryIndex();
        if (oldDictionaryVisible != dictionaryPanelVisible() || dictionaryTargetChanged) {
            recalculateSize();
        }
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
        return;
    }

    if (hitIndex >= 0 &&
        hitIndex < static_cast<int>(items_.size()) &&
        items_[hitIndex].candidateInfo.hasDictionaryEntry(displayPreferences_) &&
        dictionaryRevealIndex_ != hitIndex) {
        dictionaryRevealIndex_ = hitIndex;
        dictionaryScrollOffset_ = 0;
        recalculateSize();
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::itemRect(int index, RECT& rect) const {
    rect.left = borderWidth_ + padX_ / 2;
    rect.top = contentTop_;
    for (int i = 0; i < index && i < static_cast<int>(itemHeights_.size()); ++i) {
        rect.top += itemHeights_[i] + rowSpacing_;
    }
    rect.right = rect.left + itemWidth(index);
    rect.bottom = rect.top + itemHeight(index);
}

int CandidateWindow::itemWidth(int index) const {
    if (index >= 0 && index < static_cast<int>(itemWidths_.size())) {
        return itemWidths_[index];
    }
    return selKeyWidth_ + labelGap_ + jyutpingColumnWidth_ + honziColumnWidth_ +
           noteColumnWidth_ + definitionColumnWidth_ + indicatorColumnWidth_;
}

int CandidateWindow::itemHeight(int index) const {
    if (index >= 0 && index < static_cast<int>(itemHeights_.size())) {
        return itemHeights_[index];
    }
    return itemHeight_;
}

int CandidateWindow::itemTextWidth(int index) const {
    if (index >= 0 && index < static_cast<int>(itemTextWidths_.size())) {
        return itemTextWidths_[index];
    }
    return textWidth_;
}

int CandidateWindow::itemCommentWidth(int index) const {
    if (index >= 0 && index < static_cast<int>(itemCommentWidths_.size())) {
        return itemCommentWidths_[index];
    }
    return commentWidth_;
}

void CandidateWindow::applyWindowShape() {
    if (!hwnd_) {
        return;
    }

    RECT rc = {};
    ::GetClientRect(hwnd_, &rc);
    if (rc.right <= rc.left || rc.bottom <= rc.top) {
        return;
    }

    const int candidatePanelWidth = candidatePanelWidth_ > 0 ? candidatePanelWidth_ : rc.right - rc.left;
    HRGN region = ::CreateRoundRectRgn(
        rc.left, rc.top,
        candidatePanelWidth + 1,
        rc.top + candidatePanelHeight_ + 1,
        borderRadius_ * 2,
        borderRadius_ * 2);
    if (dictionaryPanelVisible()) {
        HRGN dictionaryRegion = ::CreateRoundRectRgn(
            candidatePanelWidth + panelGap_,
            rc.top + dictionaryPanelTop_,
            candidatePanelWidth + panelGap_ + dictionaryPanelWidth_ + 1,
            rc.top + dictionaryPanelTop_ + dictionaryPanelHeight_ + 1,
            borderRadius_ * 2,
            borderRadius_ * 2);
        ::CombineRgn(region, region, dictionaryRegion, RGN_OR);
        ::DeleteObject(dictionaryRegion);
    }
    ::SetWindowRgn(hwnd_, region, TRUE);
}

} // namespace Moqi
