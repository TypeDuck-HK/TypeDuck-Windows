//
//	Copyright (C) 2015 - 2016 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
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

#include <Windows.h>
#include <Lmcons.h> // for UNLEN
#include <Shellapi.h>
#include <ShlObj.h>
#include <Wincrypt.h> // for CryptBinaryToString (used for base64 encoding)
#include <algorithm>
#include <cassert>
#include <chrono>  // C++ 11 clock functions
#include <codecvt> // for utf8 conversion
#include <cstring>
#include <cstdlib>
#include <locale> // for wstring_convert
#include <map>
#include <string>
#include <vector>

#include <json/json.h>

#include "BackendServer.h"
#include "PipeClient.h"
#include "PipeServer.h"
#include "../proto/ProtoFraming.h"
#include "proto/moqi.pb.h"

using namespace std;

namespace Moqi {

static wstring_convert<codecvt_utf8<wchar_t>> utf8Codec;
static constexpr auto MAX_RESPONSE_WAITING_TIME =
    30; // if a backend is non-responsive for 30 seconds, it's considered dead
namespace {

} // namespace

static DWORD trayNotificationInfoFlags(moqi::protocol::TrayNotificationIcon icon) {
  switch (icon) {
  case moqi::protocol::TRAY_NOTIFICATION_ICON_WARNING:
    return NIIF_WARNING;
  case moqi::protocol::TRAY_NOTIFICATION_ICON_ERROR:
    return NIIF_ERROR;
  case moqi::protocol::TRAY_NOTIFICATION_ICON_INFO:
  case moqi::protocol::TRAY_NOTIFICATION_ICON_UNSPECIFIED:
  default:
    return NIIF_INFO;
  }
}

static std::string getUtf8CurrentDir() {
  char dirPath[MAX_PATH];
  size_t len = MAX_PATH;
  uv_cwd(dirPath, &len);
  return dirPath;
}

static std::vector<std::string> getUtf8EnvironmentVariables() {
  // build our own new environments
  auto env_strs = GetEnvironmentStringsW();
  vector<string> utf8Environ;
  for (auto penv = env_strs; *penv; penv += wcslen(penv) + 1) {
    utf8Environ.emplace_back(utf8Codec.to_bytes(penv));
  }
  FreeEnvironmentStringsW(env_strs);
  return utf8Environ;
}

BackendServer::BackendServer(PipeServer *pipeServer, const Json::Value &info)
    : pipeServer_{pipeServer}, name_(info["name"].asString()),
      process_{nullptr}, stdinPipe_{nullptr}, stdoutPipe_{nullptr}, stderrPipe_{nullptr},
      stdoutFrameBuf_{Proto::kMaxBackendFramePayloadBytes},
      command_(info["command"].asString()), params_(info["params"].asString()),
      workingDir_(info["workingDir"].asString()) {}

BackendServer::~BackendServer() { terminateProcess(); }

std::shared_ptr<spdlog::logger> &BackendServer::logger() {
  return pipeServer_->logger();
}

void BackendServer::handleClientMessage(PipeClient *client,
                                        const moqi::protocol::ClientRequest &request) {
  if (!isProcessRunning()) {
    if (!startProcess()) {
      client->writeTypeDuckErrorResponse(
          request.seq_num(),
          moqi::protocol::TYPEDUCK_ERROR_ENGINE_INIT_FAILED,
          "TypeDuck backend bridge failed to start",
          moqi::protocol::TYPEDUCK_HEALTH_FAILED,
          false,
          name_);
      return;
    }
  }

  moqi::protocol::ClientRequest backendRequest = request;
  backendRequest.set_client_id(client->clientId_);
  std::string framedMessage;
  if (!Proto::serializeMessageBounded(
          backendRequest, framedMessage, Proto::kMaxBackendFramePayloadBytes)) {
    logger()->error("Failed to serialize backend request for client {}",
                    client->clientId_);
    client->writeTypeDuckErrorResponse(
        request.seq_num(),
        moqi::protocol::TYPEDUCK_ERROR_MALFORMED_PAYLOAD,
        "TypeDuck backend request could not be serialized",
        moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
        true,
        name_);
    return;
  }

  // write the message to the backend server
  if (stdinPipe_ == nullptr) {
    client->writeTypeDuckErrorResponse(
        request.seq_num(),
        moqi::protocol::TYPEDUCK_ERROR_ENGINE_INIT_FAILED,
        "TypeDuck backend stdin is unavailable",
        moqi::protocol::TYPEDUCK_HEALTH_FAILED,
        true,
        name_);
    return;
  }
  stdinPipe_->write(std::move(framedMessage));
}

TypeDuck::ApplyResult BackendServer::applyTypeDuckPreferences(
    const TypeDuck::RimeSideEffects& effects) {
  if (!isProcessRunning() && !startProcess()) {
    return {false, "設定已儲存，但 TypeDuck 後端未能啟動 / Settings were saved, but the TypeDuck backend could not start"};
  }
  if (stdinPipe_ == nullptr) {
    return {false, "設定已儲存，但 TypeDuck 後端未能接收重新部署指令 / Settings were saved, but the TypeDuck backend could not receive redeploy"};
  }

  moqi::protocol::ClientRequest request;
  request.set_method(moqi::protocol::METHOD_TYPEDUCK_SETTINGS_UPDATE);
  request.set_client_id("settings");
  auto* update = request.mutable_typeduck_settings_update();
  update->set_page_size(static_cast<uint32_t>(effects.pageSize));
  update->set_enable_completion(effects.enableCompletion);
  update->set_enable_correction(effects.enableCorrection);
  update->set_enable_sentence(effects.enableSentence);
  update->set_enable_learning(effects.enableLearning);
  update->set_is_cangjie5(effects.isCangjie5);

  std::string framedMessage;
  if (!Proto::serializeMessageBounded(
          request, framedMessage, Proto::kMaxBackendFramePayloadBytes)) {
    return {false, "設定已儲存，但重新部署指令未能建立 / Settings were saved, but the redeploy request could not be created"};
  }

  stdinPipe_->write(std::move(framedMessage));
  return {true, "設定已套用 / Settings applied"};
}

TypeDuck::ApplyResult BackendServer::requestTypeDuckDeploy() {
  if (!isProcessRunning() && !startProcess()) {
    return {false, "TypeDuck 後端未能啟動 / TypeDuck backend could not start"};
  }
  if (stdinPipe_ == nullptr) {
    return {false,
            "TypeDuck 後端未能接收重新部署指令 / TypeDuck backend could not receive redeploy"};
  }

  moqi::protocol::ClientRequest request;
  request.set_method(moqi::protocol::METHOD_TYPEDUCK_DEPLOY);
  request.set_client_id("launcher-tray");
  request.mutable_typeduck_deploy_request()->set_force(true);

  std::string framedMessage;
  if (!Proto::serializeMessageBounded(
          request, framedMessage, Proto::kMaxBackendFramePayloadBytes)) {
    logger()->error("Failed to serialize TypeDuck tray redeploy request");
    return {false,
            "重新部署指令未能建立 / Redeploy request could not be created"};
  }

  stdinPipe_->write(std::move(framedMessage));
  return {true, ""};
}

void BackendServer::uploadCloudClipboardText(const std::string& utf8Text) {
  if (utf8Text.empty()) {
    return;
  }
  if (!isProcessRunning()) {
    if (!startProcess()) {
      return;
    }
  }
  if (stdinPipe_ == nullptr) {
    logger()->warn("Skip cloud clipboard upload: backend stdin is unavailable");
    return;
  }

  moqi::protocol::ClientRequest request;
  request.set_method(moqi::protocol::METHOD_CLOUD_CLIPBOARD_UPLOAD);
  request.set_client_id("clipboard");
  request.set_cloud_clipboard_text(utf8Text);
  std::string framedMessage;
  if (!Proto::serializeMessageBounded(
          request, framedMessage, Proto::kMaxBackendFramePayloadBytes)) {
    logger()->error("Failed to serialize cloud clipboard upload");
    return;
  }
  stdinPipe_->write(std::move(framedMessage));
}

uv::Pipe *BackendServer::createStdinPipe() {
  auto stdinPipe = new uv::Pipe();
  stdinPipe->setCloseCallback([stdinPipe]() { delete stdinPipe; });
  return stdinPipe;
}

uv::Pipe *BackendServer::createStdoutPipe() {
  auto stdoutPipe = new uv::Pipe();
  stdoutPipe->setReadCallback(
      [this](const char *buf, size_t len) { onStdoutRead(buf, len); });
  stdoutPipe->setReadErrorCallback([this](int error) { onReadError(error); });
  stdoutPipe->setCloseCallback([stdoutPipe]() { delete stdoutPipe; });
  return stdoutPipe;
}

uv::Pipe *BackendServer::createStderrPipe() {
  auto stderrPipe = new uv::Pipe();
  stderrPipe->setReadCallback(
      [this](const char *buf, size_t len) { onStderrRead(buf, len); });
  stderrPipe->setReadErrorCallback([this](int error) { onReadError(error); });
  stderrPipe->setCloseCallback([this, stderrPipe]() { delete stderrPipe; });
  return stderrPipe;
}

bool BackendServer::startProcess() {
  process_ = new uv_process_t{};
  // create pipes for stdio of the child process
  stdoutPipe_ = createStdoutPipe();
  stdoutFrameBuf_.clear();
  stdinPipe_ = createStdinPipe();
  stderrPipe_ = createStderrPipe();

  constexpr auto pipeFlags =
      uv_stdio_flags(UV_CREATE_PIPE | UV_READABLE_PIPE | UV_WRITABLE_PIPE);
  uv_stdio_container_t stdio_containers[3];
  stdio_containers[0].data.stream = stdinPipe_->streamHandle();
  stdio_containers[0].flags = pipeFlags;
  stdio_containers[1].data.stream = stdoutPipe_->streamHandle();
  stdio_containers[1].flags = pipeFlags;
  stdio_containers[2].data.stream = stderrPipe_->streamHandle();
  stdio_containers[2].flags = pipeFlags;

  auto utf8CurrentDirPath = getUtf8CurrentDir();
  auto executablePath = utf8CurrentDirPath + '\\' + command_;
  const char *argv[] = {executablePath.c_str(), params_.c_str(), nullptr};
  uv_process_options_t options = {0};
  options.flags =
      UV_PROCESS_WINDOWS_HIDE; //  UV_PROCESS_WINDOWS_VERBATIM_ARGUMENTS;
  options.file = executablePath.c_str();
  options.args = const_cast<char **>(argv);

  auto backendWorkingDirPath = utf8CurrentDirPath + '\\' + workingDir_;
  options.cwd = backendWorkingDirPath.c_str();

  // build our own new environments
  auto utf8EnvVars = getUtf8EnvironmentVariables();
  // add our own environment variables
  // NOTE: Force python to output UTF-8 encoded strings
  // Reference:
  // https://docs.python.org/3/using/cmdline.html#envvar-PYTHONIOENCODING By
  // default, python uses ANSI encoding in Windows and this breaks our unicode
  // support.
  // FIXME: makes this configurable from backend.json.
  utf8EnvVars.emplace_back("PYTHONIOENCODING=utf-8:ignore");

  // convert to a null terminated char* array.
  std::vector<const char *> env;
  for (auto &v : utf8EnvVars) {
    env.emplace_back(v.c_str());
  }
  env.emplace_back(nullptr);
  options.env = const_cast<char **>(env.data());

  options.stdio_count = 3;
  options.stdio = stdio_containers;
  int ret = uv_spawn(uv_default_loop(), process_, &options);
  if (ret < 0) {
    logger()->error("Failed to start backend {}: {}", name_, uv_strerror(ret));
    delete process_;
    process_ = nullptr;
    closeStdioPipes();
    return false;
  }

  // start receiving data from the backend server
  stdoutPipe_->startRead();
  stderrPipe_->startRead();
  return true;
}

bool BackendServer::restartProcess() {
  terminateProcess(false);
  return startProcess();
}

void BackendServer::terminateProcess(bool notifyClients) {
  if (process_) {
    closeStdioPipes();

    uv_process_kill(process_, SIGTERM);
    uv_close(reinterpret_cast<uv_handle_t *>(process_),
             [](uv_handle_t *handle) {
               delete reinterpret_cast<uv_process_t *>(handle);
             });

    process_ = nullptr;
  }
  if (notifyClients) {
    pipeServer_->onBackendClosed(this);
  }
}

// check if the backend server process is running
bool BackendServer::isProcessRunning() { return process_ != nullptr; }

void BackendServer::onStdoutRead(const char *buf, size_t len) {
  stdoutFrameBuf_.append(buf, len);
  if (stdoutFrameBuf_.hasViolation()) {
    const auto frameError = stdoutFrameBuf_.lastError();
    pipeServer_->notifyClientsOfBackendError(
        this,
        frameError == Proto::FrameError::PayloadTooLarge
            ? moqi::protocol::TYPEDUCK_ERROR_PAYLOAD_TOO_LARGE
            : moqi::protocol::TYPEDUCK_ERROR_BACKEND_RESTART,
        frameError == Proto::FrameError::PayloadTooLarge
            ? "TypeDuck backend stdout frame exceeds launcher payload limit"
            : "Malformed TypeDuck backend stdout frame",
        moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
        true,
        frameError == Proto::FrameError::MalformedHeader
            ? "FrameError::MalformedHeader"
            : "FrameError::PayloadTooLarge");
    stdoutFrameBuf_.clear();
    restartProcess();
    return;
  }
  handleBackendReply();
  if (stdoutFrameBuf_.hasViolation()) {
    const auto frameError = stdoutFrameBuf_.lastError();
    pipeServer_->notifyClientsOfBackendError(
        this,
        frameError == Proto::FrameError::PayloadTooLarge
            ? moqi::protocol::TYPEDUCK_ERROR_PAYLOAD_TOO_LARGE
            : moqi::protocol::TYPEDUCK_ERROR_BACKEND_RESTART,
        frameError == Proto::FrameError::PayloadTooLarge
            ? "TypeDuck backend stdout frame exceeds launcher payload limit"
            : "Malformed TypeDuck backend stdout frame",
        moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
        true,
        frameError == Proto::FrameError::MalformedHeader
            ? "FrameError::MalformedHeader"
            : "FrameError::PayloadTooLarge");
    stdoutFrameBuf_.clear();
    restartProcess();
  }
}

void BackendServer::onReadError(int status) {
  // the backend server is broken, restart it.
  pipeServer_->notifyClientsOfBackendError(
      this,
      moqi::protocol::TYPEDUCK_ERROR_BACKEND_RESTART,
      "TypeDuck backend bridge read failed; restarting backend bridge",
      moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
      true,
      uv_strerror(status));
  restartProcess();
}

void BackendServer::onStderrRead(const char *buf, size_t len) {
  // FIXME: need to do output buffering since we might not receive a full line
  // log the error message
  logger()->error("[Backend error] {}", std::string(buf, len));
}

void BackendServer::closeStdioPipes() {
  if (stdinPipe_ != nullptr) {
    stdinPipe_->close();
    stdinPipe_ = nullptr;
  }

  if (stdoutPipe_ != nullptr) {
    stdoutPipe_->close();
    stdoutPipe_ = nullptr;
    stdoutFrameBuf_.clear();
  }

  if (stderrPipe_ != nullptr) {
    stderrPipe_->close();
    stderrPipe_ = nullptr;
  }
}

void BackendServer::handleBackendReply() {
  std::string payload;
  while (stdoutFrameBuf_.nextFrame(payload)) {
    moqi::protocol::ServerResponse response;
    if (!Proto::parsePayload(payload, response)) {
      logger()->error("Failed to parse protobuf response from backend {}", name_);
      pipeServer_->notifyClientsOfBackendError(
          this,
          moqi::protocol::TYPEDUCK_ERROR_BACKEND_RESTART,
          "Malformed TypeDuck backend protobuf response",
          moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
          true,
          "FrameError::MalformedHeader");
      stdoutFrameBuf_.clear();
      restartProcess();
      return;
    }

    const bool isLauncherTrayDeployResponse =
        response.client_id() == "launcher-tray";
    if (isLauncherTrayDeployResponse && response.success()) {
      pipeServer_->clearTrayNotification();
    } else if (response.has_tray_notification()) {
      const auto &notification = response.tray_notification();
      pipeServer_->enqueueTrayNotification(
          utf8Codec.from_bytes(notification.title()),
          utf8Codec.from_bytes(notification.message()),
          trayNotificationInfoFlags(notification.icon()));
    }

    if (response.client_id().empty()) {
      logger()->warn("Ignoring backend response without client_id from {}", name_);
      continue;
    }

    if (response.client_id() == "clipboard") {
      continue;
    }

    if (auto client = pipeServer_->clientFromId(response.client_id())) {
      const auto framedPayload = Proto::framePayload(payload);
      client->writeBackendResponse(
          response.seq_num(), framedPayload.data(), framedPayload.size());
    }
  }
}

} // namespace Moqi
