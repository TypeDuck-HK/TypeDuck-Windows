//
//    Copyright (C) 2026
//

#pragma once

#include <map>
#include <string>
#include <vector>

namespace Moqi::TypeDuck {

enum class DisplayLanguage {
  English,
  Hindi,
  Indonesian,
  Nepali,
  Urdu,
};

enum class JyutpingVisibility {
  Always,
  ReverseLookupOnly,
  Hidden,
};

enum class ChineseTypeface {
  Sung,
  Hei,
};

struct LanguageRow {
  std::wstring code;
  std::wstring name;
  std::wstring value;
};

struct OtherDataRow {
  std::wstring key;
  std::wstring name;
  std::wstring value;
};

struct DisplayPreferences {
  std::vector<DisplayLanguage> displayLanguages;
  DisplayLanguage mainLanguage;
  JyutpingVisibility jyutpingVisibility;
  ChineseTypeface chineseTypeface;
  bool showReverseCode;

  DisplayPreferences();
  bool shouldShowJyutping(bool isReverseLookup) const;
};

const std::vector<DisplayLanguage>& allDisplayLanguages();
std::wstring languageCode(DisplayLanguage language);
std::wstring languageName(DisplayLanguage language);
std::wstring languageLabel(DisplayLanguage language);
std::wstring jyutpingVisibilityLabel(JyutpingVisibility visibility);
std::wstring chineseTypefaceLabel(ChineseTypeface typeface);

struct CandidateEntry {
  std::wstring matchInputBuffer;
  std::wstring honzi;
  std::wstring jyutping;
  std::wstring canonicalHonzi;
  std::wstring canonicalJyutping;
  std::wstring pronLabel;
  std::wstring litColReading;
  std::wstring partOfSpeech;
  std::wstring registerValue;
  std::wstring label;
  std::wstring writtenForm;
  std::wstring vernacularForm;
  std::wstring collocation;
  std::map<DisplayLanguage, std::wstring> definitions;
  bool isJyutpingOnly = false;
  bool malformed = false;

  static CandidateEntry fromCsvRow(const std::wstring& csv);
  static CandidateEntry jyutpingOnly(std::wstring honzi, std::wstring jyutping);

  std::wstring definition(DisplayLanguage language) const;
  std::wstring canonicalReference() const;
  std::wstring pronunciationType() const;
  std::vector<std::wstring> formattedPartsOfSpeech() const;
  std::vector<std::wstring> formattedRegister() const;
  std::vector<std::wstring> formattedLabels() const;
  std::vector<OtherDataRow> otherData() const;
  std::vector<LanguageRow> otherLanguages(const DisplayPreferences& preferences) const;
  bool isDictionaryEntry(const DisplayPreferences& preferences) const;
};

struct CandidateInfo {
  std::wstring label;
  std::wstring text;
  std::wstring rawComment;
  bool isReverseLookup = false;
  std::wstring note;
  std::vector<CandidateEntry> entries;

  CandidateInfo();
  CandidateInfo(std::wstring label, std::wstring text, std::wstring commentString);
  CandidateInfo(std::wstring label,
                std::wstring text,
                std::wstring commentString,
                std::wstring fallbackJyutping);

  std::vector<CandidateEntry> matchedEntries() const;
  bool hasDictionaryEntry(const DisplayPreferences& preferences) const;
  std::wstring displayCandidateText(const DisplayPreferences& preferences) const;
  std::wstring displayComment(const DisplayPreferences& preferences) const;
};

} // namespace Moqi::TypeDuck
