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
	language_available_list_.AddColumn(L"Available Languages", 0);
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
  language_available_list_.DeleteAllItems();
  auto languageConfig = settings_->GetActiveLanguages();
  for (int i = 0; i < LanguageConfigSize; ++i)
  {
    const int index = language_available_list_.AddItem(i, 0, LanguageList[i].c_str());
    language_available_list_.SetCheckState(index, languageConfig[i]);
  }
}

LRESULT HintSettingsDialog::OnOK(WORD, WORD code, HWND, BOOL&) 
{
  std::vector<bool> selectedLanguages;
  for (int i = 0; i < LanguageConfigSize; ++i)
  {
	  selectedLanguages.push_back(language_available_list_.GetCheckState(i));
  }
  settings_->SetLanguageList(selectedLanguages);
  
  EndDialog(code);
  return 0;
}

LRESULT HintSettingsDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&)
{
  EndDialog(IDCANCEL);
  return 0;
}
