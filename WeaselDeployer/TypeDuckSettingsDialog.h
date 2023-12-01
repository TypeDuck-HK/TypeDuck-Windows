#pragma once
#include "resource.h"
#include "TypeDuckSettings.h"

class TypeDuckSettingsDialog :
	public CDialogImpl<TypeDuckSettingsDialog>,
	public CWinDataExchange<TypeDuckSettingsDialog>
{
public:
	BEGIN_DDX_MAP(TypeDuckSettingsDialog)
		DDX_RADIO(IDC_ROMAN_ALWAYS, settings_->showRomanization)
		DDX_CHECK(IDC_SHOW_REVERSE_CODE, settings_->showReverseCode)
		DDX_CHECK(IDC_ENABLE_CORRECTION, settings_->enableCorrection)
		DDX_RADIO(IDC_CANGJIE_3, settings_->isCangjie5)
	END_DDX_MAP()

	TypeDuckSettingsDialog(TypeDuckSettings* settings) : settings_(settings) {}
	~TypeDuckSettingsDialog() {}

	enum { IDD = IDD_HINT_SETTING };

protected:
	BEGIN_MSG_MAP(TypeDuckSettingsDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDiaLog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()

	LRESULT OnInitDiaLog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnOK(WORD, WORD, HWND, BOOL&);
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);

	void Populate();
	bool SaveSettings();
	
	CCheckListViewCtrl language_available_list_;
	TypeDuckSettings* settings_;
};
