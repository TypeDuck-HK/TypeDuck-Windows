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
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnOK(WORD, WORD, HWND, BOOL&);
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);

	CStatic title_;
	CStatic description_;
};
