#pragma once
#include <WeaselCommon.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
class InfoMultiHint {
public:
	InfoMultiHint(const std::string& json);
	std::string Jyutping;
	std::string Eng;
	std::string Pos;
	std::string Label;
private:
	void parseJSON(std::string input);

};

class MultiHintPanel
{
public:
	void applyMultiHint(weasel::Text& comment);
};

