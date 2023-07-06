#pragma once
#include <rime_levers_api.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <map>
#include <regex>

extern const std::wstring LanguageList[];
extern const std::string LanguageConfigNameList[];
extern const int LanguageConfigSize;

class HintSettings
{
public:
	HintSettings();
	
	RimeCustomSettings* settings() { return settings_; }
	std::vector<bool> HintSettings::GetActiveLanguages();
	bool SetLanguageList(const std::vector<bool> result);

private:
	RimeLeversApi* api_;
	RimeCustomSettings* settings_;
};
