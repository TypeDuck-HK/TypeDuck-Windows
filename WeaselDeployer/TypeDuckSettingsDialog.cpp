#include "stdafx.h"
#include "TypeDuckSettingsDialog.h"

LRESULT TypeDuckSettingsDialog::OnInitDiaLog(UINT, WPARAM, LPARAM, BOOL &) {
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

    Populate();
    settings_->Load();
    if (!DoDataExchange(false))
        return FALSE;

    CenterWindow();
    BringWindowToTop();
    return TRUE;
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
    return settings_->SetLanguageList(selectedLanguages) && settings_->Save();
}

LRESULT TypeDuckSettingsDialog::OnOK(WORD, WORD code, HWND, BOOL&) {
    return SaveSettings() & EndDialog(code);
}

LRESULT TypeDuckSettingsDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&) {
    return SaveSettings() & EndDialog(IDCANCEL);
}
