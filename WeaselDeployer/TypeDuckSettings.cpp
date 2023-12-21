#include "stdafx.h"
#include "TypeDuckSettings.h"

constexpr static char* DISPLAY_LANGUAGES_KEY = "typeduck/display_languages";
constexpr static char* SHOW_ROMANIZATION_KEY = "typeduck/show_romanization";
constexpr static char* SHOW_REVERSE_CODE_KEY = "typeduck/show_reverse_code";
constexpr static char* SHOW_CANGJIE_ROOTS_VALUE = "common:/show_cangjie_roots";
constexpr static char* ENABLE_CORRECTION_VALUE = "common:/enable_correction";
constexpr static char* USE_CANGJIE3_VALUE = "common:/use_cangjie3";

static inline char* toIndexKey(const size_t i)
{
	char buffer[8] {};
	snprintf(buffer, sizeof(buffer), "@%d", i);
	return buffer;
}

TypeDuckSettings::TypeDuckSettings(RimeLeversApi* api) : api_(api)
{
	settings_ = api_->custom_settings_init("weasel", "Weasel::TypeDuckSettings");
	custom_settings_ = api_->custom_settings_init("common", "Weasel::TypeDuckSettings");
}

std::vector<bool> TypeDuckSettings::GetActiveLanguages()
{
	RimeConfig config = { 0 };
	api_->settings_get_config(settings_, &config);
	RimeApi* rime = rime_get_api();
	RimeConfigIterator it = { 0 };
	if (!rime->config_begin_list(&it, &config, DISPLAY_LANGUAGES_KEY))
		return { true, false, false, false, false };
	std::unordered_set<std::string> columns;
	while (rime->config_next(&it)) {
		char buffer[512];
		if (rime->config_get_string(&config, it.path, buffer, sizeof(buffer) - 1)) columns.insert(buffer);
	}
	rime->config_end(&it);
	std::vector<bool> languageConfig;
	for (size_t i = 0; i < LanguageConfigSize; ++i) {
		languageConfig.push_back(columns.find(LanguageConfigNameList[i]) != columns.end());
	}
	return languageConfig;
}

bool TypeDuckSettings::SetLanguageList(const std::vector<bool> result)
{
	bool success = true;
	size_t j = 0;
	RimeApi* rime = rime_get_api();
	RimeConfig config = { 0 };
	success &= rime->config_init(&config);
	success &= rime->config_create_list(&config, "");
	for (size_t i = 0; i < LanguageConfigSize; ++i) {
		if (!result[i])
		{
			continue;
		}
		success &= rime->config_list_append_string(&config, "", LanguageConfigNameList[i]);
	}
	int languageListSize = rime->config_list_size(&config, "");
	rime->config_set_item(&config, "", &config);
	success &= api_->customize_item(settings_, DISPLAY_LANGUAGES_KEY, &config);
	return success;
}

bool TypeDuckSettings::Load()
{
	showRomanization = 0;
	showReverseCode = true;
	enableCorrection = false;
	isCangjie5 = true;

	bool success = true;
	RimeConfig config = { 0 };
	api_->settings_get_config(settings_, &config);
	RimeApi* rime = rime_get_api();
	char buffer[32];
	success &= rime->config_get_string(&config, SHOW_ROMANIZATION_KEY, buffer, sizeof(buffer) - 1);
	for (size_t i = 0; i < ShowRomanizationSize; ++i) {
		if (!strcmp(buffer, ShowRomanizationValues[i])) {
			showRomanization = i;
			break;
		}
	}
	Bool temp = True;
	if (rime->config_get_bool(&config, SHOW_REVERSE_CODE_KEY, &temp)) showReverseCode = !!temp;
	else success = false;

	RimeConfig customConfig = { 0 };
	api_->settings_get_config(custom_settings_, &customConfig);
	RimeConfigIterator it = { 0 };
	success &= rime->config_begin_list(&it, &customConfig, "");
	while (rime->config_next(&it)) {
		char buffer[512];
		if (!rime->config_get_string(&customConfig, it.path, buffer, sizeof(buffer) - 1)) continue;
		if (!strcmp(buffer, ENABLE_CORRECTION_VALUE)) enableCorrection = true;
		else if (!strcmp(buffer, USE_CANGJIE3_VALUE)) isCangjie5 = false;
	}
	rime->config_end(&it);
	return success;
}

bool TypeDuckSettings::Save()
{
	bool success = true;
	success &= api_->customize_string(settings_, SHOW_ROMANIZATION_KEY, ShowRomanizationValues[showRomanization]);
	success &= api_->customize_bool(settings_, SHOW_REVERSE_CODE_KEY, showReverseCode);
	size_t j = 0;
	RimeApi* rime = rime_get_api();
	RimeConfig config = { 0 };
	success &= rime->config_init(&config);
	success &= rime->config_create_list(&config, "");
	success &= rime->config_list_append_string(&config, "", SHOW_CANGJIE_ROOTS_VALUE);
  if (enableCorrection) success &= rime->config_list_append_string(&config, "", ENABLE_CORRECTION_VALUE);
	if (!isCangjie5) success &= rime->config_list_append_string(&config, "" , USE_CANGJIE3_VALUE);
	rime->config_set_item(&config, "", &config);
	success &= api_->customize_item(custom_settings_, "common/__patch", &config);
	return success;
}
