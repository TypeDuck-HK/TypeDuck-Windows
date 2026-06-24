add_executable(TypeDuckCandidateInfo_test
  TypeDuckCandidateInfo_test.cpp
  "${PROJECT_SOURCE_DIR}/MoqiTextService/TypeDuckCandidateInfo.cpp"
)

target_include_directories(TypeDuckCandidateInfo_test PRIVATE
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/MoqiTextService"
)

target_link_libraries(TypeDuckCandidateInfo_test
  gtest_main
  gmock_main
)

add_test(NAME TypeDuckCandidateInfo_test COMMAND TypeDuckCandidateInfo_test)
