#include "stdafx.h"
#include "HintSettings.h"

HintSettings::HintSettings()
{
	api_ = (RimeLeversApi*)rime_get_api()->find_module("levers")->get_api();
	settings_ = api_->custom_settings_init("weasel", "Weasel::HintSettings");
}

bool HintSettings::GetLanguageList(std::vector<HintSettingsInfo> *result)
{
	if(!result) return false;
	result->clear();
	RimeConfig config = {0};
	api_->settings_get_config(settings_, &config);
	RimeApi* rime = rime_get_api();
	RimeConfigIterator available = {0};
	if(!rime->config_begin_list(&available, &config, "language_list"))
	{
		return false;
	}
	char textBuf[96];
	while(rime->config_next(&available))
	{
		HintSettingsInfo info;
		rime->config_get_string(&config, available.path, textBuf, 96);
		info.column_name = std::string(textBuf);
		result->push_back(info);
	}
  return true;
}

bool HintSettings::GetActiveLanguages(std::vector<HintSettingsInfo> *result)
{
	if(!result) return false;
	result->clear();
	RimeConfig config = {0};
	char str[512];
	api_->settings_get_config(settings_, &config);
	RimeApi* rime = rime_get_api();
	rime->config_get_string(&config, "style/language_list", str, 512);
	
	// split str by ',' 
 	std::string lList(str);
	std::string delimiter = ",";
	size_t pos = 0;
	HintSettingsInfo info;
	auto updateInfo = [&info](const std::string& column_name){ 
		info.column_name = column_name;
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

bool HintSettings::SetLanguageList(const std::vector<HintSettingsInfo> &result)
{
	std::string settingStr = "";
	for (auto& info : result) {
		// concat settingStr and info.column_name
		settingStr += info.column_name;
		settingStr += ",";
	}
	settingStr.pop_back();
	bool created = api_->customize_string(settings_, "style/language_list", settingStr.c_str());
  return true;
}
