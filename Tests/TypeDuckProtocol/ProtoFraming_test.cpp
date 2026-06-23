#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <cstdint>
#include <cstring>
#include <string>

#include "proto/ProtoFraming.h"
#include "proto/moqi.pb.h"

namespace {

std::string frameWithDeclaredSize(std::uint32_t declaredSize,
                                  const std::string &body = std::string()) {
  std::string frame(sizeof(std::uint32_t), '\0');
  std::memcpy(&frame[0], &declaredSize, sizeof(declaredSize));
  frame.append(body);
  return frame;
}

} // namespace

TEST(TypeDuckProtoFraming, RejectsOversizedFrameAndDropsBody) {
  Moqi::Proto::FrameBuffer buffer(8);
  const std::string frame = frameWithDeclaredSize(9, "abcdefghi");

  buffer.append(frame.data(), frame.size());

  std::string payload;
  EXPECT_FALSE(buffer.nextFrame(payload));
  EXPECT_EQ(buffer.lastError(), Moqi::Proto::FrameError::PayloadTooLarge);
  EXPECT_TRUE(buffer.hasViolation());
  EXPECT_EQ(buffer.bufferedBytes(), 0u);
  EXPECT_TRUE(payload.empty());
}

TEST(TypeDuckProtoFraming, TruncatedFrameRemainsPendingOnlyWithinLimit) {
  Moqi::Proto::FrameBuffer buffer(8);
  const std::string partial = frameWithDeclaredSize(8, "abc");

  buffer.append(partial.data(), partial.size());

  std::string payload;
  EXPECT_FALSE(buffer.nextFrame(payload));
  EXPECT_EQ(buffer.lastError(), Moqi::Proto::FrameError::Incomplete);
  EXPECT_FALSE(buffer.hasViolation());
  EXPECT_EQ(buffer.bufferedBytes(), partial.size());

  const std::string oversizedHeader = frameWithDeclaredSize(9);
  buffer.clear();
  buffer.append(oversizedHeader.data(), oversizedHeader.size());

  EXPECT_FALSE(buffer.nextFrame(payload));
  EXPECT_EQ(buffer.lastError(), Moqi::Proto::FrameError::PayloadTooLarge);
  EXPECT_TRUE(buffer.hasViolation());
  EXPECT_EQ(buffer.bufferedBytes(), 0u);
}

TEST(TypeDuckProtoFraming, MalformedPayloadDoesNotParseRequestOrResponse) {
  const std::string malformed("\x08\xff\xff\xff", 4);

  moqi::protocol::ClientRequest request;
  moqi::protocol::ServerResponse response;

  EXPECT_FALSE(Moqi::Proto::parsePayload(malformed, request));
  EXPECT_FALSE(request.IsInitialized());
  EXPECT_FALSE(Moqi::Proto::parsePayload(malformed, response));
  EXPECT_FALSE(response.IsInitialized());
}

TEST(TypeDuckProtocol, RawLookupSeparatorsSurviveCandidateComments) {
  moqi::protocol::CandidateEntry candidate;
  const std::string rawComment =
      "\vReverse lookup marker\f\rmatch_input_buffer,honzi,jyutping\rnei,\xE4\xBD\xA0,nei5";

  candidate.set_id("nei-1");
  candidate.set_text("\xE4\xBD\xA0");
  candidate.set_input_code("nei");
  candidate.set_jyutping("nei5");
  candidate.set_raw_lookup_comment(rawComment);
  candidate.set_comment(rawComment);

  std::string serialized;
  ASSERT_TRUE(candidate.SerializeToString(&serialized));

  moqi::protocol::CandidateEntry parsed;
  ASSERT_TRUE(parsed.ParseFromString(serialized));
  EXPECT_EQ(parsed.raw_lookup_comment(), rawComment);
  EXPECT_EQ(parsed.comment(), rawComment);
}
