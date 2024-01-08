#pragma once
#include "resource.h"

class AboutDialog: public CDialogImpl<AboutDialog> {
public:
	enum { IDD = IDD_ABOUTBOX };

	AboutDialog();
	~AboutDialog();

protected:
	BEGIN_MSG_MAP(AboutDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtrlColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtrlColorStatic)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnOK(WORD, WORD, HWND, BOOL&);
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnCtrlColor(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnCtrlColorStatic(UINT, WPARAM, LPARAM, BOOL&);

	CStatic description_;
};
