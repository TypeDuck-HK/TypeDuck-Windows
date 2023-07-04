#pragma once

#include <WeaselCommon.h>
#include <boost/tokenizer.hpp>
#include <locale>
#include <codecvt>

struct InfoDefinition {
	std::string Eng;
	std::string Urd;
	std::string Nep;
	std::string Hin;
	std::string Ind;
};

struct InfoProperties {
	std::string PartOfSpeech;
	std::string Register;
	std::string Label;
	std::string Normalized;
	std::string Written;
	std::string Colloquial;
	std::string Collocation;
	InfoDefinition Definition;
};

struct InfoMultiHint {
	InfoMultiHint(const std::string& input);
	std::string Honzi;
	std::string Jyutping;
	std::string Sandhi;
	std::string LitColReading;
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
