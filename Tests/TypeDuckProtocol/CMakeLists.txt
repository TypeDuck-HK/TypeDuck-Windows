add_executable(ProtoFraming_test ProtoFraming_test.cpp)

target_include_directories(ProtoFraming_test PRIVATE
  "${PROJECT_SOURCE_DIR}"
  "${MOQI_GENERATED_PROTO_ROOT}"
)

target_link_libraries(ProtoFraming_test
  gtest_main
  gmock_main
  ${MOQI_PROTOBUF_TARGET}
)

add_dependencies(ProtoFraming_test moqi_proto_cpp_gen)

target_sources(ProtoFraming_test PRIVATE "${MOQI_PROTO_CPP}")

add_test(NAME ProtoFraming_test COMMAND ProtoFraming_test)
