#include "stdafx.h"
#include "HintSettingsDialog.h"
#include "WeaselUtility.h"

HintSettingsDialog::HintSettingsDialog(HintSettings* settings) : settings_(settings)
{
  
}

LRESULT HintSettingsDialog::OnInitDiaLog(UINT, WPARAM, LPARAM, BOOL &)
{
  language_available_list_.Attach(GetDlgItem(IDC_HINT_LIST));
  language_available_list_.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
	language_available_list_.AddColumn(L"Available Language", 0);
  WTL::CRect rc;
	language_available_list_.GetClientRect(&rc);
	language_available_list_.SetColumnWidth(0, rc.Width() - 20);

  Populate();

  CenterWindow();
  BringWindowToTop();
  return TRUE;
}

void HintSettingsDialog::Populate()
{
  std::vector<HintSettingsInfo> languages;
  settings_->GetLanguageList(&languages);
  language_available_list_.DeleteAllItems();
  std::vector<HintSettingsInfo> active_languages;
  settings_->GetActiveLanguages(&active_languages);
  for (auto& language : languages)
  {
    auto currentLine = language_available_list_.GetItemCount();
    int index = language_available_list_.AddItem(currentLine, 0, language.column_name.c_str());
    // check if the language is in active_languages
    auto it = std::find_if(active_languages.begin(), active_languages.end(), 
                          [&language](const HintSettingsInfo& info) {
      return info.settingIndex == language.settingIndex;
    });
    bool isActive = it != active_languages.end();
    language_available_list_.SetCheckState(index, isActive);

  }

}

LRESULT HintSettingsDialog::OnOK(WORD, WORD code, HWND, BOOL&) 
{
  auto itemCount = language_available_list_.GetItemCount();
  std::vector<int> checkedItemIndices;
  for (int i = 0; i < itemCount; ++i)
  {
    if (language_available_list_.GetCheckState(i))
    {
      checkedItemIndices.push_back(i);
    }
  }
  std::vector<HintSettingsInfo> selected_languages;

  for (auto& index : checkedItemIndices)
  {
    // get string from checked item
    ATL::CString language;
    language_available_list_.GetItemText(index, 0, language);  
    // ATL::CString to std::string
    std::string language_str = CW2A(language.GetString());
    HintSettingsInfo info;
    info.column_name = language.GetString();
    info.settingIndex = index;
    selected_languages.push_back(info);
  }
  settings_->SetLanguageList(selected_languages);
  
  EndDialog(code);
  return 0;
}

LRESULT HintSettingsDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&)
{
  EndDialog(IDCANCEL);
  return 0;
}
