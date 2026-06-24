#include "gtest/gtest.h"

#include "MoqiTextService/TypeDuckCandidateInfo.h"

#include <string>
#include <vector>

using Moqi::TypeDuck::CandidateEntry;
using Moqi::TypeDuck::CandidateInfo;
using Moqi::TypeDuck::ChineseTypeface;
using Moqi::TypeDuck::DisplayLanguage;
using Moqi::TypeDuck::DisplayPreferences;
using Moqi::TypeDuck::JyutpingVisibility;

namespace {

std::wstring NeiRow() {
  return L"1,你,nei5,,,,,,,oth,,,,,,you (singular),आप,تم,तपाईं,kamu";
}

std::wstring NeiCanonicalRow() {
  return L"1,呢,nei1,,ni1,,,,,part,yue,,這,,,this,(particle),(particle),,(imbuhan kata)";
}

std::wstring HousamRow() {
  return L"1,好心,hou2 sam1,,,,,,,adj|adv,yue,,拜托,,,kind; come on,दयालु,نرم دل,दयालु,baik hati";
}

std::wstring HousamNeiRow() {
  return L"1,你,nei5,,,,,,,oth,,,,,,you (singular),आप,تم,तपाईं,kamu";
}

DisplayPreferences MultilingualIndonesianPreferences() {
  DisplayPreferences prefs;
  prefs.displayLanguages = {
      DisplayLanguage::English,
      DisplayLanguage::Hindi,
      DisplayLanguage::Indonesian,
      DisplayLanguage::Nepali,
      DisplayLanguage::Urdu};
  prefs.mainLanguage = DisplayLanguage::Indonesian;
  prefs.jyutpingVisibility = JyutpingVisibility::Always;
  prefs.chineseTypeface = ChineseTypeface::Hei;
  return prefs;
}

} // namespace

TEST(TypeDuckCandidateInfoTest, PreservesLookupFilterControlSeparatorSemantics) {
  const std::wstring raw = L"\vtyped note\f\r" + NeiRow() + L"\r" + NeiCanonicalRow();
  const CandidateInfo info(L"1.", L"你", raw);

  EXPECT_TRUE(info.isReverseLookup);
  EXPECT_EQ(info.note, L"typed note");
  EXPECT_EQ(info.rawComment, raw);
  ASSERT_EQ(info.entries.size(), 2u);
  EXPECT_EQ(info.entries[0].honzi, L"你");
  EXPECT_EQ(info.entries[1].canonicalReference(), L"ni1");
}

TEST(TypeDuckCandidateInfoTest, ParsesCsvRowsWithLookupFilterHeaderOrderAndQuotes) {
  const std::wstring csv =
      L"1,\"你\",nei5,,,,,,,oth,,,,,,\"quoted \"\"meaning\"\", with comma\",,,,";
  const CandidateEntry entry = CandidateEntry::fromCsvRow(csv);

  EXPECT_EQ(entry.matchInputBuffer, L"1");
  EXPECT_EQ(entry.honzi, L"你");
  EXPECT_EQ(entry.jyutping, L"nei5");
  EXPECT_EQ(entry.definition(DisplayLanguage::English), L"quoted \"meaning\", with comma");
  EXPECT_TRUE(entry.definition(DisplayLanguage::Hindi).empty());
}

TEST(TypeDuckCandidateInfoTest, MapsNeiToCandidateDictionaryAndMoreLanguages) {
  const CandidateInfo info(L"1.", L"你", L"note\f\r" + NeiRow());
  const DisplayPreferences prefs = MultilingualIndonesianPreferences();

  ASSERT_EQ(info.matchedEntries().size(), 1u);
  const CandidateEntry& entry = info.matchedEntries()[0];
  EXPECT_EQ(info.displayCandidateText(prefs), L"你");
  EXPECT_EQ(entry.jyutping, L"nei5");
  EXPECT_EQ(entry.definition(DisplayLanguage::English), L"you (singular)");
  EXPECT_TRUE(info.hasDictionaryEntry(prefs));
  ASSERT_EQ(entry.otherLanguages(prefs).size(), 4u);
  EXPECT_EQ(entry.otherLanguages(prefs)[0].name, L"English");
  EXPECT_EQ(entry.otherLanguages(prefs)[0].value, L"you (singular)");
}

TEST(TypeDuckCandidateInfoTest, MapsCompoundHousamToMultipleDictionaryEntries) {
  const CandidateInfo info(L"2.", L"好心你", L"\f\r" + HousamRow() + L"\r" + HousamNeiRow());
  const DisplayPreferences prefs = MultilingualIndonesianPreferences();

  ASSERT_EQ(info.entries.size(), 2u);
  EXPECT_EQ(info.entries[0].honzi, L"好心");
  EXPECT_EQ(info.entries[0].formattedPartsOfSpeech()[0], L"adjective 形容詞");
  EXPECT_EQ(info.entries[0].formattedPartsOfSpeech()[1], L"adverb 副詞");
  EXPECT_EQ(info.entries[0].formattedRegister()[0], L"vernacular 口語");
  ASSERT_EQ(info.entries[0].otherData().size(), 1u);
  EXPECT_EQ(info.entries[0].otherData()[0].name, L"Written Form 書面語");
  EXPECT_EQ(info.entries[0].otherData()[0].value, L"拜托");
  EXPECT_EQ(info.entries[0].definition(prefs.mainLanguage), L"baik hati");
  EXPECT_EQ(info.entries[1].honzi, L"你");
}

TEST(TypeDuckCandidateInfoTest, CarriesJyutpingVisibilityAndTypefacePreferences) {
  CandidateInfo reverseLookup(L"1.", L"你", L"\vcode\f\r" + NeiRow());
  DisplayPreferences prefs;
  prefs.jyutpingVisibility = JyutpingVisibility::ReverseLookupOnly;
  prefs.chineseTypeface = ChineseTypeface::Hei;

  EXPECT_TRUE(prefs.shouldShowJyutping(reverseLookup.isReverseLookup));
  EXPECT_EQ(Moqi::TypeDuck::jyutpingVisibilityLabel(prefs.jyutpingVisibility),
            L"僅反查 Only in Reverse Lookup");
  EXPECT_EQ(Moqi::TypeDuck::chineseTypefaceLabel(prefs.chineseTypeface), L"黑體 Hei");

  prefs.jyutpingVisibility = JyutpingVisibility::Hidden;
  EXPECT_FALSE(prefs.shouldShowJyutping(reverseLookup.isReverseLookup));
}
