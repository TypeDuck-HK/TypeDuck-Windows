#include "stdafx.h"
#include "HintSettings.h"
#include <WeaselUtility.h>
#include <StringAlgorithm.hpp>

const static std::wstring LanguageList[] {
	L"Jyutping",
	L"Code in Reverse Lookup",
	L"English",
	L"Hindi",
	L"Indonesian",
	L"Urdu",
	L"Nepali",
};

const static std::string LanguageConfigNameList[] = {
	"Jyutping",
	"Reverse",
	"Eng",
	"Hin",
	"Ind",
	"Urd",
	"Nep",
};

const int LanguageConfigSize = sizeof(LanguageConfigNameList) / sizeof(std::string);

HintSettings::HintSettings()
{
	api_ = (RimeLeversApi*)rime_get_api()->find_module("levers")->get_api();
	settings_ = api_->custom_settings_init("weasel", "Weasel::HintSettings");
}

std::vector<bool> HintSettings::GetActiveLanguages()
{
	RimeConfig config = { 0 };
	char buffer[511];
	api_->settings_get_config(settings_, &config);
	RimeApi* rime = rime_get_api();
	rime->config_get_string(&config, "style/language_list", buffer, 512);
	std::wstring str = utf8towcs(buffer);
	str = std::regex_replace(str, std::wregex(L"\\s*,\\s*"), L",");
	str = std::regex_replace(str, std::wregex(L"^\\s*|\\s*$"), L"");
	std::unordered_set<std::wstring> columns = ws_split(str, L",");

	std::vector<bool> languageConfig;
	for (int i = 0; i < LanguageConfigSize; ++i) {
		languageConfig.push_back(columns.find(utf8towcs(LanguageConfigNameList[i].c_str())) != columns.end());
	}
	return languageConfig;
}

bool HintSettings::SetLanguageList(const std::vector<bool> result)
{
	std::string settingStr = "";
	for (int i = 0; i < LanguageConfigSize; ++i) {
		if (result[i]) settingStr += LanguageConfigNameList[i] + ", ";
	}
	settingStr.pop_back();
	settingStr.pop_back();
	return api_->customize_string(settings_, "style/language_list", settingStr.c_str());
}
