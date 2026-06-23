#pragma once

#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

namespace Moqi {
namespace Proto {

inline constexpr std::size_t kMaxClientFramePayloadBytes = 1024 * 1024;
inline constexpr std::size_t kMaxBackendFramePayloadBytes = 1024 * 1024;
inline constexpr std::size_t kDefaultMaxFramePayloadBytes =
    kMaxBackendFramePayloadBytes;

enum class FrameError {
  None,
  Incomplete,
  PayloadTooLarge,
  MalformedHeader,
};

class FrameBuffer {
public:
  explicit FrameBuffer(
      std::size_t maxPayloadBytes = kDefaultMaxFramePayloadBytes)
      : maxPayloadBytes_(maxPayloadBytes) {}

  void append(const char *data, size_t len) {
    if (violation_) {
      return;
    }

    if (len > maxBufferedBytes() || buffer_.size() > maxBufferedBytes() - len) {
      fail(FrameError::PayloadTooLarge);
      return;
    }

    buffer_.append(data, len);
  }

  bool nextFrame(std::string &payload) {
    payload.clear();

    if (violation_) {
      return false;
    }

    if (buffer_.size() < sizeof(std::uint32_t)) {
      lastError_ = FrameError::Incomplete;
      return false;
    }

    std::uint32_t payloadSize = 0;
    std::memcpy(&payloadSize, buffer_.data(), sizeof(payloadSize));

    if (payloadSize > maxPayloadBytes_) {
      fail(FrameError::PayloadTooLarge);
      return false;
    }

    if (payloadSize >
        (std::numeric_limits<std::size_t>::max)() - sizeof(payloadSize)) {
      fail(FrameError::MalformedHeader);
      return false;
    }

    if (buffer_.size() < sizeof(payloadSize) + payloadSize) {
      lastError_ = FrameError::Incomplete;
      return false;
    }

    payload.assign(buffer_.data() + sizeof(payloadSize), payloadSize);
    buffer_.erase(0, sizeof(payloadSize) + payloadSize);
    lastError_ = FrameError::None;
    return true;
  }

  void clear() {
    buffer_.clear();
    violation_ = false;
    lastError_ = FrameError::None;
  }

  bool hasViolation() const { return violation_; }

  FrameError lastError() const { return lastError_; }

  std::size_t bufferedBytes() const { return buffer_.size(); }

  std::size_t maxPayloadBytes() const { return maxPayloadBytes_; }

private:
  std::size_t maxBufferedBytes() const {
    return maxPayloadBytes_ + sizeof(std::uint32_t);
  }

  void fail(FrameError error) {
    buffer_.clear();
    violation_ = true;
    lastError_ = error;
  }

  std::string buffer_;
  std::size_t maxPayloadBytes_;
  bool violation_ = false;
  FrameError lastError_ = FrameError::None;
};

inline bool tryFramePayload(const std::string &payload,
                            std::string &framedPayload,
                            std::size_t maxPayloadBytes =
                                kDefaultMaxFramePayloadBytes) {
  framedPayload.clear();
  if (payload.size() > maxPayloadBytes ||
      payload.size() >
          static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)())) {
    return false;
  }

  std::string framed(sizeof(std::uint32_t), '\0');
  const auto payloadSize = static_cast<std::uint32_t>(payload.size());
  std::memcpy(&framed[0], &payloadSize, sizeof(payloadSize));
  framed.append(payload);
  framedPayload.swap(framed);
  return true;
}

inline std::string framePayload(const std::string &payload) {
  std::string framed;
  if (!tryFramePayload(payload, framed,
                       static_cast<std::size_t>(
                           (std::numeric_limits<std::uint32_t>::max)()))) {
    return {};
  }
  return framed;
}

template <typename Message>
bool serializeMessageBounded(
    const Message &message,
    std::string &framedPayload,
    std::size_t maxPayloadBytes = kDefaultMaxFramePayloadBytes) {
  std::string payload;
  if (!message.SerializeToString(&payload)) {
    return false;
  }
  return tryFramePayload(payload, framedPayload, maxPayloadBytes);
}

template <typename Message>
bool serializeMessage(const Message &message, std::string &framedPayload) {
  std::string payload;
  if (!message.SerializeToString(&payload)) {
    return false;
  }
  framedPayload = framePayload(payload);
  return true;
}

template <typename Message>
bool parsePayload(const std::string &payload, Message &message) {
  if (!message.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
    message.Clear();
    return false;
  }
  return true;
}

} // namespace Proto
} // namespace Moqi
