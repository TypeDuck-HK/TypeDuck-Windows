#ifndef TYPEDUCK_PROFILE_H
#define TYPEDUCK_PROFILE_H

#include "../libIME2/src/ImeModule.h"

#include <Windows.h>
#include <string>

namespace Moqi {
namespace TypeDuck {

extern const GUID kTextServiceClsid;
extern const GUID kProfileGuid;

const wchar_t* serviceName();
const wchar_t* profileDisplayName();
const wchar_t* localeName();
const wchar_t* fallbackLocaleName();
const wchar_t* deployedDllName();
const wchar_t* programDirEnvVar();
const wchar_t* legacyProgramDirEnvVar();
const wchar_t* installDirName();

std::wstring defaultProgramDir(const wchar_t* programFilesDir);
Ime::LangProfileInfo makeLangProfile(const std::wstring& iconFile, int iconIndex);

}  // namespace TypeDuck
}  // namespace Moqi

#endif  // TYPEDUCK_PROFILE_H
