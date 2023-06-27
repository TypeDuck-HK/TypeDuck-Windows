#pragma once
#include <rime_levers_api.h>
#include <string>
#include <vector>
std::wstring LanguageList[];
std::string LanguageConfigNameList[];

struct HintSettingsInfo {
	std::wstring column_name;
	bool disable;
	unsigned int settingIndex;
	HintSettingsInfo() : disable(false),settingIndex(0) {}
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
  void dumpLanguageList(std::vector<HintSettingsInfo>* result);
	RimeLeversApi* api_;
	RimeCustomSettings* settings_;
};

