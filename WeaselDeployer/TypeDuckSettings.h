#pragma once
#include <rime_levers_api.h>
#include <string>
#include <vector>
#include <unordered_set>

#define DEFAULT_DISPLAY_LANGUAGES { true, false, false, false, false }

constexpr static wchar_t* LanguageList[] {
	L"英語 English",
	L"印地語 Hindi",
	L"印尼語 Indonesian",
	L"烏爾都語 Urdu",
	L"尼泊爾語 Nepali",
};

constexpr static char* LanguageConfigNameList[] = {
	"Eng",
	"Hin",
	"Ind",
	"Urd",
	"Nep",
};

constexpr static int LanguageConfigSize = sizeof(LanguageConfigNameList) / sizeof(char*);

constexpr static char* ShowRomanizationValues[] = {
	"always",
	"reverse_only",
	"never",
};

constexpr static int ShowRomanizationSize = sizeof(ShowRomanizationValues) / sizeof(char*);

class TypeDuckSettings
{
public:
	TypeDuckSettings(RimeLeversApi* api);
	
	RimeCustomSettings* settings() { return settings_; }
	RimeCustomSettings* custom_settings() { return custom_settings_; }
	std::vector<bool> GetActiveLanguages();
	bool SetLanguageList(const std::vector<bool> result);
	bool Load();
	bool Save();

	int showRomanization = 0;
	Bool inputCodeInWindow = true;
	Bool enableCompletion = true;
	Bool enableCorrection = false;
	Bool enableSentence = true;
	Bool enableLearning = true;
	Bool showReverseCode = true;
	Bool isCangjie5 = true;

private:
	RimeLeversApi* api_;
	RimeCustomSettings* settings_;
	RimeCustomSettings* custom_settings_;
};
