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

bool matchesExpectedSequence(const moqi::protocol::ServerResponse &response,
                             std::uint32_t expectedSeq) {
  return expectedSeq != 0 && response.seq_num() == expectedSeq;
}

} // namespace

TEST(TypeDuckProtocolRecovery, ValidCantoneseFrameRoundTripsWithCandidates) {
  moqi::protocol::ClientRequest request;
  request.set_seq_num(42);
  request.set_method(moqi::protocol::METHOD_ON_KEY_DOWN);
  auto *key = request.mutable_key_event();
  key->set_key_code('N');
  key->set_char_code('n');
  key->set_repeat_count(1);

  std::string framedRequest;
  ASSERT_TRUE(Moqi::Proto::serializeMessageBounded(
      request, framedRequest, Moqi::Proto::kMaxClientFramePayloadBytes));

  Moqi::Proto::FrameBuffer requestBuffer(Moqi::Proto::kMaxClientFramePayloadBytes);
  requestBuffer.append(framedRequest.data(), framedRequest.size());
  std::string requestPayload;
  ASSERT_TRUE(requestBuffer.nextFrame(requestPayload));

  moqi::protocol::ClientRequest parsedRequest;
  ASSERT_TRUE(Moqi::Proto::parsePayload(requestPayload, parsedRequest));
  EXPECT_EQ(parsedRequest.seq_num(), 42u);
  EXPECT_EQ(parsedRequest.method(), moqi::protocol::METHOD_ON_KEY_DOWN);
  EXPECT_EQ(parsedRequest.key_event().char_code(), static_cast<std::uint32_t>('n'));

  moqi::protocol::ServerResponse response;
  response.set_seq_num(parsedRequest.seq_num());
  response.set_success(true);
  response.set_show_candidates(true);
  auto *candidate = response.add_candidate_entries();
  candidate->set_id("nei-1");
  candidate->set_text("\xE4\xBD\xA0");
  candidate->set_input_code("nei");
  candidate->set_jyutping("nei5");
  candidate->set_comment("nei5");

  std::string framedResponse;
  ASSERT_TRUE(Moqi::Proto::serializeMessageBounded(
      response, framedResponse, Moqi::Proto::kMaxBackendFramePayloadBytes));
  Moqi::Proto::FrameBuffer responseBuffer(Moqi::Proto::kMaxBackendFramePayloadBytes);
  responseBuffer.append(framedResponse.data(), framedResponse.size());
  std::string responsePayload;
  ASSERT_TRUE(responseBuffer.nextFrame(responsePayload));

  moqi::protocol::ServerResponse parsedResponse;
  ASSERT_TRUE(Moqi::Proto::parsePayload(responsePayload, parsedResponse));
  EXPECT_TRUE(matchesExpectedSequence(parsedResponse, parsedRequest.seq_num()));
  ASSERT_EQ(parsedResponse.candidate_entries_size(), 1);
  EXPECT_EQ(parsedResponse.candidate_entries(0).text(), "\xE4\xBD\xA0");
  EXPECT_EQ(parsedResponse.candidate_entries(0).input_code(), "nei");
  EXPECT_EQ(parsedResponse.candidate_entries(0).jyutping(), "nei5");
}

TEST(TypeDuckProtocolRecovery, RawLookupSeparatorsSurviveServerResponseCandidatePage) {
  const std::string rawComment =
      "\vReverse lookup marker\f\rmatch_input_buffer,honzi,jyutping\rnei,\xE4\xBD\xA0,nei5";

  moqi::protocol::ServerResponse response;
  response.set_seq_num(91);
  response.set_success(true);
  response.set_show_candidates(true);
  auto *page = response.mutable_typeduck_candidate_page();
  page->set_page_index(0);
  page->set_page_size(5);
  page->set_total_count(1);
  auto *candidate = response.add_candidate_entries();
  candidate->set_id("lookup-nei");
  candidate->set_text("\xE4\xBD\xA0");
  candidate->set_raw_lookup_comment(rawComment);
  candidate->set_comment(rawComment);

  std::string framed;
  ASSERT_TRUE(Moqi::Proto::serializeMessageBounded(
      response, framed, Moqi::Proto::kMaxBackendFramePayloadBytes));
  Moqi::Proto::FrameBuffer buffer(Moqi::Proto::kMaxBackendFramePayloadBytes);
  buffer.append(framed.data(), framed.size());
  std::string payload;
  ASSERT_TRUE(buffer.nextFrame(payload));

  moqi::protocol::ServerResponse parsed;
  ASSERT_TRUE(Moqi::Proto::parsePayload(payload, parsed));
  ASSERT_TRUE(parsed.has_typeduck_candidate_page());
  ASSERT_EQ(parsed.candidate_entries_size(), 1);
  EXPECT_EQ(parsed.candidate_entries(0).raw_lookup_comment(), rawComment);
  EXPECT_EQ(parsed.candidate_entries(0).comment(), rawComment);
  EXPECT_TRUE(parsed.candidate_entries(0).raw_lookup_comment().find('\v') != std::string::npos);
  EXPECT_TRUE(parsed.candidate_entries(0).raw_lookup_comment().find('\f') != std::string::npos);
  EXPECT_TRUE(parsed.candidate_entries(0).raw_lookup_comment().find('\r') != std::string::npos);
}

TEST(TypeDuckProtocolRecovery, InvalidProtobufPayloadClearsMessages) {
  const std::string invalidPayload("\x08\xff\xff\xff", 4);

  moqi::protocol::ClientRequest request;
  request.set_seq_num(12);
  request.set_method(moqi::protocol::METHOD_INIT);
  EXPECT_FALSE(Moqi::Proto::parsePayload(invalidPayload, request));
  EXPECT_EQ(request.ByteSizeLong(), 0u);

  moqi::protocol::ServerResponse response;
  response.set_seq_num(12);
  response.set_success(true);
  EXPECT_FALSE(Moqi::Proto::parsePayload(invalidPayload, response));
  EXPECT_EQ(response.ByteSizeLong(), 0u);
}

TEST(TypeDuckProtocolRecovery, MalformedAndOversizedFramesClearBufferedBytes) {
  Moqi::Proto::FrameBuffer oversized(8);
  const std::string oversizedFrame = frameWithDeclaredSize(9, "abcdefghi");
  oversized.append(oversizedFrame.data(), oversizedFrame.size());

  std::string payload;
  EXPECT_FALSE(oversized.nextFrame(payload));
  EXPECT_TRUE(oversized.hasViolation());
  EXPECT_EQ(oversized.lastError(), Moqi::Proto::FrameError::PayloadTooLarge);
  EXPECT_EQ(oversized.bufferedBytes(), 0u);
  EXPECT_TRUE(payload.empty());

  Moqi::Proto::FrameBuffer malformed(8);
  const std::string tooMuchBuffered = frameWithDeclaredSize(8, "abcdefghijkl");
  malformed.append(tooMuchBuffered.data(), tooMuchBuffered.size());
  EXPECT_TRUE(malformed.hasViolation());
  EXPECT_EQ(malformed.lastError(), Moqi::Proto::FrameError::PayloadTooLarge);
  EXPECT_EQ(malformed.bufferedBytes(), 0u);
}

TEST(TypeDuckProtocolRecovery, StaleAndMismatchedSequencesAreIdentifiable) {
  moqi::protocol::ServerResponse current;
  current.set_seq_num(501);
  current.set_success(true);
  EXPECT_TRUE(matchesExpectedSequence(current, 501));

  moqi::protocol::ServerResponse stale;
  stale.set_seq_num(500);
  stale.set_success(true);
  EXPECT_FALSE(matchesExpectedSequence(stale, 501));

  moqi::protocol::ServerResponse missing;
  missing.set_seq_num(0);
  missing.set_success(false);
  EXPECT_FALSE(matchesExpectedSequence(missing, 501));
}
