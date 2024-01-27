#include "stdafx.h"
#include "TypeDuckSettingsDialog.h"

#define GET_CORRECT(method, i, n) \
    (i) ? (i) == (n) - 1 \
        ? method((i) - 2) * 2 - method((i) - 3) \
        : method((i) - 1) \
        : method(0) * 2 - method(1)

LRESULT TypeDuckSettingsDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
    HFONT font = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, CHINESEBIG5_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft JhengHei UI");
    display_languages_title_.Attach(GetDlgItem(IDC_DISPLAY_LANGUAGES_TITLE));
    display_languages_title_.SetFont(font);
    reverse_settings_title_.Attach(GetDlgItem(IDC_REVERSE_SETTINGS_TITLE));
    reverse_settings_title_.SetFont(font);

    language_available_list_.Attach(GetDlgItem(IDC_HINT_LIST));
    language_available_list_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
    language_available_list_.AddColumn(L"Available Languages", 0);
    WTL::CRect rc;
    language_available_list_.GetClientRect(&rc);
    language_available_list_.SetColumnWidth(0, rc.Width() - 20);

    candidate_count_.Attach(GetDlgItem(IDC_CANDIDATE_COUNT));
    candidate_count_.SetRange(4, 10);

    Populate();
    settings_->Load();
    if (!DoDataExchange(false))
        return FALSE;
    candidate_count_.SetPos(settings_->candidateCount);

    CenterWindow();
    BringWindowToTop();
    return TRUE;
}

LRESULT TypeDuckSettingsDialog::OnPaint(UINT, WPARAM, LPARAM, BOOL&) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(&ps);
    SetBkMode(hdc, TRANSPARENT);
    SelectObject(hdc, CreateFont(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, CHINESEBIG5_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft JhengHei UI"));
    int nNumTics = candidate_count_.GetNumTics();
    WTL::CRect sliderRect;
    candidate_count_.GetWindowRect(&sliderRect);
    ScreenToClient(&sliderRect);
    int lineSize = candidate_count_.GetLineSize();
    for (size_t i = 0; i < nNumTics; i++) {
        int x = GET_CORRECT(candidate_count_.GetTicPos, i, nNumTics);
        WTL::CString strLabel;
        strLabel.Format(L"%d", GET_CORRECT(candidate_count_.GetTic, i, nNumTics));
        WTL::CRect rect = {sliderRect.left + x - 10, sliderRect.bottom, sliderRect.left + x + lineSize + 10, sliderRect.bottom + 16};
        DrawText(hdc, strLabel, strLabel.GetLength(), &rect, DT_CENTER | DT_BOTTOM | DT_SINGLELINE);
    }
    EndPaint(&ps);
    return 0;
}

void TypeDuckSettingsDialog::Populate() {
    language_available_list_.DeleteAllItems();
    auto languageConfig = settings_->GetActiveLanguages();
    for (size_t i = 0; i < LanguageConfigSize; ++i) {
        const int index = language_available_list_.AddItem(i, 0, LanguageList[i]);
        language_available_list_.SetCheckState(index, languageConfig[i]);
    }
}

bool TypeDuckSettingsDialog::SaveSettings() {
    if (!DoDataExchange(true))
        return false;
    std::vector<bool> selectedLanguages;
    for (size_t i = 0; i < LanguageConfigSize; ++i) {
        selectedLanguages.push_back(language_available_list_.GetCheckState(i));
    }
    settings_->candidateCount = candidate_count_.GetPos();
    return settings_->SetLanguageList(selectedLanguages) && settings_->Save();
}

LRESULT TypeDuckSettingsDialog::OnOK(WORD, WORD code, HWND, BOOL&) {
    return SaveSettings() & EndDialog(code);
}

LRESULT TypeDuckSettingsDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&) {
    return SaveSettings() & EndDialog(IDCANCEL);
}
