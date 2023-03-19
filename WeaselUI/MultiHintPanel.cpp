#include "stdafx.h"
#include "MultiHintPanel.h"
#include <locale>
#include <codecvt>

void MultiHintPanel::applyMultiHint(weasel::Text& comment)
{
	//setup converter
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	std::wstring& str = comment.str;
	if (str.find(',') == std::wstring::npos) { // std::count(str.begin(), str.end(), ',') < 16
		return;
	}
	InfoMultiHint info_(converter.to_bytes(str));

	std::string& eng = info_.Definition.English;
	std::string hint =
		info_.Jyutping + "\t" +
		(info_.Definition.Pos.empty() ? "" : "(" + info_.Definition.Pos + ")") + "\t" +
		(info_.Definition.Label.empty() ? "" : "[" + info_.Definition.Label + "]") + "\t" +
		(eng.length() > 25 ? eng.substr(0, eng.find_last_not_of(" ", 20) + 1) : eng) + "\t" +
		info_.Definition.Language.Urd;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	str = converter.from_bytes(hint);
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
