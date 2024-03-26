#include "stdafx.h"
#include "AboutDialog.h"

AboutDialog::AboutDialog() {}
AboutDialog::~AboutDialog() {}

LRESULT AboutDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	description_.Attach(GetDlgItem(IDC_DESCRIPTION));
	description_.SetWindowTextW(
		L"歡迎使用 TypeDuck 打得 —— 設有少數族裔語言提示粵拼輸入法！有字想打？一裝即用，毋須再等，即刻打得！\n"
		L"Welcome to TypeDuck: a Cantonese input keyboard with minority language prompts! Got something you want to type? Have your fingers ready, get, set, TYPE DUCK!\n"
		L"\n"
		L"如有任何查詢，歡迎電郵至 info@typeduck.hk 或 lchaakming@eduhk.hk。\n"
		L"Should you have any enquiries, please email info@typeduck.hk or lchaakming@eduhk.hk.\n"
		L"\n"
		L"本輸入法由香港教育大學語言學及現代語言系開發。特別鳴謝「語文教育及研究常務委員會」資助本計劃。\n"
		L"This input method is developed by the Department of Linguistics and Modern Language Studies, the Education University of Hong Kong. Special thanks to the Standing Committee on Language Education and Research for funding this project."
	);

	CenterWindow();
	BringWindowToTop();
	return TRUE;
}

LRESULT AboutDialog::OnOK(WORD, WORD code, HWND, BOOL&) {
	EndDialog(code);
	return 0;
}

LRESULT AboutDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&) {
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT AboutDialog::OnCtrlColor(UINT, WPARAM, LPARAM, BOOL&) {
	return (LRESULT) GetStockObject(WHITE_BRUSH);
}

LRESULT AboutDialog::OnCtrlColorStatic(UINT, WPARAM, LPARAM, BOOL&) {
	return (LRESULT) CreateSolidBrush(RGB(255, 255, 255));
}
