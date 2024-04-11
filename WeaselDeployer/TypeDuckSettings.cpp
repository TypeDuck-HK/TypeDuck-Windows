#include "stdafx.h"
#include "TypeDuckSettings.h"

#define PAGE_SIZE_KEY "menu/page_size"
#define DISPLAY_LANGUAGES_KEY "typeduck/display_languages"
#define SHOW_ROMANIZATION_KEY "typeduck/show_romanization"
#define INLINE_PREEDIT_KEY "style/inline_preedit"
#define ENABLE_COMPLETION_KEY "typeduck/enable_completion"
#define ENABLE_CORRECTION_KEY "typeduck/enable_correction"
#define ENABLE_SENTENCE_KEY "typeduck/enable_sentence"
#define ENABLE_LEARNING_KEY "typeduck/enable_learning"
#define SHOW_REVERSE_CODE_KEY "typeduck/show_reverse_code"
#define USE_CANGJIE5_KEY "typeduck/use_cangjie5"

#define PATCH_DIRECTIVE "__patch"
#define SHOW_CANGJIE_ROOTS_VALUE "common:/show_cangjie_roots"
#define DISABLE_COMPLETION_VALUE "common:/disable_completion"
#define ENABLE_CORRECTION_VALUE "common:/enable_correction"
#define DISABLE_SENTENCE_VALUE "common:/disable_sentence"
#define DISABLE_LEARNING_VALUE "common:/disable_learning"
#define USE_CANGJIE3_VALUE "common:/use_cangjie3"

TypeDuckSettings::TypeDuckSettings(RimeLeversApi* api) : api_(api)
{
	default_settings_ = api_->custom_settings_init("default", "Weasel::TypeDuckSettings");
	settings_ = api_->custom_settings_init("weasel", "Weasel::TypeDuckSettings");
	common_settings_ = api_->custom_settings_init("common", "Weasel::TypeDuckSettings");
}

std::vector<bool> TypeDuckSettings::GetActiveLanguages()
{
	RimeConfig config = { 0 };
	api_->settings_get_config(settings_, &config);
	RimeApi* rime = rime_get_api();
	RimeConfigIterator it = { 0 };
	if (!rime->config_begin_list(&it, &config, DISPLAY_LANGUAGES_KEY))
		return DEFAULT_DISPLAY_LANGUAGES;
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
	Bool success = true;
	size_t j = 0;
	RimeApi* rime = rime_get_api();
	RimeConfig config = { 0 };
	success &= rime->config_init(&config);
	success &= rime->config_create_list(&config, "");
	for (size_t i = 0; i < LanguageConfigSize; ++i) {
		if (result[i]) success &= rime->config_list_append_string(&config, "", LanguageConfigNameList[i]);
	}
	rime->config_set_item(&config, "", &config);
	success &= api_->customize_item(settings_, DISPLAY_LANGUAGES_KEY, &config);
	return !!success;
}

bool TypeDuckSettings::Load()
{
	candidateCount = 6;
	showRomanization = 0;
	inputCodeInWindow = true;
	enableCompletion = true;
	enableCorrection = false;
	enableSentence = true;
	enableLearning = true;
	showReverseCode = true;
	isCangjie5 = true;

	Bool success = true;
	RimeConfig default_config = { 0 };
	api_->settings_get_config(default_settings_, &default_config);
	RimeApi* rime = rime_get_api();
	success &= rime->config_get_int(&default_config, PAGE_SIZE_KEY, &candidateCount);

	RimeConfig config = { 0 };
	api_->settings_get_config(settings_, &config);
	char buffer[32];
	success &= rime->config_get_string(&config, SHOW_ROMANIZATION_KEY, buffer, sizeof(buffer) - 1);
	for (size_t i = 0; i < ShowRomanizationSize; ++i) {
		if (!strcmp(buffer, ShowRomanizationValues[i])) {
			showRomanization = i;
			break;
		}
	}

	Bool inlinePreedit = false;
	success &= rime->config_get_bool(&config, INLINE_PREEDIT_KEY, &inlinePreedit);
	inputCodeInWindow = !inlinePreedit;

	success &= rime->config_get_bool(&config, ENABLE_COMPLETION_KEY, &enableCompletion);
	success &= rime->config_get_bool(&config, ENABLE_CORRECTION_KEY, &enableCorrection);
	success &= rime->config_get_bool(&config, ENABLE_SENTENCE_KEY, &enableSentence);
	success &= rime->config_get_bool(&config, ENABLE_LEARNING_KEY, &enableLearning);
	success &= rime->config_get_bool(&config, SHOW_REVERSE_CODE_KEY, &showReverseCode);
	success &= rime->config_get_bool(&config, USE_CANGJIE5_KEY, &isCangjie5);
	return !!success;
}

bool TypeDuckSettings::Save()
{
	Bool success = true;
	success &= api_->customize_int(default_settings_, PAGE_SIZE_KEY, candidateCount);
	success &= api_->customize_string(settings_, SHOW_ROMANIZATION_KEY, ShowRomanizationValues[showRomanization]);
	success &= api_->customize_bool(settings_, INLINE_PREEDIT_KEY, !inputCodeInWindow);
	success &= api_->customize_bool(settings_, ENABLE_COMPLETION_KEY, enableCompletion);
	success &= api_->customize_bool(settings_, ENABLE_CORRECTION_KEY, enableCorrection);
	success &= api_->customize_bool(settings_, ENABLE_SENTENCE_KEY, enableSentence);
	success &= api_->customize_bool(settings_, ENABLE_LEARNING_KEY, enableLearning);
	success &= api_->customize_bool(settings_, SHOW_REVERSE_CODE_KEY, showReverseCode);
	success &= api_->customize_bool(settings_, USE_CANGJIE5_KEY, isCangjie5);
	size_t j = 0;
	RimeApi* rime = rime_get_api();
	RimeConfig config = { 0 };
	success &= rime->config_init(&config);
	success &= rime->config_create_list(&config, "");
	                       success &= rime->config_list_append_string(&config, "", SHOW_CANGJIE_ROOTS_VALUE);
	if (!enableCompletion) success &= rime->config_list_append_string(&config, "", DISABLE_COMPLETION_VALUE);
	if (enableCorrection)  success &= rime->config_list_append_string(&config, "", ENABLE_CORRECTION_VALUE);
	if (!enableSentence)   success &= rime->config_list_append_string(&config, "", DISABLE_SENTENCE_VALUE);
	if (!enableLearning)   success &= rime->config_list_append_string(&config, "", DISABLE_LEARNING_VALUE);
	if (!isCangjie5)       success &= rime->config_list_append_string(&config, "", USE_CANGJIE3_VALUE);
	rime->config_set_item(&config, "", &config);
	success &= api_->customize_item(common_settings_, PATCH_DIRECTIVE, &config);
	return !!success;
}
