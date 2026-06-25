#include "TypeDuckAboutDialog.h"

#include "TypeDuckSettingsVersion.h"
#include "resource.h"

#include <shellapi.h>
#include <string>
#include <vector>

namespace Moqi::TypeDuckSettings {

namespace {

constexpr const wchar_t* kAboutClassName = L"TypeDuckAboutDialogWindow";
constexpr int kAboutWidth = 760;
constexpr int kAboutHeight = 700;

enum AboutControlId : int {
  kClose = 3000,
  kLinkBase = 3100,
};

struct AboutLink {
  const wchar_t* label;
  const wchar_t* url;
};

constexpr const wchar_t* kAboutBodyText =
    L"歡迎使用 TypeDuck 打得 —— 設有少數族裔語言提示粵拼輸入法！有字想打？一裝即用，毋須再等，即刻打得！\r\n"
    L"Welcome to TypeDuck: a Cantonese input keyboard with minority language prompts! Got something you want to type? Have your fingers ready, get, set, TYPE DUCK!\r\n"
    L"\r\n"
    L"如有任何查詢，歡迎電郵至 info@typeduck.hk 或 lchaakming@eduhk.hk。\r\n"
    L"Should you have any enquiries, please email info@typeduck.hk or lchaakming@eduhk.hk.\r\n"
    L"\r\n"
    L"本輸入法由香港教育大學語言學及現代語言系開發。特別鳴謝「語文教育及研究常務委員會」資助本計劃。\r\n"
    L"This input method is developed by the Department of Linguistics and Modern Language Studies, the Education University of Hong Kong. Special thanks to the Standing Committee on Language Education and Research for funding this project.";

std::wstring attributionText() {
  std::wstring text;
  text += L"TypeDuck Windows IME version / 版本 ";
  text += TYPEDUCK_VERSION_TEXT;
  text += L"\r\n";
  text += L"引擎 Engine: TypeDuck-HK librime fork v1.1.4 pinned Windows x64 runtime\r\n";
  text += L"字典查詢 Dictionary lookup: rime-dictionary-lookup-filter 3671814d4e4aeab8d616ceea3c7f6d88e96bba02\r\n";
  text += L"方案 Schema: TypeDuck-HK schema aap2-alpha b5ab44e187d010fa13c71bd320c07259b224ae5b with built schema artifacts";
  return text;
}

const std::vector<AboutLink>& aboutLinks() {
  static const std::vector<AboutLink> links{
      {L"TypeDuck 網站 Website", L"https://typeduck.hk"},
      {L"LearnDuck 粵拼打字入門 Introduction to Cantonese Jyutping Typing",
       L"https://learn.typeduck.hk"},
      {L"粵拼方案 Jyutping Scheme", L"https://lshk.org/jyutping-scheme/"},
      {L"TypeDuck 原始碼 Source Code",
       L"https://github.com/TypeDuck-HK/TypeDuck-Windows"},
  };
  return links;
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
      self = reinterpret_cast<AboutDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (!self) {
      return DefWindowProcW(hwnd, message, wparam, lparam);
    }
    return self->handleMessage(message, wparam, lparam);
  }

  LRESULT handleMessage(UINT message, WPARAM wparam, LPARAM) {
    switch (message) {
      case WM_CREATE:
        createControls();
        return 0;
      case WM_COMMAND:
        handleCommand(LOWORD(wparam), HIWORD(wparam));
        return 0;
      case WM_CLOSE:
        DestroyWindow(window_);
        return 0;
      case WM_DESTROY:
        window_ = nullptr;
        return 0;
    }
    return DefWindowProcW(window_, message, wparam, 0);
  }

  HWND addStatic(const wchar_t* text, int x, int y, int width, int height,
                 DWORD style = 0) {
    HWND control = CreateWindowExW(0, L"STATIC", text,
                                   WS_CHILD | WS_VISIBLE | style,
                                   x, y, width, height, window_, nullptr,
                                   instance_, nullptr);
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    return control;
  }

  HWND addEdit(const wchar_t* text, int x, int y, int width, int height) {
    HWND control = CreateWindowExW(0, L"EDIT", text,
                                   WS_CHILD | WS_VISIBLE | ES_MULTILINE |
                                       ES_READONLY | ES_AUTOVSCROLL,
                                   x, y, width, height, window_, nullptr,
                                   instance_, nullptr);
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    return control;
  }

  HWND addButton(const wchar_t* text, int id, int x, int y, int width,
                 int height) {
    HWND control = CreateWindowExW(0, L"BUTTON", text,
                                   WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                                       BS_PUSHBUTTON,
                                   x, y, width, height, window_,
                                   reinterpret_cast<HMENU>(
                                       static_cast<INT_PTR>(id)),
                                   instance_, nullptr);
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font_), TRUE);
    return control;
  }

  HWND addBitmap(int resourceId, int x, int y, int width, int height) {
    HBITMAP bitmap = reinterpret_cast<HBITMAP>(
        LoadImageW(instance_, MAKEINTRESOURCEW(resourceId), IMAGE_BITMAP, 0, 0,
                   LR_DEFAULTCOLOR));
    HWND control = CreateWindowExW(0, L"STATIC", L"",
                                   WS_CHILD | WS_VISIBLE | SS_BITMAP,
                                   x, y, width, height, window_, nullptr,
                                   instance_, nullptr);
    SendMessageW(control, STM_SETIMAGE, IMAGE_BITMAP,
                 reinterpret_cast<LPARAM>(bitmap));
    return control;
  }

  void createControls() {
    font_ = CreateFontW(-17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
                        L"Microsoft JhengHei UI");

    addBitmap(IDB_TYPEDUCK_ABOUT_BANNER, 18, 16, 706, 96);
    addEdit(kAboutBodyText, 18, 128, 706, 174);
    addBitmap(IDB_TYPEDUCK_CREDIT_LOGOS, 18, 318, 706, 92);
    const auto attribution = attributionText();
    addEdit(attribution.c_str(), 18, 426, 706, 84);

    int y = 524;
    int index = 0;
    for (const auto& link : aboutLinks()) {
      addButton(link.label, kLinkBase + index, 18, y, 706, 28);
      y += 34;
      ++index;
    }
    addButton(L"關閉 Close", kClose, 632, 638, 92, 30);
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
      ShellExecuteW(window_, L"open", aboutLinks()[linkIndex].url, nullptr, nullptr,
                    SW_SHOWNORMAL);
    }
  }

  HINSTANCE instance_;
  HWND owner_;
  HWND window_ = nullptr;
  HFONT font_ = nullptr;
};

} // namespace

void ShowTypeDuckAboutDialog(HINSTANCE instance, HWND owner) {
  AboutDialog dialog(instance, owner);
  dialog.show();
}

} // namespace Moqi::TypeDuckSettings
