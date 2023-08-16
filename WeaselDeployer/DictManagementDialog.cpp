#include "stdafx.h"
#include "DictManagementDialog.h"
#include "Configurator.h"
#include <WeaselUtility.h>
#include <rime_api.h>

DictManagementDialog::DictManagementDialog()
{
	api_ = (RimeLeversApi*)rime_get_api()->find_module("levers")->get_api();
}

DictManagementDialog::~DictManagementDialog()
{
}

void DictManagementDialog::Populate() {
	RimeUserDictIterator iter = {0};
	api_->user_dict_iterator_init(&iter);
	while (const char* dict = api_->next_user_dict(&iter)) {
		user_dict_list_.AddString(utf8towcs(dict));
	}
	api_->user_dict_iterator_destroy(&iter);
	user_dict_list_.SetCurSel(-1);
}

LRESULT DictManagementDialog::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	user_dict_list_.Attach(GetDlgItem(IDC_USER_DICT_LIST));
	backup_.Attach(GetDlgItem(IDC_BACKUP));
	backup_.EnableWindow(FALSE);
	restore_.Attach(GetDlgItem(IDC_RESTORE));
	restore_.EnableWindow(TRUE);
	export_.Attach(GetDlgItem(IDC_EXPORT));
	export_.EnableWindow(FALSE);
	import_.Attach(GetDlgItem(IDC_IMPORT));
	import_.EnableWindow(FALSE);
	backup_restore_description_.Attach(GetDlgItem(IDC_BACKUP_RESTORE_DESCRIPTION));
	backup_restore_description_.SetWindowTextW(L"To migrate a user dictionary, which contains your typing practices, to another device equipped with TypeDuck, select the name of the dictionary to export and press the “Export Snapshot” button below. On another device, load the dictionary by the “Import Snapshot” button such that the entries in the snapshot file are merged into the dictionary to which they belong.");
	export_import_description_.Attach(GetDlgItem(IDC_EXPORT_IMPORT_DESCRIPTION));
	export_import_description_.SetWindowTextW(L"“Export Corpus” is a function designed to complement the lexicon of a schema by exporting new phrases created during typing in a RIME-compatible format for viewing and editing. To convert data from other sources into TSV format, use the “Import Corpus” function.");
	
	Populate();
	
	CenterWindow();
	BringWindowToTop();
	return TRUE;
}

LRESULT DictManagementDialog::OnClose(UINT, WPARAM, LPARAM, BOOL&) {
	EndDialog(IDCANCEL);
	return 0;
}

LRESULT DictManagementDialog::OnBackup(WORD, WORD code, HWND, BOOL&) {
	int sel = user_dict_list_.GetCurSel();
	if (sel < 0 || sel >= user_dict_list_.GetCount()) {
		MessageBox(L"Please select the name of the dictionary to export in the left column.", L"Empty Selection", MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	std::wstring path;
	{
		char dir[MAX_PATH] = {0};
		rime_get_api()->get_user_data_sync_dir(dir, _countof(dir));
		WCHAR wdir[MAX_PATH] = {0};
		MultiByteToWideChar(CP_ACP, 0, dir, -1, wdir, _countof(wdir));
		path = wdir;
	}
	WCHAR dict_name[100] = {0};
	user_dict_list_.GetText(sel, dict_name);
	if (!api_->backup_user_dict(wcstoutf8(dict_name))) {
		if (_waccess_s(path.c_str(), 0) != 0 &&
			!CreateDirectoryW(path.c_str(), NULL) &&
			GetLastError() == ERROR_PATH_NOT_FOUND) {
			MessageBox(L"Unable to export user dictionaries. Please ensure that the folder is accessible by TypeDuck.", L"Export Failed", MB_OK | MB_ICONERROR);
			return 0;
		}
		if (!api_->backup_user_dict(wcstoutf8(dict_name))) {
			MessageBox(L"Unable to export user dictionaries. An unknown error has occurred.", L"Export Failed", MB_OK | MB_ICONERROR);
			return 0;
		}
	}
	path += std::wstring(L"\\") + dict_name + L".userdb.txt";
	if (_waccess(path.c_str(), 0) != 0) {
		MessageBox(L"Unable to export user dictionaries. The exported file cannot be found.", L"Export Failed", MB_OK | MB_ICONERROR);
		return 0;
	}
	std::wstring param = L"/select, \"" + path + L"\"";
	ShellExecute(NULL, L"open", L"explorer.exe", param.c_str(), NULL, SW_SHOWNORMAL);
	return 0;
}

LRESULT DictManagementDialog::OnRestore(WORD, WORD code, HWND, BOOL&) {
	CFileDialog dlg(TRUE, L"snapshot", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		L"Dictionary Snapshot\0*.userdb.txt\0KCSS Dictionary Snapshot\0*.userdb.kct.snapshot\0All Files\0*.*\0");
	if (IDOK == dlg.DoModal()) {
		char path[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, dlg.m_szFileName, -1, path, _countof(path), NULL, NULL);
		if (!api_->restore_user_dict(path)) {
			MessageBox(L"Unable to restore user dictionaries. An unknown error has occurred.", L"Restoration Failed", MB_OK | MB_ICONERROR);
		}
		else {
			MessageBox(L"Successfully restored user dictionaries.", L"Restoration Completed", MB_OK | MB_ICONINFORMATION);
		}
	}
	return 0;
}

LRESULT DictManagementDialog::OnExport(WORD, WORD code, HWND, BOOL&) {
	int sel = user_dict_list_.GetCurSel();
	if (sel < 0 || sel >= user_dict_list_.GetCount()) {
		MessageBox(L"Please select the name of the dictionary to export in the left column.", L"Empty Selection", MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	WCHAR dict_name[MAX_PATH] = {0};
	user_dict_list_.GetText(sel, dict_name);
	std::wstring file_name(dict_name);
	file_name += L"_export.txt";
	CFileDialog dlg(FALSE, L"txt", file_name.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, L"Text Files\0*.txt\0All Files\0*.*\0");
	if (IDOK == dlg.DoModal()) {
		char path[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, dlg.m_szFileName, -1, path, _countof(path), NULL, NULL);
		int result = api_->export_user_dict(wcstoutf8(dict_name), path);
		if (result < 0) {
			MessageBox(L"Unable to export user dictionaries. An unknown error has occurred.", L"Export Failed", MB_OK | MB_ICONERROR);
		}
		else if (_waccess(dlg.m_szFileName, 0) != 0) {
			MessageBox(L"Unable to export user dictionaries. The exported file cannot be found.", L"Export Failed", MB_OK | MB_ICONERROR);
		}
		else {
			std::wstring report(L"Successfully exported " + std::to_wstring(result) + L" records.");
			MessageBox(report.c_str(), L"Export Completed", MB_OK | MB_ICONINFORMATION);
			std::wstring param = L"/select, \"" + std::wstring(dlg.m_szFileName) + L"\"";
			ShellExecute(NULL, L"open", L"explorer.exe", param.c_str(), NULL, SW_SHOWNORMAL);
		}
	}
	return 0;
}

LRESULT DictManagementDialog::OnImport(WORD, WORD code, HWND, BOOL&) {
	int sel = user_dict_list_.GetCurSel();
	if (sel < 0 || sel >= user_dict_list_.GetCount()) {
		MessageBox(L"Please select the name of the dictionary to import in the left column.", L"Empty Selection", MB_OK | MB_ICONINFORMATION);
		return 0;
	}
	WCHAR dict_name[MAX_PATH] = {0};
	user_dict_list_.GetText(sel, dict_name);
	std::wstring file_name(dict_name);
	file_name += L"_export.txt";
	CFileDialog dlg(TRUE, L"txt", file_name.c_str(), OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, L"Text Files\0*.txt\0All Files\0*.*\0");
	if (IDOK == dlg.DoModal()) {
		char path[MAX_PATH] = {0};
		WideCharToMultiByte(CP_ACP, 0, dlg.m_szFileName, -1, path, _countof(path), NULL, NULL);
		int result = api_->import_user_dict(wcstoutf8(dict_name), path);
		if (result < 0) {
			MessageBox(L"Unable to import user dictionaries. An unknown error has occurred.", L"Import Failed", MB_OK | MB_ICONERROR);
		}
		else {
			std::wstring report(L"Successfully imported " + std::to_wstring(result) + L" records.");
			MessageBox(report.c_str(), L"Import Completed", MB_OK | MB_ICONINFORMATION);
		}
	}
	return 0;
}

LRESULT DictManagementDialog::OnUserDictListSelChange(WORD, WORD, HWND, BOOL&) {
	int index = user_dict_list_.GetCurSel();
	BOOL enabled = index < 0 ? FALSE : TRUE;
	backup_.EnableWindow(enabled);
	export_.EnableWindow(enabled);
	import_.EnableWindow(enabled);
	return 0;
}
