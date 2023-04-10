#pragma once

#include <unordered_set>
#include <boost/tokenizer.hpp>
#include <locale>
#include <codecvt>

typedef boost::char_separator<char32_t> sep;
typedef boost::tokenizer<sep, std::u32string::const_iterator, std::u32string> columns;

const std::unordered_set<char32_t> charWithZeroSyllables = { U'\uff0c', U'\uff1a' };
const std::unordered_set<char32_t> charWithTwoSyllablesIndividual = {
	U'\u5161', U'\u74f8', U'\u41c9', U'\u7ae1', U'\u5c23', U'\u515b', U'\u74e9', U'\u7acf', U'\U00025a95', U'\u515d',
	U'\u74f0', U'\u7ad5', U'\u55e7', U'\u6d6c', U'\u515e', U'\u74f1', U'\u7ad3', U'\u544e', U'\u540b', U'\u5562', U'\U00020e96',
	U'\u5163', U'\u7cce', U'\u7505', U'\u7af0', U'\u5345', U'\U00020be2', U'\u5159', U'\u74e7', U'\u41c6', U'\u7acd', U'\u534c',
};
const std::unordered_set<char32_t> charWithTwoSyllablesInPhrase = { U'\u5345', U'\U00020be2', U'\u534c' };

const sep spaceSep = sep(U" ");
const sep tabSep = sep(U"\t");
const sep lineSep = sep(U"\n");

static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv32to8;
static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv16to8;

static std::u32string conv16to32(std::wstring str) {
	return conv32to8.from_bytes(conv16to8.to_bytes(str));
}
static std::wstring conv32to16(std::u32string str) {
	return conv16to8.from_bytes(conv32to8.to_bytes(str));
}
