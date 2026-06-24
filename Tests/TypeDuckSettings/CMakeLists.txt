add_executable(TypeDuckPreferences_test
  TypeDuckPreferences_test.cpp
  "${PROJECT_SOURCE_DIR}/MoqLauncher/TypeDuckPreferences.cpp"
)

target_include_directories(TypeDuckPreferences_test PRIVATE
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/MoqLauncher"
  "${jsoncpp_SOURCE_DIR}/include"
)

target_link_libraries(TypeDuckPreferences_test
  jsoncpp_lib_static
  gtest_main
  gmock_main
)

add_test(NAME TypeDuckPreferences_test COMMAND TypeDuckPreferences_test)
