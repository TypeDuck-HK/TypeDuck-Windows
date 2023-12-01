#pragma once

#include <WeaselCommon.h>
#include <boost/tokenizer.hpp>
#include "fontClasses.h"

class MultiHintPanel;

struct InfoLanguage {
	std::wstring Key;
	std::wstring Value;
	IDWriteTextFormat1* TextFormat;
};

struct InfoDefinition {
	std::wstring Eng;
	std::wstring Urd;
	std::wstring Nep;
	std::wstring Hin;
	std::wstring Ind;
	std::vector<InfoLanguage> Get(MultiHintPanel* panel, DirectWriteResources* pDWR) const;
};

struct InfoProperties {
	std::wstring PartOfSpeech;
	std::wstring Register;
	std::wstring Label;
	std::wstring Normalized;
	std::wstring Written;
	std::wstring Vernacular;
	std::wstring Collocation;
	std::vector<std::wstring> GetPartsOfSpeech() const;
	std::wstring GetRegister() const;
	std::vector<std::wstring> GetLabels() const;
	std::vector<std::vector<std::wstring> > GetOtherData() const;
	InfoDefinition Definition;
};

struct InfoMultiHint {
	InfoMultiHint() {}
	InfoMultiHint(const std::wstring& input);
	std::wstring MatchInputBuffer;
	std::wstring Honzi;
	std::wstring Jyutping;
	std::wstring PronOrder;
	std::wstring Sandhi;
	std::wstring LitColReading;
	std::wstring GetPronType() const;
	InfoProperties Properties;
};

enum class StatusHintColumn {
	None     = 0,
	Jyutping = 1 << 0,
	Reverse  = 1 << 1,
	Eng      = 1 << 2,
	Urd      = 1 << 3,
	Nep      = 1 << 4,
	Hin      = 1 << 5,
	Ind      = 1 << 6,
};

using StatusHintSetting = int;

class MultiHintPanel
{
public:
 	static MultiHintPanel* GetInstance();
	void setMultiHintOptions(const UIStyle& style);
	bool isHintEnabled(int column) const;
	bool isHintEnabled(StatusHintColumn column) const;
	bool isAnyLanguageEnabled() const;
	bool isDictionaryEntry(InfoMultiHint* info) const;
	bool neverShowRomanization() const { return neverShowRomanization_; }
	bool shouldShowDictionary() const { return shouldShowDictionary_; }
	void setShowDictionary(bool show) { shouldShowDictionary_ = show; }
private:
	MultiHintPanel() {}
	~MultiHintPanel() {}
	StatusHintSetting settingsStatus_ = (int)StatusHintColumn::None;
	bool neverShowRomanization_ = false;
	bool shouldShowDictionary_ = false;
	// Private static pointer
	static MultiHintPanel* volatile instance;
};
