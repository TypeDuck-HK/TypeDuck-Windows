//
//    Copyright (C) 2026
//

#include "TypeDuckCandidateInfo.h"

#include <algorithm>
#include <array>
#include <cwctype>
#include <set>
#include <utility>

namespace Moqi::TypeDuck {
namespace {

constexpr size_t kMaxRawCommentLength = 64 * 1024;
constexpr size_t kMaxCsvRowLength = 16 * 1024;
constexpr size_t kMaxCsvFieldLength = 4096;
constexpr size_t kLookupFilterColumnCount = 20;

class ConsumedString {
public:
  explicit ConsumedString(std::wstring value) : value_(std::move(value)) {}

  bool isNotEmpty() const {
    return offset_ < value_.size();
  }

  bool consume(wchar_t ch) {
    if (isNotEmpty() && value_[offset_] == ch) {
      ++offset_;
      return true;
    }
    return false;
  }

  std::wstring consumeUntil(wchar_t ch) {
    const size_t start = offset_;
    while (isNotEmpty()) {
      if (value_[offset_] == ch) {
        const size_t end = offset_;
        ++offset_;
        return value_.substr(start, end - start);
      }
      ++offset_;
    }
    return value_.substr(start);
  }

  std::wstring remaining() const {
    return value_.substr(offset_);
  }

private:
  std::wstring value_;
  size_t offset_ = 0;
};

std::vector<std::wstring> split(const std::wstring& value, wchar_t delimiter) {
  std::vector<std::wstring> parts;
  size_t start = 0;
  while (start <= value.size()) {
    const size_t pos = value.find(delimiter, start);
    if (pos == std::wstring::npos) {
      parts.push_back(value.substr(start));
      break;
    }
    parts.push_back(value.substr(start, pos - start));
    start = pos + 1;
  }
  return parts;
}

std::vector<std::wstring> parseCsv(const std::wstring& csv) {
  std::vector<std::wstring> fields;
  if (csv.size() > kMaxCsvRowLength) {
    return fields;
  }

  bool isQuoted = false;
  std::wstring value;
  for (size_t i = 0; i < csv.size(); ++i) {
    const wchar_t ch = csv[i];
    if (isQuoted) {
      if (ch == L'"') {
        if (i + 1 < csv.size() && csv[i + 1] == L'"') {
          value.push_back(csv[++i]);
        } else {
          isQuoted = false;
        }
      } else {
        value.push_back(ch);
      }
    } else if (value.empty() && ch == L'"') {
      isQuoted = true;
    } else if (ch == L',') {
      fields.push_back(value.size() > kMaxCsvFieldLength
                           ? value.substr(0, kMaxCsvFieldLength)
                           : value);
      value.clear();
    } else {
      value.push_back(ch);
    }
  }
  fields.push_back(value.size() > kMaxCsvFieldLength
                       ? value.substr(0, kMaxCsvFieldLength)
                       : value);
  return fields;
}

std::wstring formatJyutping(const std::wstring& jyutping) {
  std::wstring result;
  for (size_t i = 0; i < jyutping.size(); ++i) {
    result.push_back(jyutping[i]);
    if (std::iswdigit(jyutping[i]) && i + 1 < jyutping.size() &&
        !std::iswspace(jyutping[i + 1])) {
      result.push_back(L' ');
    }
  }
  return result;
}

bool isAscii(const std::wstring& value) {
  return std::all_of(value.begin(), value.end(), [](wchar_t ch) {
    return ch >= 0 && ch < 0x80;
  });
}

std::wstring formatGlyphonString(const std::wstring& value) {
  if (isAscii(value)) {
    return formatJyutping(value);
  }

  std::wstring honzi;
  std::wstring romanization;
  for (wchar_t ch : value) {
    if (ch >= 0 && ch < 0x80) {
      romanization.push_back(ch);
    } else {
      honzi.push_back(ch);
    }
  }
  return honzi + L"(" + formatJyutping(romanization) + L")";
}

template <typename T>
void appendUnique(std::vector<T>& values, T value) {
  if (value.empty()) {
    return;
  }
  if (std::find(values.begin(), values.end(), value) == values.end()) {
    values.push_back(std::move(value));
  }
}

std::wstring mapValue(
    const std::map<std::wstring, std::wstring>& values,
    const std::wstring& key) {
  const auto found = values.find(key);
  return found == values.end() ? key : found->second;
}

const std::map<std::wstring, std::wstring>& pronunciationLabels() {
  static const std::map<std::wstring, std::wstring> kValues = {
      {L"sandhi", L"changed tone 變音"},
      {L"semantic_reading", L"semantic reading 訓讀"},
      {L"creative_reading", L"creative reading 生造音"},
  };
  return kValues;
}

const std::map<std::wstring, std::wstring>& litColReadings() {
  static const std::map<std::wstring, std::wstring> kValues = {
      {L"lit", L"literary reading 文讀"},
      {L"col", L"colloquial reading 白讀"},
  };
  return kValues;
}

const std::map<std::wstring, std::wstring>& registers() {
  static const std::map<std::wstring, std::wstring> kValues = {
      {L"cmn", L"written 書面語"},
      {L"yue", L"vernacular 口語"},
      {L"lzh", L"formal 公文體"},
      {L"och", L"classical Chinese 文言"},
      {L"sit", L"chars. for proper noun 專有名詞用字"},
  };
  return kValues;
}

const std::map<std::wstring, std::wstring>& partsOfSpeech() {
  static const std::map<std::wstring, std::wstring> kValues = {
      {L"n", L"noun 名詞"},
      {L"v", L"verb 動詞"},
      {L"adj", L"adjective 形容詞"},
      {L"adv", L"adverb 副詞"},
      {L"morph", L"morpheme 語素"},
      {L"mw", L"measure word 量詞"},
      {L"part", L"particle 助詞"},
      {L"oth", L"other 其他"},
      {L"x", L"non-morpheme 非語素"},
  };
  return kValues;
}

const std::map<std::wstring, std::wstring>& labels() {
  static const std::map<std::wstring, std::wstring> kValues = {
      {L"abbrev", L"abbreviation 簡稱"},
      {L"astro", L"astronomy 天文"},
      {L"ChinMeta", L"sexagenary cycle 干支"},
      {L"horo", L"horoscope 星座"},
      {L"org", L"organisation 機構"},
      {L"person", L"person 人名"},
      {L"place", L"place 地名"},
      {L"pop", L"popular culture 流行文化"},
      {L"reli", L"religion 宗教"},
      {L"rare", L"rare 罕見"},
      {L"composition", L"compound 詞組"},
  };
  return kValues;
}

std::vector<std::wstring> mapPipeValues(
    const std::wstring& value,
    const std::map<std::wstring, std::wstring>& table) {
  std::vector<std::wstring> mapped;
  for (const auto& part : split(value, L'|')) {
    appendUnique(mapped, mapValue(table, part));
  }
  return mapped;
}

} // namespace

DisplayPreferences::DisplayPreferences()
    : displayLanguages({DisplayLanguage::English}),
      mainLanguage(DisplayLanguage::English),
      jyutpingVisibility(JyutpingVisibility::Always),
      chineseTypeface(ChineseTypeface::Sung),
      showReverseCode(true) {
}

bool DisplayPreferences::shouldShowJyutping(bool isReverseLookup) const {
  switch (jyutpingVisibility) {
  case JyutpingVisibility::Always:
    return true;
  case JyutpingVisibility::ReverseLookupOnly:
    return isReverseLookup;
  case JyutpingVisibility::Hidden:
    return false;
  }
  return true;
}

const std::vector<DisplayLanguage>& allDisplayLanguages() {
  static const std::vector<DisplayLanguage> kLanguages = {
      DisplayLanguage::English,
      DisplayLanguage::Hindi,
      DisplayLanguage::Indonesian,
      DisplayLanguage::Nepali,
      DisplayLanguage::Urdu,
  };
  return kLanguages;
}

std::wstring languageCode(DisplayLanguage language) {
  switch (language) {
  case DisplayLanguage::English:
    return L"en";
  case DisplayLanguage::Hindi:
    return L"hi";
  case DisplayLanguage::Indonesian:
    return L"id";
  case DisplayLanguage::Nepali:
    return L"ne";
  case DisplayLanguage::Urdu:
    return L"ur";
  }
  return L"";
}

std::wstring languageName(DisplayLanguage language) {
  switch (language) {
  case DisplayLanguage::English:
    return L"English";
  case DisplayLanguage::Hindi:
    return L"Hindi";
  case DisplayLanguage::Indonesian:
    return L"Indonesian";
  case DisplayLanguage::Nepali:
    return L"Nepali";
  case DisplayLanguage::Urdu:
    return L"Urdu";
  }
  return L"";
}

std::wstring languageLabel(DisplayLanguage language) {
  switch (language) {
  case DisplayLanguage::English:
    return L"英語 English";
  case DisplayLanguage::Hindi:
    return L"印地語 Hindi";
  case DisplayLanguage::Indonesian:
    return L"印尼語 Indonesian";
  case DisplayLanguage::Nepali:
    return L"尼泊爾語 Nepali";
  case DisplayLanguage::Urdu:
    return L"烏爾都語 Urdu";
  }
  return L"";
}

std::wstring jyutpingVisibilityLabel(JyutpingVisibility visibility) {
  switch (visibility) {
  case JyutpingVisibility::Always:
    return L"顯示 Always Show";
  case JyutpingVisibility::ReverseLookupOnly:
    return L"僅反查 Only in Reverse Lookup";
  case JyutpingVisibility::Hidden:
    return L"隱藏 Hide";
  }
  return L"";
}

std::wstring chineseTypefaceLabel(ChineseTypeface typeface) {
  return typeface == ChineseTypeface::Hei ? L"黑體 Hei" : L"宋體 Sung";
}

CandidateEntry CandidateEntry::fromCsvRow(const std::wstring& csv) {
  CandidateEntry entry;
  const std::vector<std::wstring> fields = parseCsv(csv);
  if (fields.size() != kLookupFilterColumnCount) {
    entry.malformed = true;
    return entry;
  }

  // D-10 lookup-filter header:
  // match_input_buffer,honzi,jyutping,canonical_honzi,canonical_jyutping,
  // components_honzi,components_jyutping,pron_label,lit_col_reading,pos,
  // register,label,written_form,vernacular_form,collocation,eng,hin,urd,nep,ind
  entry.matchInputBuffer = fields[0];
  entry.honzi = fields[1];
  entry.jyutping = formatJyutping(fields[2]);
  entry.canonicalHonzi = fields[3];
  entry.canonicalJyutping = formatJyutping(fields[4]);
  entry.pronLabel = fields[7];
  entry.litColReading = fields[8];
  entry.partOfSpeech = fields[9];
  entry.registerValue = fields[10];
  entry.label = fields[11];
  entry.writtenForm = fields[12];
  entry.vernacularForm = fields[13];
  entry.collocation = fields[14];
  entry.definitions[DisplayLanguage::English] = fields[15];
  entry.definitions[DisplayLanguage::Hindi] = fields[16];
  entry.definitions[DisplayLanguage::Urdu] = fields[17];
  entry.definitions[DisplayLanguage::Nepali] = fields[18];
  entry.definitions[DisplayLanguage::Indonesian] = fields[19];
  return entry;
}

CandidateEntry CandidateEntry::jyutpingOnly(std::wstring honzi, std::wstring jyutping) {
  CandidateEntry entry;
  entry.isJyutpingOnly = true;
  entry.honzi = std::move(honzi);
  entry.jyutping = formatJyutping(std::move(jyutping));
  return entry;
}

std::wstring CandidateEntry::definition(DisplayLanguage language) const {
  const auto found = definitions.find(language);
  return found == definitions.end() ? L"" : found->second;
}

std::wstring CandidateEntry::canonicalReference() const {
  if (canonicalHonzi.empty() && canonicalJyutping.empty()) {
    return L"";
  }
  if (!canonicalHonzi.empty()) {
    return canonicalJyutping.empty()
               ? canonicalHonzi
               : canonicalHonzi + L"(" + canonicalJyutping + L")";
  }
  return canonicalJyutping;
}

std::wstring CandidateEntry::pronunciationType() const {
  std::vector<std::wstring> types;
  for (const auto& part : split(pronLabel, L'|')) {
    appendUnique(types, mapValue(pronunciationLabels(), part));
  }

  if (!litColReading.empty()) {
    const size_t greater = litColReading.find(L'>');
    const size_t less = litColReading.find(L'<');
    const size_t dirPos = greater == std::wstring::npos ? less : greater;
    if (dirPos != std::wstring::npos) {
      const std::wstring litOrCol = litColReading.substr(0, dirPos);
      const std::wstring dir(1, litColReading[dirPos]);
      const std::wstring relatedReadings = litColReading.substr(dirPos + 1);
      const std::wstring mapped = mapValue(litColReadings(), litOrCol);
      std::wstring related;
      for (const auto& reading : split(relatedReadings, L'|')) {
        if (!related.empty()) {
          related += L"/";
        }
        related += formatGlyphonString(reading);
      }
      if (!mapped.empty() && !related.empty()) {
        types.push_back(mapped + L" " + dir + L" " + related);
      }
    }
  }

  if (types.empty()) {
    return L"";
  }
  std::wstring result = L"(";
  for (size_t i = 0; i < types.size(); ++i) {
    if (i > 0) {
      result += L", ";
    }
    result += types[i];
  }
  result += L")";
  return result;
}

std::vector<std::wstring> CandidateEntry::formattedPartsOfSpeech() const {
  return mapPipeValues(partOfSpeech, partsOfSpeech());
}

std::vector<std::wstring> CandidateEntry::formattedRegister() const {
  return mapPipeValues(registerValue, registers());
}

std::vector<std::wstring> CandidateEntry::formattedLabels() const {
  std::vector<std::wstring> mapped;
  for (const auto& word : split(label, L'|')) {
    for (const auto& part : split(word, L'_')) {
      const auto found = labels().find(part);
      if (found != labels().end()) {
        appendUnique(mapped, L"(" + found->second + L")");
        break;
      }
    }
  }
  return mapped;
}

std::vector<OtherDataRow> CandidateEntry::otherData() const {
  std::vector<OtherDataRow> rows;
  if (!writtenForm.empty()) {
    rows.push_back({L"written", L"Written Form 書面語", writtenForm});
  }
  if (!vernacularForm.empty()) {
    rows.push_back({L"vernacular", L"Vernacular Form 口語", vernacularForm});
  }
  if (!collocation.empty()) {
    rows.push_back({L"collocation", L"Collocation 配搭", collocation});
  }
  return rows;
}

std::vector<LanguageRow> CandidateEntry::otherLanguages(
    const DisplayPreferences& preferences) const {
  if (!canonicalReference().empty()) {
    return {};
  }

  std::vector<LanguageRow> rows;
  for (DisplayLanguage language : preferences.displayLanguages) {
    if (language == preferences.mainLanguage) {
      continue;
    }
    const std::wstring value = definition(language);
    if (!value.empty()) {
      rows.push_back({languageCode(language), languageName(language), value});
    }
  }
  return rows;
}

bool CandidateEntry::isDictionaryEntry(const DisplayPreferences& preferences) const {
  if (isJyutpingOnly || malformed) {
    return false;
  }
  if (!partOfSpeech.empty() || !registerValue.empty() || !writtenForm.empty() ||
      !vernacularForm.empty() || !collocation.empty() || !canonicalReference().empty()) {
    return true;
  }
  return std::any_of(preferences.displayLanguages.begin(), preferences.displayLanguages.end(),
                     [this](DisplayLanguage language) {
                       return !definition(language).empty();
                     });
}

CandidateInfo::CandidateInfo() = default;

CandidateInfo::CandidateInfo(
    std::wstring candidateLabel,
    std::wstring candidateText,
    std::wstring commentString)
    : CandidateInfo(std::move(candidateLabel), std::move(candidateText),
                    std::move(commentString), L"") {
}

CandidateInfo::CandidateInfo(
    std::wstring candidateLabel,
    std::wstring candidateText,
    std::wstring commentString,
    std::wstring fallbackJyutping)
    : label(std::move(candidateLabel)),
      text(std::move(candidateText)),
      rawComment(commentString.size() > kMaxRawCommentLength
                     ? commentString.substr(0, kMaxRawCommentLength)
                     : commentString) {
  ConsumedString comment(rawComment);
  isReverseLookup = comment.consume(L'\v');
  note = comment.consumeUntil(L'\f');
  if (!comment.isNotEmpty()) {
    if (!fallbackJyutping.empty()) {
      entries.push_back(CandidateEntry::jyutpingOnly(text, std::move(fallbackJyutping)));
    }
    return;
  }

  if (comment.consume(L'\r')) {
    for (const auto& row : split(comment.remaining(), L'\r')) {
      if (!row.empty()) {
        CandidateEntry entry = CandidateEntry::fromCsvRow(row);
        if (!entry.malformed) {
          entries.push_back(std::move(entry));
        }
      }
    }
  } else {
    for (std::wstring pron : split(comment.remaining(), L'\f')) {
      pron.erase(std::remove(pron.begin(), pron.end(), L'\v'), pron.end());
      constexpr const wchar_t* kTrailingSemicolon = L"; ";
      if (pron.size() >= 2 && pron.compare(pron.size() - 2, 2, kTrailingSemicolon) == 0) {
        pron.erase(pron.size() - 2);
      }
      if (!pron.empty()) {
        entries.push_back(CandidateEntry::jyutpingOnly(text, std::move(pron)));
      }
    }
  }

  if (entries.empty() && !fallbackJyutping.empty()) {
    entries.push_back(CandidateEntry::jyutpingOnly(text, std::move(fallbackJyutping)));
  }
}

std::vector<CandidateEntry> CandidateInfo::matchedEntries() const {
  std::vector<CandidateEntry> matched;
  for (const auto& entry : entries) {
    if (entry.matchInputBuffer == L"1") {
      matched.push_back(entry);
    }
  }
  return matched;
}

bool CandidateInfo::hasDictionaryEntry(const DisplayPreferences& preferences) const {
  return std::any_of(entries.begin(), entries.end(), [&preferences](const CandidateEntry& entry) {
    return entry.isDictionaryEntry(preferences);
  });
}

std::wstring CandidateInfo::displayCandidateText(const DisplayPreferences&) const {
  const auto matched = matchedEntries();
  if (!matched.empty() && !matched[0].honzi.empty()) {
    return matched[0].honzi;
  }
  return text;
}

std::wstring CandidateInfo::displayComment(const DisplayPreferences& preferences) const {
  const auto matched = matchedEntries();
  const std::vector<CandidateEntry>& source = matched.empty() ? entries : matched;
  for (const auto& entry : source) {
    const std::wstring reference = entry.canonicalReference();
    if (!reference.empty()) {
      return L"→" + reference;
    }
    const std::wstring mainDefinition = entry.definition(preferences.mainLanguage);
    if (!mainDefinition.empty()) {
      return mainDefinition;
    }
    for (DisplayLanguage language : preferences.displayLanguages) {
      const std::wstring definition = entry.definition(language);
      if (!definition.empty()) {
        return definition;
      }
    }
    const auto labels = entry.formattedLabels();
    if (!labels.empty()) {
      return labels[0];
    }
  }
  if (isReverseLookup && !preferences.showReverseCode) {
    return L"";
  }
  return note;
}

} // namespace Moqi::TypeDuck
