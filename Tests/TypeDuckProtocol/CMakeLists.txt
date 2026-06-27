add_executable(ProtoFraming_test ProtoFraming_test.cpp)

target_include_directories(ProtoFraming_test PRIVATE
  "${MOQI_GENERATED_PROTO_ROOT}"
  "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(ProtoFraming_test
  gtest_main
  gmock_main
  ${MOQI_PROTOBUF_TARGET}
)

add_dependencies(ProtoFraming_test moqi_proto_cpp_gen)

target_sources(ProtoFraming_test PRIVATE "${MOQI_PROTO_CPP}")

add_test(NAME ProtoFraming_test COMMAND ProtoFraming_test)

add_executable(ProtocolRecovery_test ProtocolRecovery_test.cpp)

target_include_directories(ProtocolRecovery_test PRIVATE
  "${MOQI_GENERATED_PROTO_ROOT}"
  "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(ProtocolRecovery_test
  gtest_main
  gmock_main
  ${MOQI_PROTOBUF_TARGET}
)

add_dependencies(ProtocolRecovery_test moqi_proto_cpp_gen)

target_sources(ProtocolRecovery_test PRIVATE "${MOQI_PROTO_CPP}")

add_test(NAME ProtocolRecovery_test COMMAND ProtocolRecovery_test)
