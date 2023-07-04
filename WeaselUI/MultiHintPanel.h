#pragma once

#include <WeaselCommon.h>
#include <boost/tokenizer.hpp>
#include <locale>
#include <codecvt>

struct InfoLanguage {
	std::string Urd;
	std::string Nep;
	std::string Hin;
	std::string Ind;
};

struct InfoDefinition {
	std::string English;
	std::string Disambiguation;
	std::string Pos;
	std::string Register;
	std::string Label;
	std::string Written;
	std::string Colloquial;
	InfoLanguage Language;
};

struct InfoMultiHint {
	InfoMultiHint(const std::string& input);
	std::string Honzi;
	std::string Jyutping;
	std::string PronOrder;
	std::string Sandhi;
	std::string LitColReading;
	std::string Freq;
	std::string Freq2;
	InfoDefinition Definition; // std::vector<std::string> if necessary
};

enum class StatusHintColumn {
	None            = 0,
	Jyutping        = 1 << 0,
	English         = 1 << 1,
	Disambiguation  = 1 << 2,
	PartOfSpeech    = 1 << 3,
	Register        = 1 << 4,
	Label           = 1 << 5,
	Written         = 1 << 6,
	Colloquial      = 1 << 7,
	Urd             = 1 << 8,
	Nep             = 1 << 9,
	Hin             = 1 << 10,
	Ind             = 1 << 11
};

using StatusHintSetting = int;

class MultiHintPanel
{
public:
 	static MultiHintPanel* GetInstance();
	bool containsCsv(weasel::Text& comment);
	bool containsCsv(const std::wstring& comment);
	void applyMultiHint(weasel::Text& comment);
	// without safety check
	std::wstring getMultiHint(const std::wstring& comment);
	std::wstring getJyutping(const std::wstring& comment);
	void setMultiHintOptions(const std::wstring& settings);
	std::string getHint(const InfoMultiHint& info, const StatusHintSetting status) const;
	bool isHintEnabled(StatusHintColumn column) const ;
	bool isEnable() const { return settingsStatus_ != 0; }
private:
	MultiHintPanel() : settingsStatus_(0) {}
	~MultiHintPanel() {}
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter_;
	StatusHintSetting settingsStatus_;
	// Private static pointer
	static MultiHintPanel* volatile instance;
};
