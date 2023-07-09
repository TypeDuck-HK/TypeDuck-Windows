#pragma once

#include <WeaselCommon.h>
#include <boost/tokenizer.hpp>

struct InfoDefinition {
	std::wstring Eng;
	std::wstring Urd;
	std::wstring Nep;
	std::wstring Hin;
	std::wstring Ind;
};

struct InfoProperties {
	std::wstring PartOfSpeech;
	std::wstring Register;
	std::wstring Label;
	std::wstring Normalized;
	std::wstring Written;
	std::wstring Colloquial;
	std::wstring Collocation;
	InfoDefinition Definition;
};

struct InfoMultiHint {
	InfoMultiHint(const std::wstring& input);
	std::wstring Jyutping;
	std::wstring Sandhi;
	std::wstring LitColReading;
	InfoProperties Properties;
};

enum class StatusHintColumn {
	None     = 0,
	Jyutping = 1 << 1,
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
	bool containsCSV(const std::wstring& comment) const;
	void setMultiHintOptions(const std::wstring& settings);
	std::wstring getHint(const std::wstring& comment) const;
	bool isHintEnabled(int column) const;
	bool isHintEnabled(StatusHintColumn column) const;
	bool isEnabled() const { return settingsStatus_ != 0; }
private:
	MultiHintPanel() : settingsStatus_(0) {}
	~MultiHintPanel() {}
	StatusHintSetting settingsStatus_;
	// Private static pointer
	static MultiHintPanel* volatile instance;
};
