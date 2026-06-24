project(TypeDuckSettings)

configure_file("TypeDuckSettingsVersion.rc.in" "TypeDuckSettingsVersion.rc" @ONLY)
configure_file("TypeDuckSettingsVersion.h.in" "TypeDuckSettingsVersion.h" @ONLY)

add_executable(TypeDuckSettings WIN32
  main.cpp
  TypeDuckSettingsWindow.cpp
  TypeDuckSettingsWindow.h
  "${CMAKE_SOURCE_DIR}/MoqLauncher/TypeDuckPreferences.cpp"
  TypeDuckSettings.rc
)

target_include_directories(TypeDuckSettings PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/MoqLauncher"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${jsoncpp_SOURCE_DIR}/include"
)

target_link_libraries(TypeDuckSettings
  jsoncpp_lib_static
  Comctl32
  Shell32
  User32
  Gdi32
)
