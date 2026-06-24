#include "gtest/gtest.h"

#include "MoqLauncher/TypeDuckPreferences.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::filesystem::path makeTempDir(const char* name) {
  auto root = std::filesystem::temp_directory_path() / name;
  std::filesystem::remove_all(root);
  std::filesystem::create_directories(root);
  return root;
}

std::string readFile(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(stream)),
                     std::istreambuf_iterator<char>());
}

} // namespace

TEST(TypeDuckPreferences, DefaultsMatchWebAlphaDecisionContract) {
  // D-17, D-20, D-21, D-42: Web DEFAULT_PREFERENCES in hooks.ts/consts.ts
  // are the source for native JSON defaults and Display Languages order.
  const auto prefs = Moqi::TypeDuck::defaultPreferences();
  EXPECT_EQ(prefs.displayLanguages, (std::vector<std::string>{"eng"}));
  EXPECT_EQ(prefs.mainLanguage, "eng");
  EXPECT_EQ(prefs.pageSize, 6);
  EXPECT_FALSE(prefs.isHeiTypeface);
  EXPECT_EQ(prefs.showRomanization, "always");
  EXPECT_TRUE(prefs.enableCompletion);
  EXPECT_FALSE(prefs.enableCorrection);
  EXPECT_TRUE(prefs.enableSentence);
  EXPECT_TRUE(prefs.enableLearning);
  EXPECT_TRUE(prefs.showReverseCode);
  EXPECT_TRUE(prefs.isCangjie5);

  const auto descriptors = Moqi::TypeDuck::preferenceDescriptors();
  ASSERT_FALSE(descriptors.empty());
  EXPECT_EQ(descriptors.front().id, "displayLanguages");
  EXPECT_EQ(descriptors.front().label, "顯示語言 Display Languages");
}

TEST(TypeDuckPreferences, PageSizeRangeAndDefaultCustomPatch) {
  // D-45: pageSize is the only preference mapped to default.custom.yaml,
  // and it must customize the Rime menu/page_size path.
  auto prefs = Moqi::TypeDuck::defaultPreferences();
  auto invalid = prefs;
  invalid.pageSize = 3;
  auto result = Moqi::TypeDuck::validatePreferences(invalid);
  EXPECT_FALSE(result.ok);
  EXPECT_EQ(result.preferences.pageSize, 6);

  for (int size = 4; size <= 10; ++size) {
    prefs.pageSize = size;
    result = Moqi::TypeDuck::validatePreferences(prefs);
    ASSERT_TRUE(result.ok) << size;
    const auto sideEffects = Moqi::TypeDuck::rimeSideEffects(result.preferences);
    EXPECT_EQ(sideEffects.defaultCustomPath, "menu/page_size");
    EXPECT_EQ(sideEffects.pageSize, size);
  }
}

TEST(TypeDuckPreferences, InterfaceOnlyPreferencesStayJsonOnly) {
  // D-43: interface-only settings mirror TypeDuck Web types.ts and must not
  // trigger common.custom.yaml or default.custom.yaml customization.
  const auto prefs = Moqi::TypeDuck::defaultPreferences();
  const auto interfaceOnly =
      Moqi::TypeDuck::interfaceOnlyPreferenceIdsForTest();
  EXPECT_EQ(interfaceOnly,
            (std::vector<std::string>{"displayLanguages", "mainLanguage",
                                      "isHeiTypeface", "showRomanization",
                                      "showReverseCode"}));
  for (const auto& id : interfaceOnly) {
    EXPECT_FALSE(Moqi::TypeDuck::preferenceAffectsRime(id)) << id;
  }
}

TEST(TypeDuckPreferences, EnginePreferencesProduceCommonPatchList) {
  // D-46: worker.ts/api.cpp/common.yaml map engine preferences into
  // common.custom.yaml __patch entries with Cangjie roots always present.
  auto prefs = Moqi::TypeDuck::defaultPreferences();
  prefs.enableCompletion = false;
  prefs.enableCorrection = true;
  prefs.enableSentence = false;
  prefs.enableLearning = false;
  prefs.isCangjie5 = false;

  const auto sideEffects = Moqi::TypeDuck::rimeSideEffects(prefs);
  EXPECT_EQ(sideEffects.commonPatchKey, "__patch");
  EXPECT_EQ(sideEffects.commonPatches,
            (std::vector<std::string>{
                "common:/show_cangjie_roots",
                "common:/disable_completion",
                "common:/enable_correction",
                "common:/disable_sentence",
                "common:/disable_learning",
                "common:/use_cangjie3"}));
}

TEST(TypeDuckPreferences, FailedApplyDoesNotCorruptJsonSourceOfTruth) {
  // D-44 and D-47: apply is batched, deploy failures return bounded bilingual
  // status, and JSON remains the readable source of truth.
  const auto root = makeTempDir("typeduck-preferences-test");
  const auto path = root / "preferences.json";
  auto prefs = Moqi::TypeDuck::defaultPreferences();
  prefs.pageSize = 8;
  ASSERT_TRUE(Moqi::TypeDuck::savePreferences(path, prefs).ok);

  const auto before = readFile(path);
  auto failed = Moqi::TypeDuck::applyPreferences(
      path, prefs,
      [](const Moqi::TypeDuck::RimeSideEffects&) {
        return Moqi::TypeDuck::ApplyResult{
            false,
            "設定未能套用，已保留原有設定 / Settings could not be applied; existing settings were kept"};
      });
  EXPECT_FALSE(failed.ok);
  EXPECT_NE(failed.message.find("設定"), std::string::npos);
  EXPECT_NE(failed.message.find("Settings"), std::string::npos);
  EXPECT_EQ(readFile(path), before);
}
