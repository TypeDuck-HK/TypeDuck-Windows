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
	if (str.size() < 2) {
		return;
	}
	if (str.at(0) != '{' || str.at(str.size() - 1) != '}') {
		return;
	}
	InfoMultiHint info_(converter.to_bytes(str));
	
	std::string hint = boost::algorithm::join(std::vector<std::string>{
		info_.Jyutping, 
		info_.Eng, 
		info_.Pos, 
		info_.Label
		}, "\n");

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	str = converter.from_bytes(hint);
}

InfoMultiHint::InfoMultiHint(const std::string& json)
{
	parseJSON(json);
}

void InfoMultiHint::parseJSON(std::string input)
{
	std::string inputStr(input.begin(), input.end());
	std::stringstream ss(inputStr);
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(ss, pt);
  std::vector<std::string> engs;
	std::vector<std::string> poss;
	std::vector<std::string> lbls;
	for (const auto& def : pt.get_child("defs")) {
		engs.push_back(def.second.get("eng",""));
		poss.push_back(def.second.get("pos", ""));
		lbls.push_back(def.second.get("lbl", ""));
	}
	Eng = boost::algorithm::join(engs, ";");
	Pos = boost::algorithm::join(poss, ";");
	Label = boost::algorithm::join(lbls, ";");
}
