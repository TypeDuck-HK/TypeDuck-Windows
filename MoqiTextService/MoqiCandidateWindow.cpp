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
constexpr COLORREF kInputBufferBackground = RGB(254, 220, 156);  // input_buffer_background
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

void appendCandidateWindowLog(const std::wstring& message) {
    if (!Ime::isTraceLoggingEnabled()) {
        return;
    }

    const wchar_t* localAppData = _wgetenv(L"LOCALAPPDATA");
    if (!localAppData || !*localAppData) {
        return;
    }

    std::wstring logDir = std::wstring(localAppData) + L"\\MoqiIM\\Log";
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
      padX_(service->isImmersive() ? 10 : 7),
      padY_(service->isImmersive() ? 6 : 3),
      labelGap_(6),
      commentGap_(8),
      borderWidth_(1),
      borderRadius_(4),
      minWidth_(200),
      preeditHeight_(0),
      preeditGap_(8),
      contentTop_(0),
      panelGap_(10),
      rowPaddingY_(5),
      rowInnerGap_(6),
      jyutpingColumnWidth_(84),
      honziColumnWidth_(86),
      noteColumnWidth_(78),
      definitionColumnWidth_(230),
      indicatorColumnWidth_(22),
      pageNavWidth_(64),
      dictionaryPanelWidth_(0),
      dictionaryPanelMinHeight_(260),
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
      currentSel_(0),
      pressedSel_(-1),
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
        : (std::min<UINT>)(10, static_cast<UINT>(items_.size()));
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
        label, item.text, rawComment);
    items_.push_back(std::move(item));
    selKeys_.push_back(selKey);
}

void CandidateWindow::clear() {
    items_.clear();
    selKeys_.clear();
    itemTextWidths_.clear();
    itemCommentWidths_.clear();
    itemWidths_.clear();
    resetDictionaryReveal();
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
        currentSel_ = sel;
        if (dictionaryRevealIndex_ >= static_cast<int>(items_.size())) {
            resetDictionaryReveal();
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
        resize(padX_ * 2 + borderWidth_ * 2, padY_ * 2 + borderWidth_ * 2);
        applyWindowShape();
        return;
    }

    HDC hdc = ::GetWindowDC(hwnd());
    if (!hdc) {
        return;
    }

    const int scaledPadX = scalePx(textService_->isImmersive() ? 12 : 10);
    const int scaledPadY = scalePx(textService_->isImmersive() ? 8 : 6);
    padX_ = scaledPadX;
    padY_ = scaledPadY;
    labelGap_ = scalePx(6);
    commentGap_ = scalePx(8);
    borderWidth_ = (std::max)(1, scalePx(1));
    borderRadius_ = scalePx(8);
    preeditGap_ = scalePx(8);
    panelGap_ = scalePx(10);
    rowPaddingY_ = scalePx(5);
    rowInnerGap_ = scalePx(6);
    pageNavWidth_ = scalePx(64);
    indicatorColumnWidth_ = scalePx(22);
    jyutpingColumnWidth_ = scalePx(84);
    honziColumnWidth_ = scalePx(86);
    noteColumnWidth_ = scalePx(78);
    definitionColumnWidth_ = scalePx(230);
    dictionaryPanelMinHeight_ = scalePx(260);
    minWidth_ = scalePx(520);

    selKeyWidth_ = 0;
    textWidth_ = 0;
    commentWidth_ = 0;
    itemHeight_ = 0;
    preeditHeight_ = 0;
    itemTextWidths_.assign(items_.size(), 0);
    itemCommentWidths_.assign(items_.size(), 0);
    itemWidths_.assign(items_.size(), 0);
    int preeditWidth = 0;

    HGDIOBJ oldFont = ::SelectObject(hdc, font_);
    TEXTMETRICW metrics = {};
    TEXTMETRICW commentMetrics = {};
    for (int i = 0, n = static_cast<int>(items_.size()); i < n; ++i) {
        SIZE selKeySize = {};
        wchar_t selKey[] = L"?.";
        selKey[0] = selKeys_[i];
        ::GetTextExtentPoint32W(hdc, selKey, 2, &selKeySize);
        selKeyWidth_ = (std::max)(selKeyWidth_, static_cast<int>(selKeySize.cx));

        SIZE candidateSize = {};
        const CandidateUiItem& item = items_[i];
        const std::wstring itemText = item.displayText();
        const std::wstring itemComment = item.displayComment();
        ::GetTextExtentPoint32W(hdc, itemText.c_str(), static_cast<int>(itemText.length()), &candidateSize);
        itemTextWidths_[i] = static_cast<int>(candidateSize.cx);
        textWidth_ = (std::max)(textWidth_, static_cast<int>(candidateSize.cx));
        int candidateHeight = static_cast<int>(candidateSize.cy);
        const int candidateEntryRows = entryRowCount(item);
        if (!itemComment.empty() && commentFont_) {
            SIZE commentSize = {};
            ::SelectObject(hdc, commentFont_);
            ::GetTextExtentPoint32W(hdc, itemComment.c_str(), static_cast<int>(itemComment.length()), &commentSize);
            ::SelectObject(hdc, font_);
            itemCommentWidths_[i] = static_cast<int>(commentSize.cx);
            commentWidth_ = (std::max)(commentWidth_, static_cast<int>(commentSize.cx));
            candidateHeight = (std::max)(candidateHeight, static_cast<int>(commentSize.cy));
        }
        itemHeight_ = (std::max)(itemHeight_, (std::max)(candidateHeight, static_cast<int>(selKeySize.cy)));
        itemHeight_ = (std::max)(itemHeight_, rowPaddingY_ * 2 + candidateEntryRows * scalePx(25));
    }
    if (!preedit_.empty()) {
        SIZE preeditSize = {};
        ::GetTextExtentPoint32W(hdc, preedit_.c_str(), static_cast<int>(preedit_.length()), &preeditSize);
        preeditWidth = static_cast<int>(preeditSize.cx);
        textWidth_ = (std::max)(textWidth_, static_cast<int>(preeditSize.cx));
        preeditHeight_ = static_cast<int>(preeditSize.cy);
    }
    ::GetTextMetricsW(hdc, &metrics);
    if (commentFont_) {
        ::SelectObject(hdc, commentFont_);
        ::GetTextMetricsW(hdc, &commentMetrics);
        ::SelectObject(hdc, font_);
    }
    ::SelectObject(hdc, oldFont);
    ::ReleaseDC(hwnd(), hdc);

    itemHeight_ = (std::max)(itemHeight_, static_cast<int>(metrics.tmHeight + metrics.tmExternalLeading + 2));
    if (commentFont_) {
        itemHeight_ = (std::max)(itemHeight_, static_cast<int>(commentMetrics.tmHeight + commentMetrics.tmExternalLeading + 2));
    }
    preeditHeight_ = preedit_.empty()
                         ? 0
                         : (std::max)(preeditHeight_, static_cast<int>(metrics.tmHeight + metrics.tmExternalLeading));

    for (int i = 0, n = static_cast<int>(items_.size()); i < n; ++i) {
        itemWidths_[i] = selKeyWidth_ + labelGap_ + jyutpingColumnWidth_ +
                         honziColumnWidth_ + noteColumnWidth_ +
                         definitionColumnWidth_ + indicatorColumnWidth_;
    }

    const int rows = static_cast<int>(items_.size());
    const int candidateContentWidth = selKeyWidth_ + labelGap_ + jyutpingColumnWidth_ +
                                      honziColumnWidth_ + noteColumnWidth_ +
                                      definitionColumnWidth_ + indicatorColumnWidth_;
    const int contentWidth = (std::max)((std::max)(candidateContentWidth, preeditWidth + pageNavWidth_), minWidth_);
    const int candidatePanelWidth = padX_ * 2 + contentWidth + borderWidth_ * 2;
    int candidatePanelHeight = rows * itemHeight_ + (std::max)(0, rows - 1) * rowSpacing_;
    if (!preedit_.empty()) {
        contentTop_ = borderWidth_ + padY_ + preeditHeight_ + preeditGap_;
        candidatePanelHeight += preeditHeight_ + preeditGap_;
    } else {
        contentTop_ = borderWidth_ + padY_;
    }
    candidatePanelHeight += padY_ * 2 + borderWidth_ * 2;
    dictionaryPanelWidth_ = dictionaryPanelVisible() ? scalePx(420) : 0;
    const int dictionaryHeight = dictionaryPanelVisible()
                                     ? (std::max)(dictionaryPanelMinHeight_, candidatePanelHeight)
                                     : 0;
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
    const int candidatePanelWidth = rc.right - rc.left -
        (dictionaryPanelVisible() ? panelGap_ + dictionaryPanelWidth_ : 0);
    RECT candidatePanelRc = {rc.left, rc.top, candidatePanelWidth, rc.bottom};
    HRGN windowRgn = ::CreateRoundRectRgn(
        candidatePanelRc.left, candidatePanelRc.top,
        candidatePanelRc.right + 1, candidatePanelRc.bottom + 1,
        borderRadius_ * 2, borderRadius_ * 2);
    ::FillRect(memdc, &rc, static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH)));
    ::FillRgn(memdc, windowRgn, backgroundBrush);
    ::FrameRgn(memdc, windowRgn, borderBrush, borderWidth_, borderWidth_);

    paintInputBuffer(memdc, candidatePanelRc);
    paintPageNavigation(memdc, candidatePanelRc);

    int y = contentTop_;
    for (int i = 0, n = static_cast<int>(items_.size()); i < n; ++i) {
        RECT rowRc = {
            borderWidth_ + padX_ / 2,
            y,
            candidatePanelRc.right - borderWidth_ - padX_ / 2,
            y + itemHeight_};
        paintCandidateRow(memdc, i, rowRc);
        y += itemHeight_ + rowSpacing_;
    }

    if (dictionaryPanelVisible()) {
        RECT dictionaryRc = {
            candidatePanelRc.right + panelGap_,
            rc.top,
            rc.right,
            rc.bottom};
        HRGN dictionaryRgn = ::CreateRoundRectRgn(
            dictionaryRc.left, dictionaryRc.top, dictionaryRc.right + 1, dictionaryRc.bottom + 1,
            borderRadius_ * 2, borderRadius_ * 2);
        HBRUSH dictionaryBrush = ::CreateSolidBrush(kDictionaryBackground);
        ::FillRgn(memdc, dictionaryRgn, dictionaryBrush);
        ::FrameRgn(memdc, dictionaryRgn, borderBrush, borderWidth_, borderWidth_);
        ::DeleteObject(dictionaryBrush);
        ::DeleteObject(dictionaryRgn);
        paintDictionaryPanel(memdc, dictionaryRc, items_[effectiveDictionaryIndex()].candidateInfo);
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
        panelRc.left + borderWidth_ + padX_,
        panelRc.top + borderWidth_ + padY_,
        panelRc.right - borderWidth_ - padX_ - pageNavWidth_,
        panelRc.top + borderWidth_ + padY_ + preeditHeight_ + scalePx(8)};
    RECT activeRc = preeditRc;
    activeRc.right = (std::min)(activeRc.right,
        activeRc.left + scalePx(18) + static_cast<int>(preedit_.length()) * scalePx(10));

    SIZE inputTextSize = {};
    ::SelectObject(hdc, font_);
    ::GetTextExtentPoint32W(hdc, preedit_.c_str(), static_cast<int>(preedit_.length()), &inputTextSize);
    activeRc.right = (std::min)(preeditRc.right, activeRc.left + scalePx(18) + static_cast<int>(inputTextSize.cx));

    HBRUSH inputBrush = ::CreateSolidBrush(kInputBufferBackground);
    HRGN activeRgn = ::CreateRoundRectRgn(activeRc.left, activeRc.top, activeRc.right + 1,
                                          activeRc.bottom + 1, scalePx(5) * 2, scalePx(5) * 2);
    ::FillRgn(hdc, activeRgn, inputBrush);
    ::DeleteObject(activeRgn);
    ::DeleteObject(inputBrush);

    RECT textRc = activeRc;
    textRc.left += scalePx(8);
    ::SelectObject(hdc, font_);
    ::SetTextColor(hdc, textColor_);
    ::DrawTextW(hdc, preedit_.c_str(), static_cast<int>(preedit_.length()), &textRc,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    paintPreeditCursor(hdc, textRc);

    const int dividerY = preeditRc.bottom + preeditGap_ / 2;
    HPEN dividerPen = ::CreatePen(PS_SOLID, 1, kDividerColor);
    HGDIOBJ oldPen = ::SelectObject(hdc, dividerPen);
    ::MoveToEx(hdc, panelRc.left + borderWidth_ + padX_, dividerY, nullptr);
    ::LineTo(hdc, panelRc.right - borderWidth_ - padX_, dividerY);
    ::SelectObject(hdc, oldPen);
    ::DeleteObject(dividerPen);
}

void CandidateWindow::paintPageNavigation(HDC hdc, const RECT& panelRc) {
    const auto* service = productTextService(textService_);
    const int pageIndex = service != nullptr ? service->candidatePageIndex() : 0;
    const int pageSize = service != nullptr ? service->candidatePageSize() : static_cast<int>(items_.size());
    const int totalCount = service != nullptr ? service->candidateTotalCount() : static_cast<int>(items_.size());
    const bool hasPrev = pageIndex > 0;
    const bool hasNext = pageSize > 0 && totalCount > (pageIndex + 1) * pageSize;

    RECT navRc = {
        panelRc.right - borderWidth_ - padX_ - pageNavWidth_,
        panelRc.top + borderWidth_ + padY_,
        panelRc.right - borderWidth_ - padX_,
        panelRc.top + borderWidth_ + padY_ + (preedit_.empty() ? scalePx(30) : preeditHeight_ + scalePx(8))};
    RECT prevRc = navRc;
    prevRc.right = prevRc.left + pageNavWidth_ / 2;
    RECT nextRc = navRc;
    nextRc.left = prevRc.right;

    ::SelectObject(hdc, font_);
    ::SetTextColor(hdc, hasPrev ? kLinkText : kDisabledText);
    ::DrawTextW(hdc, L"‹", 1, &prevRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    ::SetTextColor(hdc, hasNext ? kLinkText : kDisabledText);
    ::DrawTextW(hdc, L"›", 1, &nextRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

void CandidateWindow::paintItem(HDC hdc, int index, int x, int y) {
    RECT rowRc = {x, y, x + itemWidth(index), y + itemHeight_};
    paintCandidateRow(hdc, index, rowRc);
}

void CandidateWindow::paintCandidateRow(HDC hdc, int index, const RECT& rowRc) {
    // Mirrors TypeDuck Web definitionLayout: enabled displayLanguages decide
    // which mainLanguage/otherLanguages definitions are visible.
    const bool selected = useCursor_ && index == currentSel_;

    const COLORREF bgColor = selected ? highlightColor_ : backgroundColor_;
    const COLORREF selColor = selected ? highlightTextColor_ : textColor_;

    if (selected) {
        HBRUSH highlightBrush = ::CreateSolidBrush(bgColor);
        HRGN rowRgn = ::CreateRoundRectRgn(rowRc.left, rowRc.top, rowRc.right + 1, rowRc.bottom + 1,
                                           scalePx(5) * 2, scalePx(5) * 2);
        ::FillRgn(hdc, rowRgn, highlightBrush);
        ::DeleteObject(rowRgn);
        ::DeleteObject(highlightBrush);
    }

    RECT selRc = rowRc;
    selRc.left += scalePx(8);
    selRc.right = selRc.left + selKeyWidth_;
    wchar_t selKey[] = L"?.";
    selKey[0] = selKeys_[index];
    const COLORREF oldColor = ::SetTextColor(hdc, selColor);
    // candidateBaselineAligned: labels, Jyutping, Honzi, and definitions share vertical centering.
    ::DrawTextW(hdc, selKey, 2, &selRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    const CandidateUiItem& item = items_[index];
    const auto matchedEntries = item.candidateInfo.matchedEntries();
    const std::vector<TypeDuck::CandidateEntry>& allEntries = item.candidateInfo.entries;
    const bool useMatched = !matchedEntries.empty();
    const int rowCount = entryRowCount(item);
    const int lineHeight = (std::max)(scalePx(20), (itemHeight_ - rowPaddingY_ * 2) / rowCount);
    int y = rowRc.top + rowPaddingY_;

    HGDIOBJ oldFont = ::SelectObject(hdc, font_);
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
            fallbackEntry.definitions[displayPreferences_.mainLanguage] = item.displayComment();
            entry = &fallbackEntry;
        }

        RECT jyutpingRc = {selRc.right + labelGap_, y,
                           selRc.right + labelGap_ + jyutpingColumnWidth_, y + lineHeight};
        RECT honziRc = {jyutpingRc.right + rowInnerGap_, y,
                        jyutpingRc.right + rowInnerGap_ + honziColumnWidth_, y + lineHeight};
        RECT noteRc = {honziRc.right + rowInnerGap_, y,
                       honziRc.right + rowInnerGap_ + noteColumnWidth_, y + lineHeight};
        RECT definitionRc = {noteRc.right + rowInnerGap_, y,
                             noteRc.right + rowInnerGap_ + definitionColumnWidth_, y + lineHeight};
        RECT indicatorRc = {definitionRc.right + rowInnerGap_, y, rowRc.right - scalePx(8), y + lineHeight};

        ::SelectObject(hdc, commentFont_ ? commentFont_ : font_);
        ::SetTextColor(hdc, selected ? commentHighlightColor_ : kPronunciationText);
        const bool showJyutping = displayPreferences_.shouldShowJyutping(item.candidateInfo.isReverseLookup);
        if (showJyutping) {
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
        if (entryIndex == 0 && !note.empty()) {
            ::DrawTextW(hdc, note.c_str(), static_cast<int>(note.length()), &noteRc,
                        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        }

        std::wstring definition = entry->definition(displayPreferences_.mainLanguage);
        if (definition.empty()) {
            const std::wstring reference = entry->canonicalReference();
            definition = reference.empty() ? item.candidateInfo.displayComment(displayPreferences_) : L"→" + reference;
        }
        ::SetTextColor(hdc, selected ? commentHighlightColor_ : kDefinitionText);
        ::DrawTextW(hdc, definition.c_str(), static_cast<int>(definition.length()), &definitionRc,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);

        if (entryIndex == 0 && item.candidateInfo.hasDictionaryEntry(displayPreferences_)) {
            ::SetTextColor(hdc, selected ? highlightTextColor_ : kSecondaryText);
            ::DrawTextW(hdc, L"ⓘ", 1, &indicatorRc,
                        DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
        y += lineHeight;
    }
    ::SelectObject(hdc, oldFont);
    ::SetTextColor(hdc, oldColor);
}

void CandidateWindow::paintDictionaryPanel(
    HDC hdc,
    const RECT& panelRc,
    const TypeDuck::CandidateInfo& info) {
    int y = panelRc.top + scalePx(12);
    RECT titleRc = {panelRc.left + scalePx(18), y, panelRc.right - scalePx(18), y + scalePx(28)};
    ::SelectObject(hdc, commentFont_ ? commentFont_ : font_);
    ::SetTextColor(hdc, kSecondaryText);
    const std::wstring title = L"字典 Dictionary";
    ::DrawTextW(hdc, title.c_str(), static_cast<int>(title.length()), &titleRc,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    y = titleRc.bottom + scalePx(4);

    for (const auto& entry : info.entries) {
        if (!entry.isDictionaryEntry(displayPreferences_)) {
            continue;
        }
        paintDictionaryEntry(hdc, y, panelRc, entry);
        if (y > panelRc.bottom - scalePx(24)) {
            break;
        }
    }
}

void CandidateWindow::paintDictionaryEntry(
    HDC hdc,
    int& y,
    const RECT& panelRc,
    const TypeDuck::CandidateEntry& entry) {
    const int padX = scalePx(18);
    const int right = panelRc.right - padX;
    RECT headRc = {panelRc.left + padX, y, right, y + scalePx(48)};
    ::SelectObject(hdc, font_);
    ::SetTextColor(hdc, textColor_);
    ::DrawTextW(hdc, entry.honzi.c_str(), static_cast<int>(entry.honzi.length()), &headRc,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);

    RECT pronunciationRc = {headRc.left + scalePx(104), y + scalePx(8), right, headRc.bottom};
    std::wstring pronunciation = entry.jyutping;
    const std::wstring pronType = entry.pronunciationType();
    if (!pronType.empty()) {
        pronunciation += L" " + pronType;
    }
    ::SelectObject(hdc, commentFont_ ? commentFont_ : font_);
    ::SetTextColor(hdc, kPronunciationText);
    ::DrawTextW(hdc, pronunciation.c_str(), static_cast<int>(pronunciation.length()), &pronunciationRc,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    y = headRc.bottom + scalePx(4);

    const int bodyTop = y;
    const int bodyBottom = y + scalePx(34);
    int bodyX = panelRc.left + padX;
    paintPartOfSpeechPills(hdc, bodyX, bodyTop + scalePx(5), right, entry.formattedPartsOfSpeech());

    std::wstring body;
    for (const auto& reg : entry.formattedRegister()) {
        if (!body.empty()) {
            body += L"  ";
        }
        body += reg;
    }
    for (const auto& label : entry.formattedLabels()) {
        if (!body.empty()) {
            body += L"  ";
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
    RECT bodyRc = {bodyX, bodyTop, right, bodyBottom};
    ::SetTextColor(hdc, kDefinitionText);
    ::DrawTextW(hdc, body.c_str(), static_cast<int>(body.length()), &bodyRc,
                DT_LEFT | DT_WORDBREAK | DT_NOPREFIX | DT_END_ELLIPSIS);
    y = bodyRc.bottom + scalePx(4);

    for (const auto& other : entry.otherData()) {
        RECT labelRc = {panelRc.left + padX, y, panelRc.left + padX + scalePx(128), y + scalePx(24)};
        RECT valueRc = {labelRc.right + scalePx(10), y, right, labelRc.bottom};
        ::SetTextColor(hdc, kSecondaryText);
        ::DrawTextW(hdc, other.name.c_str(), static_cast<int>(other.name.length()), &labelRc,
                    DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        ::SetTextColor(hdc, textColor_);
        ::DrawTextW(hdc, other.value.c_str(), static_cast<int>(other.value.length()), &valueRc,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
        y += scalePx(24);
    }

    const auto moreLanguages = entry.otherLanguages(displayPreferences_);
    if (!moreLanguages.empty()) {
        RECT captionRc = {panelRc.left + padX, y, right, y + scalePx(26)};
        const std::wstring caption = L"More Languages";
        ::SetTextColor(hdc, textColor_);
        ::DrawTextW(hdc, caption.c_str(), static_cast<int>(caption.length()), &captionRc,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        y = captionRc.bottom;
        for (const auto& language : moreLanguages) {
            RECT labelRc = {panelRc.left + padX, y, panelRc.left + padX + scalePx(128), y + scalePx(24)};
            RECT valueRc = {labelRc.right + scalePx(10), y, right, labelRc.bottom};
            ::SetTextColor(hdc, kSecondaryText);
            ::DrawTextW(hdc, language.name.c_str(), static_cast<int>(language.name.length()), &labelRc,
                        DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            ::SetTextColor(hdc, kDefinitionText);
            ::DrawTextW(hdc, language.value.c_str(), static_cast<int>(language.value.length()), &valueRc,
                        DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
            y += scalePx(24);
        }
    }

    y += scalePx(14);
}

void CandidateWindow::paintPartOfSpeechPills(
    HDC hdc,
    int& x,
    int y,
    int maxRight,
    const std::vector<std::wstring>& values) {
    if (values.empty()) {
        return;
    }

    HGDIOBJ oldFont = ::SelectObject(hdc, commentFont_ ? commentFont_ : font_);
    HPEN borderPen = ::CreatePen(PS_SOLID, (std::max)(1, scalePx(1)), kPosPillBorder);
    HBRUSH fillBrush = ::CreateSolidBrush(kPosPillBackground);
    HGDIOBJ oldPen = ::SelectObject(hdc, borderPen);
    HGDIOBJ oldBrush = ::SelectObject(hdc, fillBrush);
    const int gap = scalePx(5);
    const int padX = scalePx(5);
    const int pillHeight = scalePx(22);

    for (const auto& value : values) {
        SIZE textSize = {};
        ::GetTextExtentPoint32W(hdc, value.c_str(), static_cast<int>(value.length()), &textSize);
        const int pillWidth = static_cast<int>(textSize.cx) + padX * 2;
        if (x + pillWidth > maxRight) {
            break;
        }

        ::RoundRect(hdc, x, y, x + pillWidth, y + pillHeight,
                    scalePx(4) * 2, scalePx(4) * 2);
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

void CandidateWindow::paintPreeditCursor(HDC hdc, const RECT& preeditRc) {
    if (preedit_.empty()) {
        return;
    }

    const int cursor = (std::max)(0, (std::min)(preeditCursor_, static_cast<int>(preedit_.length())));
    SIZE beforeSize = {};
    if (cursor > 0) {
        ::GetTextExtentPoint32W(hdc, preedit_.c_str(), cursor, &beforeSize);
    }
    const int cursorX = (std::min)(preeditRc.right - 1, preeditRc.left + static_cast<int>(beforeSize.cx));
    const int cursorWidth = 2;
    RECT cursorRc = {
        cursorX,
        preeditRc.top + 1,
        cursorX + cursorWidth,
        preeditRc.bottom - 1};
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
    for (int i = 0, n = static_cast<int>(items_.size()); i < n; ++i) {
        RECT rect = {};
        itemRect(i, rect);
        if (candPerRow_ == 1) {
            rect.left = borderWidth_ + padX_;
            rect.right = clientRc.right - borderWidth_ - padX_;
        }
        if (::PtInRect(&rect, pt)) {
            return i;
        }
    }
    return -1;
}

void CandidateWindow::onLButtonDown(WPARAM wp, LPARAM lp) {
    POINT pt = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
    const int hitIndex = hitTestCandidate(pt);
    if (hitIndex >= 0) {
        pressedSel_ = hitIndex;
        draggingWindow_ = false;
        ::SetCapture(hwnd_);
        return;
    }

    pressedSel_ = -1;
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
    const int hitIndex = hitTestCandidate(pt);
    if (pressedSel_ >= 0 && hitIndex >= 0 &&
        (hitIndex == currentSel_ || hitIndex == pressedSel_)) {
        if (auto* textService = static_cast<Moqi::TextService*>(textService_)) {
            textService->selectCandidate(hitIndex);
        }
    }
    pressedSel_ = -1;
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
    updateDictionaryRevealFromMovement(pt);
}

void CandidateWindow::onMouseLeave() {
    trackingMouse_ = false;
    resetDictionaryReveal();
    recalculateSize();
    if (isVisible()) {
        ::InvalidateRect(hwnd_, NULL, TRUE);
    }
}

void CandidateWindow::onMouseWheel(WPARAM wp, LPARAM) {
    const short delta = GET_WHEEL_DELTA_WPARAM(wp);
    if (delta == 0) {
        return;
    }
    resetDictionaryReveal();
    if (auto* textService = static_cast<Moqi::TextService*>(textService_)) {
        textService->changeCandidatePage(delta > 0);
    }
}

int CandidateWindow::scalePx(int value) const {
    HDC hdc = hwnd_ ? ::GetDC(hwnd_) : ::GetDC(nullptr);
    const int dpi = hdc ? ::GetDeviceCaps(hdc, LOGPIXELSX) : 96;
    if (hdc) {
        ::ReleaseDC(hwnd_ ? hwnd_ : nullptr, hdc);
    }
    return ::MulDiv(value, dpi, 96);
}

int CandidateWindow::entryRowCount(const CandidateUiItem& item) const {
    const auto matched = item.candidateInfo.matchedEntries();
    if (!matched.empty()) {
        return (std::min)(3, (std::max)(1, static_cast<int>(matched.size())));
    }
    if (!item.candidateInfo.entries.empty()) {
        return (std::min)(3, (std::max)(1, static_cast<int>(item.candidateInfo.entries.size())));
    }
    return 1;
}

int CandidateWindow::effectiveDictionaryIndex() const {
    if (dictionaryRevealIndex_ >= 0 &&
        dictionaryRevealIndex_ < static_cast<int>(items_.size())) {
        return dictionaryRevealIndex_;
    }
    return -1;
}

bool CandidateWindow::dictionaryPanelVisible() const {
    const int index = effectiveDictionaryIndex();
    return index >= 0 && items_[index].candidateInfo.hasDictionaryEntry(displayPreferences_);
}

void CandidateWindow::resetDictionaryReveal() {
    dictionaryRevealIndex_ = -1;
    dictionaryHoverIndex_ = -1;
    actualPointerMovementCount_ = 0;
    hasLastMouseMovePoint_ = false;
    lastMouseMovePoint_ = {0, 0};
}

void CandidateWindow::updateDictionaryRevealFromMovement(POINT pt) {
    const int hitIndex = hitTestCandidate(pt);
    if (hitIndex < 0) {
        if (dictionaryRevealIndex_ >= 0 || dictionaryHoverIndex_ >= 0) {
            resetDictionaryReveal();
            recalculateSize();
            if (isVisible()) {
                ::InvalidateRect(hwnd_, NULL, TRUE);
            }
        }
        return;
    }

    if (dictionaryHoverIndex_ != hitIndex) {
        dictionaryHoverIndex_ = hitIndex;
        actualPointerMovementCount_ = 0;
        hasLastMouseMovePoint_ = true;
        lastMouseMovePoint_ = pt;
        return;
    }

    const bool actualPointerMovement =
        !hasLastMouseMovePoint_ ||
        lastMouseMovePoint_.x != pt.x ||
        lastMouseMovePoint_.y != pt.y;
    if (!actualPointerMovement) {
        return;
    }
    hasLastMouseMovePoint_ = true;
    lastMouseMovePoint_ = pt;
    ++actualPointerMovementCount_;

    if (actualPointerMovementCount_ >= movementRevealThreshold_ &&
        hitIndex >= 0 &&
        hitIndex < static_cast<int>(items_.size()) &&
        items_[hitIndex].candidateInfo.hasDictionaryEntry(displayPreferences_) &&
        dictionaryRevealIndex_ != hitIndex) {
        dictionaryRevealIndex_ = hitIndex;
        currentSel_ = hitIndex;
        recalculateSize();
        if (isVisible()) {
            ::InvalidateRect(hwnd_, NULL, TRUE);
        }
    }
}

void CandidateWindow::itemRect(int index, RECT& rect) const {
    rect.left = borderWidth_ + padX_ / 2;
    rect.top = contentTop_ + index * (itemHeight_ + rowSpacing_);
    rect.right = rect.left + itemWidth(index);
    rect.bottom = rect.top + itemHeight_;
}

int CandidateWindow::itemWidth(int index) const {
    if (index >= 0 && index < static_cast<int>(itemWidths_.size())) {
        return itemWidths_[index];
    }
    return selKeyWidth_ + labelGap_ + jyutpingColumnWidth_ + honziColumnWidth_ +
           noteColumnWidth_ + definitionColumnWidth_ + indicatorColumnWidth_;
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

    HRGN region = ::CreateRoundRectRgn(
        rc.left, rc.top, rc.right + 1, rc.bottom + 1,
        borderRadius_ * 2, borderRadius_ * 2);
    ::SetWindowRgn(hwnd_, region, TRUE);
}

} // namespace Moqi
