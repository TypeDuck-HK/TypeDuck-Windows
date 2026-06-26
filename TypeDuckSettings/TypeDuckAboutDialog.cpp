#include "TypeDuckAboutDialog.h"

#include "TypeDuckSettingsVersion.h"
#include "resource.h"

#include <shellapi.h>
#include <string>
#include <vector>

namespace Moqi::TypeDuckSettings {

namespace {

constexpr const wchar_t* kAboutClassName = L"TypeDuckAboutDialogWindow";
constexpr int kAboutWidth = 1000;
constexpr int kAboutHeight = 858;
constexpr int kMargin = 28;
constexpr int kContentWidth = 928;
constexpr int kBannerWidth = 490;
constexpr int kBannerHeight = 90;
constexpr int kCreditLogoWidth = 530;
constexpr int kCreditLogoHeight = 258;
constexpr int kLinkButtonGap = 6;
constexpr int kLinkButtonWidth = (kContentWidth - kLinkButtonGap) / 2;
constexpr int kLinkButtonHeight = 38;
constexpr int kCloseButtonWidth = 144;
constexpr int kCloseButtonHeight = 36;
constexpr COLORREF kBackgroundColor = RGB(255, 255, 255);
constexpr COLORREF kTextColor = RGB(29, 35, 42);
constexpr COLORREF kMutedTextColor = RGB(82, 90, 100);
constexpr COLORREF kDividerColor = RGB(225, 229, 234);

enum AboutControlId : int {
  kClose = 3000,
  kLinkBase = 3100,
};

struct AboutLink {
  const wchar_t* label;
  const wchar_t* url;
};

struct PaintFont {
  HFONT handle = nullptr;
  ~PaintFont() {
    if (handle) {
      DeleteObject(handle);
    }
  }
};

HFONT createUiFont(int pointSize, int weight = FW_NORMAL) {
  HDC screen = GetDC(nullptr);
  const int dpiY = GetDeviceCaps(screen, LOGPIXELSY);
  ReleaseDC(nullptr, screen);
  return CreateFontW(-MulDiv(pointSize, dpiY, 72), 0, 0, 0, weight, FALSE,
                     FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                     CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                     DEFAULT_PITCH | FF_SWISS, L"Microsoft JhengHei UI");
}

constexpr const wchar_t* kIntroText =
    L"歡迎使用 TypeDuck 打得 —— 設有少數族裔語言提示粵拼輸入法！有字想打？一裝即用，毋須再等，即刻打得！\r\n"
    L"Welcome to TypeDuck: a Cantonese input keyboard with minority language prompts! Got something you want to type? Have your fingers ready, get, set, TYPE DUCK!";

constexpr const wchar_t* kContactText =
    L"如有任何查詢，歡迎電郵至 info@typeduck.hk 或 lchaakming@eduhk.hk。\r\n"
    L"Should you have any enquiries, please email info@typeduck.hk or lchaakming@eduhk.hk.";

constexpr const wchar_t* kCreditText =
    L"本輸入法由香港教育大學語言學及現代語言系開發。特別鳴謝「語文教育及研究常務委員會」資助本計劃。\r\n"
    L"This input method is developed by the Department of Linguistics and Modern Language Studies, the Education University of Hong Kong. Special thanks to the Standing Committee on Language Education and Research for funding this project.";

std::wstring attributionText() {
  std::wstring text;
  text += L"TypeDuck Windows IME 版本 Version ";
  text += TYPEDUCK_VERSION_TEXT;
  text += L"\r\n";
  text += L"Engine: TypeDuck-HK librime fork v1.1.4\r\n";
  text += L"Schema: TypeDuck-HK schema v2.0.0";
  return text;
}

const std::vector<AboutLink>& aboutLinks() {
  static const std::vector<AboutLink> links{
      {L"TypeDuck 網站 Website", L"https://typeduck.hk"},
      {L"LearnDuck 粵拼打字入門 Introduction to Jyutping Typing",
       L"https://learn.typeduck.hk"},
      {L"粵拼方案 Jyutping Scheme", L"https://lshk.org/jyutping-scheme/"},
      {L"TypeDuck 原始碼 Source Code",
       L"https://github.com/TypeDuck-HK/TypeDuck-Windows"},
  };
  return links;
}

void drawTextBlock(HDC dc, HFONT font, COLORREF color, const std::wstring& text,
                   RECT rect, UINT format = DT_LEFT | DT_WORDBREAK) {
  const HGDIOBJ oldFont = SelectObject(dc, font);
  SetTextColor(dc, color);
  SetBkMode(dc, TRANSPARENT);
  DrawTextW(dc, text.c_str(), -1, &rect, format);
  SelectObject(dc, oldFont);
}

void drawDivider(HDC dc, int x, int y, int width) {
  HPEN pen = CreatePen(PS_SOLID, 1, kDividerColor);
  const HGDIOBJ oldPen = SelectObject(dc, pen);
  MoveToEx(dc, x, y, nullptr);
  LineTo(dc, x + width, y);
  SelectObject(dc, oldPen);
  DeleteObject(pen);
}

void drawBitmap(HDC dc, HBITMAP bitmap, int x, int y, int width, int height) {
  if (!bitmap) {
    return;
  }
  BITMAP info{};
  GetObjectW(bitmap, sizeof(info), &info);
  HDC memoryDc = CreateCompatibleDC(dc);
  const HGDIOBJ oldBitmap = SelectObject(memoryDc, bitmap);
  SetStretchBltMode(dc, HALFTONE);
  StretchBlt(dc, x, y, width, height, memoryDc, 0, 0, info.bmWidth,
             info.bmHeight, SRCCOPY);
  SelectObject(memoryDc, oldBitmap);
  DeleteDC(memoryDc);
}

class AboutDialog {
 public:
  AboutDialog(HINSTANCE instance, HWND owner) : instance_(instance), owner_(owner) {}

  void show() {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hInstance = instance_;
    wc.lpfnWndProc = &AboutDialog::windowProc;
    wc.lpszClassName = kAboutClassName;
    wc.hIcon = LoadIconW(instance_, MAKEINTRESOURCEW(IDI_TYPEDUCK_SETTINGS));
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.hIconSm = wc.hIcon;
    RegisterClassExW(&wc);

    window_ = CreateWindowExW(
        WS_EX_DLGMODALFRAME, kAboutClassName,
        L"關於 TypeDuck / About TypeDuck",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, kAboutWidth, kAboutHeight,
        owner_, nullptr, instance_, this);
    if (!window_) {
      return;
    }

    if (owner_) {
      EnableWindow(owner_, FALSE);
    }
    ShowWindow(window_, SW_SHOW);
    UpdateWindow(window_);

    MSG msg{};
    while (IsWindow(window_) && GetMessageW(&msg, nullptr, 0, 0) > 0) {
      if (!IsDialogMessageW(window_, &msg)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
      }
    }
    if (owner_) {
      EnableWindow(owner_, TRUE);
      SetForegroundWindow(owner_);
    }
  }

 private:
  static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wparam,
                                     LPARAM lparam) {
    AboutDialog* self = nullptr;
    if (message == WM_NCCREATE) {
      const auto create = reinterpret_cast<CREATESTRUCTW*>(lparam);
      self = reinterpret_cast<AboutDialog*>(create->lpCreateParams);
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
      self->window_ = hwnd;
    } else {
      self = reinterpret_cast<AboutDialog*>(
          GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (!self) {
      return DefWindowProcW(hwnd, message, wparam, lparam);
    }
    return self->handleMessage(message, wparam, lparam);
  }

  LRESULT handleMessage(UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
      case WM_CREATE:
        createResources();
        createControls();
        return 0;
      case WM_PAINT:
        paint();
        return 0;
      case WM_ERASEBKGND:
        return 1;
      case WM_COMMAND:
        handleCommand(LOWORD(wparam), HIWORD(wparam));
        return 0;
      case WM_CLOSE:
        DestroyWindow(window_);
        return 0;
      case WM_DESTROY:
        destroyResources();
        window_ = nullptr;
        return 0;
    }
    return DefWindowProcW(window_, message, wparam, lparam);
  }

  HWND addButton(const wchar_t* text, int id, int x, int y, int width,
                 int height, bool defaultButton = false) {
    const DWORD buttonStyle = defaultButton ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON;
    HWND control = CreateWindowExW(
        0, L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | buttonStyle,
        x, y, width, height, window_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), instance_, nullptr);
    SendMessageW(control, WM_SETFONT,
                 reinterpret_cast<WPARAM>(uiFont_.handle),
                 TRUE);
    return control;
  }

  void createResources() {
    uiFont_.handle = createUiFont(12);
    smallFont_.handle = createUiFont(11);
    linkSmallFont_.handle = createUiFont(11);
    headingFont_.handle = createUiFont(14, FW_SEMIBOLD);
    bannerBitmap_ = reinterpret_cast<HBITMAP>(
        LoadImageW(instance_, MAKEINTRESOURCEW(IDB_TYPEDUCK_ABOUT_BANNER),
                   IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR));
    creditBitmap_ = reinterpret_cast<HBITMAP>(
        LoadImageW(instance_, MAKEINTRESOURCEW(IDB_TYPEDUCK_CREDIT_LOGOS),
                   IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR));
  }

  void createControls() {
    const int leftColumn = kMargin;
    const int rightColumn = kMargin + kLinkButtonWidth + kLinkButtonGap;
    const auto& links = aboutLinks();
    addButton(links[0].label, kLinkBase, leftColumn, 644,
              kLinkButtonWidth, kLinkButtonHeight);
    addButton(links[1].label, kLinkBase + 1, rightColumn, 644,
              kLinkButtonWidth, kLinkButtonHeight);
    addButton(links[2].label, kLinkBase + 2, leftColumn, 686,
              kLinkButtonWidth, kLinkButtonHeight);
    addButton(links[3].label, kLinkBase + 3, rightColumn, 686,
              kLinkButtonWidth, kLinkButtonHeight);
    addButton(L"關閉 Close", kClose,
              kAboutWidth - kMargin - kCloseButtonWidth - 16, 756,
              kCloseButtonWidth, kCloseButtonHeight, true);
  }

  void paint() {
    PAINTSTRUCT ps{};
    HDC dc = BeginPaint(window_, &ps);
    RECT client{};
    GetClientRect(window_, &client);
    HBRUSH background = CreateSolidBrush(kBackgroundColor);
    FillRect(dc, &client, background);
    DeleteObject(background);

    const int bannerX = kMargin;
    drawBitmap(dc, bannerBitmap_, bannerX, 26, kBannerWidth, kBannerHeight);

    RECT introRect{kMargin, 136, kMargin + kContentWidth, 196};
    drawTextBlock(dc, uiFont_.handle, kTextColor, kIntroText, introRect);

    RECT contactRect{kMargin, 209, kMargin + kContentWidth, 263};
    drawTextBlock(dc, uiFont_.handle, kTextColor, kContactText, contactRect);

    drawDivider(dc, kMargin, 268, kContentWidth);

    const int logoX = kMargin + (kContentWidth - kCreditLogoWidth) / 2;
    drawBitmap(dc, creditBitmap_, logoX, 273, kCreditLogoWidth,
               kCreditLogoHeight);

    RECT creditRect{kMargin, 536, kMargin + kContentWidth, 602};
    drawTextBlock(dc, smallFont_.handle, kMutedTextColor, kCreditText,
                  creditRect);

    drawDivider(dc, kMargin, 607, kContentWidth);

    RECT linksHeading{kMargin, 616, kMargin + kContentWidth, 638};
    drawTextBlock(dc, headingFont_.handle, kMutedTextColor, L"連結 Links",
                  linksHeading);

    RECT attribution{kMargin, 734, kAboutWidth - kCloseButtonWidth - 70, 800};
    drawTextBlock(dc, smallFont_.handle, kMutedTextColor, attributionText(),
                  attribution, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);

    EndPaint(window_, &ps);
  }

  void destroyResources() {
    if (bannerBitmap_) {
      DeleteObject(bannerBitmap_);
      bannerBitmap_ = nullptr;
    }
    if (creditBitmap_) {
      DeleteObject(creditBitmap_);
      creditBitmap_ = nullptr;
    }
  }

  void handleCommand(int id, int notification) {
    if (notification != BN_CLICKED) {
      return;
    }
    if (id == kClose) {
      DestroyWindow(window_);
      return;
    }
    const int linkIndex = id - kLinkBase;
    if (linkIndex >= 0 && linkIndex < static_cast<int>(aboutLinks().size())) {
      ShellExecuteW(window_, L"open", aboutLinks()[linkIndex].url, nullptr,
                    nullptr, SW_SHOWNORMAL);
    }
  }

  HINSTANCE instance_;
  HWND owner_;
  HWND window_ = nullptr;
  PaintFont uiFont_;
  PaintFont smallFont_;
  PaintFont linkSmallFont_;
  PaintFont headingFont_;
  HBITMAP bannerBitmap_ = nullptr;
  HBITMAP creditBitmap_ = nullptr;
};

} // namespace

void ShowTypeDuckAboutDialog(HINSTANCE instance, HWND owner) {
  AboutDialog dialog(instance, owner);
  dialog.show();
}

} // namespace Moqi::TypeDuckSettings
