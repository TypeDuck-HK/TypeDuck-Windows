#include "TypeDuckProfile.h"

namespace Moqi {
namespace TypeDuck {
// TypeDuck text service CLSID {7D92985A-BC53-47B5-A5CC-6E47F86B9D18}
const GUID kTextServiceClsid = {
    0x7d92985a,
    0xbc53,
    0x47b5,
    {0xa5, 0xcc, 0x6e, 0x47, 0xf8, 0x6b, 0x9d, 0x18}};

// TypeDuck Cantonese zh-HK profile GUID {C6E8F5DF-6504-44F9-B7CF-17A195373A83}
const GUID kProfileGuid = {
    0xc6e8f5df,
    0x6504,
    0x44f9,
    {0xb7, 0xcf, 0x17, 0xa1, 0x95, 0x37, 0x3a, 0x83}};

const wchar_t* serviceName() {
  return L"TypeDuckTextService";
}

const wchar_t* profileDisplayName() {
  return L"TypeDuck 粵語輸入法 / TypeDuck Cantonese IME";
}

const wchar_t* localeName() {
  return L"zh-HK";
}

const wchar_t* fallbackLocaleName() {
  return L"zh-Hant-HK";
}

const wchar_t* deployedDllName() {
  return L"TypeDuckTextService.dll";
}

const wchar_t* programDirEnvVar() {
  return L"TYPEDUCK_PROGRAM_DIR";
}

const wchar_t* legacyProgramDirEnvVar() {
  return L"MOQI_PROGRAM_DIR";
}

const wchar_t* installDirName() {
  return L"TypeDuckIME";
}

std::wstring defaultProgramDir(const wchar_t* programFilesDir) {
  if (programFilesDir == nullptr || programFilesDir[0] == L'\0') {
    return std::wstring();
  }
  std::wstring programDir = programFilesDir;
  programDir += L"\\";
  programDir += installDirName();
  return programDir;
}

Ime::LangProfileInfo makeLangProfile(const std::wstring& iconFile, int iconIndex) {
  return Ime::LangProfileInfo{
      profileDisplayName(),
      kProfileGuid,
      localeName(),
      fallbackLocaleName(),
      iconFile,
      iconIndex};
}

}  // namespace TypeDuck
}  // namespace Moqi
