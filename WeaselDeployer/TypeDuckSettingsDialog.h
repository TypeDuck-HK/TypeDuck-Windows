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
		DDX_RADIO(IDC_INLINE_PREEDIT_ENABLED, settings_->inputCodeInWindow)
		DDX_CHECK(IDC_ENABLE_COMPLETION, settings_->enableCompletion)
		DDX_CHECK(IDC_ENABLE_CORRECTION, settings_->enableCorrection)
		DDX_CHECK(IDC_ENABLE_SENTENCE, settings_->enableSentence)
		DDX_CHECK(IDC_ENABLE_LEARNING, settings_->enableLearning)
		DDX_CHECK(IDC_SHOW_REVERSE_CODE, settings_->showReverseCode)
		DDX_RADIO(IDC_CANGJIE_3, settings_->isCangjie5)
	END_DDX_MAP()

	TypeDuckSettingsDialog(TypeDuckSettings* settings) : settings_(settings) {}
	~TypeDuckSettingsDialog() {}

	enum { IDD = IDD_HINT_SETTING };

protected:
	BEGIN_MSG_MAP(TypeDuckSettingsDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnOK(WORD, WORD, HWND, BOOL&);
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);

	void Populate();
	bool SaveSettings();
	
	CStatic display_languages_title_;
	CStatic reverse_settings_title_;
	CCheckListViewCtrl language_available_list_;
	CTrackBarCtrl candidate_count_;
	TypeDuckSettings* settings_;
};
