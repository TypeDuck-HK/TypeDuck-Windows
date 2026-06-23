//
//	Copyright (C) 2015 - 2020 Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
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

#include "PipeClient.h"
#include "PipeServer.h"
#include "Utils.h"
#include "BackendServer.h"
#include "proto/moqi.pb.h"
#include "../proto/ProtoFraming.h"

using namespace std;

namespace Moqi {

static wstring_convert<codecvt_utf8<wchar_t>> utf8Codec;

// default to 30 seconds
static constexpr std::uint64_t BACKEND_REQUEST_TIMEOUT_MS = 30 * 1000;


PipeClient::PipeClient(PipeServer* server, DWORD pipeMode, SECURITY_ATTRIBUTES* securityAttributes) :
    server_{ server },
    backend_(nullptr),
    // generate a new uuid for client ID
    clientId_{ generateUuidStr() },
    pipe_{ pipeMode, securityAttributes },
    waitResponseTimer_{ std::make_unique<uv_timer_t>() },
    readBuffer_{ Proto::kMaxClientFramePayloadBytes } {

    pipe_.setBlocking(false);

    pipe_.setReadCallback(
        [this](const char* data, size_t len) {
            handleClientMessage(data, len);
        }
    );
    pipe_.setReadErrorCallback(
        [this](int status) {
            onReadError(status);
        }
    );
    pipe_.setCloseCallback(
        [this]() {
            server_->removeClient(this);
        }
    );

    // setup a timer to detect request timeout
	uv_timer_init(uv_default_loop(), waitResponseTimer_.get());
	waitResponseTimer_->data = this;
}

PipeClient::~PipeClient() {
    stopRequestTimeoutTimer();

    // Close the uv timer and free its resources.
    // NOTE: The operation is async and it's not safe to free the memory here.
    // We release the ownership to the unique_ptr and delete the raw pointer in the callback of uv_close().
    waitResponseTimer_->data = nullptr;  // Avoid referencing to this since this object is destructing.
    uv_close(reinterpret_cast<uv_handle_t*>(waitResponseTimer_.release()), [](uv_handle_t* handle) {
        delete reinterpret_cast<uv_timer_t*>(handle);
        }
    );
}

std::shared_ptr<spdlog::logger>& PipeClient::logger() {
	return server_->logger();
}

void PipeClient::close() {
    pipe_.close();
}

void PipeClient::onReadError(int error) {
    // the client connection seems to be broken. close it.
    disconnectFromBackend();
    close();
}

void PipeClient::handleClientMessage(const char* readBuf, size_t len) {
    readBuffer_.append(readBuf, len);
    if (readBuffer_.hasViolation()) {
        handleClientFrameViolation();
        return;
    }

    std::string payload;
    while (readBuffer_.nextFrame(payload)) {
        moqi::protocol::ClientRequest request;
        if (!Proto::parsePayload(payload, request)) {
            logger()->error("Failed to parse protobuf request from client {}", clientId_);
            writeTypeDuckErrorResponse(
                0,
                moqi::protocol::TYPEDUCK_ERROR_MALFORMED_PAYLOAD,
                "Malformed TypeDuck client protobuf payload",
                moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
                true);
            continue;
        }

        if (!backend_ && !initBackend(request)) {
            continue;
        }

        if (backend_) {
            pendingSeqNum_ = request.seq_num();
            pendingMethod_ = request.method();
            startRequestTimeoutTimer(BACKEND_REQUEST_TIMEOUT_MS);
            backend_->handleClientMessage(this, request);
        }
    }

    if (readBuffer_.hasViolation()) {
        handleClientFrameViolation();
    }
}

bool PipeClient::initBackend(const moqi::protocol::ClientRequest &params) {
	if (params.method() == moqi::protocol::METHOD_INIT) {  // the client connects to us the first time
		// find a backend for the client text service
		const auto& guid = params.guid();
		backend_ = server_->backendFromLangProfileGuid(guid.c_str());
		if (backend_ != nullptr) {
			return true;
		}
		else {
			logger()->critical("Backend is not found for text service: {}", guid);
            writeTypeDuckErrorResponse(
                params.seq_num(),
                moqi::protocol::TYPEDUCK_ERROR_ENGINE_INIT_FAILED,
                "TypeDuck backend bridge is not mapped for this text service profile",
                moqi::protocol::TYPEDUCK_HEALTH_FAILED,
                false,
                guid);
		}
	}
	return false;
}

bool PipeClient::writeBackendResponse(std::uint32_t seqNum, const char* data, size_t len) {
    if (pendingSeqNum_ != 0 && seqNum != pendingSeqNum_) {
        logger()->warn(
            "Dropping stale backend response for client {}: response seq {}, pending seq {}",
            clientId_,
            seqNum,
            pendingSeqNum_);
        return false;
    }

    writePipe(data, len);
    return true;
}

void PipeClient::writeTypeDuckErrorResponse(
    std::uint32_t seqNum,
    moqi::protocol::TypeDuckErrorCode errorCode,
    const std::string& message,
    moqi::protocol::TypeDuckHealthStatus healthStatus,
    bool recoverable,
    const std::string& detail) {
    stopRequestTimeoutTimer();

    const auto responseSeqNum = seqNum != 0 ? seqNum : pendingSeqNum_;
    pendingSeqNum_ = 0;
    pendingMethod_ = moqi::protocol::METHOD_UNSPECIFIED;

    moqi::protocol::ServerResponse response;
    response.set_client_id(clientId_);
    response.set_seq_num(responseSeqNum);
    response.set_success(false);
    response.set_return_value(0);
    response.set_error(message);

    auto* health = response.mutable_typeduck_engine_health();
    health->set_status(healthStatus);
    health->set_backend_name(backend_ != nullptr ? backend_->name() : "");
    health->set_message(message);
    health->set_recoverable(recoverable);

    auto* error = response.mutable_typeduck_error();
    error->set_code(errorCode);
    error->set_message(message);
    error->set_recoverable(recoverable);
    error->set_detail(detail);

    std::string framedMessage;
    if (!Proto::serializeMessageBounded(
            response, framedMessage, Proto::kMaxClientFramePayloadBytes)) {
        logger()->error("Failed to serialize TypeDuck error response for client {}", clientId_);
        return;
    }
    pipe_.write(std::move(framedMessage));
}

void PipeClient::handleClientFrameViolation() {
    const auto frameError = readBuffer_.lastError();
    const auto errorCode =
        frameError == Proto::FrameError::PayloadTooLarge
            ? moqi::protocol::TYPEDUCK_ERROR_PAYLOAD_TOO_LARGE
            : moqi::protocol::TYPEDUCK_ERROR_MALFORMED_FRAME;
    const auto message =
        frameError == Proto::FrameError::PayloadTooLarge
            ? "TypeDuck client frame exceeds launcher payload limit"
            : "Malformed TypeDuck client frame";
    const auto detail =
        frameError == Proto::FrameError::MalformedHeader
            ? "FrameError::MalformedHeader"
            : "FrameError::PayloadTooLarge";

    logger()->error("Rejecting client {} frame: {}", clientId_, detail);
    readBuffer_.clear();
    writeTypeDuckErrorResponse(
        0,
        errorCode,
        message,
        moqi::protocol::TYPEDUCK_HEALTH_DEGRADED,
        true,
        detail);
}

void PipeClient::disconnectFromBackend() {
	if (backend_ != nullptr) {
		moqi::protocol::ClientRequest request;
		request.set_method(moqi::protocol::METHOD_CLOSE);
		backend_->handleClientMessage(this, request);
	}
}

void PipeClient::startRequestTimeoutTimer(std::uint64_t timeoutMs) {
	uv_timer_start(waitResponseTimer_.get(), [](uv_timer_t* handle) {
        if (handle->data) {
            auto pThis = reinterpret_cast<PipeClient*>(handle->data);
            pThis->onRequestTimeout();
        }
	}, timeoutMs, 0);
}

void PipeClient::stopRequestTimeoutTimer() {
	uv_timer_stop(waitResponseTimer_.get());
}

void PipeClient::onRequestTimeout() {
	// We sent a message to the backend server, but haven't got any response before the timeout
	// Assume that the backend server is dead. => Try to restart
    stopRequestTimeoutTimer();
    writeTypeDuckErrorResponse(
        pendingSeqNum_,
        moqi::protocol::TYPEDUCK_ERROR_BACKEND_TIMEOUT,
        "TypeDuck backend request timed out; restarting backend bridge",
        moqi::protocol::TYPEDUCK_HEALTH_RESTARTING,
        true);
	if (backend_) {
		logger()->critical("Backend {} seems to be dead. Try to restart!", backend_->name());
		backend_->restartProcess();
	}
}

} // namespace Moqi
