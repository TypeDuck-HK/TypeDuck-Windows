#include "TypeDuckSettingsWindow.h"

#include "MoqLauncher/TypeDuckPreferences.h"
#include "resource.h"

#include <algorithm>
#include <commctrl.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace Moqi::TypeDuckSettings {

namespace {

constexpr const wchar_t* kWindowClassName = L"TypeDuckSettingsWindow";
constexpr int kWindowWidth = 940;
constexpr int kWindowHeight = 620;
constexpr int kMargin = 22;
constexpr int kLeftColumnX = 28;
constexpr int kRightColumnX = 486;
constexpr int kColumnWidth = 412;
constexpr int kDisplayLanguageGroupHeight = 192;
constexpr int kDisplayLanguageMainX = kLeftColumnX + 20;
constexpr int kDisplayLanguageDisplayX = kLeftColumnX + 248;
constexpr int kDisplayLanguageMainWidth = 190;
constexpr int kDisplayLanguageDisplayWidth = 160;
constexpr int kPageSizeTrackWidth = kColumnWidth;
constexpr int kRowHeight = 28;
constexpr int kPageTickWidth = 32;

enum ControlId : int {
  kDisplayLanguageBase = 1100,
  kMainLanguageBase = 1200,
  kPageSizeTrack = 1300,
  kPageSizeValue = 1301,
  kTypefaceSung = 1400,
  kTypefaceHei = 1401,
  kRomanizationAlways = 1500,
  kRomanizationReverseOnly = 1501,
  kRomanizationNever = 1502,
  kEnableCompletion = 1600,
  kEnableCorrection = 1601,
  kEnableSentence = 1602,
  kEnableLearning = 1603,
  kShowReverseCode = 1700,
  kCangjie3 = 1800,
  kCangjie5 = 1801,
  kConfirm = 1900,
  kCancel = 1901,
};

struct LanguageOption {
  const char* id;
  const wchar_t* label;
};

const std::vector<LanguageOption>& languageOptions() {
  static const std::vector<LanguageOption> options{
      {"eng", L"英語 English"},
      {"hin", L"印地語 Hindi"},
      {"ind", L"印尼語 Indonesian"},
      {"nep", L"尼泊爾語 Nepali"},
      {"urd", L"烏爾都語 Urdu"},
  };
  return options;
}

std::wstring utf8ToWide(const std::string& value) {
  if (value.empty()) {
    return L"";
  }
  const int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
  if (size <= 0) {
    return L"";
  }
  std::wstring result(size - 1, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, result.data(), size);
  return result;
}

std::string wideToUtf8(const std::wstring& value) {
  if (value.empty()) {
    return "";
  }
  const int size = WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, nullptr, 0,
                                      nullptr, nullptr);
  if (size <= 0) {
    return "";
  }
  std::string result(size - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.c_str(), -1, result.data(), size,
                      nullptr, nullptr);
  return result;
}

bool containsLanguage(const Moqi::TypeDuck::Preferences& preferences,
                      const char* language) {
  return std::find(preferences.displayLanguages.begin(),
                   preferences.displayLanguages.end(),
                   language) != preferences.displayLanguages.end();
}

std::filesystem::path rimeUserDir() {
  const wchar_t* appData = _wgetenv(L"APPDATA");
  if (appData == nullptr || appData[0] == L'\0') {
    return {};
  }
  return std::filesystem::path(appData) / L"Moqi" / L"Rime";
}

bool writeUtf8File(const std::filesystem::path& path, const std::string& content) {
  std::error_code ec;
  std::filesystem::create_directories(path.parent_path(), ec);
  if (ec) {
    return false;
  }
  std::ofstream stream(path, std::ios::binary | std::ios::trunc);
  if (!stream.is_open()) {
    return false;
  }
  stream << content;
  return stream.good();
}

std::string defaultCustomYaml(const Moqi::TypeDuck::RimeSideEffects& effects) {
  std::ostringstream content;
  content << "config_version: '" << effects.pageSize << "'\n"
          << "patch:\n"
          << "  " << effects.defaultCustomPath << ": " << effects.pageSize << "\n";
  return content.str();
}

std::string commonCustomYaml(const Moqi::TypeDuck::RimeSideEffects& effects) {
  std::ostringstream content;
  content << "patch:\n"
          << "  " << effects.commonPatchKey << ":\n";
  for (const auto& patch : effects.commonPatches) {
    content << "    - " << patch << "\n";
  }
  return content.str();
}

Moqi::TypeDuck::ApplyResult applyRimeSettings(
    const Moqi::TypeDuck::RimeSideEffects& effects) {
  const auto userDir = rimeUserDir();
  if (userDir.empty()) {
    return {false, "設定未能套用：找不到 Rime 使用者目錄 / Settings could not be applied: the Rime user directory was unavailable"};
  }
  if (!writeUtf8File(userDir / effects.defaultCustomFile, defaultCustomYaml(effects)) ||
      !writeUtf8File(userDir / effects.commonCustomFile, commonCustomYaml(effects))) {
    return {false, "設定未能套用：Rime 自訂設定未能寫入 / Settings could not be applied: Rime custom settings could not be written"};
  }
  return {true, "設定已套用 / Settings applied"};
}

class SettingsWindow {
 public:
  SettingsWindow(HINSTANCE instance) : instance_(instance) {}

  int run(int showCommand) {
    INITCOMMONCONTROLSEX init{};
    init.dwSize = sizeof(init);
    init.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&init);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hInstance = instance_;
    wc.lpfnWndProc = &SettingsWindow::windowProc;
    wc.lpszClassName = kWindowClassName;
    wc.hIcon = LoadIconW(instance_, MAKEINTRESOURCEW(IDI_TYPEDUCK_SETTINGS));
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.hIconSm = wc.hIcon;
    RegisterClassExW(&wc);

    window_ = CreateWindowExW(
        0, kWindowClassName,
        L"TypeDuck 設定 / TypeDuck Settings",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, kWindowWidth, kWindowHeight,
        nullptr, nullptr, instance_, this);
    if (!window_) {
      return 1;
    }

    ShowWindow(window_, showCommand);
    UpdateWindow(window_);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
  }

 private:
  static LRESULT CALLBACK windowProc(HWND hwnd, UINT message, WPARAM wparam,
                                     LPARAM lparam) {
    SettingsWindow* self = nullptr;
    if (message == WM_NCCREATE) {
      const auto create = reinterpret_cast<CREATESTRUCTW*>(lparam);
      self = reinterpret_cast<SettingsWindow*>(create->lpCreateParams);
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
      self->window_ = hwnd;
    } else {
      self = reinterpret_cast<SettingsWindow*>(
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
        loadPreferences();
        createControls();
        applyStateToControls();
        return 0;
      case WM_HSCROLL:
        if (reinterpret_cast<HWND>(lparam) == pageSizeTrack_) {
          updatePageSizeLabel();
        }
        return 0;
      case WM_COMMAND:
        handleCommand(LOWORD(wparam), HIWORD(wparam));
        return 0;
      case WM_CLOSE:
        confirmAndClose();
        return 0;
      case WM_CTLCOLORDLG:
      case WM_CTLCOLORSTATIC:
      case WM_CTLCOLORBTN:
        SetBkColor(reinterpret_cast<HDC>(wparam), GetSysColor(COLOR_WINDOW));
        SetTextColor(reinterpret_cast<HDC>(wparam), GetSysColor(COLOR_WINDOWTEXT));
        return reinterpret_cast<LRESULT>(
            GetSysColorBrush(COLOR_WINDOW));
      case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window_, message, wparam, lparam);
  }

  HWND addStatic(const wchar_t* text, int x, int y, int width, int height,
                 DWORD style = 0) {
    return CreateWindowExW(0, L"STATIC", text,
                           WS_CHILD | WS_VISIBLE | style,
                           x, y, width, height, window_, nullptr, instance_,
                           nullptr);
  }

  HWND addButton(const wchar_t* text, int id, int x, int y, int width,
                 int height, DWORD style) {
    return CreateWindowExW(0, L"BUTTON", text,
                           WS_CHILD | WS_VISIBLE | WS_TABSTOP | style,
                           x, y, width, height, window_,
                           reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                           instance_, nullptr);
  }

  void createControls() {
    font_ = CreateFontW(-18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
                        L"Microsoft JhengHei UI");
    headerFont_ = CreateFontW(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              DEFAULT_PITCH | FF_SWISS,
                              L"Microsoft JhengHei UI");

    addSectionHeader(L"輸入法設定 IME Settings", kLeftColumnX, 18, 360, 32);

    createDisplayLanguages();
    createCandidateControls();
    createEngineControls();
    createReverseLookupControls();

    addButton(L"確定 Confirm", kConfirm, 676, 526, 112, 32, BS_DEFPUSHBUTTON);
    addButton(L"取消 Cancel", kCancel, 804, 526, 112, 32, BS_PUSHBUTTON);
    applyFontToChildren();
    applyHeaderFont();
  }

  void createDisplayLanguages() {
    addButton(L"顯示語言 Display Languages", 0, kLeftColumnX, 58,
              kColumnWidth, kDisplayLanguageGroupHeight, BS_GROUPBOX);
    addStatic(L"主要語言 Main Language", kDisplayLanguageMainX, 84,
              kDisplayLanguageMainWidth, 24);
    addStatic(L"顯示 Display", kDisplayLanguageDisplayX, 84,
              kDisplayLanguageDisplayWidth, 24);
    int y = 114;
    int index = 0;
    for (const auto& option : languageOptions()) {
      addButton(option.label, kMainLanguageBase + index, kDisplayLanguageMainX,
                y, kDisplayLanguageMainWidth, 24, BS_AUTORADIOBUTTON);
      addButton(L"", kDisplayLanguageBase + index, kDisplayLanguageDisplayX,
                y, 22, 22, BS_AUTOCHECKBOX);
      y += kRowHeight;
      ++index;
    }
  }

  void createCandidateControls() {
    int y = 266;
    addStatic(L"每頁候選詞數量 No. of Candidates Per Page", kLeftColumnX, y,
              kColumnWidth, 24);
    pageSizeTrack_ = CreateWindowExW(
        0, TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_AUTOTICKS,
        kLeftColumnX, y + 28, kPageSizeTrackWidth, 34, window_,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kPageSizeTrack)), instance_,
        nullptr);
    SendMessageW(pageSizeTrack_, TBM_SETRANGE, TRUE, MAKELPARAM(4, 10));
    SendMessageW(pageSizeTrack_, TBM_SETTICFREQ, 1, 0);
    addPageSizeTickLabels(y + 64);

    y += 106;
    addStatic(L"中文字體 Chinese Typeface", kLeftColumnX, y, 190, 24);
    addButton(L"宋體 Sung", kTypefaceSung, kLeftColumnX + 198, y - 4, 104, 28,
              BS_AUTORADIOBUTTON);
    addButton(L"黑體 Hei", kTypefaceHei, kLeftColumnX + 314, y - 4, 98, 28,
              BS_AUTORADIOBUTTON);

    y += 42;
    addButton(L"候選詞粵拼 Candidates Jyutping", 0, kLeftColumnX, y,
              kColumnWidth, 116, BS_GROUPBOX);
    addButton(L"顯示 Always Show", kRomanizationAlways, kLeftColumnX + 18, y + 30,
              190, 24, BS_AUTORADIOBUTTON);
    addButton(L"僅反查 Only in Reverse Lookup", kRomanizationReverseOnly,
              kLeftColumnX + 18, y + 58, 260, 24, BS_AUTORADIOBUTTON);
    addButton(L"隱藏 Hide", kRomanizationNever, kLeftColumnX + 18, y + 86, 160,
              24, BS_AUTORADIOBUTTON);
  }

  void createEngineControls() {
    int y = 58;
    addSectionHeader(L"引擎設定 Engine Settings", kRightColumnX, y - 38, 280, 30);
    addButton(L"自動完成 Auto-completion", kEnableCompletion, kRightColumnX, y,
              270, 26, BS_AUTOCHECKBOX);
    addButton(L"自動校正 Auto-correction", kEnableCorrection, kRightColumnX,
              y + 40, 270, 26, BS_AUTOCHECKBOX);
    addButton(L"自動組詞 Auto-composition", kEnableSentence, kRightColumnX,
              y + 80, 270, 26, BS_AUTOCHECKBOX);
    addButton(L"輸入記憶 Input Memory", kEnableLearning, kRightColumnX,
              y + 120, 270, 26, BS_AUTOCHECKBOX);
  }

  void createReverseLookupControls() {
    int y = 288;
    addSectionHeader(L"反查設定 Reverse Lookup Settings", kRightColumnX, y, 330,
                     28);
    addButton(L"顯示完整輸入碼 Show Full Input Code", kShowReverseCode,
              kRightColumnX, y + 40, 330, 26, BS_AUTOCHECKBOX);
    addStatic(L"倉頡版本 Cangjie Version", kRightColumnX, y + 90, 230, 24);
    addButton(L"三代 Version 3", kCangjie3, kRightColumnX, y + 120, 160, 26,
              BS_AUTORADIOBUTTON);
    addButton(L"五代 Version 5", kCangjie5, kRightColumnX + 180, y + 120, 160,
              26, BS_AUTORADIOBUTTON);
  }

  void applyFontToChildren() {
    EnumChildWindows(
        window_,
        [](HWND child, LPARAM param) -> BOOL {
          SendMessageW(child, WM_SETFONT, param, TRUE);
          return TRUE;
        },
        reinterpret_cast<LPARAM>(font_));
  }

  void applyHeaderFont() {
    for (HWND header : sectionHeaders_) {
      SendMessageW(header, WM_SETFONT, reinterpret_cast<WPARAM>(headerFont_),
                   TRUE);
    }
  }

  HWND addSectionHeader(const wchar_t* text, int x, int y, int width,
                        int height) {
    HWND header = addStatic(text, x, y, width, height);
    sectionHeaders_.push_back(header);
    return header;
  }

  void addPageSizeTickLabels(int y) {
    constexpr const wchar_t* kPageSizeTickLabels[] = {
        L"4", L"5", L"6", L"7", L"8", L"9", L"10"};
    constexpr int kPageSizeTickLabelCount =
        sizeof(kPageSizeTickLabels) / sizeof(kPageSizeTickLabels[0]);
    for (int index = 0; index < kPageSizeTickLabelCount; ++index) {
      const int x = kLeftColumnX + (index * kPageSizeTrackWidth / 6) -
                    (kPageTickWidth / 2);
      addStatic(kPageSizeTickLabels[index], x, y, kPageTickWidth, 22,
                SS_CENTER);
    }
  }

  void loadPreferences() {
    const auto loaded = Moqi::TypeDuck::loadPreferences(
        Moqi::TypeDuck::defaultPreferencesPath());
    preferences_ = loaded.preferences;
    if (!loaded.message.empty()) {
      MessageBoxW(window_, utf8ToWide(loaded.message).c_str(),
                  L"TypeDuck 設定 / TypeDuck Settings", MB_ICONWARNING);
    }
  }

  void applyStateToControls() {
    int index = 0;
    for (const auto& option : languageOptions()) {
      CheckDlgButton(window_, kDisplayLanguageBase + index,
                     containsLanguage(preferences_, option.id) ? BST_CHECKED
                                                               : BST_UNCHECKED);
      CheckDlgButton(window_, kMainLanguageBase + index,
                     preferences_.mainLanguage == option.id ? BST_CHECKED
                                                            : BST_UNCHECKED);
      ++index;
    }
    SendMessageW(pageSizeTrack_, TBM_SETPOS, TRUE, preferences_.pageSize);
    CheckDlgButton(window_, preferences_.isHeiTypeface ? kTypefaceHei
                                                       : kTypefaceSung,
                   BST_CHECKED);
    CheckDlgButton(window_, romanizationControl(preferences_.showRomanization),
                   BST_CHECKED);
    CheckDlgButton(window_, kEnableCompletion,
                   preferences_.enableCompletion ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(window_, kEnableCorrection,
                   preferences_.enableCorrection ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(window_, kEnableSentence,
                   preferences_.enableSentence ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(window_, kEnableLearning,
                   preferences_.enableLearning ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(window_, kShowReverseCode,
                   preferences_.showReverseCode ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(window_, preferences_.isCangjie5 ? kCangjie5 : kCangjie3,
                   BST_CHECKED);
  }

  int romanizationControl(const std::string& value) const {
    if (value == "reverse_only") {
      return kRomanizationReverseOnly;
    }
    if (value == "never") {
      return kRomanizationNever;
    }
    return kRomanizationAlways;
  }

  void updatePageSizeLabel() {
    InvalidateRect(pageSizeTrack_, nullptr, FALSE);
  }

  void handleCommand(int id, int notification) {
    if (notification != BN_CLICKED) {
      return;
    }
    if (id >= kMainLanguageBase &&
        id < kMainLanguageBase + static_cast<int>(languageOptions().size())) {
      const int index = id - kMainLanguageBase;
      CheckDlgButton(window_, kDisplayLanguageBase + index, BST_CHECKED);
      return;
    }
    switch (id) {
      case kConfirm:
        confirmAndClose();
        break;
      case kCancel:
        DestroyWindow(window_);
        break;
    }
  }

  void collectState() {
    preferences_.displayLanguages.clear();
    int index = 0;
    for (const auto& option : languageOptions()) {
      if (IsDlgButtonChecked(window_, kDisplayLanguageBase + index) == BST_CHECKED) {
        preferences_.displayLanguages.push_back(option.id);
      }
      if (IsDlgButtonChecked(window_, kMainLanguageBase + index) == BST_CHECKED) {
        preferences_.mainLanguage = option.id;
      }
      ++index;
    }
    preferences_.pageSize =
        static_cast<int>(SendMessageW(pageSizeTrack_, TBM_GETPOS, 0, 0));
    preferences_.isHeiTypeface =
        IsDlgButtonChecked(window_, kTypefaceHei) == BST_CHECKED;
    if (IsDlgButtonChecked(window_, kRomanizationReverseOnly) == BST_CHECKED) {
      preferences_.showRomanization = "reverse_only";
    } else if (IsDlgButtonChecked(window_, kRomanizationNever) == BST_CHECKED) {
      preferences_.showRomanization = "never";
    } else {
      preferences_.showRomanization = "always";
    }
    preferences_.enableCompletion =
        IsDlgButtonChecked(window_, kEnableCompletion) == BST_CHECKED;
    preferences_.enableCorrection =
        IsDlgButtonChecked(window_, kEnableCorrection) == BST_CHECKED;
    preferences_.enableSentence =
        IsDlgButtonChecked(window_, kEnableSentence) == BST_CHECKED;
    preferences_.enableLearning =
        IsDlgButtonChecked(window_, kEnableLearning) == BST_CHECKED;
    preferences_.showReverseCode =
        IsDlgButtonChecked(window_, kShowReverseCode) == BST_CHECKED;
    preferences_.isCangjie5 = IsDlgButtonChecked(window_, kCangjie5) == BST_CHECKED;
  }

  bool apply() {
    collectState();
    const auto result = Moqi::TypeDuck::applyPreferences(
        Moqi::TypeDuck::defaultPreferencesPath(), preferences_, applyRimeSettings);
    if (!result.ok) {
      MessageBoxW(window_, utf8ToWide(result.message).c_str(),
                  L"TypeDuck 設定 / TypeDuck Settings", MB_ICONWARNING);
      return false;
    }
    return true;
  }

  void confirmAndClose() {
    if (apply()) {
      DestroyWindow(window_);
    }
  }

  HINSTANCE instance_;
  HWND window_ = nullptr;
  HWND pageSizeTrack_ = nullptr;
  HFONT font_ = nullptr;
  HFONT headerFont_ = nullptr;
  std::vector<HWND> sectionHeaders_;
  Moqi::TypeDuck::Preferences preferences_;
};

} // namespace

int RunSettingsWindow(HINSTANCE instance, int showCommand) {
  SettingsWindow window(instance);
  return window.run(showCommand);
}

} // namespace Moqi::TypeDuckSettings
