#include "stdafx.h"
#include "MultiHintPanel.h"
#include <locale>
#include <codecvt>
#include <boost/algorithm/string.hpp>
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

bool MultiHintPanel::containsCsv(weasel::Text &comment)
{
	return containsCsv(comment.str);
}

bool MultiHintPanel::containsCsv(const std::wstring &comment)
{
  return comment.find(',') != std::wstring::npos;
}

void MultiHintPanel::applyMultiHint(weasel::Text &comment)
{
	//setup converter
	std::wstring& str = comment.str;
	// if Jyutping set to empty don't print the default comment whatever it is.
	auto status = settingsStatus_;
	
	if (str.find(',') == std::wstring::npos) { // std::count(str.begin(), str.end(), ',') < 16
		return;
	}
	comment = getMultiHint(str);
}

std::wstring MultiHintPanel::getMultiHint(const std::wstring &comment)
{
	std::wstring str = comment;
	auto status = settingsStatus_;
	InfoMultiHint info(converter_.to_bytes(str));
	// Disable Jyutping
	status = status & ~(int)StatusHintColumn::Jyutping;
	std::string hint = getHint(info, status);
	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	str = converter_.from_bytes(hint);
	return str;
}

std::wstring MultiHintPanel::getJyutping(const std::wstring &comment)
{
	InfoMultiHint info(converter_.to_bytes(comment));
	return converter_.from_bytes(info.Jyutping);
}

const std::pair<std::wstring, int> columns[] = {
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
	std::vector<std::wstring> options;
	boost::split(options, settings, boost::is_any_of(","));
	for (auto& column : columns) {
		if (std::find(options.begin(), options.end(), column.first) != options.end()) {
			status |= column.second;
		}
	}
	settingsStatus_ = status;
}

std::string MultiHintPanel::getHint(const InfoMultiHint& info, const StatusHintSetting status) const
{
	std::vector<std::string> hints;
	if (status & (int)StatusHintColumn::Jyutping) hints.push_back(info.Jyutping);
	if (status & (int)StatusHintColumn::Eng) hints.push_back(info.Properties.Definition.Eng);
	if(status & (int)StatusHintColumn::Urd) hints.push_back(info.Properties.Definition.Urd);
	if(status & (int)StatusHintColumn::Nep) hints.push_back(info.Properties.Definition.Nep);
	if(status & (int)StatusHintColumn::Hin) hints.push_back(info.Properties.Definition.Hin);
	if(status & (int)StatusHintColumn::Ind) hints.push_back(info.Properties.Definition.Ind);
	return join(hints, "\t");
}

bool MultiHintPanel::isHintEnabled(StatusHintColumn column) const
{
  return (settingsStatus_ & (int)column) != 0;
}

InfoMultiHint::InfoMultiHint(const std::string& input) {
	boost::tokenizer<boost::escaped_list_separator<char>> columns(input);
	auto column = columns.begin();
	/*  0 */ Honzi = *column++;
	/*  1 */ Jyutping = *column++;
	/*  2 */ Sandhi = *column++;
	/*  3 */ LitColReading = *column++;
	Properties = InfoProperties();
	/*  4 */ Properties.PartOfSpeech = *column++;
	/*  5 */ Properties.Register = *column++;
	/*  6 */ Properties.Label = *column++;
	/*  7 */ Properties.Normalized = *column++;
	/*  8 */ Properties.Written = *column++;
	/*  9 */ Properties.Colloquial = *column++;
	/* 10 */ Properties.Collocation = *column++;
	Properties.Definition = InfoDefinition();
	/* 11 */ Properties.Definition.Eng = *column++;
	/* 12 */ Properties.Definition.Urd = *column++;
	/* 13 */ Properties.Definition.Nep = *column++;
	/* 14 */ Properties.Definition.Hin = *column++;
	/* 15 */ Properties.Definition.Ind = *column++;
}
