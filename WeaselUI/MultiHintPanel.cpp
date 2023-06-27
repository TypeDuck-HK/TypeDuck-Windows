#include "stdafx.h"
#include "MultiHintPanel.h"
#include <locale>
#include <codecvt>
#include <boost/container/vector.hpp>
#include <boost/algorithm/string.hpp>
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
	status = status & ~(int)StatusHintColumn::JyutPing;
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
	{L"Jyutping", (int)StatusHintColumn::JyutPing},
	{L"English", (int)StatusHintColumn::English},
	{L"Disambiguatory Information", (int)StatusHintColumn::Disambiguation},
	{L"Part of Speech", (int)StatusHintColumn::PartOfSpeech},
	{L"Register", (int)StatusHintColumn::Register},
	{L"Label", (int)StatusHintColumn::Label},
	{L"Written", (int)StatusHintColumn::Written},
	{L"Colloquial", (int)StatusHintColumn::Colloquial},
	{L"Urd", (int)StatusHintColumn::Urd},
	{L"Nep", (int)StatusHintColumn::Nep},
	{L"Hin", (int)StatusHintColumn::Hin},
	{L"Ind", (int)StatusHintColumn::Ind}
};

void MultiHintPanel::setMultiHintOptions(const std::wstring& settings)
{
	StatusHintSetting status = (int)StatusHintColumn::None;
	for (auto& column : columns) {
		if (settings.find(column.first) != std::wstring::npos) {
			status = status | column.second;
		}
	}
	settingsStatus_ = status;
}

std::string MultiHintPanel::getHint(const InfoMultiHint& info, const StatusHintSetting status) const
{
	using namespace boost::container;
	vector<std::string> textContainer;
	auto pushText = [&textContainer](const std::string& text){
		if (text.size() > 0) {
			textContainer.push_back(text);
		}
	};
	if(status & (int)StatusHintColumn::JyutPing)
	{ pushText(info.Jyutping); }

	if(status & (int)StatusHintColumn::English)
	{ pushText(info.Definition.English); }

	if(status & (int)StatusHintColumn::Disambiguation)
	{ pushText(info.Definition.Disambiguation); }

	if(status & (int)StatusHintColumn::PartOfSpeech 
			&& info.Definition.Pos.size() > 0)
	{ 
		const auto& pos = info.Definition.Pos;
		pushText("(" + pos + ")");
	}

	if(status & (int)StatusHintColumn::Register 
			&& info.Definition.Register.size() > 0)
	{ 
		const auto& reg = info.Definition.Register;
		pushText("[" + reg + "]"); 
	}

	if(status & (int)StatusHintColumn::Label) 
	{ pushText(info.Definition.Label); }

	if(status & (int)StatusHintColumn::Written) 
	{ pushText(info.Definition.Written); }

	if(status & (int)StatusHintColumn::Colloquial) 
	{ pushText(info.Definition.Colloquial); }

	if(status & (int)StatusHintColumn::Urd) 
	{ pushText(info.Definition.Language.Urd); }

	if(status & (int)StatusHintColumn::Nep) 
	{ pushText(info.Definition.Language.Nep); }

	if(status & (int)StatusHintColumn::Hin) 
	{ pushText(info.Definition.Language.Hin); }

	if(status & (int)StatusHintColumn::Ind) 
	{ pushText(info.Definition.Language.Ind); }

	return boost::algorithm::join_if(textContainer, "\t ", 
											[](const std::string& s) { return s.size() > 0; });
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
	/*  2 */ PronOrder = *column++;
	/*  3 */ Sandhi = *column++;
	/*  4 */ LitColReading = *column++;
	/*  5 */ Freq = *column++;
	/*  6 */ Freq2 = *column++;
	Definition = InfoDefinition();
	/*  7 */ Definition.English = *column++;
	/*  8 */ Definition.Disambiguation = *column++;
	/*  9 */ Definition.Pos = *column++;
	/* 10 */ Definition.Register = *column++;
	/* 11 */ Definition.Label = *column++;
	/* 12 */ Definition.Written = *column++;
	/* 13 */ Definition.Colloquial = *column++;
	Definition.Language = InfoLanguage();
	/* 14 */ Definition.Language.Urd = *column++;
	/* 15 */ Definition.Language.Nep = *column++;
	/* 16 */ Definition.Language.Hin = *column++;
	/* 17 */ Definition.Language.Ind = *column++;
}
