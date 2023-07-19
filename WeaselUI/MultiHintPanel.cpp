#include "stdafx.h"
#include "MultiHintPanel.h"
#include <unordered_set>
#include <regex>
#include <map>
#include <StringAlgorithm.hpp>

MultiHintPanel* volatile MultiHintPanel::instance = nullptr;
MultiHintPanel* MultiHintPanel::GetInstance()
{
  	if (instance == NULL)
	{
		if (instance == NULL)
		{
			instance = new MultiHintPanel();
		}
	}
	return instance;
}

bool MultiHintPanel::containsCSV(const std::wstring& comment) const
{
	return comment.find(',') != std::wstring::npos;
}

const static std::pair<std::wstring, int> columns[] = {
	{ L"Jyutping", (int)StatusHintColumn::Jyutping },
	{ L"Eng", (int)StatusHintColumn::Eng },
	{ L"Urd", (int)StatusHintColumn::Urd },
	{ L"Nep", (int)StatusHintColumn::Nep },
	{ L"Hin", (int)StatusHintColumn::Hin },
	{ L"Ind", (int)StatusHintColumn::Ind }
};

void MultiHintPanel::setMultiHintOptions(const std::wstring& settings)
{
	StatusHintSetting status = (int)StatusHintColumn::None;
	std::wstring str = settings;
	str = std::regex_replace(str, std::wregex(L"\\s*,\\s*"), L",");
	str = std::regex_replace(str, std::wregex(L"^\\s*|\\s*$"), L"");
	std::unordered_set<std::wstring> options = ws_split(str, L",");
	for (auto& column : columns) {
		if (options.find(column.first) != options.end()) {
			status |= column.second;
		}
	}
	settingsStatus_ = status;
}

std::wstring MultiHintPanel::getHint(const std::wstring& comment) const
{
	if (!containsCSV(comment)) {
		return comment;
	}
	InfoMultiHint info(comment);
	std::wstring str = L"";
	if (isHintEnabled(StatusHintColumn::Jyutping)) str += info.Jyutping + L"\t";
	if (isHintEnabled(StatusHintColumn::Eng)) str += info.Properties.Definition.Eng + L"\t";
	if (isHintEnabled(StatusHintColumn::Urd)) str += info.Properties.Definition.Urd + L"\t";
	if (isHintEnabled(StatusHintColumn::Nep)) str += info.Properties.Definition.Nep + L"\t";
	if (isHintEnabled(StatusHintColumn::Hin)) str += info.Properties.Definition.Hin + L"\t";
	if (isHintEnabled(StatusHintColumn::Ind)) str += info.Properties.Definition.Ind + L"\t";
	str.pop_back();
	return str;
}

bool MultiHintPanel::isHintEnabled(int column) const
{
  return (settingsStatus_ & column) != 0;
}

bool MultiHintPanel::isHintEnabled(StatusHintColumn column) const
{
  return isHintEnabled((int)column);
}

InfoMultiHint::InfoMultiHint(const std::wstring& input) {
	boost::tokenizer<boost::escaped_list_separator<wchar_t>, std::wstring::const_iterator, std::wstring> columns(input);
	auto column = columns.begin();
	/*  0 */ Jyutping = *column++;
	/*  1 */ Sandhi = *column++;
	/*  2 */ LitColReading = *column++;
	Properties = InfoProperties();
	/*  3 */ Properties.PartOfSpeech = *column++;
	/*  4 */ Properties.Register = *column++;
	/*  5 */ Properties.Label = *column++;
	/*  6 */ Properties.Normalized = *column++;
	/*  7 */ Properties.Written = *column++;
	/*  8 */ Properties.Vernacular = *column++;
	/*  9 */ Properties.Collocation = *column++;
	Properties.Definition = InfoDefinition();
	/* 10 */ Properties.Definition.Eng = *column++;
	/* 11 */ Properties.Definition.Urd = *column++;
	/* 12 */ Properties.Definition.Nep = *column++;
	/* 13 */ Properties.Definition.Hin = *column++;
	/* 14 */ Properties.Definition.Ind = *column++;
	Jyutping = std::regex_replace(Jyutping, std::wregex(L"\\d"), L"$& ");
	Jyutping.pop_back();
}

const static std::vector<std::pair<std::wstring, std::wstring InfoProperties::*> > otherData = {
	{ L"Standard Form", &InfoProperties::Normalized },
	{ L"Written Form", &InfoProperties::Written },
	{ L"Vernacular Form", &InfoProperties::Vernacular },
	{ L"Word Form", &InfoProperties::Collocation },
};

const static std::map<std::wstring, std::wstring> litColReadings = {
	{ L"lit", L"literary reading 文讀" },
	{ L"col", L"colloquial reading 白讀" },
};

const static std::map<std::wstring, std::wstring> registers = {
	{ L"wri", L"written" },
	{ L"ver", L"vernacular" },
	{ L"for", L"formal" },
	{ L"lzh", L"archaic" },
};

const static std::map<std::wstring, std::wstring> partsOfSpeech = {
	{ L"n", L"noun 名詞" },
	{ L"v", L"verb 動詞" },
	{ L"adj", L"adjective 形容詞" },
	{ L"adv", L"adverb 副詞" },
	{ L"conj", L"conjunction 連接詞" },
	{ L"prep", L"preposition 前置詞" },
	{ L"pron", L"pronoun 代名詞" },
	{ L"morph", L"morpheme 語素" },
	{ L"mw", L"measure word 量詞" },
	{ L"part", L"particle 助詞" },
	{ L"oth", L"other 其他" },
	{ L"x", L"non-morpheme 非語素" },
};

std::wstring InfoMultiHint::GetPronType() const {
	std::wstring pronType = L"";
	if (Sandhi == L"1") pronType += L"changed tone 變音, ";
	auto it = litColReadings.find(LitColReading);
	if (it != litColReadings.end()) pronType += it->second + L", ";
	if (pronType.empty()) return pronType;
	pronType.pop_back();
	pronType.pop_back();
	return L"(" + pronType + L")";
}

std::vector<std::wstring> InfoProperties::GetPartsOfSpeech() const {
	std::vector<std::wstring> posSplitted, posFiltered;
	split(posSplitted, PartOfSpeech, L" ");
	for (const std::wstring pos : posSplitted) {
		if (pos.empty()) continue;
		auto it = partsOfSpeech.find(pos);
		posFiltered.push_back(it == partsOfSpeech.end() ? pos : it->second);
	}
	return posFiltered;
}

std::wstring InfoProperties::GetRegister() const {
	auto it = registers.find(Register);
	return it == registers.end() ? L"" : it->second;
}

std::vector<std::wstring> InfoProperties::GetLabels() const {
	std::vector<std::wstring> labelSplitted, labelFiltered;
	split(labelSplitted, Label, L" ");
	for (const std::wstring label : labelSplitted) {
		if (label.empty()) continue;
		labelFiltered.push_back(L"(" + label + L")");
	}
	return labelFiltered;
}

std::vector<std::vector<std::wstring> > InfoProperties::GetOtherData() const {
	std::vector<std::vector<std::wstring> > data;
	for (const auto& field : otherData) {
		std::wstring str = this->*field.second;
		if (str.empty()) continue;
		std::vector<std::wstring> item = { field.first };
		split(item, str, L"，", false);
		data.push_back(item);
	}
	return data;
}

std::vector<InfoLanguage> InfoDefinition::Get(MultiHintPanel* panel, DirectWriteResources* pDWR) const {
	std::vector<InfoLanguage> info;
	if (panel->isHintEnabled(StatusHintColumn::Eng) && !Eng.empty()) info.push_back({ L"English", Eng, pDWR->pEngTextFormat });
	if (panel->isHintEnabled(StatusHintColumn::Eng) && !Urd.empty()) info.push_back({ L"Urdu", Urd, pDWR->pUrdTextFormat });
	if (panel->isHintEnabled(StatusHintColumn::Eng) && !Nep.empty()) info.push_back({ L"Nepali", Nep, pDWR->pNepTextFormat });
	if (panel->isHintEnabled(StatusHintColumn::Eng) && !Hin.empty()) info.push_back({ L"Hindi", Hin, pDWR->pHinTextFormat });
	if (panel->isHintEnabled(StatusHintColumn::Eng) && !Ind.empty()) info.push_back({ L"Indonesian", Ind, pDWR->pIndTextFormat });
	return info;
}
