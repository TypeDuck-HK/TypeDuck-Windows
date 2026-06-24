#include <windows.h>
#include <windowsx.h>

#include "MoqiTextService/TypeDuckCandidateInfo.h"

#include <algorithm>
#include <cwctype>
#include <fstream>
#include <string>
#include <vector>

using Moqi::TypeDuck::CandidateInfo;
using Moqi::TypeDuck::ChineseTypeface;
using Moqi::TypeDuck::DisplayLanguage;
using Moqi::TypeDuck::DisplayPreferences;
using Moqi::TypeDuck::JyutpingVisibility;

namespace {

constexpr wchar_t kWindowClassName[] = L"TypeDuckCandidatePreviewWindow";
constexpr wchar_t kWindowTitle[] =
    L"TypeDuck 候選詞及字典預覽 Candidate and dictionary preview";

constexpr COLORREF kAppBackground = RGB(250, 249, 246);
constexpr COLORREF kPanelBackground = RGB(255, 255, 255);       // panel_background
constexpr COLORREF kDictionaryBackground = RGB(246, 243, 237);  // dictionary_background
constexpr COLORREF kInputBufferBackground = RGB(254, 220, 156); // input_buffer_background
constexpr COLORREF kPanelBorder = RGB(222, 217, 207);           // panel_border
constexpr COLORREF kSelectionBackground = RGB(254, 220, 156);   // selection_background
constexpr COLORREF kAccent = RGB(223, 168, 82);                 // accent
constexpr COLORREF kTextPrimary = RGB(36, 34, 30);              // text_primary
constexpr COLORREF kTextSecondary = RGB(105, 98, 88);           // text_secondary
constexpr COLORREF kPronunciationText = RGB(102, 93, 82);       // pronunciation_text
constexpr COLORREF kDefinitionText = RGB(78, 72, 63);           // definition_text
constexpr COLORREF kDisabledText = RGB(168, 160, 148);          // disabled_text
constexpr COLORREF kLinkText = RGB(151, 102, 31);               // link_text

struct PreviewCandidate {
  std::wstring label;
  std::wstring text;
  std::wstring rawComment;
};

struct PreviewScenario {
  std::wstring id;
  std::wstring title;
  std::wstring inputBuffer;
  std::vector<PreviewCandidate> candidates;
  DisplayPreferences preferences;
  int selectedIndex = 0;
  bool showDictionary = true;
  double dpiScale = 1.0;
  POINT anchor = {24, 24};
  SIZE workArea = {1100, 680};
};

std::wstring row(
    const wchar_t* matchInput,
    const wchar_t* honzi,
    const wchar_t* jyutping,
    const wchar_t* canonicalHonzi,
    const wchar_t* canonicalJyutping,
    const wchar_t* pronLabel,
    const wchar_t* litColReading,
    const wchar_t* pos,
    const wchar_t* registerValue,
    const wchar_t* label,
    const wchar_t* written,
    const wchar_t* vernacular,
    const wchar_t* collocation,
    const wchar_t* eng,
    const wchar_t* hin,
    const wchar_t* urd,
    const wchar_t* nep,
    const wchar_t* ind) {
  std::wstring value;
  const std::vector<std::wstring> fields = {
      matchInput, honzi, jyutping, canonicalHonzi, canonicalJyutping,
      L"", L"", pronLabel, litColReading, pos, registerValue, label,
      written, vernacular, collocation, eng, hin, urd, nep, ind};
  for (size_t i = 0; i < fields.size(); ++i) {
    if (i > 0) {
      value += L",";
    }
    value += fields[i];
  }
  return value;
}

std::wstring dictionaryComment(std::wstring note, std::vector<std::wstring> rows) {
  std::wstring value = std::move(note);
  value.push_back(L'\f');
  value.push_back(L'\r');
  for (size_t i = 0; i < rows.size(); ++i) {
    if (i > 0) {
      value.push_back(L'\r');
    }
    value += rows[i];
  }
  return value;
}

DisplayPreferences MultilingualIndonesianPreferences() {
  DisplayPreferences prefs;
  prefs.displayLanguages = {
      DisplayLanguage::English,
      DisplayLanguage::Hindi,
      DisplayLanguage::Indonesian,
      DisplayLanguage::Nepali,
      DisplayLanguage::Urdu};
  prefs.mainLanguage = DisplayLanguage::Indonesian;
  prefs.jyutpingVisibility = JyutpingVisibility::Always;
  prefs.chineseTypeface = ChineseTypeface::Hei;
  prefs.showReverseCode = true;
  return prefs;
}

PreviewScenario MakeNeiSample() {
  DisplayPreferences prefs;
  prefs.displayLanguages = {DisplayLanguage::English};
  prefs.mainLanguage = DisplayLanguage::English;
  return {
      L"nei",
      L"nei - English default",
      L"nei",
      {
          {L"1.", L"你", dictionaryComment(L"", {row(L"1", L"你", L"nei5", L"", L"", L"", L"", L"oth", L"", L"", L"", L"", L"", L"you (singular)", L"आप", L"تم", L"तपाईं", L"kamu")})},
          {L"2.", L"尼", dictionaryComment(L"", {row(L"1", L"尼", L"nei4", L"", L"", L"", L"", L"n", L"", L"", L"", L"", L"", L"nun", L"", L"", L"", L"kamu")})},
          {L"3.", L"呢", dictionaryComment(L"→ni1", {row(L"1", L"呢", L"nei1", L"", L"ni1", L"", L"", L"part", L"yue", L"", L"這", L"", L"", L"this", L"(particle)", L"(particle)", L"", L"(imbuhan kata)")})},
          {L"4.", L"妮", dictionaryComment(L"", {row(L"1", L"妮", L"nei4", L"", L"", L"", L"", L"n", L"", L"", L"", L"", L"", L"little girl", L"", L"", L"", L"")})},
          {L"5.", L"彌", dictionaryComment(L"", {row(L"1", L"彌", L"nei4", L"", L"", L"", L"", L"v", L"", L"", L"", L"", L"", L"full; fill", L"", L"", L"", L"penuh")})},
          {L"6.", L"妳", dictionaryComment(L"", {row(L"1", L"妳", L"nei5", L"", L"", L"", L"", L"oth", L"", L"", L"", L"", L"", L"you", L"आप", L"تم", L"तपाईं", L"Kamu")})},
      },
      prefs,
      0,
      true,
      1.0,
      {24, 24},
      {1100, 680}};
}

PreviewScenario MakeMultilingualIndonesianSample() {
  PreviewScenario scenario = MakeNeiSample();
  scenario.id = L"multilingual";
  scenario.title = L"nei - all display languages, Indonesian main";
  scenario.preferences = MultilingualIndonesianPreferences();
  scenario.selectedIndex = 2;
  return scenario;
}

PreviewScenario MakeHousamSample() {
  return {
      L"housam",
      L"housam - compound candidate",
      L"housam",
      {
          {L"1.", L"好心", dictionaryComment(L"", {row(L"1", L"好心", L"hou2 sam1", L"", L"", L"", L"", L"adj|adv", L"yue", L"", L"拜托", L"", L"", L"kind; come on", L"दयालु", L"نرم دل", L"दयालु", L"baik hati")})},
          {L"2.", L"好心你", dictionaryComment(L"", {
              row(L"1", L"好心", L"hou2 sam1", L"", L"", L"", L"", L"adj|adv", L"yue", L"", L"拜托", L"", L"", L"kind; come on", L"दयालु", L"نرم دل", L"दयालु", L"baik hati"),
              row(L"1", L"你", L"nei5", L"", L"", L"", L"", L"oth", L"", L"", L"", L"", L"", L"you (singular)", L"आप", L"تم", L"तपाईं", L"kamu")})},
      },
      MultilingualIndonesianPreferences(),
      1,
      true,
      1.0,
      {24, 24},
      {1100, 680}};
}

PreviewScenario MakeReverseLookupSample() {
  PreviewScenario scenario = MakeMultilingualIndonesianSample();
  scenario.id = L"reverse";
  scenario.title = L"reverse lookup - Cangjie onf";
  scenario.inputBuffer = L"onf〔倉頡五代〕（人弓火）";
  for (auto& candidate : scenario.candidates) {
    candidate.rawComment = L"\v" + candidate.rawComment;
  }
  scenario.selectedIndex = 0;
  return scenario;
}

PreviewScenario MakePlacementSample(const std::wstring& id) {
  PreviewScenario scenario = MakeHousamSample();
  scenario.id = id;
  if (id == L"edge") {
    scenario.title = L"edge clamp - bottom right work area";
    scenario.anchor = {980, 620};
  } else if (id == L"dpi") {
    scenario.title = L"high DPI scale - 150 percent";
    scenario.dpiScale = 1.5;
  } else {
    scenario.title = L"fallback anchor - no composition rectangle";
    scenario.anchor = {80, 80};
  }
  return scenario;
}

int scale(const PreviewScenario& scenario, int value) {
  return static_cast<int>(value * scenario.dpiScale + 0.5);
}

std::wstring lower(std::wstring value) {
  std::transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) {
    return static_cast<wchar_t>(std::towlower(ch));
  });
  return value;
}

PreviewScenario scenarioById(const std::wstring& id) {
  const std::wstring normalized = lower(id);
  if (normalized == L"multilingual") {
    return MakeMultilingualIndonesianSample();
  }
  if (normalized == L"housam") {
    return MakeHousamSample();
  }
  if (normalized == L"reverse") {
    return MakeReverseLookupSample();
  }
  if (normalized == L"edge" || normalized == L"dpi" || normalized == L"fallback") {
    return MakePlacementSample(normalized);
  }
  return MakeNeiSample();
}

std::vector<CandidateInfo> buildCandidateInfos(const PreviewScenario& scenario) {
  std::vector<CandidateInfo> infos;
  for (const auto& candidate : scenario.candidates) {
    infos.emplace_back(candidate.label, candidate.text, candidate.rawComment);
  }
  return infos;
}

void fillRect(HDC hdc, const RECT& rect, COLORREF color) {
  HBRUSH brush = CreateSolidBrush(color);
  FillRect(hdc, &rect, brush);
  DeleteObject(brush);
}

void frameRoundRect(HDC hdc, const RECT& rect, int radius, COLORREF fill, COLORREF border) {
  HRGN region = CreateRoundRectRgn(rect.left, rect.top, rect.right + 1, rect.bottom + 1,
                                   radius * 2, radius * 2);
  HBRUSH fillBrush = CreateSolidBrush(fill);
  HBRUSH borderBrush = CreateSolidBrush(border);
  FillRgn(hdc, region, fillBrush);
  FrameRgn(hdc, region, borderBrush, 1, 1);
  DeleteObject(fillBrush);
  DeleteObject(borderBrush);
  DeleteObject(region);
}

void drawText(HDC hdc, const std::wstring& text, RECT rect, COLORREF color, UINT format) {
  SetTextColor(hdc, color);
  DrawTextW(hdc, text.c_str(), static_cast<int>(text.size()), &rect,
            format | DT_NOPREFIX);
}

bool writeBitmapFile(const std::wstring& path, HBITMAP bitmap, int width, int height) {
  BITMAPINFOHEADER bih = {};
  bih.biSize = sizeof(BITMAPINFOHEADER);
  bih.biWidth = width;
  bih.biHeight = -height;
  bih.biPlanes = 1;
  bih.biBitCount = 32;
  bih.biCompression = BI_RGB;

  const DWORD pixelBytes = static_cast<DWORD>(width * height * 4);
  std::vector<BYTE> pixels(pixelBytes);
  HDC hdc = GetDC(nullptr);
  const int ok = GetDIBits(hdc, bitmap, 0, static_cast<UINT>(height), pixels.data(),
                           reinterpret_cast<BITMAPINFO*>(&bih), DIB_RGB_COLORS);
  ReleaseDC(nullptr, hdc);
  if (!ok) {
    return false;
  }

  BITMAPFILEHEADER bfh = {};
  bfh.bfType = 0x4D42;
  bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  bfh.bfSize = bfh.bfOffBits + pixelBytes;

  std::ofstream stream(path, std::ios::binary);
  if (!stream.is_open()) {
    return false;
  }
  stream.write(reinterpret_cast<const char*>(&bfh), sizeof(bfh));
  stream.write(reinterpret_cast<const char*>(&bih), sizeof(bih));
  stream.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
  return stream.good();
}

class PreviewWindow {
public:
  explicit PreviewWindow(PreviewScenario scenario) : scenario_(std::move(scenario)) {}

  bool create(HINSTANCE instance, bool visible) {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &PreviewWindow::staticWndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = kWindowClassName;
    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
      return false;
    }

    hwnd_ = CreateWindowExW(WS_EX_NOACTIVATE, kWindowClassName, kWindowTitle,
                            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                            CW_USEDEFAULT, CW_USEDEFAULT, 980, 620, nullptr, nullptr,
                            instance, this);
    if (!hwnd_) {
      return false;
    }

    recreateFonts();
    recalculateLayout();
    resizeWindowToContent();
    if (visible) {
      ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
      UpdateWindow(hwnd_);
    }
    return true;
  }

  bool capture(const std::wstring& path) {
    recalculateLayout();
    HDC screen = GetDC(hwnd_);
    HDC memdc = CreateCompatibleDC(screen);
    HBITMAP bitmap = CreateCompatibleBitmap(screen, contentWidth_, contentHeight_);
    HGDIOBJ oldBitmap = SelectObject(memdc, bitmap);
    paintContent(memdc);
    SelectObject(memdc, oldBitmap);
    DeleteDC(memdc);
    ReleaseDC(hwnd_, screen);
    const bool ok = writeBitmapFile(path, bitmap, contentWidth_, contentHeight_);
    DeleteObject(bitmap);
    return ok;
  }

private:
  static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    PreviewWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
      auto* create = reinterpret_cast<CREATESTRUCTW*>(lp);
      self = static_cast<PreviewWindow*>(create->lpCreateParams);
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
      self->hwnd_ = hwnd;
    } else {
      self = reinterpret_cast<PreviewWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    return self ? self->wndProc(msg, wp, lp) : DefWindowProcW(hwnd, msg, wp, lp);
  }

  LRESULT wndProc(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_DPICHANGED:
      recreateFonts();
      recalculateLayout();
      InvalidateRect(hwnd_, nullptr, TRUE);
      return 0;
    case WM_KEYDOWN:
      if (handleKeyDown(static_cast<UINT>(wp))) {
        return 0;
      }
      break;
    case WM_PAINT:
      onPaint();
      return 0;
    case WM_DESTROY:
      destroyFonts();
      PostQuitMessage(0);
      return 0;
    default:
      break;
    }
    return DefWindowProcW(hwnd_, msg, wp, lp);
  }

  bool handleKeyDown(UINT key) {
    switch (key) {
    case '1':
      scenario_ = MakeNeiSample();
      break;
    case '2':
      scenario_ = MakeMultilingualIndonesianSample();
      break;
    case '3':
      scenario_ = MakeHousamSample();
      break;
    case '4':
      scenario_ = MakeReverseLookupSample();
      break;
    case 'E':
      scenario_ = MakePlacementSample(L"edge");
      break;
    case 'D':
      scenario_ = MakePlacementSample(L"dpi");
      break;
    case 'F':
      scenario_ = MakePlacementSample(L"fallback");
      break;
    case 'H':
      scenario_.showDictionary = !scenario_.showDictionary;
      break;
    case VK_UP:
      scenario_.selectedIndex = (std::max)(0, scenario_.selectedIndex - 1);
      break;
    case VK_DOWN:
      scenario_.selectedIndex = (std::min)(
          static_cast<int>(scenario_.candidates.size()) - 1, scenario_.selectedIndex + 1);
      break;
    default:
      return false;
    }
    recalculateLayout();
    resizeWindowToContent();
    InvalidateRect(hwnd_, nullptr, TRUE);
    return true;
  }

  void recreateFonts() {
    destroyFonts();
    const int bodyHeight = -scale(scenario_, 16);
    bodyFont_ = CreateFontW(bodyHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                            L"Microsoft JhengHei UI");
    const int sungHeight = -scale(scenario_, 18);
    sungFont_ = CreateFontW(sungHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                            scenario_.preferences.chineseTypeface == ChineseTypeface::Hei
                                ? L"Microsoft JhengHei"
                                : L"MingLiU_HKSCS");
    const int smallHeight = -scale(scenario_, 13);
    smallFont_ = CreateFontW(smallHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    const int headHeight = -scale(scenario_, 36);
    headFont_ = CreateFontW(headHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                            scenario_.preferences.chineseTypeface == ChineseTypeface::Hei
                                ? L"Microsoft JhengHei"
                                : L"MingLiU_HKSCS");
  }

  void destroyFonts() {
    if (bodyFont_) DeleteObject(bodyFont_);
    if (sungFont_) DeleteObject(sungFont_);
    if (smallFont_) DeleteObject(smallFont_);
    if (headFont_) DeleteObject(headFont_);
    bodyFont_ = sungFont_ = smallFont_ = headFont_ = nullptr;
  }

  void recalculateLayout() {
    infos_ = buildCandidateInfos(scenario_);
    outerPadding_ = scale(scenario_, 20);
    panelGap_ = scale(scenario_, 12);
    rowHeight_ = scale(scenario_, 46);
    candidatePanelWidth_ = scale(scenario_, 560);
    dictionaryPanelWidth_ = scenario_.showDictionary ? scale(scenario_, 430) : 0;
    candidatePanelHeight_ = scale(scenario_, 62) +
        static_cast<int>(infos_.size()) * rowHeight_ + scale(scenario_, 12);
    dictionaryPanelHeight_ = scenario_.showDictionary
        ? (std::max)(candidatePanelHeight_, scale(scenario_, 410))
        : 0;
    contentWidth_ = outerPadding_ * 2 + candidatePanelWidth_ +
        (scenario_.showDictionary ? panelGap_ + dictionaryPanelWidth_ : 0);
    contentHeight_ = outerPadding_ * 2 +
        (std::max)(candidatePanelHeight_, dictionaryPanelHeight_) + scale(scenario_, 38);
  }

  void resizeWindowToContent() {
    RECT outer = {0, 0, contentWidth_, contentHeight_};
    const DWORD style = static_cast<DWORD>(GetWindowLongPtrW(hwnd_, GWL_STYLE));
    const DWORD exStyle = static_cast<DWORD>(GetWindowLongPtrW(hwnd_, GWL_EXSTYLE));
    AdjustWindowRectEx(&outer, style, FALSE, exStyle);
    SetWindowPos(hwnd_, nullptr, 0, 0, outer.right - outer.left, outer.bottom - outer.top,
                 SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
  }

  void paintContent(HDC hdc) {
    RECT client = {0, 0, contentWidth_, contentHeight_};
    fillRect(hdc, client, kAppBackground);
    SetBkMode(hdc, TRANSPARENT);

    RECT titleRc = {outerPadding_, scale(scenario_, 8), contentWidth_ - outerPadding_,
                    outerPadding_};
    SelectObject(hdc, smallFont_);
    drawText(hdc, scenario_.title, titleRc, kTextSecondary,
             DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    RECT candidatePanel = {outerPadding_, outerPadding_,
                           outerPadding_ + candidatePanelWidth_,
                           outerPadding_ + candidatePanelHeight_};
    frameRoundRect(hdc, candidatePanel, scale(scenario_, 8),
                   kPanelBackground, kPanelBorder);
    paintCandidatePanel(hdc, candidatePanel);

    if (scenario_.showDictionary && scenario_.selectedIndex >= 0 &&
        scenario_.selectedIndex < static_cast<int>(infos_.size())) {
      RECT dictionaryPanel = {candidatePanel.right + panelGap_, candidatePanel.top,
                              candidatePanel.right + panelGap_ + dictionaryPanelWidth_,
                              candidatePanel.top + dictionaryPanelHeight_};
      frameRoundRect(hdc, dictionaryPanel, scale(scenario_, 6),
                     kDictionaryBackground, kPanelBorder);
      paintDictionaryPanel(hdc, dictionaryPanel, infos_[scenario_.selectedIndex]);
    }
  }

  void paintCandidatePanel(HDC hdc, const RECT& panel) {
    const int pad = scale(scenario_, 10);
    RECT inputRc = {panel.left + pad, panel.top + pad,
                    panel.right - scale(scenario_, 82), panel.top + scale(scenario_, 48)};
    RECT activeRc = inputRc;
    activeRc.right = activeRc.left + scale(scenario_, 18) +
        static_cast<int>(scenario_.inputBuffer.size()) * scale(scenario_, 10);
    frameRoundRect(hdc, activeRc, scale(scenario_, 5),
                   kInputBufferBackground, kInputBufferBackground);
    SelectObject(hdc, bodyFont_);
    RECT inputText = activeRc;
    inputText.left += scale(scenario_, 8);
    drawText(hdc, scenario_.inputBuffer, inputText, kTextPrimary,
             DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    RECT prevRc = {panel.right - scale(scenario_, 72), panel.top + pad,
                   panel.right - scale(scenario_, 40), panel.top + scale(scenario_, 48)};
    RECT nextRc = {panel.right - scale(scenario_, 38), panel.top + pad,
                   panel.right - pad, panel.top + scale(scenario_, 48)};
    drawText(hdc, L"‹", prevRc, kDisabledText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    drawText(hdc, L"›", nextRc, kLinkText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    int y = panel.top + scale(scenario_, 58);
    for (int i = 0; i < static_cast<int>(infos_.size()); ++i) {
      RECT rowRc = {panel.left + scale(scenario_, 6), y,
                    panel.right - scale(scenario_, 6), y + rowHeight_};
      paintCandidateRow(hdc, rowRc, infos_[i], i == scenario_.selectedIndex);
      y += rowHeight_;
    }
  }

  void paintCandidateRow(HDC hdc, const RECT& rowRc, const CandidateInfo& info, bool selected) {
    if (selected) {
      frameRoundRect(hdc, rowRc, scale(scenario_, 5),
                     kSelectionBackground, kSelectionBackground);
    }

    const auto matched = info.matchedEntries();
    const auto& entry = matched.empty() ? info.entries.front() : matched.front();
    const int x0 = rowRc.left + scale(scenario_, 8);
    RECT labelRc = {x0, rowRc.top, x0 + scale(scenario_, 32), rowRc.bottom};
    SelectObject(hdc, smallFont_);
    drawText(hdc, info.label, labelRc, kTextSecondary,
             DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    const bool showJyutping = scenario_.preferences.shouldShowJyutping(info.isReverseLookup);
    RECT jyutpingRc = {labelRc.right + scale(scenario_, 4), rowRc.top + scale(scenario_, 3),
                       labelRc.right + scale(scenario_, 112), rowRc.top + scale(scenario_, 21)};
    if (showJyutping) {
      drawText(hdc, entry.jyutping, jyutpingRc, kPronunciationText,
               DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }

    SelectObject(hdc, sungFont_);
    RECT honziRc = {labelRc.right + scale(scenario_, 4),
                    rowRc.top + (showJyutping ? scale(scenario_, 19) : scale(scenario_, 5)),
                    labelRc.right + scale(scenario_, 116), rowRc.bottom};
    drawText(hdc, entry.honzi.empty() ? info.text : entry.honzi, honziRc, kTextPrimary,
             DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, smallFont_);
    RECT noteRc = {honziRc.right + scale(scenario_, 6), rowRc.top,
                   honziRc.right + scale(scenario_, 92), rowRc.bottom};
    drawText(hdc, (!info.isReverseLookup || scenario_.preferences.showReverseCode) ? info.note : L"",
             noteRc, kTextSecondary, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    RECT definitionRc = {noteRc.right + scale(scenario_, 6), rowRc.top,
                         rowRc.right - scale(scenario_, 34), rowRc.bottom};
    std::wstring definition = entry.definition(scenario_.preferences.mainLanguage);
    if (definition.empty()) {
      definition = info.displayComment(scenario_.preferences);
    }
    drawText(hdc, definition, definitionRc, kDefinitionText,
             DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    RECT infoRc = {rowRc.right - scale(scenario_, 30), rowRc.top,
                   rowRc.right - scale(scenario_, 8), rowRc.bottom};
    drawText(hdc, info.hasDictionaryEntry(scenario_.preferences) ? L"ⓘ" : L"",
             infoRc, kTextSecondary, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
  }

  void paintDictionaryPanel(HDC hdc, const RECT& panel, const CandidateInfo& info) {
    int y = panel.top + scale(scenario_, 12);
    const int padX = scale(scenario_, 18);
    for (const auto& entry : info.entries) {
      if (!entry.isDictionaryEntry(scenario_.preferences)) {
        continue;
      }

      RECT headRc = {panel.left + padX, y, panel.right - padX, y + scale(scenario_, 52)};
      SelectObject(hdc, headFont_);
      drawText(hdc, entry.honzi, headRc, kTextPrimary, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
      SelectObject(hdc, bodyFont_);
      RECT jpRc = {headRc.left + scale(scenario_, 118), y + scale(scenario_, 12),
                   headRc.right, headRc.bottom};
      drawText(hdc, entry.jyutping + L" " + entry.pronunciationType(), jpRc,
               kPronunciationText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
      y += scale(scenario_, 58);

      RECT bodyRc = {panel.left + padX, y, panel.right - padX, y + scale(scenario_, 28)};
      std::wstring body;
      for (const auto& pos : entry.formattedPartsOfSpeech()) {
        if (!body.empty()) body += L"  ";
        body += L"[" + pos + L"]";
      }
      for (const auto& reg : entry.formattedRegister()) {
        if (!body.empty()) body += L"  ";
        body += reg;
      }
      const std::wstring canonical = entry.canonicalReference();
      if (!canonical.empty()) {
        if (!body.empty()) body += L"  ";
        body += L"→" + canonical;
      } else {
        const std::wstring mainDefinition = entry.definition(scenario_.preferences.mainLanguage);
        if (!mainDefinition.empty()) {
          if (!body.empty()) body += L"  ";
          body += mainDefinition;
        }
      }
      SelectObject(hdc, smallFont_);
      drawText(hdc, body, bodyRc, kDefinitionText,
               DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
      y += scale(scenario_, 34);

      for (const auto& other : entry.otherData()) {
        RECT labelRc = {panel.left + padX, y, panel.left + padX + scale(scenario_, 132),
                        y + scale(scenario_, 24)};
        RECT valueRc = {labelRc.right + scale(scenario_, 10), y,
                        panel.right - padX, labelRc.bottom};
        drawText(hdc, other.name, labelRc, kTextSecondary,
                 DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, sungFont_);
        drawText(hdc, other.value, valueRc, kTextPrimary,
                 DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        SelectObject(hdc, smallFont_);
        y += scale(scenario_, 26);
      }

      const auto languages = entry.otherLanguages(scenario_.preferences);
      if (!languages.empty()) {
        RECT captionRc = {panel.left + padX, y, panel.right - padX, y + scale(scenario_, 26)};
        drawText(hdc, L"More Languages", captionRc, kTextPrimary,
                 DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        y += scale(scenario_, 28);
        for (const auto& language : languages) {
          RECT labelRc = {panel.left + padX, y, panel.left + padX + scale(scenario_, 132),
                          y + scale(scenario_, 24)};
          RECT valueRc = {labelRc.right + scale(scenario_, 10), y,
                          panel.right - padX, labelRc.bottom};
          drawText(hdc, language.name, labelRc, kTextSecondary,
                   DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
          drawText(hdc, language.value, valueRc, kDefinitionText,
                   DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
          y += scale(scenario_, 26);
        }
      }

      y += scale(scenario_, 12);
    }
  }

  void onPaint() {
    PAINTSTRUCT ps = {};
    HDC hdc = BeginPaint(hwnd_, &ps);
    HDC memdc = CreateCompatibleDC(hdc);
    HBITMAP bitmap = CreateCompatibleBitmap(hdc, contentWidth_, contentHeight_);
    HGDIOBJ oldBitmap = SelectObject(memdc, bitmap);
    paintContent(memdc);
    BitBlt(hdc, 0, 0, contentWidth_, contentHeight_, memdc, 0, 0, SRCCOPY);
    SelectObject(memdc, oldBitmap);
    DeleteObject(bitmap);
    DeleteDC(memdc);
    EndPaint(hwnd_, &ps);
  }

private:
  HWND hwnd_ = nullptr;
  PreviewScenario scenario_;
  std::vector<CandidateInfo> infos_;
  HFONT bodyFont_ = nullptr;
  HFONT sungFont_ = nullptr;
  HFONT smallFont_ = nullptr;
  HFONT headFont_ = nullptr;
  int outerPadding_ = 20;
  int panelGap_ = 12;
  int rowHeight_ = 46;
  int candidatePanelWidth_ = 560;
  int candidatePanelHeight_ = 360;
  int dictionaryPanelWidth_ = 430;
  int dictionaryPanelHeight_ = 410;
  int contentWidth_ = 980;
  int contentHeight_ = 620;
};

std::wstring optionValue(const std::wstring& commandLine, const std::wstring& option) {
  const size_t pos = commandLine.find(option);
  if (pos == std::wstring::npos) {
    return L"";
  }
  size_t start = pos + option.size();
  while (start < commandLine.size() && std::iswspace(commandLine[start])) {
    ++start;
  }
  if (start >= commandLine.size()) {
    return L"";
  }
  if (commandLine[start] == L'"') {
    const size_t end = commandLine.find(L'"', start + 1);
    return end == std::wstring::npos
        ? commandLine.substr(start + 1)
        : commandLine.substr(start + 1, end - start - 1);
  }
  size_t end = start;
  while (end < commandLine.size() && !std::iswspace(commandLine[end])) {
    ++end;
  }
  return commandLine.substr(start, end - start);
}

void SavePreviewCaptureCommand(const std::wstring&) {
  // Marker used by scripts/Test-TypeDuckCandidateWindow.ps1 to keep documented
  // capture support tied to this executable.
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR commandLine, int) {
  const std::wstring args = commandLine ? commandLine : L"";
  const std::wstring sample = optionValue(args, L"--sample");
  const std::wstring capturePath = optionValue(args, L"--capture");
  PreviewWindow window(scenarioById(sample.empty() ? L"nei" : sample));
  if (!window.create(instance, capturePath.empty())) {
    return 1;
  }

  if (!capturePath.empty()) {
    SavePreviewCaptureCommand(capturePath);
    return window.capture(capturePath) ? 0 : 2;
  }

  MSG msg = {};
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  return static_cast<int>(msg.wParam);
}
