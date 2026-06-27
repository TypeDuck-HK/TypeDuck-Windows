//
//	Copyright (C) 2013 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
//
//	This library is free software; you can redistribute it and/or
//	modify it under the terms of the GNU Library General Public
//	License as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	You should have received a copy of the GNU Library General Public
//	License along with this library; if not, write to the
//	Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
//	Boston, MA  02110-1301, USA.
//

#include "MoqiImeModule.h"
#include "../libIME2/src/Utils.h"
#include "MoqiTextService.h"
#include "TypeDuckProfile.h"
#include <Shellapi.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <cstring>
#include <fstream>
#include <json/json.h>
#include <string>

namespace Moqi {

const GUID g_textServiceClsid = TypeDuck::kTextServiceClsid;

namespace {

constexpr const wchar_t* kTypeDuckSettingsExecutable = L"TypeDuckSettings.exe";
constexpr const wchar_t* kTypeDuckRuntimeDir = L"TypeDuckRuntime";

std::wstring getConfiguredProgramDir() {
  wchar_t path[MAX_PATH] = {};
  DWORD len = ::GetEnvironmentVariableW(TypeDuck::programDirEnvVar(), path, _countof(path));
  if (len > 0 && len < _countof(path)) {
    return path;
  }

  len = ::GetEnvironmentVariableW(TypeDuck::legacyProgramDirEnvVar(), path, _countof(path));
  if (len > 0 && len < _countof(path)) {
    return path;
  }

  HRESULT result;
  result = ::SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, path);
  if (result != S_OK) {
    result = ::SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, 0, path);
  }
  if (result == S_OK) {
    return TypeDuck::defaultProgramDir(path);
  }
  return std::wstring();
}

std::wstring buildTypeDuckSettingsPath(const std::wstring& programDir) {
  if (programDir.empty()) {
    return kTypeDuckSettingsExecutable;
  }
  std::wstring settingsPath = programDir;
  settingsPath += L"\\";
  settingsPath += kTypeDuckSettingsExecutable;
  return settingsPath;
}

bool launchTypeDuckSettings(HWND hwndParent, const std::wstring& programDir) {
  const std::wstring settingsPath = buildTypeDuckSettingsPath(programDir);
  const HINSTANCE result = ::ShellExecuteW(
      hwndParent, L"open", settingsPath.c_str(), nullptr,
      programDir.empty() ? nullptr : programDir.c_str(), SW_SHOWNORMAL);
  return reinterpret_cast<INT_PTR>(result) > 32;
}

void loadFixedRuntimeDirs(std::vector<std::wstring>& backendDirs) {
  backendDirs.clear();
  backendDirs.emplace_back(kTypeDuckRuntimeDir);
}

}  // namespace

ImeModule::ImeModule(HMODULE module)
    : Ime::ImeModule(module, g_textServiceClsid) {
  programDir_ = getConfiguredProgramDir();
  if (!programDir_.empty()) {
    loadFixedRuntimeDirs(backendDirs_);
  }
}

ImeModule::~ImeModule(void) {}

// virtual
Ime::TextService *ImeModule::createTextService() {
  TextService *service = new Moqi::TextService(this);
  return service;
}

bool ImeModule::loadImeInfo(const std::string &guid, std::wstring &filePath,
                            Json::Value &content) {
  bool found = false;
  // find the input method module
  for (const auto backendDir : backendDirs_) {
    std::wstring dirPath = programDir_;
    dirPath += '\\';
    dirPath += backendDir;
    dirPath += L"\\input_methods";
    // scan the dir for lang profile definition files
    WIN32_FIND_DATA findData = {0};
    HANDLE hFind = ::FindFirstFile((dirPath + L"\\*").c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
      do {
        if (findData.dwFileAttributes &
            FILE_ATTRIBUTE_DIRECTORY) { // this is a subdir
          if (findData.cFileName[0] != '.') {
            std::wstring imejson = dirPath;
            imejson += '\\';
            imejson += findData.cFileName;
            imejson += L"\\ime.json";
            std::ifstream fp(imejson, std::ifstream::binary);
            if (fp) {
              content.clear();
              fp >> content;
              if (stricmp(guid.c_str(), content["guid"].asCString()) == 0) {
                // found the language profile
                found = true;
                filePath = imejson;
                break;
              }
            }
          }
        }
      } while (::FindNextFile(hFind, &findData));
      ::FindClose(hFind);
    }
    if (found)
      break;
  }
  return found;
}

// virtual
bool ImeModule::onConfigure(HWND hwndParent, LANGID langid,
                            REFGUID rguidProfile) {
  (void)langid;
  (void)rguidProfile;

  if (!launchTypeDuckSettings(hwndParent, programDir_)) {
    ::MessageBoxW(
        hwndParent,
        L"未能開啟 TypeDuck 設定。請確認 TypeDuckSettings.exe 已安裝。\n"
        L"Unable to open TypeDuck Settings. Please confirm TypeDuckSettings.exe is installed.",
        L"TypeDuck 設定 / TypeDuck Settings", MB_OK | MB_ICONWARNING);
    return false;
  }
  return true;
}

} // namespace Moqi
