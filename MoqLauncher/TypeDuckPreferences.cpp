#include "TypeDuckPreferences.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <set>

#include <json/json.h>

namespace Moqi::TypeDuck {

namespace {

constexpr const char* kPreferencesFileName = "TypeDuckPreferences.json";
constexpr const char* kJsonSource = "TypeDuckPreferences.json";
constexpr const char* kApplyFailureMessage =
    "設定未能套用，已保留原有設定 / Settings could not be applied; existing settings were kept";

const std::vector<std::string>& languageOrder() {
  static const std::vector<std::string> languages{"eng", "hin", "ind", "nep", "urd"};
  return languages;
}

bool isKnownLanguage(const std::string& language) {
  const auto& languages = languageOrder();
  return std::find(languages.begin(), languages.end(), language) != languages.end();
}

bool isKnownRomanizationMode(const std::string& value) {
  return value == "always" || value == "reverse_only" || value == "never";
}

Json::Value preferencesToJson(const Preferences& preferences) {
  Json::Value root;
  Json::Value languages(Json::arrayValue);
  for (const auto& language : preferences.displayLanguages) {
    languages.append(language);
  }
  root["displayLanguages"] = languages;
  root["mainLanguage"] = preferences.mainLanguage;
  root["pageSize"] = preferences.pageSize;
  root["isHeiTypeface"] = preferences.isHeiTypeface;
  root["showRomanization"] = preferences.showRomanization;
  root["enableCompletion"] = preferences.enableCompletion;
  root["enableCorrection"] = preferences.enableCorrection;
  root["enableSentence"] = preferences.enableSentence;
  root["enableLearning"] = preferences.enableLearning;
  root["showReverseCode"] = preferences.showReverseCode;
  root["isCangjie5"] = preferences.isCangjie5;
  return root;
}

Preferences preferencesFromJson(const Json::Value& root) {
  Preferences preferences = defaultPreferences();
  if (root["displayLanguages"].isArray()) {
    preferences.displayLanguages.clear();
    for (const auto& item : root["displayLanguages"]) {
      if (item.isString()) {
        preferences.displayLanguages.push_back(item.asString());
      }
    }
  }
  if (root["mainLanguage"].isString()) {
    preferences.mainLanguage = root["mainLanguage"].asString();
  }
  if (root["pageSize"].isInt()) {
    preferences.pageSize = root["pageSize"].asInt();
  }
  if (root["isHeiTypeface"].isBool()) {
    preferences.isHeiTypeface = root["isHeiTypeface"].asBool();
  }
  if (root["showRomanization"].isString()) {
    preferences.showRomanization = root["showRomanization"].asString();
  }
  if (root["enableCompletion"].isBool()) {
    preferences.enableCompletion = root["enableCompletion"].asBool();
  }
  if (root["enableCorrection"].isBool()) {
    preferences.enableCorrection = root["enableCorrection"].asBool();
  }
  if (root["enableSentence"].isBool()) {
    preferences.enableSentence = root["enableSentence"].asBool();
  }
  if (root["enableLearning"].isBool()) {
    preferences.enableLearning = root["enableLearning"].asBool();
  }
  if (root["showReverseCode"].isBool()) {
    preferences.showReverseCode = root["showReverseCode"].asBool();
  }
  if (root["isCangjie5"].isBool()) {
    preferences.isCangjie5 = root["isCangjie5"].asBool();
  }
  return preferences;
}

std::string roamingAppDataPath() {
  const char* value = std::getenv("APPDATA");
  return value != nullptr ? value : "";
}

} // namespace

Preferences defaultPreferences() {
  return Preferences{
      {"eng"},
      "eng",
      6,
      false,
      "always",
      true,
      false,
      true,
      true,
      true,
      true,
  };
}

std::vector<PreferenceDescriptor> preferenceDescriptors() {
  return {
      {"displayLanguages", "顯示語言 Display Languages", "radio-checkbox-list", false},
      {"pageSize", "每頁候選詞數量 No. of Candidates Per Page", "range", true},
      {"isHeiTypeface", "中文字體 Chinese Typeface", "segmented", false},
      {"showRomanization", "候選詞粵拼 Candidates Jyutping", "radio-list", false},
      {"enableCompletion", "自動完成 Auto-completion", "toggle", true},
      {"enableCorrection", "自動校正 Auto-correction", "toggle", true},
      {"enableSentence", "自動組詞 Auto-composition", "toggle", true},
      {"enableLearning", "輸入記憶 Input Memory", "toggle", true},
      {"reverseLookupHeader", "反查設定 Reverse Lookup Settings", "section", false},
      {"showReverseCode", "顯示完整輸入碼 Show Full Input Code", "toggle", false},
      {"isCangjie5", "倉頡版本 Cangjie Version", "segmented", true},
  };
}

std::vector<CapabilityMetadata> defaultCapabilities() {
  return {
      {"pageSize", true, ""},
      {"enableCompletion", true, ""},
      {"enableCorrection", true, ""},
      {"enableSentence", true, ""},
      {"enableLearning", true, ""},
      {"isCangjie5", true, ""},
      {"showReverseCode", true, ""},
      {"displayLanguages", true, ""},
      {"mainLanguage", true, ""},
      {"isHeiTypeface", true, ""},
      {"showRomanization", true, ""},
  };
}

std::vector<std::string> interfaceOnlyPreferenceIdsForTest() {
  return {"displayLanguages", "mainLanguage", "isHeiTypeface",
          "showRomanization", "showReverseCode"};
}

bool preferenceAffectsRime(const std::string& id) {
  return id == "pageSize" || id == "enableCompletion" ||
         id == "enableCorrection" || id == "enableSentence" ||
         id == "enableLearning" || id == "isCangjie5";
}

ValidationResult validatePreferences(const Preferences& preferences) {
  Preferences normalized = preferences;
  bool ok = true;
  std::string message;

  std::vector<std::string> languages;
  std::set<std::string> seen;
  for (const auto& language : preferences.displayLanguages) {
    if (!isKnownLanguage(language)) {
      ok = false;
      continue;
    }
    if (seen.insert(language).second) {
      languages.push_back(language);
    }
  }
  if (languages.empty()) {
    ok = false;
    languages = {"eng"};
  }
  normalized.displayLanguages = languages;

  if (!isKnownLanguage(normalized.mainLanguage)) {
    ok = false;
    normalized.mainLanguage = "eng";
  }
  if (std::find(normalized.displayLanguages.begin(),
                normalized.displayLanguages.end(),
                normalized.mainLanguage) == normalized.displayLanguages.end()) {
    normalized.displayLanguages.push_back(normalized.mainLanguage);
  }

  if (normalized.pageSize < 4 || normalized.pageSize > 10) {
    ok = false;
    normalized.pageSize = 6;
  }
  if (!isKnownRomanizationMode(normalized.showRomanization)) {
    ok = false;
    normalized.showRomanization = "always";
  }

  if (!ok) {
    message = "設定值無效，已使用預設值 / Invalid settings were replaced with defaults";
  }
  return {ok, normalized, message};
}

RimeSideEffects rimeSideEffects(const Preferences& preferences) {
  const auto normalized = validatePreferences(preferences).preferences;
  RimeSideEffects effects;
  effects.defaultCustomFile = "default.custom.yaml";
  effects.defaultCustomPath = "menu/page_size";
  effects.pageSize = normalized.pageSize;
  effects.enableCompletion = normalized.enableCompletion;
  effects.enableCorrection = normalized.enableCorrection;
  effects.enableSentence = normalized.enableSentence;
  effects.enableLearning = normalized.enableLearning;
  effects.isCangjie5 = normalized.isCangjie5;
  effects.commonCustomFile = "common.custom.yaml";
  effects.commonPatchKey = "__patch";
  effects.commonPatches.push_back("common:/show_cangjie_roots");
  if (!normalized.enableCompletion) {
    effects.commonPatches.push_back("common:/disable_completion");
  }
  if (normalized.enableCorrection) {
    effects.commonPatches.push_back("common:/enable_correction");
  }
  if (!normalized.enableSentence) {
    effects.commonPatches.push_back("common:/disable_sentence");
  }
  if (!normalized.enableLearning) {
    effects.commonPatches.push_back("common:/disable_learning");
  }
  if (!normalized.isCangjie5) {
    effects.commonPatches.push_back("common:/use_cangjie3");
  }
  return effects;
}

std::filesystem::path defaultPreferencesPath() {
  const auto roaming = roamingAppDataPath();
  if (roaming.empty()) {
    return std::filesystem::path(kPreferencesFileName);
  }
  return std::filesystem::path(roaming) / "TypeDuckIME" / kPreferencesFileName;
}

ValidationResult loadPreferences(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream.is_open()) {
    auto defaults = defaultPreferences();
    return {true, defaults, ""};
  }

  Json::CharReaderBuilder builder;
  Json::Value root;
  std::string errors;
  if (!Json::parseFromStream(builder, stream, &root, &errors) || !root.isObject()) {
    return {false, defaultPreferences(),
            "設定檔無效，已使用預設值 / Settings file is invalid; defaults were loaded"};
  }
  auto result = validatePreferences(preferencesFromJson(root));
  if (!result.ok && result.message.empty()) {
    result.message =
        "設定檔無效，已使用預設值 / Settings file is invalid; defaults were loaded";
  }
  return result;
}

SaveResult savePreferences(const std::filesystem::path& path,
                           const Preferences& preferences) {
  const auto result = validatePreferences(preferences);
  if (!result.ok) {
    return {false, result.message};
  }
  std::error_code ec;
  std::filesystem::create_directories(path.parent_path(), ec);
  if (ec) {
    return {false, kApplyFailureMessage};
  }

  Json::StreamWriterBuilder builder;
  builder["indentation"] = "  ";
  std::ofstream stream(path, std::ios::binary | std::ios::trunc);
  if (!stream.is_open()) {
    return {false, kApplyFailureMessage};
  }
  stream << Json::writeString(builder, preferencesToJson(result.preferences));
  if (!stream.good()) {
    return {false, kApplyFailureMessage};
  }
  return {true, kJsonSource};
}

ApplyResult applyPreferences(const std::filesystem::path& path,
                             const Preferences& preferences,
                             RimeApplier applyRime) {
  const auto validation = validatePreferences(preferences);
  if (!validation.ok) {
    return {false, validation.message};
  }
  const auto saved = savePreferences(path, validation.preferences);
  if (!saved.ok) {
    return {false, saved.message};
  }
  if (applyRime) {
    const auto applied = applyRime(rimeSideEffects(validation.preferences));
    if (!applied.ok) {
      return {false, applied.message.empty() ? kApplyFailureMessage : applied.message};
    }
    return {true, applied.message.empty() ? "設定已套用 / Settings applied" : applied.message};
  }
  return {true, "設定已儲存 / Settings saved"};
}

} // namespace Moqi::TypeDuck
