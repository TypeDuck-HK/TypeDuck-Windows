#include "stdafx.h"
#include "MultiHintPanel.h"
#include <unordered_set>
#include <regex>
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
	/*  8 */ Properties.Colloquial = *column++;
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
