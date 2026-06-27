#include <windows.h>

#include <shellapi.h>

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

constexpr int kExitSuccess = 0;
constexpr int kExitFailure = 1;
constexpr int kExitRestartRequired = 2;
constexpr int kExitInvalidArgs = 3;
constexpr wchar_t kProgramDirEnvVar[] = L"TYPEDUCK_PROGRAM_DIR";
// Transition-only compatibility alias for Plan 03-01 registration paths.
constexpr wchar_t kLegacyProgramDirEnvVar[] = L"MOQI_PROGRAM_DIR";
constexpr wchar_t kReregisterTaskName[] = L"TypeDuckIME-ReRegisterTSF";
constexpr wchar_t kTextServiceDllName[] = L"TypeDuckTextService.dll";
constexpr wchar_t kSetupHelperCaption[] = L"TypeDuckSetupHelper";

enum class Action {
  kHelp,
  kInstall,
  kReregister,
  kUninstall,
};

struct Options {
  Action action = Action::kHelp;
  bool silent = false;
  std::wstring app_dir;
};

struct EnvironmentVariableSnapshot {
  std::wstring name;
  bool had_value = false;
  std::wstring value;
};

bool NeedsCommandLineQuoting(const std::wstring& value) {
  if (value.empty()) {
    return true;
  }
  for (const wchar_t ch : value) {
    if (ch == L' ' || ch == L'\t' || ch == L'"') {
      return true;
    }
  }
  return false;
}

std::wstring QuoteCommandLineArgument(const std::wstring& value) {
  if (!NeedsCommandLineQuoting(value)) {
    return value;
  }

  std::wstring quoted;
  quoted.push_back(L'"');
  size_t backslash_count = 0;
  for (const wchar_t ch : value) {
    if (ch == L'\\') {
      ++backslash_count;
      continue;
    }

    if (ch == L'"') {
      quoted.append(backslash_count * 2 + 1, L'\\');
      quoted.push_back(L'"');
      backslash_count = 0;
      continue;
    }

    quoted.append(backslash_count, L'\\');
    backslash_count = 0;
    quoted.push_back(ch);
  }

  quoted.append(backslash_count * 2, L'\\');
  quoted.push_back(L'"');
  return quoted;
}

std::wstring FormatWindowsErrorMessage(const DWORD error_code) {
  if (error_code == 0) {
    return L"Win32 error 0";
  }

  LPWSTR buffer = nullptr;
  const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                      FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS;
  const DWORD length = FormatMessageW(flags, nullptr, error_code, 0,
                                      reinterpret_cast<LPWSTR>(&buffer), 0,
                                      nullptr);
  std::wstring message = L"Win32 error " + std::to_wstring(error_code);
  if (length > 0 && buffer != nullptr) {
    DWORD trimmed_length = length;
    while (trimmed_length > 0 &&
           (buffer[trimmed_length - 1] == L'\r' ||
            buffer[trimmed_length - 1] == L'\n')) {
      buffer[trimmed_length - 1] = L'\0';
      --trimmed_length;
    }
    if (*buffer != L'\0') {
      message += L": ";
      message += buffer;
    }
  }
  if (buffer != nullptr) {
    LocalFree(buffer);
  }
  return message;
}

std::wstring Bilingual(const std::wstring& zh, const std::wstring& en) {
  return zh + L"\n" + en;
}

EnvironmentVariableSnapshot CaptureEnvironmentVariable(const wchar_t* name) {
  EnvironmentVariableSnapshot snapshot;
  snapshot.name = name;
  const DWORD length = GetEnvironmentVariableW(name, nullptr, 0);
  if (length > 0) {
    snapshot.had_value = true;
    snapshot.value.resize(length - 1);
    GetEnvironmentVariableW(name, snapshot.value.data(), length);
  }
  return snapshot;
}

void RestoreEnvironmentVariable(const EnvironmentVariableSnapshot& snapshot) {
  if (snapshot.had_value) {
    SetEnvironmentVariableW(snapshot.name.c_str(), snapshot.value.c_str());
  } else {
    SetEnvironmentVariableW(snapshot.name.c_str(), nullptr);
  }
}

std::wstring GetModulePath() {
  std::wstring path(MAX_PATH, L'\0');
  while (true) {
    const DWORD written = GetModuleFileNameW(nullptr, path.data(),
                                             static_cast<DWORD>(path.size()));
    if (written == 0) {
      return L"";
    }
    if (written < path.size() - 1) {
      path.resize(written);
      return path;
    }
    path.resize(path.size() * 2);
  }
}

std::wstring GetModuleDirectory() {
  const fs::path module_path(GetModulePath());
  return module_path.parent_path().wstring();
}

std::wstring JoinArguments(const std::vector<std::wstring>& args,
                           const size_t start_index) {
  std::wstring result;
  for (size_t i = start_index; i < args.size(); ++i) {
    if (!result.empty()) {
      result += L' ';
    }
    result += QuoteCommandLineArgument(args[i]);
  }
  return result;
}

void ShowMessage(const std::wstring& text,
                 const std::wstring& caption,
                 const UINT flags,
                 const bool silent) {
  if (!silent) {
    MessageBoxW(nullptr, text.c_str(), caption.c_str(), flags);
  }
}

std::vector<std::wstring> GetCommandLineArguments() {
  int argc = 0;
  LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (argv == nullptr) {
    return {};
  }

  std::vector<std::wstring> args;
  args.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }
  LocalFree(argv);
  return args;
}

bool IsRunningAsAdmin() {
  BOOL is_admin = FALSE;
  SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
  PSID admin_group = nullptr;
  if (!AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                &admin_group)) {
    return false;
  }

  if (!CheckTokenMembership(nullptr, admin_group, &is_admin)) {
    is_admin = FALSE;
  }
  FreeSid(admin_group);
  return is_admin == TRUE;
}

int RestartElevated(const std::vector<std::wstring>& args, const bool silent) {
  SHELLEXECUTEINFOW exec_info = {};
  exec_info.cbSize = sizeof(exec_info);
  exec_info.fMask = SEE_MASK_NOCLOSEPROCESS;
  exec_info.lpVerb = L"runas";
  const std::wstring module_path = GetModulePath();
  const std::wstring parameters = JoinArguments(args, 1);
  exec_info.lpFile = module_path.c_str();
  exec_info.lpParameters = parameters.empty() ? nullptr : parameters.c_str();
  exec_info.nShow = silent ? SW_HIDE : SW_SHOWNORMAL;

  if (!ShellExecuteExW(&exec_info)) {
    return kExitFailure;
  }

  WaitForSingleObject(exec_info.hProcess, INFINITE);
  DWORD exit_code = kExitFailure;
  if (!GetExitCodeProcess(exec_info.hProcess, &exit_code)) {
    exit_code = kExitFailure;
  }
  CloseHandle(exec_info.hProcess);
  return static_cast<int>(exit_code);
}

std::wstring GetWindowsDirectoryPath() {
  std::wstring path(MAX_PATH, L'\0');
  while (true) {
    const UINT written =
        GetWindowsDirectoryW(path.data(), static_cast<UINT>(path.size()));
    if (written == 0) {
      return L"";
    }
    if (written < path.size()) {
      path.resize(written);
      return path;
    }
    path.resize(written + 1);
  }
}

std::wstring GetSyswow64DirectoryPath() {
  std::wstring path(MAX_PATH, L'\0');
  const UINT written =
      GetSystemWow64DirectoryW(path.data(), static_cast<UINT>(path.size()));
  if (written > 0 && written < path.size()) {
    path.resize(written);
    return path;
  }

  const UINT fallback =
      GetSystemDirectoryW(path.data(), static_cast<UINT>(path.size()));
  if (fallback == 0) {
    return L"";
  }
  path.resize(fallback);
  return path;
}

std::wstring GetNativeSystemDirectoryPath() {
  const fs::path sysnative =
      fs::path(GetWindowsDirectoryPath()) / L"Sysnative";
  if (fs::exists(sysnative)) {
    return sysnative.wstring();
  }

  std::wstring path(MAX_PATH, L'\0');
  const UINT written =
      GetSystemDirectoryW(path.data(), static_cast<UINT>(path.size()));
  if (written == 0) {
    return L"";
  }
  path.resize(written);
  return path;
}

std::wstring GetNativeSystemDirectoryForChildProcess() {
  return (fs::path(GetWindowsDirectoryPath()) / L"System32").wstring();
}

std::wstring NormalizePathForPendingOperation(const std::wstring& path) {
  const std::wstring sysnative_prefix =
      fs::path(GetWindowsDirectoryPath() + L"\\Sysnative").wstring() + L"\\";
  if (_wcsnicmp(path.c_str(), sysnative_prefix.c_str(),
                sysnative_prefix.length()) == 0) {
    return (fs::path(GetWindowsDirectoryPath()) / L"System32" /
            path.substr(sysnative_prefix.length()))
        .wstring();
  }
  return path;
}

bool RunProcess(const std::wstring& application_path,
                std::wstring command,
                const std::wstring& working_dir,
                DWORD* exit_code,
                DWORD* error_code = nullptr) {
  if (error_code != nullptr) {
    *error_code = 0;
  }
  STARTUPINFOW startup_info = {};
  startup_info.cb = sizeof(startup_info);
  startup_info.dwFlags = STARTF_USESHOWWINDOW;
  startup_info.wShowWindow = SW_HIDE;
  PROCESS_INFORMATION process_info = {};

  const BOOL created =
      CreateProcessW(application_path.c_str(), command.data(), nullptr, nullptr,
                     FALSE, 0, nullptr,
                     working_dir.empty() ? nullptr : working_dir.c_str(),
                     &startup_info, &process_info);
  if (!created) {
    if (error_code != nullptr) {
      *error_code = GetLastError();
    }
    return false;
  }

  WaitForSingleObject(process_info.hProcess, INFINITE);
  DWORD process_exit_code = 0;
  const BOOL got_exit_code =
      GetExitCodeProcess(process_info.hProcess, &process_exit_code);
  CloseHandle(process_info.hThread);
  CloseHandle(process_info.hProcess);
  if (exit_code != nullptr) {
    *exit_code = got_exit_code ? process_exit_code : static_cast<DWORD>(-1);
  }
  if (!got_exit_code && error_code != nullptr) {
    *error_code = GetLastError();
  }
  return got_exit_code == TRUE;
}

bool RunRegsvr(const fs::path& regsvr_path,
               const fs::path& dll_path_for_process,
               const fs::path& program_dir,
               const bool unregister) {
  if (!fs::exists(dll_path_for_process)) {
    return true;
  }

  std::wstring command = QuoteCommandLineArgument(regsvr_path.wstring());
  if (unregister) {
    command += L" /u";
  }
  command += L" /s " + QuoteCommandLineArgument(dll_path_for_process.wstring());

  std::wstring mutable_command = command;
  std::wstring working_dir = dll_path_for_process.parent_path().wstring();
  const EnvironmentVariableSnapshot previous_program_dir =
      CaptureEnvironmentVariable(kProgramDirEnvVar);
  const EnvironmentVariableSnapshot previous_legacy_program_dir =
      CaptureEnvironmentVariable(kLegacyProgramDirEnvVar);
  SetEnvironmentVariableW(kProgramDirEnvVar, program_dir.c_str());
  SetEnvironmentVariableW(kLegacyProgramDirEnvVar, program_dir.c_str());

  DWORD exit_code = 0;
  const bool ran =
      RunProcess(regsvr_path.wstring(), mutable_command, working_dir, &exit_code);
  RestoreEnvironmentVariable(previous_program_dir);
  RestoreEnvironmentVariable(previous_legacy_program_dir);
  if (!ran) {
    return false;
  }
  return exit_code == 0;
}

fs::path BuildOldPath(const fs::path& destination) {
  for (int i = 0; i < 16; ++i) {
    fs::path old_path = destination;
    old_path += L".old." + std::to_wstring(i);
    if (!fs::exists(old_path)) {
      return old_path;
    }
  }
  fs::path old_path = destination;
  old_path += L".old";
  return old_path;
}

fs::path BuildPendingRebootPath(const fs::path& source) {
  for (int i = 0; i < 16; ++i) {
    fs::path pending_path = source;
    pending_path += L".pending.reboot." + std::to_wstring(i);
    if (!fs::exists(pending_path)) {
      return pending_path;
    }
  }
  fs::path pending_path = source;
  pending_path += L".pending.reboot";
  return pending_path;
}

void CleanupStalePendingFiles(const fs::path& destination) {
  std::error_code ec;
  const fs::path directory = destination.parent_path();
  if (!fs::exists(directory, ec)) {
    return;
  }

  const std::wstring prefix = destination.filename().wstring() + L".pending.";
  for (const auto& entry : fs::directory_iterator(directory, ec)) {
    if (ec || !entry.is_regular_file(ec)) {
      continue;
    }
    const std::wstring name = entry.path().filename().wstring();
    if (name.rfind(prefix, 0) == 0) {
      fs::remove(entry.path(), ec);
      ec.clear();
    }
  }
}

void CleanupStaleRebootCopies(const fs::path& source) {
  std::error_code ec;
  const fs::path directory = source.parent_path();
  if (!fs::exists(directory, ec)) {
    return;
  }

  const std::wstring prefix = source.filename().wstring() + L".pending.reboot";
  for (const auto& entry : fs::directory_iterator(directory, ec)) {
    if (ec || !entry.is_regular_file(ec)) {
      continue;
    }
    const std::wstring name = entry.path().filename().wstring();
    if (name.rfind(prefix, 0) == 0) {
      fs::remove(entry.path(), ec);
      ec.clear();
    }
  }
}

bool RenameFileForDeleteOnReboot(const fs::path& path, bool& reboot_required) {
  if (!fs::exists(path)) {
    return true;
  }

  const fs::path old_path = BuildOldPath(path);
  if (MoveFileExW(path.c_str(), old_path.c_str(), MOVEFILE_REPLACE_EXISTING) ==
      TRUE) {
    const std::wstring pending_delete_path =
        NormalizePathForPendingOperation(old_path.wstring());
    if (MoveFileExW(pending_delete_path.c_str(), nullptr,
                    MOVEFILE_DELAY_UNTIL_REBOOT) ==
        TRUE) {
      reboot_required = true;
      return true;
    }
    MoveFileExW(old_path.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING);
    return false;
  }
  return false;
}

bool ScheduleReplaceOnReboot(const fs::path& source,
                            const fs::path& destination,
                            bool& reboot_required,
                            std::wstring* error) {
  CleanupStaleRebootCopies(source);

  const fs::path staged_source = BuildPendingRebootPath(source);
  if (CopyFileW(source.c_str(), staged_source.c_str(), FALSE) != TRUE) {
    if (error != nullptr) {
      *error = Bilingual(
          L"未能建立重啟後替換用的暫存副本: " + staged_source.wstring(),
          L"Failed to create staged reboot copy: " + staged_source.wstring()) +
               L"\n(" + FormatWindowsErrorMessage(GetLastError()) + L").";
    }
    return false;
  }

  const std::wstring normalized_staged_source =
      NormalizePathForPendingOperation(staged_source.wstring());
  const std::wstring normalized_destination =
      NormalizePathForPendingOperation(destination.wstring());
  if (MoveFileExW(normalized_staged_source.c_str(),
                  normalized_destination.c_str(),
                  MOVEFILE_DELAY_UNTIL_REBOOT | MOVEFILE_REPLACE_EXISTING) !=
      TRUE) {
    const DWORD move_error = GetLastError();
    std::error_code ec;
    fs::remove(staged_source, ec);
    if (error != nullptr) {
      *error = Bilingual(
          L"未能安排重啟後替換 TSF DLL: " + destination.wstring(),
          L"Failed to schedule reboot replacement for TSF DLL: " +
              destination.wstring()) +
               L"\n" + staged_source.wstring() + L"\n(" +
               FormatWindowsErrorMessage(move_error) + L").";
    }
    return false;
  }

  reboot_required = true;
  if (error != nullptr) {
    error->clear();
  }
  return true;
}

void CleanupStaleOldFiles(const fs::path& destination) {
  std::error_code ec;
  const fs::path directory = destination.parent_path();
  if (!fs::exists(directory, ec)) {
    return;
  }

  const std::wstring prefix = destination.filename().wstring() + L".old";
  for (const auto& entry : fs::directory_iterator(directory, ec)) {
    if (ec || !entry.is_regular_file(ec)) {
      continue;
    }
    const std::wstring name = entry.path().filename().wstring();
    if (name.rfind(prefix, 0) == 0) {
      fs::remove(entry.path(), ec);
      ec.clear();
    }
  }
}

bool DeleteReregisterTask() {
  const fs::path schtasks =
      fs::path(GetNativeSystemDirectoryForChildProcess()) / L"schtasks.exe";
  std::wstring command = QuoteCommandLineArgument(schtasks.wstring()) +
                         L" /Delete /TN " +
                         QuoteCommandLineArgument(kReregisterTaskName) + L" /F";
  DWORD exit_code = 0;
  if (!RunProcess(schtasks.wstring(), command, GetModuleDirectory(), &exit_code)) {
    return false;
  }
  return exit_code == 0 || exit_code == 1;
}

bool ScheduleReregisterTask(const Options& options, std::wstring& error) {
  const fs::path schtasks =
      fs::path(GetNativeSystemDirectoryForChildProcess()) / L"schtasks.exe";
  const std::wstring task_command =
      QuoteCommandLineArgument(GetModulePath()) + L" /r /s --appdir " +
      QuoteCommandLineArgument(options.app_dir);
  std::wstring command = QuoteCommandLineArgument(schtasks.wstring()) +
                         L" /Create /TN " +
                         QuoteCommandLineArgument(kReregisterTaskName) +
                         L" /SC ONSTART /RU SYSTEM /RL HIGHEST /TR " +
                         QuoteCommandLineArgument(task_command) + L" /F";
  DWORD exit_code = 0;
  DWORD error_code = 0;
  if (!RunProcess(schtasks.wstring(), command, GetModuleDirectory(), &exit_code,
                  &error_code)) {
    error = Bilingual(L"未能啟動 schtasks.exe。",
                      L"Failed to launch schtasks.exe.") +
            L"\n(" + FormatWindowsErrorMessage(error_code) + L").";
    return false;
  }
  if (exit_code != 0) {
    error = Bilingual(
        L"未能安排 Windows 重啟後重新註冊 TypeDuck TSF。",
        L"Failed to schedule TypeDuck TSF re-registration after reboot.") +
            L"\n(schtasks exit code " + std::to_wstring(exit_code) + L").";
    return false;
  }
  return true;
}

bool DeleteFileWithFallback(const fs::path& path, bool& reboot_required) {
  if (!fs::exists(path)) {
    return true;
  }
  if (DeleteFileW(path.c_str()) == TRUE) {
    return true;
  }
  return RenameFileForDeleteOnReboot(path, reboot_required);
}

bool CopyFileWithFallback(const fs::path& source,
                         const fs::path& destination,
                         bool& reboot_required,
                         std::wstring* error,
                         DWORD* initial_copy_error = nullptr,
                         DWORD* fallback_error = nullptr) {
  if (initial_copy_error != nullptr) {
    *initial_copy_error = 0;
  }
  if (fallback_error != nullptr) {
    *fallback_error = 0;
  }
  if (!fs::exists(source)) {
    if (error != nullptr) {
      *error = Bilingual(L"找不到來源檔案: " + source.wstring(),
                         L"Source file does not exist: " + source.wstring());
    }
    return false;
  }
  CleanupStalePendingFiles(destination);
  if (CopyFileW(source.c_str(), destination.c_str(), FALSE) == TRUE) {
    if (error != nullptr) {
      error->clear();
    }
    return true;
  }
  const DWORD initial_copy_error_code = GetLastError();
  if (initial_copy_error != nullptr) {
    *initial_copy_error = initial_copy_error_code;
  }
  if (RenameFileForDeleteOnReboot(destination, reboot_required)) {
    if (CopyFileW(source.c_str(), destination.c_str(), FALSE) == TRUE) {
      if (error != nullptr) {
        error->clear();
      }
      CleanupStalePendingFiles(destination);
      return true;
    }
    const DWORD retry_copy_error = GetLastError();
    if (fallback_error != nullptr) {
      *fallback_error = retry_copy_error;
    }
    if (error != nullptr) {
      *error = Bilingual(
          L"初次複製失敗；已安排舊檔重啟後刪除，但重新複製仍然失敗。",
          L"Initial copy failed; the old file was scheduled for delete-on-reboot, but retry copy also failed.") +
          L"\n(" + FormatWindowsErrorMessage(initial_copy_error_code) +
          L")\n(" + FormatWindowsErrorMessage(retry_copy_error) + L").";
    }
    return false;
  }
  if (error != nullptr) {
    const DWORD rename_error = GetLastError();
    if (fallback_error != nullptr) {
      *fallback_error = rename_error;
    }
    *error = Bilingual(
        L"初次複製失敗；後備重新命名及重啟後刪除亦失敗。",
        L"Initial copy failed; fallback rename/delete-on-reboot also failed.") +
        L"\n(" + FormatWindowsErrorMessage(initial_copy_error_code) +
        L")\n(" + FormatWindowsErrorMessage(rename_error) + L").";
  }
  return false;
}

int ShowFailureAndReturn(const std::wstring& message, const bool silent) {
  ShowMessage(message, kSetupHelperCaption, MB_ICONERROR | MB_OK, silent);
  return kExitFailure;
}

std::wstring TypingSetupFailureMessage() {
  return Bilingual(
      L"TypeDuck 未能連接至 Windows 輸入功能。請重新啟動電腦，然後再次執行安裝程式。",
      L"TypeDuck could not connect to Windows typing. Please restart your computer, then run the installer again.");
}

int RunReregister(const Options& options) {
  const fs::path app_dir(options.app_dir);
  const fs::path source32 = app_dir / kTextServiceDllName;
  const fs::path source64 = app_dir / L"x64" / kTextServiceDllName;
  const fs::path dest32 = fs::path(GetSyswow64DirectoryPath()) / kTextServiceDllName;
  const fs::path dest64 = fs::path(GetNativeSystemDirectoryPath()) / kTextServiceDllName;
  const fs::path dest64_for_regsvr =
      fs::path(GetNativeSystemDirectoryForChildProcess()) / kTextServiceDllName;
  const fs::path regsvr32 = fs::path(GetSyswow64DirectoryPath()) / L"regsvr32.exe";
  const fs::path regsvr64 = fs::path(GetNativeSystemDirectoryPath()) / L"regsvr32.exe";

  CleanupStaleOldFiles(dest32);
  CleanupStaleOldFiles(dest64);
  CleanupStaleRebootCopies(source32);
  CleanupStaleRebootCopies(source64);

  if (!RunRegsvr(regsvr32, dest32, app_dir, false)) {
    return ShowFailureAndReturn(TypingSetupFailureMessage(), options.silent);
  }
  if (!RunRegsvr(regsvr64, dest64_for_regsvr, app_dir, false)) {
    return ShowFailureAndReturn(TypingSetupFailureMessage(), options.silent);
  }
  DeleteReregisterTask();
  return kExitSuccess;
}

int RunInstall(const Options& options) {
  const fs::path app_dir(options.app_dir);
  const fs::path source32 = app_dir / kTextServiceDllName;
  const fs::path source64 = app_dir / L"x64" / kTextServiceDllName;
  // TSF DLLs must live in system directories, or IME input will not work in
  // some games such as CS2.
  const fs::path dest32 = fs::path(GetSyswow64DirectoryPath()) / kTextServiceDllName;
  const fs::path dest64 = fs::path(GetNativeSystemDirectoryPath()) / kTextServiceDllName;
  const fs::path dest64_for_regsvr =
      fs::path(GetNativeSystemDirectoryForChildProcess()) / kTextServiceDllName;
  const fs::path regsvr32 = fs::path(GetSyswow64DirectoryPath()) / L"regsvr32.exe";
  const fs::path regsvr64 = fs::path(GetNativeSystemDirectoryPath()) / L"regsvr32.exe";

  if (!fs::exists(source32)) {
    return ShowFailureAndReturn(
        Bilingual(L"缺少 Win32 TypeDuck 安裝檔案: " + source32.wstring(),
                  L"Missing Win32 TypeDuck payload: " + source32.wstring()),
        options.silent);
  }
  if (!fs::exists(source64)) {
    return ShowFailureAndReturn(
        Bilingual(L"缺少 x64 TypeDuck 安裝檔案: " + source64.wstring(),
                  L"Missing x64 TypeDuck payload: " + source64.wstring()),
        options.silent);
  }

  DeleteReregisterTask();
  // During an in-place reinstall/upgrade, unregistering first removes the TIP
  // from the user's language profile list. Re-registering the DLL does not
  // always restore that list entry reliably, so keep the existing registration
  // in place and overwrite the system DLLs before registering again.

  bool reboot_required = false;
  std::wstring copy_error;
  DWORD initial_copy_error = 0;
  DWORD fallback_error = 0;
  if (!CopyFileWithFallback(source32, dest32, reboot_required, &copy_error,
                            &initial_copy_error, &fallback_error)) {
    if (!((initial_copy_error == ERROR_SHARING_VIOLATION ||
           initial_copy_error == ERROR_ACCESS_DENIED ||
           fallback_error == ERROR_SHARING_VIOLATION ||
           fallback_error == ERROR_ACCESS_DENIED) &&
          ScheduleReplaceOnReboot(source32, dest32, reboot_required,
                                  &copy_error))) {
      return ShowFailureAndReturn(
          Bilingual(L"未能更新 Win32 TypeDuck TSF DLL: " + dest32.wstring(),
                    L"Failed to update Win32 TypeDuck TSF DLL: " +
                        dest32.wstring()) +
              L"\n\n" + copy_error,
          options.silent);
    }
  }
  initial_copy_error = 0;
  fallback_error = 0;
  if (!CopyFileWithFallback(source64, dest64, reboot_required, &copy_error,
                            &initial_copy_error, &fallback_error)) {
    if (!((initial_copy_error == ERROR_SHARING_VIOLATION ||
           initial_copy_error == ERROR_ACCESS_DENIED ||
           fallback_error == ERROR_SHARING_VIOLATION ||
           fallback_error == ERROR_ACCESS_DENIED) &&
          ScheduleReplaceOnReboot(source64, dest64, reboot_required,
                                  &copy_error))) {
      return ShowFailureAndReturn(
          Bilingual(L"未能更新 x64 TypeDuck TSF DLL: " + dest64.wstring(),
                    L"Failed to update x64 TypeDuck TSF DLL: " +
                        dest64.wstring()) +
              L"\n\n" + copy_error,
          options.silent);
    }
  }

  if (reboot_required) {
    std::wstring schedule_error;
    if (!ScheduleReregisterTask(options, schedule_error)) {
      return ShowFailureAndReturn(schedule_error, options.silent);
    }
    return kExitRestartRequired;
  }

  if (!RunRegsvr(regsvr32, dest32, app_dir, false)) {
    return ShowFailureAndReturn(TypingSetupFailureMessage(), options.silent);
  }
  if (!RunRegsvr(regsvr64, dest64_for_regsvr, app_dir, false)) {
    return ShowFailureAndReturn(TypingSetupFailureMessage(), options.silent);
  }
  return kExitSuccess;
}

int RunUninstall(const Options& options) {
  const fs::path app_dir(options.app_dir);
  const fs::path dest32 = fs::path(GetSyswow64DirectoryPath()) / kTextServiceDllName;
  const fs::path dest64 = fs::path(GetNativeSystemDirectoryPath()) / kTextServiceDllName;
  const fs::path dest64_for_regsvr =
      fs::path(GetNativeSystemDirectoryForChildProcess()) / kTextServiceDllName;
  const fs::path regsvr32 = fs::path(GetSyswow64DirectoryPath()) / L"regsvr32.exe";
  const fs::path regsvr64 = fs::path(GetNativeSystemDirectoryPath()) / L"regsvr32.exe";

  DeleteReregisterTask();
  RunRegsvr(regsvr32, dest32, app_dir, true);
  RunRegsvr(regsvr64, dest64_for_regsvr, app_dir, true);

  bool reboot_required = false;
  if (!DeleteFileWithFallback(dest32, reboot_required)) {
    return ShowFailureAndReturn(
        Bilingual(L"未能移除 Win32 TypeDuck TSF DLL: " + dest32.wstring(),
                  L"Failed to remove Win32 TypeDuck TSF DLL: " +
                      dest32.wstring()),
        options.silent);
  }
  if (!DeleteFileWithFallback(dest64, reboot_required)) {
    return ShowFailureAndReturn(
        Bilingual(L"未能移除 x64 TypeDuck TSF DLL: " + dest64.wstring(),
                  L"Failed to remove x64 TypeDuck TSF DLL: " +
                      dest64.wstring()),
        options.silent);
  }
  return reboot_required ? kExitRestartRequired : kExitSuccess;
}

void ShowUsage() {
  const std::wstring help_text =
      L"用法 / Usage: TypeDuckSetupHelper.exe /i|/r|/u [/s] [--appdir <path>]\n"
      L"  /i       安裝或更新 TypeDuck TSF DLLs / Install or upgrade the TypeDuck TSF DLLs.\n"
      L"  /r       重啟後重新註冊 TSF DLLs / Register the TSF DLLs after a reboot.\n"
      L"  /u       解除安裝 TSF DLLs / Uninstall the TSF DLLs.\n"
      L"  /s       靜默模式 / Silent mode.\n"
      L"  --appdir 指定應用程式資料夾 / Explicit application directory.\n";
  MessageBoxW(nullptr, help_text.c_str(), kSetupHelperCaption,
              MB_ICONINFORMATION | MB_OK);
}

bool ParseOptions(const std::vector<std::wstring>& args,
                  Options& options,
                  std::wstring& error) {
  options.app_dir = GetModuleDirectory();

  for (size_t i = 1; i < args.size(); ++i) {
    const std::wstring& arg = args[i];
    if (arg == L"/i") {
      if (options.action != Action::kHelp) {
        error = Bilingual(L"只可以指定一個動作。",
                          L"Only one action may be specified.");
        return false;
      }
      options.action = Action::kInstall;
    } else if (arg == L"/r") {
      if (options.action != Action::kHelp) {
        error = Bilingual(L"只可以指定一個動作。",
                          L"Only one action may be specified.");
        return false;
      }
      options.action = Action::kReregister;
    } else if (arg == L"/u") {
      if (options.action != Action::kHelp) {
        error = Bilingual(L"只可以指定一個動作。",
                          L"Only one action may be specified.");
        return false;
      }
      options.action = Action::kUninstall;
    } else if (arg == L"/s") {
      options.silent = true;
    } else if (arg == L"/?" || arg == L"/help" || arg == L"--help") {
      options.action = Action::kHelp;
    } else if (arg == L"--appdir") {
      if (i + 1 >= args.size()) {
        error = Bilingual(L"--appdir 需要路徑。",
                          L"--appdir requires a path.");
        return false;
      }
      options.app_dir = args[++i];
    } else if (arg.rfind(L"--appdir=", 0) == 0) {
      options.app_dir = arg.substr(9);
    } else {
      error = Bilingual(L"不明參數: " + arg,
                        L"Unknown argument: " + arg);
      return false;
    }
  }

  if (options.action == Action::kHelp && args.size() > 1 &&
      options.app_dir == GetModuleDirectory()) {
    error = Bilingual(L"未指定動作。", L"No action specified.");
    return false;
  }
  return true;
}

}  // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  const std::vector<std::wstring> args = GetCommandLineArguments();
  Options options;
  std::wstring error;
  if (!ParseOptions(args, options, error)) {
    ShowMessage(error, kSetupHelperCaption, MB_ICONERROR | MB_OK, false);
    ShowUsage();
    return kExitInvalidArgs;
  }

  if (options.action == Action::kHelp) {
    ShowUsage();
    return kExitSuccess;
  }

  if (!IsRunningAsAdmin()) {
    return RestartElevated(args, options.silent);
  }

  if (options.action == Action::kInstall) {
    return RunInstall(options);
  }
  if (options.action == Action::kReregister) {
    return RunReregister(options);
  }
  return RunUninstall(options);
}
