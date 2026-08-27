#include "TypeDuckProfile.h"

namespace Moqi {
namespace TypeDuck {
namespace {

constexpr const wchar_t* kInstallRegistrySubkey = L"Software\\TypeDuckIME";
constexpr const wchar_t* kInstallDirRegistryValue = L"InstallDir";

bool is64BitWindows() {
#if defined(_WIN64)
  return true;
#else
  BOOL isWow64 = FALSE;
  return ::IsWow64Process(::GetCurrentProcess(), &isWow64) && isWow64 == TRUE;
#endif
}

std::wstring registryInstallDirForView(REGSAM view) {
  HKEY key = nullptr;
  const LSTATUS openStatus = ::RegOpenKeyExW(
      HKEY_LOCAL_MACHINE, kInstallRegistrySubkey, 0,
      KEY_QUERY_VALUE | view, &key);
  if (openStatus != ERROR_SUCCESS) {
    return std::wstring();
  }

  DWORD type = 0;
  DWORD bytes = 0;
  LSTATUS queryStatus = ::RegQueryValueExW(
      key, kInstallDirRegistryValue, nullptr, &type, nullptr, &bytes);
  if (queryStatus != ERROR_SUCCESS || type != REG_SZ || bytes < sizeof(wchar_t)) {
    ::RegCloseKey(key);
    return std::wstring();
  }

  std::wstring value(bytes / sizeof(wchar_t), L'\0');
  queryStatus = ::RegQueryValueExW(
      key, kInstallDirRegistryValue, nullptr, &type,
      reinterpret_cast<LPBYTE>(value.data()), &bytes);
  ::RegCloseKey(key);
  if (queryStatus != ERROR_SUCCESS || type != REG_SZ) {
    return std::wstring();
  }

  value.resize(wcsnlen_s(value.c_str(), value.size()));
  return value;
}

std::wstring registryInstallDir() {
  if (std::wstring value = registryInstallDirForView(0); !value.empty()) {
    return value;
  }
  if (!is64BitWindows()) {
    return std::wstring();
  }
  if (std::wstring value = registryInstallDirForView(KEY_WOW64_64KEY); !value.empty()) {
    return value;
  }
  return registryInstallDirForView(KEY_WOW64_32KEY);
}

std::wstring environmentProgramDir() {
  wchar_t path[MAX_PATH] = {};
  DWORD length = ::GetEnvironmentVariableW(
      programDirEnvVar(), path, static_cast<DWORD>(_countof(path)));
  if (length > 0 && length < _countof(path)) {
    return path;
  }

  return std::wstring();
}

std::wstring preferredSmallIconPath(const std::wstring& fallbackIconFile) {
  std::wstring iconPath = environmentProgramDir();
  if (iconPath.empty()) {
    return fallbackIconFile;
  }

  if (!iconPath.empty() && iconPath.back() != L'\\' && iconPath.back() != L'/') {
    iconPath += L'\\';
  }
  iconPath += L"resources\\TypeDuck_Small.ico";
  if (::GetFileAttributesW(iconPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
    return fallbackIconFile;
  }
  return iconPath;
}

}  // namespace

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

const wchar_t* installDirName() {
  return L"TypeDuckIME";
}

std::wstring configuredProgramDir() {
  if (std::wstring value = environmentProgramDir(); !value.empty()) {
    return value;
  }
  return registryInstallDir();
}

Ime::LangProfileInfo makeLangProfile(const std::wstring& iconFile, int iconIndex) {
  const std::wstring profileIconFile = preferredSmallIconPath(iconFile);
  const int profileIconIndex = profileIconFile == iconFile ? iconIndex : 0;
  return Ime::LangProfileInfo{
      profileDisplayName(),
      kProfileGuid,
      localeName(),
      fallbackLocaleName(),
      profileIconFile,
      profileIconIndex};
}

}  // namespace TypeDuck
}  // namespace Moqi
