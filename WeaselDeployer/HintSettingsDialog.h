#pragma once
#include "resource.h"
#include "HintSettings.h"

class HintSettingsDialog : public CDialogImpl<HintSettingsDialog>
{
public:
	enum { IDD = IDD_HINT_SETTING };
protected:
	BEGIN_MSG_MAP(HintSettingsDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDiaLog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()

  HintSettingsDialog(HintSettings*);
	~HintSettingsDialog() {	}
	LRESULT OnInitDiaLog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnOK(WORD, WORD, HWND, BOOL&);
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);

	void Populate();
	
	CCheckListViewCtrl language_available_list_;
	HintSettings* settings_;
	bool loaded_;

};

