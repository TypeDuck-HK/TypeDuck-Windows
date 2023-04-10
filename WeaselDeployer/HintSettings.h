#pragma once
#include <rime_levers_api.h>
#include <string>
#include <vector>

struct HintSettingsInfo {
	std::string column_name;
};

class HintSettings
{
public:
	HintSettings();
	
	RimeCustomSettings* settings() { return settings_; }
	bool GetLanguageList(std::vector<HintSettingsInfo>* result);
	bool GetActiveLanguages(std::vector<HintSettingsInfo>* result);
	bool SetLanguageList(const std::vector<HintSettingsInfo>& result);
	
private:
	RimeLeversApi* api_;
	RimeCustomSettings* settings_;
};

