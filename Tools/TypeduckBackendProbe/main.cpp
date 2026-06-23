#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "proto/ProtoFraming.h"
#include "proto/moqi.pb.h"

namespace {

constexpr std::uint32_t kMaxFrameBytes = 4 * 1024 * 1024;
constexpr DWORD kDefaultTimeoutMs = 15000;
constexpr wchar_t kRimeGuid[] = L"{3F6B5A12-8D44-4E71-9A2E-6B4F9C1D2A30}";

struct Options {
  std::filesystem::path serverExe;
  std::filesystem::path workingDir;
  std::filesystem::path runtimeManifest;
  std::string runtimeManifestSha256;
  std::filesystem::path adapterRoot;
  std::string adapterCommit;
  std::string adapterServerSha256;
  std::filesystem::path outputPath;
  std::filesystem::path frameLogPath;
  DWORD timeoutMs = kDefaultTimeoutMs;
};

struct Candidate {
  std::string text;
  std::string comment;
};

struct PageState {
  bool showCandidates = false;
  int candidateCursor = -1;
  std::string selectKeys;
};

struct InputEvidence {
  std::string name;
  std::vector<char> keys;
  std::string composition;
  std::vector<Candidate> candidates;
  std::vector<std::string> candidateList;
  std::vector<std::string> jyutpingOrComments;
  PageState page;
  std::string commit;
};

std::string toUtf8(const std::wstring &value) {
  if (value.empty()) {
    return {};
  }
  const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                                       nullptr, 0, nullptr, nullptr);
  if (size <= 0) {
    return {};
  }
  std::string result(size, '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                      result.data(), size, nullptr, nullptr);
  return result;
}

std::wstring quoteArg(const std::filesystem::path &path) {
  std::wstring value = path.wstring();
  std::wstring escaped = L"\"";
  for (wchar_t ch : value) {
    if (ch == L'"') {
      escaped += L"\\\"";
    } else {
      escaped += ch;
    }
  }
  escaped += L"\"";
  return escaped;
}

std::string jsonEscape(const std::string &value) {
  std::ostringstream out;
  for (unsigned char ch : value) {
    switch (ch) {
    case '"':
      out << "\\\"";
      break;
    case '\\':
      out << "\\\\";
      break;
    case '\b':
      out << "\\b";
      break;
    case '\f':
      out << "\\f";
      break;
    case '\n':
      out << "\\n";
      break;
    case '\r':
      out << "\\r";
      break;
    case '\t':
      out << "\\t";
      break;
    default:
      if (ch < 0x20) {
        out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(ch)
            << std::dec << std::setfill(' ');
      } else {
        out << static_cast<char>(ch);
      }
      break;
    }
  }
  return out.str();
}

std::string jsonString(const std::string &value) {
  return "\"" + jsonEscape(value) + "\"";
}

std::string jsonString(const char *value) {
  return jsonString(std::string(value != nullptr ? value : ""));
}

std::string jsonString(const std::filesystem::path &path) {
  return jsonString(toUtf8(path.wstring()));
}

std::string base64Encode(const std::string &value) {
  static constexpr char kTable[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string result;
  result.reserve(((value.size() + 2) / 3) * 4);
  for (size_t i = 0; i < value.size(); i += 3) {
    const auto b0 = static_cast<unsigned char>(value[i]);
    const auto b1 = (i + 1 < value.size()) ? static_cast<unsigned char>(value[i + 1]) : 0;
    const auto b2 = (i + 2 < value.size()) ? static_cast<unsigned char>(value[i + 2]) : 0;
    result.push_back(kTable[b0 >> 2]);
    result.push_back(kTable[((b0 & 0x03) << 4) | (b1 >> 4)]);
    result.push_back(i + 1 < value.size() ? kTable[((b1 & 0x0f) << 2) | (b2 >> 6)] : '=');
    result.push_back(i + 2 < value.size() ? kTable[b2 & 0x3f] : '=');
  }
  return result;
}

std::string nowUtcIso() {
  SYSTEMTIME st{};
  GetSystemTime(&st);
  std::ostringstream out;
  out << std::setfill('0') << std::setw(4) << st.wYear << "-" << std::setw(2) << st.wMonth
      << "-" << std::setw(2) << st.wDay << "T" << std::setw(2) << st.wHour << ":"
      << std::setw(2) << st.wMinute << ":" << std::setw(2) << st.wSecond << "Z";
  return out.str();
}

void ensureParentDirectory(const std::filesystem::path &path) {
  const auto parent = path.parent_path();
  if (!parent.empty()) {
    std::filesystem::create_directories(parent);
  }
}

std::map<std::wstring, std::wstring> parseArgs(int argc, wchar_t **argv) {
  std::map<std::wstring, std::wstring> result;
  for (int i = 1; i < argc; ++i) {
    std::wstring key = argv[i];
    if (key.rfind(L"--", 0) != 0) {
      throw std::runtime_error("Unexpected argument: " + toUtf8(key));
    }
    if (i + 1 >= argc) {
      throw std::runtime_error("Missing value for argument: " + toUtf8(key));
    }
    result[key.substr(2)] = argv[++i];
  }
  return result;
}

std::wstring requiredArg(const std::map<std::wstring, std::wstring> &args, const wchar_t *name) {
  const auto it = args.find(name);
  if (it == args.end() || it->second.empty()) {
    throw std::runtime_error("Missing required argument: --" + toUtf8(name));
  }
  return it->second;
}

Options readOptions(int argc, wchar_t **argv) {
  const auto args = parseArgs(argc, argv);
  Options options;
  options.serverExe = requiredArg(args, L"server-exe");
  options.workingDir = requiredArg(args, L"working-dir");
  options.runtimeManifest = requiredArg(args, L"runtime-manifest");
  options.runtimeManifestSha256 = toUtf8(requiredArg(args, L"runtime-manifest-sha256"));
  options.adapterRoot = requiredArg(args, L"adapter-root");
  options.adapterCommit = toUtf8(requiredArg(args, L"adapter-commit"));
  options.adapterServerSha256 = toUtf8(requiredArg(args, L"adapter-server-sha256"));
  options.outputPath = requiredArg(args, L"output");
  options.frameLogPath = requiredArg(args, L"frames");
  const auto timeoutIt = args.find(L"timeout-ms");
  if (timeoutIt != args.end()) {
    options.timeoutMs = std::max<DWORD>(1000, std::stoul(timeoutIt->second));
  }
  return options;
}

class Handle {
public:
  Handle() = default;
  explicit Handle(HANDLE handle) : handle_(handle) {}
  ~Handle() { reset(); }

  Handle(const Handle &) = delete;
  Handle &operator=(const Handle &) = delete;

  Handle(Handle &&other) noexcept : handle_(other.handle_) { other.handle_ = nullptr; }
  Handle &operator=(Handle &&other) noexcept {
    if (this != &other) {
      reset();
      handle_ = other.handle_;
      other.handle_ = nullptr;
    }
    return *this;
  }

  HANDLE get() const { return handle_; }
  HANDLE release() {
    HANDLE result = handle_;
    handle_ = nullptr;
    return result;
  }
  void reset(HANDLE handle = nullptr) {
    if (handle_ != nullptr && handle_ != INVALID_HANDLE_VALUE) {
      CloseHandle(handle_);
    }
    handle_ = handle;
  }

private:
  HANDLE handle_ = nullptr;
};

class AdapterProcess {
public:
  explicit AdapterProcess(const Options &options) {
    SECURITY_ATTRIBUTES security{};
    security.nLength = sizeof(security);
    security.bInheritHandle = TRUE;

    HANDLE stdinReadRaw = nullptr;
    HANDLE stdinWriteRaw = nullptr;
    HANDLE stdoutReadRaw = nullptr;
    HANDLE stdoutWriteRaw = nullptr;
    HANDLE stderrReadRaw = nullptr;
    HANDLE stderrWriteRaw = nullptr;

    if (!CreatePipe(&stdinReadRaw, &stdinWriteRaw, &security, 0) ||
        !CreatePipe(&stdoutReadRaw, &stdoutWriteRaw, &security, 0) ||
        !CreatePipe(&stderrReadRaw, &stderrWriteRaw, &security, 0)) {
      throw std::runtime_error("CreatePipe failed.");
    }

    Handle stdinRead(stdinReadRaw);
    stdinWrite_.reset(stdinWriteRaw);
    stdoutRead_.reset(stdoutReadRaw);
    Handle stdoutWrite(stdoutWriteRaw);
    stderrRead_.reset(stderrReadRaw);
    Handle stderrWrite(stderrWriteRaw);

    SetHandleInformation(stdinWrite_.get(), HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(stdoutRead_.get(), HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(stderrRead_.get(), HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = stdinRead.get();
    startup.hStdOutput = stdoutWrite.get();
    startup.hStdError = stderrWrite.get();

    std::wstring commandLine = quoteArg(options.serverExe);
    PROCESS_INFORMATION processInfo{};
    if (!CreateProcessW(options.serverExe.c_str(), commandLine.data(), nullptr, nullptr, TRUE,
                        CREATE_NO_WINDOW, nullptr, options.workingDir.c_str(), &startup,
                        &processInfo)) {
      throw std::runtime_error("CreateProcessW failed for adapter server.");
    }

    process_.reset(processInfo.hProcess);
    thread_.reset(processInfo.hThread);
  }

  ~AdapterProcess() {
    stdinWrite_.reset();
    if (process_.get()) {
      WaitForSingleObject(process_.get(), 500);
      DWORD exitCode = 0;
      if (GetExitCodeProcess(process_.get(), &exitCode) && exitCode == STILL_ACTIVE) {
        TerminateProcess(process_.get(), 0);
        WaitForSingleObject(process_.get(), 1000);
      }
    }
  }

  void writeAll(const std::string &data) {
    size_t offset = 0;
    while (offset < data.size()) {
      DWORD written = 0;
      const auto chunk = static_cast<DWORD>(std::min<size_t>(data.size() - offset, 64 * 1024));
      if (!WriteFile(stdinWrite_.get(), data.data() + offset, chunk, &written, nullptr)) {
        throw std::runtime_error("WriteFile to adapter stdin failed.");
      }
      offset += written;
    }
  }

  bool readExact(char *data, size_t size, DWORD timeoutMs) {
    size_t offset = 0;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (offset < size) {
      DWORD available = 0;
      if (!PeekNamedPipe(stdoutRead_.get(), nullptr, 0, nullptr, &available, nullptr)) {
        return false;
      }
      if (available == 0) {
        DWORD exitCode = 0;
        if (GetExitCodeProcess(process_.get(), &exitCode) && exitCode != STILL_ACTIVE) {
          return false;
        }
        if (std::chrono::steady_clock::now() >= deadline) {
          return false;
        }
        Sleep(10);
        continue;
      }
      DWORD read = 0;
      const auto chunk = static_cast<DWORD>(std::min<size_t>(size - offset, available));
      if (!ReadFile(stdoutRead_.get(), data + offset, chunk, &read, nullptr)) {
        return false;
      }
      offset += read;
    }
    return true;
  }

  std::string readFrame(DWORD timeoutMs) {
    std::uint32_t size = 0;
    if (!readExact(reinterpret_cast<char *>(&size), sizeof(size), timeoutMs)) {
      throw std::runtime_error("Timed out waiting for adapter response frame header.");
    }
    if (size == 0 || size > kMaxFrameBytes) {
      throw std::runtime_error("Adapter response frame size is invalid or too large.");
    }
    std::string payload(size, '\0');
    if (!readExact(payload.data(), payload.size(), timeoutMs)) {
      throw std::runtime_error("Timed out waiting for adapter response frame payload.");
    }
    return payload;
  }

private:
  Handle process_;
  Handle thread_;
  Handle stdinWrite_;
  Handle stdoutRead_;
  Handle stderrRead_;
};

std::string methodName(moqi::protocol::Method method) {
  return moqi::protocol::Method_Name(method);
}

void writeFrameLog(std::ofstream &log, const char *direction, std::uint32_t seq,
                   moqi::protocol::Method method, const std::string &payload,
                   const moqi::protocol::ServerResponse *response = nullptr) {
  log << "{\"time\":" << jsonString(nowUtcIso()) << ",\"direction\":" << jsonString(direction)
      << ",\"seq\":" << seq << ",\"method\":" << jsonString(methodName(method))
      << ",\"payload_base64\":" << jsonString(base64Encode(payload));
  if (response != nullptr) {
    log << ",\"success\":" << (response->success() ? "true" : "false")
        << ",\"return_value\":" << response->return_value()
        << ",\"composition\":" << jsonString(response->composition_string())
        << ",\"commit\":" << jsonString(response->commit_string())
        << ",\"candidate_count\":" << response->candidate_entries_size();
  }
  log << "}\n";
}

moqi::protocol::ServerResponse sendRequest(AdapterProcess &adapter, std::ofstream &frameLog,
                                           moqi::protocol::ClientRequest &request,
                                           DWORD timeoutMs) {
  std::string framed;
  if (!Moqi::Proto::serializeMessage(request, framed)) {
    throw std::runtime_error("Failed to serialize ClientRequest.");
  }
  writeFrameLog(frameLog, "request", request.seq_num(), request.method(), framed);
  adapter.writeAll(framed);

  const auto payload = adapter.readFrame(timeoutMs);
  moqi::protocol::ServerResponse response;
  if (!Moqi::Proto::parsePayload(payload, response)) {
    throw std::runtime_error("Failed to parse ServerResponse.");
  }
  writeFrameLog(frameLog, "response", response.seq_num(), request.method(), payload, &response);
  if (!response.success()) {
    throw std::runtime_error("Adapter request failed: " + response.error());
  }
  return response;
}

moqi::protocol::ClientRequest baseRequest(const std::string &clientId, std::uint32_t seq,
                                          moqi::protocol::Method method) {
  moqi::protocol::ClientRequest request;
  request.set_client_id(clientId);
  request.set_seq_num(seq);
  request.set_method(method);
  request.set_is_windows8_above(true);
  request.set_is_keyboard_open(true);
  request.set_opened(true);
  return request;
}

moqi::protocol::ClientRequest keyRequest(const std::string &clientId, std::uint32_t seq,
                                         moqi::protocol::Method method, char key) {
  auto request = baseRequest(clientId, seq, method);
  auto *event = request.mutable_key_event();
  const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(key)));
  event->set_key_code(static_cast<std::uint32_t>(upper));
  event->set_char_code(static_cast<std::uint32_t>(key));
  event->set_repeat_count(1);
  return request;
}

bool hasNonAscii(const std::string &value) {
  return std::any_of(value.begin(), value.end(), [](unsigned char ch) { return ch >= 0x80; });
}

bool hasJyutpingLikeText(const std::string &value) {
  bool hasAlpha = false;
  bool hasDigit = false;
  for (unsigned char ch : value) {
    hasAlpha = hasAlpha || ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
    hasDigit = hasDigit || (ch >= '1' && ch <= '6');
  }
  return hasAlpha && hasDigit;
}

void captureResponse(InputEvidence &evidence, const moqi::protocol::ServerResponse &response) {
  if (!response.composition_string().empty()) {
    evidence.composition = response.composition_string();
  }
  if (response.show_candidates()) {
    evidence.page.showCandidates = true;
  }
  if (response.has_candidate_cursor()) {
    evidence.page.candidateCursor = response.candidate_cursor();
  }
  if (!response.set_sel_keys().empty()) {
    evidence.page.selectKeys = response.set_sel_keys();
  }
  if (response.candidate_entries_size() > 0) {
    evidence.candidates.clear();
    evidence.jyutpingOrComments.clear();
    for (const auto &candidate : response.candidate_entries()) {
      evidence.candidates.push_back({candidate.text(), candidate.comment()});
      if (!candidate.comment().empty()) {
        evidence.jyutpingOrComments.push_back(candidate.comment());
      }
    }
  }
  if (response.candidate_list_size() > 0) {
    evidence.candidateList.assign(response.candidate_list().begin(), response.candidate_list().end());
  }
  if (!response.commit_string().empty()) {
    evidence.commit = response.commit_string();
  }
}

InputEvidence runInput(AdapterProcess &adapter, std::ofstream &frameLog, const std::string &name,
                       const std::vector<char> &keys, std::uint32_t &seq, DWORD timeoutMs) {
  const std::string clientId = "typeduck-proof-" + name;
  InputEvidence evidence;
  evidence.name = name;
  evidence.keys = keys;

  auto init = baseRequest(clientId, seq++, moqi::protocol::METHOD_INIT);
  init.set_guid(toUtf8(kRimeGuid));
  captureResponse(evidence, sendRequest(adapter, frameLog, init, timeoutMs));

  auto activate = baseRequest(clientId, seq++, moqi::protocol::METHOD_ON_ACTIVATE);
  captureResponse(evidence, sendRequest(adapter, frameLog, activate, timeoutMs));

  for (const char key : keys) {
    auto filter = keyRequest(clientId, seq++, moqi::protocol::METHOD_FILTER_KEY_DOWN, key);
    captureResponse(evidence, sendRequest(adapter, frameLog, filter, timeoutMs));
    auto down = keyRequest(clientId, seq++, moqi::protocol::METHOD_ON_KEY_DOWN, key);
    captureResponse(evidence, sendRequest(adapter, frameLog, down, timeoutMs));
  }

  auto select = baseRequest(clientId, seq++, moqi::protocol::METHOD_SELECT_CANDIDATE);
  select.set_candidate_index(0);
  captureResponse(evidence, sendRequest(adapter, frameLog, select, timeoutMs));

  auto close = baseRequest(clientId, seq++, moqi::protocol::METHOD_CLOSE);
  captureResponse(evidence, sendRequest(adapter, frameLog, close, timeoutMs));
  return evidence;
}

void validateEvidence(const std::vector<InputEvidence> &inputs) {
  for (const auto &input : inputs) {
    if (input.composition.empty()) {
      throw std::runtime_error("Missing composition evidence for " + input.name);
    }
    if (input.candidates.empty()) {
      throw std::runtime_error("Missing candidate entries for " + input.name);
    }
    if (!input.page.showCandidates) {
      throw std::runtime_error("Missing candidate page state for " + input.name);
    }
    if (input.commit.empty()) {
      throw std::runtime_error("Missing commit output for " + input.name);
    }
    bool hasTypeduckCandidate = false;
    bool hasJyutping = false;
    for (const auto &candidate : input.candidates) {
      hasTypeduckCandidate = hasTypeduckCandidate || hasNonAscii(candidate.text);
      hasJyutping = hasJyutping || hasJyutpingLikeText(candidate.comment);
    }
    for (const auto &item : input.candidateList) {
      hasJyutping = hasJyutping || hasJyutpingLikeText(item);
    }
    if (!hasTypeduckCandidate) {
      throw std::runtime_error("Candidate evidence does not contain TypeDuck CJK output for " + input.name);
    }
    if (!hasJyutping) {
      throw std::runtime_error("Candidate evidence does not contain Jyutping/comment output for " + input.name);
    }
  }
}

void writeStringArray(std::ofstream &out, const std::vector<std::string> &values) {
  out << "[";
  for (size_t i = 0; i < values.size(); ++i) {
    if (i != 0) {
      out << ",";
    }
    out << jsonString(values[i]);
  }
  out << "]";
}

void writeProof(const Options &options, const std::vector<InputEvidence> &inputs) {
  ensureParentDirectory(options.outputPath);
  std::ofstream out(options.outputPath, std::ios::binary | std::ios::trunc);
  if (!out) {
    throw std::runtime_error("Failed to open proof output.");
  }

  out << "{\n";
  out << "  \"schema\":\"typeduck-phase02-adapter-proof-v1\",\n";
  out << "  \"generated_at_utc\":" << jsonString(nowUtcIso()) << ",\n";
  out << "  \"probe\":{\"name\":\"TypeduckBackendProbe\",\"max_frame_bytes\":" << kMaxFrameBytes
      << ",\"timeout_ms\":" << options.timeoutMs << "},\n";
  out << "  \"runtime_manifest\":{\"path\":" << jsonString(options.runtimeManifest)
      << ",\"sha256\":" << jsonString(options.runtimeManifestSha256) << "},\n";
  out << "  \"adapter\":{\"root\":" << jsonString(options.adapterRoot)
      << ",\"commit\":" << jsonString(options.adapterCommit)
      << ",\"server_exe\":" << jsonString(options.serverExe)
      << ",\"server_sha256\":" << jsonString(options.adapterServerSha256)
      << ",\"boundary\":\"moqi-ime framed protobuf stdio adapter\"},\n";
  out << "  \"frame_log\":" << jsonString(options.frameLogPath) << ",\n";
  out << "  \"inputs\":[\n";
  for (size_t i = 0; i < inputs.size(); ++i) {
    const auto &input = inputs[i];
    if (i != 0) {
      out << ",\n";
    }
    out << "    {\"name\":" << jsonString(input.name) << ",\"keys\":[";
    for (size_t j = 0; j < input.keys.size(); ++j) {
      if (j != 0) {
        out << ",";
      }
      out << jsonString(std::string(1, input.keys[j]));
    }
    out << "],\"composition\":" << jsonString(input.composition) << ",\"page\":{\"show_candidates\":"
        << (input.page.showCandidates ? "true" : "false")
        << ",\"candidate_cursor\":" << input.page.candidateCursor
        << ",\"select_keys\":" << jsonString(input.page.selectKeys) << "},\"candidates\":[";
    for (size_t j = 0; j < input.candidates.size(); ++j) {
      if (j != 0) {
        out << ",";
      }
      out << "{\"text\":" << jsonString(input.candidates[j].text)
          << ",\"comment\":" << jsonString(input.candidates[j].comment) << "}";
    }
    out << "],\"candidate_list\":";
    writeStringArray(out, input.candidateList);
    out << ",\"jyutping_or_comments\":";
    writeStringArray(out, input.jyutpingOrComments);
    out << ",\"commit\":" << jsonString(input.commit) << "}";
  }
  out << "\n  ],\n";
  out << "  \"command_exit_codes\":{\"probe\":0,\"adapter\":\"terminated_after_close\"},\n";
  out << "  \"legacy_fallback_rejected\":true\n";
  out << "}\n";
}

int run(int argc, wchar_t **argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  const auto options = readOptions(argc, argv);
  ensureParentDirectory(options.frameLogPath);
  std::ofstream frameLog(options.frameLogPath, std::ios::binary | std::ios::trunc);
  if (!frameLog) {
    throw std::runtime_error("Failed to open frame log.");
  }

  AdapterProcess adapter(options);
  std::uint32_t seq = 1;
  std::vector<InputEvidence> inputs;
  inputs.push_back(runInput(adapter, frameLog, "nei", {'n', 'e', 'i'}, seq, options.timeoutMs));
  inputs.push_back(runInput(adapter, frameLog, "housam", {'h', 'o', 'u', 's', 'a', 'm'}, seq,
                            options.timeoutMs));

  validateEvidence(inputs);
  writeProof(options, inputs);
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}

} // namespace

int wmain(int argc, wchar_t **argv) {
  try {
    return run(argc, argv);
  } catch (const std::exception &ex) {
    std::cerr << "TypeduckBackendProbe failed: " << ex.what() << "\n";
    return 1;
  }
}
