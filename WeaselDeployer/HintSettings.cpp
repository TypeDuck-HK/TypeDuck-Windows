#include "stdafx.h"
#include "HintSettings.h"
#include <WeaselUtility.h>

static std::wstring LanguageList[] {
		L"JyutPing ¸f«÷", L"English", L"Disambiguatory Information",
		L"Part Of Speech", L"Register", L"Label",
		L"Written", L"Colloquial", L"Urdu",
		L"Nepel", L"Hindi", L"Indian"
};

static std::string LanguageConfigNameList[] = {
	"Jyutping", "English", "Disambiguatory Information",
	"Part of Speech","Register","Label",
	"Written", "Colloquial","Urd",
	"Nep","Hin","Ind"
};
int LanguageConfigSize = sizeof(LanguageConfigNameList) / sizeof(std::string);
HintSettings::HintSettings()
{
	api_ = (RimeLeversApi*)rime_get_api()->find_module("levers")->get_api();
	settings_ = api_->custom_settings_init("weasel", "Weasel::HintSettings");
}

bool HintSettings::GetLanguageList(std::vector<HintSettingsInfo>* result)
{
	if (!result) return false;
	result->clear();
	dumpLanguageList(result);
	return true;
}

bool HintSettings::GetActiveLanguages(std::vector<HintSettingsInfo>* result)
{
	if (!result) return false;
	result->clear();
	RimeConfig config = { 0 };
	char str[512];
	api_->settings_get_config(settings_, &config);
	RimeApi* rime = rime_get_api();
	rime->config_get_string(&config, "style/language_list", str, 512);

	// split str by ',' 
	std::string lList = "";
	std::string delimiter = ",";
	// temporarily default Jyutping
	lList = LanguageConfigNameList[0] + delimiter + str;
	size_t pos = 0;
	HintSettingsInfo info;
	auto updateInfo = [&info](const std::string& column_name) {
		info.column_name = utf8towcs(column_name.c_str());
		auto target = std::find(std::begin(LanguageConfigNameList), 
								 std::end(LanguageConfigNameList), column_name);
		int index = target == std::end(LanguageConfigNameList) ? 
								-1 : std::distance(std::begin(LanguageConfigNameList), target);
		info.settingIndex = index; 
	};
	while ((pos = lList.find(delimiter)) != std::string::npos) {
		updateInfo(lList.substr(0, pos));
		result->push_back(info);
		lList.erase(0, pos + delimiter.length());
	}
	if (lList.size() != 0)
	{
		updateInfo(lList);
		result->push_back(info);
	}
	return true;
}

bool HintSettings::SetLanguageList(const std::vector<HintSettingsInfo>& result)
{
	std::string settingStr = " ";
	int sizeOfConfigName = std::size(LanguageConfigNameList);
	for (auto& info : result) {
		// concat settingStr and info.column_name
		if (info.settingIndex < 0 || info.settingIndex > sizeOfConfigName){
			continue;
		}
		settingStr += LanguageConfigNameList[info.settingIndex];
		settingStr += ",";
	}
	settingStr.pop_back();
	bool created = api_->customize_string(settings_, "style/language_list", settingStr.c_str());
	return true;
}

void HintSettings::dumpLanguageList(std::vector<HintSettingsInfo>* result)
{
	//language_list: ["Jyutping","English","Disambiguatory Information","Part of Speech","Register","Label","Written","Colloquial","Urd","Nep","Hin","Ind"]
	if (!result)
	{
		return;
	}
	HintSettingsInfo info;

	// Jyutping
	info.column_name = LanguageList[0];
	info.disable = true;
	info.settingIndex = 0;
	result->push_back(info);
	int size = std::size(LanguageList);
	for(int i = 1; i < size; ++i) {
		info.column_name = LanguageList[i];
		info.disable = false;
		info.settingIndex = i;
		result->push_back(info);
	}	
}
