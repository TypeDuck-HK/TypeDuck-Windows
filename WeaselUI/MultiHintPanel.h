#pragma once

#include <WeaselCommon.h>
#include <boost/tokenizer.hpp>

struct InfoLanguage {
	std::string Urd;
	std::string Nep;
	std::string Hin;
	std::string Ind;
};

struct InfoDefinition {
	std::string English;
	std::string Disambiguation;
	std::string Pos;
	std::string Register;
	std::string Label;
	std::string Written;
	std::string Colloquial;
	InfoLanguage Language;
};

struct InfoMultiHint {
	InfoMultiHint(const std::string& input);
	std::string Honzi;
	std::string Jyutping;
	std::string PronOrder;
	std::string Sandhi;
	std::string LitColReading;
	std::string Freq;
	std::string Freq2;
	InfoDefinition Definition; // std::vector<std::string> if necessary
};

class MultiHintPanel
{
public:
	void applyMultiHint(weasel::Text& comment);
};
