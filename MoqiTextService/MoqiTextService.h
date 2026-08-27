//
//	Copyright (C) 2013 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
//
//	This library is free software; you can redistribute it and/or
//	modify it under the terms of the GNU Library General Public
//	License as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the
//	Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
//	Boston, MA  02110-1301, USA.
//

#ifndef NODE_TEXT_SERVICE_H
#define NODE_TEXT_SERVICE_H

#include <LibIME2/src/TextService.h>
#include <LibIME2/src/MessageWindow.h>
#include <LibIME2/src/EditSession.h>
#include <LibIME2/src/LangBarButton.h>
#include "MoqiImeModule.h"
#include "MoqiCandidateWindow.h"
#include <sys/types.h>
#include "MoqiClient.h"
#include <algorithm>
#include <map>
#include <memory>


namespace Moqi {

enum class CandidateFontRole {
	Interface,
	InputBuffer,
	SelectionLabel,
	Chinese,
	PageNavigation,
	InfoIcon,
	DictionaryHeadword,
	DictionaryPronunciation,
	DictionaryPronunciationType,
	DictionaryMeta,
	DictionaryPartOfSpeech,
	DictionaryBody,
	DictionaryValue,
	DictionaryCaption,
	CandidateDefinition,
	DictionaryLanguage,
	DisplayLanguageEnglish,
	DisplayLanguageHindi,
	DisplayLanguageIndonesian,
	DisplayLanguageNepali,
	DisplayLanguageUrdu,
};

enum class CandidateColorRole {
	PanelBackground,
	DictionaryBackground,
	PanelBorder,
	SelectionBackground,
	InputBufferBackground,
	InputBufferText,
	ItemText,
	LabelText,
	DefinitionText,
	PronunciationText,
	MetalanguageText,
	ActiveText,
	DisabledText,
	PosPillBorder,
	DictionaryScrollTrack,
	DictionaryScrollThumb,
};

class TextService: public Ime::TextService {
	friend class Client;
public:
	static constexpr bool kTypeDuckInlinePreedit = true;

	TextService(ImeModule* module);

	virtual void onActivate();
	virtual void onDeactivate();

	virtual void onFocus();
	virtual void onSetFocus() override;
	virtual void onKillFocus() override;
	virtual void onSetThreadFocus() override;
	virtual void onKillThreadFocus() override;

	virtual bool filterKeyDown(Ime::KeyEvent& keyEvent);
	virtual bool onKeyDown(Ime::KeyEvent& keyEvent, Ime::EditSession* session);

	virtual bool filterKeyUp(Ime::KeyEvent& keyEvent);
	virtual bool onKeyUp(Ime::KeyEvent& keyEvent, Ime::EditSession* session);

	virtual bool onPreservedKey(const GUID& guid);
	STDMETHODIMP OnPreservedKey(ITfContext* pContext, REFGUID rguid, BOOL* pfEaten) override;

	virtual bool onCommand(UINT id, CommandType type);

	// called when a language bar button needs a menu
	virtual bool onMenu(LangBarButton* btn, ITfMenu* pMenu);

	// called when a language bar button needs a menu
	virtual HMENU onMenu(LangBarButton* btn);

	// called when a compartment value is changed
	virtual void onCompartmentChanged(const GUID& key);

	// called when the keyboard is opened or closed
	virtual void onKeyboardStatusChanged(bool opened);

	// called just before current composition is terminated for doing cleanup.
	// if forced is true, the composition is terminated by others, such as
	// the input focus is grabbed by another application.
	// if forced is false, the composition is terminated gracefully by endComposition().
	virtual void onCompositionTerminated(bool forced);

	virtual void onLangProfileActivated(REFIID lang);

	virtual void onLangProfileDeactivated(REFIID lang);

	virtual void onLayoutChange(ITfContext* context, TfLayoutCode code, ITfContextView* view) override;

	// methods called by Moqi::Client
	int candidatePageSize() const {
		return candidatePageSize_;
	}

	void setCandidatePageSize(int candidatePageSize) {
		candidatePageSize_ = (std::max)(0, candidatePageSize);
	}

	int candidatePageIndex() const {
		return candidatePageIndex_;
	}

	void setCandidatePageIndex(int candidatePageIndex) {
		candidatePageIndex_ = (std::max)(0, candidatePageIndex);
	}

	int candidateTotalCount() const {
		return candidateTotalCount_;
	}

	void setCandidateTotalCount(int candidateTotalCount) {
		candidateTotalCount_ = (std::max)(0, candidateTotalCount);
	}

	bool candidateHasPrevious() const {
		return candidateHasPrevious_;
	}

	void setCandidateHasPrevious(bool candidateHasPrevious) {
		candidateHasPrevious_ = candidateHasPrevious;
	}

	bool candidateHasNext() const {
		return candidateHasNext_;
	}

	void setCandidateHasNext(bool candidateHasNext) {
		candidateHasNext_ = candidateHasNext;
	}

	std::wstring selKeys() const {
		return selKeys_;
	}

	void setSelKeys(std::wstring selKeys) {
		selKeys_ = selKeys;
	}

	bool effectiveUiLess() const {
		return isUiLess() || autoUiLessOverride_ || manualUiLessOverride_;
	}

	bool effectiveInlinePreedit() const {
		if (autoInlinePreeditDisabled_) {
			return false;
		}
		return effectiveUiLess() || kTypeDuckInlinePreedit;
	}

	bool effectiveExternalPreedit() const {
		return !effectiveInlinePreedit();
	}

	bool tsfCandidateUiEnabled() const {
		return !autoDisableTsfCandidateUi_;
	}

	HFONT createCandidateFontForDpi(CandidateFontRole role, int dpiY) const;
	HFONT createCandidateLanguageFontForDpi(TypeDuck::DisplayLanguage language, CandidateFontRole role, int dpiY) const;
	COLORREF candidateColor(CandidateColorRole role) const;
	void reloadCandidateAppearanceTheme();

	virtual bool inlinePreeditEnabledForComposition() const override {
		return effectiveInlinePreedit();
	}

	virtual bool shouldUseDummyCompositionAnchor() const override {
		return !effectiveUiLess() && autoDummyAnchorCompat_;
	}

	void setTypeDuckDisplayPreferences(TypeDuck::DisplayPreferences preferences);

	void suppressNextCompositionTerminatedNotification() {
		suppressNextCompositionTerminatedNotification_ = true;
	}

	const std::wstring& candidatePreedit() const {
		return candidatePreedit_;
	}

	void setCandidatePreedit(std::wstring preedit) {
		if (candidatePreedit_ == preedit) {
			return;
		}
		candidatePreedit_ = preedit;
		if (candidatePreeditCursor_ > static_cast<int>(candidatePreedit_.length())) {
			candidatePreeditCursor_ = static_cast<int>(candidatePreedit_.length());
		}
		if (candidatePreeditSelectionStart_ > static_cast<int>(candidatePreedit_.length())) {
			candidatePreeditSelectionStart_ = static_cast<int>(candidatePreedit_.length());
		}
		if (candidatePreeditSelectionEnd_ > static_cast<int>(candidatePreedit_.length())) {
			candidatePreeditSelectionEnd_ = static_cast<int>(candidatePreedit_.length());
		}
		if (candidateWindow_) {
			candidateWindow_->setPreeditText(candidatePreedit_);
			candidateWindow_->setPreeditCursor(candidatePreeditCursor_);
			candidateWindow_->setPreeditSelection(candidatePreeditSelectionStart_, candidatePreeditSelectionEnd_);
		}
	}

	void setCandidatePreeditCursor(int cursor) {
		cursor = (std::max)(0, (std::min)(cursor, static_cast<int>(candidatePreedit_.length())));
		if (candidatePreeditCursor_ == cursor) {
			return;
		}
		candidatePreeditCursor_ = cursor;
		if (candidateWindow_) {
			candidateWindow_->setPreeditCursor(candidatePreeditCursor_);
		}
	}

	void setCandidatePreeditSelection(int start, int end) {
		const int length = static_cast<int>(candidatePreedit_.length());
		start = (std::max)(0, (std::min)(start, length));
		end = (std::max)(0, (std::min)(end, length));
		if (end < start) {
			std::swap(start, end);
		}
		if (candidatePreeditSelectionStart_ == start && candidatePreeditSelectionEnd_ == end) {
			return;
		}
		candidatePreeditSelectionStart_ = start;
		candidatePreeditSelectionEnd_ = end;
		if (candidateWindow_) {
			candidateWindow_->setPreeditSelection(candidatePreeditSelectionStart_, candidatePreeditSelectionEnd_);
		}
	}

	bool showingCandidates() {
		return showingCandidates_;
	}

	bool pendingCandidateRecovery() const {
		return pendingCandidateRecovery_;
	}

	// candidate window
	void showCandidates(Ime::EditSession* session);
	void updateCandidates(Ime::EditSession* session);
	void updateCandidatesWithoutSession();
    void updateCandidatesWindow(Ime::EditSession* session);
	void hideCandidates(bool preserveRecoveryState = false);
	void resetTypeDuckDegradedState(Ime::EditSession* session = nullptr);
	bool highlightCandidate(int index);
	bool selectCandidate(int index);
	bool changeCandidatePage(bool backward);

	void refreshCandidates();
	bool setCandidateCursor(int cursor);
	bool hasCandidateWindow() const {
		return candidateWindow_ != nullptr;
	}

	// message window
	void showMessage(Ime::EditSession* session, std::wstring message, int duration = 3);
    void updateMessageWindow(Ime::EditSession* session);
	void hideMessage();

private:
	virtual ~TextService(void);  // COM object should only be deleted using Release()

	void onMessageTimeout();
	static void CALLBACK onMessageTimeout(HWND hwnd, UINT msg, UINT_PTR id, DWORD time);

	void updateLangButtons(); // update status of language bar buttons

	void createCandidateWindow(Ime::EditSession* session);
	void destroyCandidateWindow();
	bool ensureCandidateWindowValid(const wchar_t* reason);
	void applyCandidateAppearanceNow();
	void refreshCandidateAppearance();
	std::wstring resolveCandidateRuntimeAppearanceThemePath() const;
	std::wstring candidateFontStack(CandidateFontRole role) const;
	int candidateFontPointSize(CandidateFontRole role) const;
	bool loadCandidateAppearanceThemeFromFile(const std::wstring& path);
	void reloadTypeDuckDisplayPreferences();
	void applyUiLessOverrideState();
	void invalidateCandidateUiCache();
	bool isCandidateContentApplied(const std::wstring& renderedPreedit) const;
	void markCandidateContentApplied(const std::wstring& renderedPreedit);
	bool moveCandidateWindowToInputRect(Ime::EditSession* session, const wchar_t* reason, bool throttleSamePosition);

	bool ensureClientForCurrentProfile(const wchar_t* reason);
	void closeClient();

private:
	bool validCandidateListElementId_;
	DWORD candidateListElementId_;
	bool shouldShowCandidateWindowUI_;
	bool manualUiLessOverride_;
	bool autoUiLessOverride_;
	bool autoDummyAnchorCompat_;
	bool autoInlinePreeditDisabled_;
	bool autoDisableTsfCandidateUi_;
	Ime::ComPtr<Moqi::CandidateWindow> candidateWindow_; // this is a ref-counted COM object and should not be managed with std::unique_ptr
	bool showingCandidates_;
	bool pendingCandidateRecovery_;
	std::vector<CandidateUiItem> candidates_; // current candidate list
	std::vector<CandidateUiItem> appliedCandidates_;
	std::wstring appliedSelKeys_;
	std::wstring appliedCandidatePreedit_;
	bool hasAppliedCandidateContent_;
	bool hasAppliedCandidateCursor_;
	int appliedCandidateCursor_;
	bool hasLastCandidateWindowPos_;
	POINT lastCandidateWindowPos_;
	ULONGLONG lastCandidateWindowMoveTick_;
	std::unique_ptr<Ime::MessageWindow> messageWindow_;
	UINT messageTimerId_;
	HFONT font_;
	HFONT commentFont_;
	bool updateFont_;
	int candidatePageIndex_;
	int candidatePageSize_;
	int candidateTotalCount_;
	bool candidateHasPrevious_;
	bool candidateHasNext_;
	std::wstring selKeys_;
	std::map<std::wstring, std::vector<std::wstring>> appearanceFontStacks_;
	std::map<std::wstring, COLORREF> appearancePalette_;
	TypeDuck::DisplayPreferences typeDuckDisplayPreferences_;
	bool suppressNextCompositionTerminatedNotification_;
	std::wstring candidatePreedit_;
	int candidatePreeditCursor_;
	int candidatePreeditSelectionStart_;
	int candidatePreeditSelectionEnd_;

	HMENU popupMenu_;

	std::unique_ptr<Client> client_; // connection client
	GUID currentLangProfile_;
};

}

#endif
