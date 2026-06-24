project(TypeDuckSettings)

configure_file("TypeDuckSettingsVersion.rc.in" "TypeDuckSettingsVersion.rc" @ONLY)
configure_file("TypeDuckSettingsVersion.h.in" "TypeDuckSettingsVersion.h" @ONLY)
configure_file("TypeDuckAboutVersion.rc.in" "TypeDuckAboutVersion.rc" @ONLY)

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

add_executable(TypeDuckAbout WIN32
  TypeDuckAboutMain.cpp
  TypeDuckAboutDialog.cpp
  TypeDuckAboutDialog.h
  TypeDuckAbout.rc
)

target_include_directories(TypeDuckAbout PRIVATE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(TypeDuckAbout
  Comctl32
  Shell32
  User32
  Gdi32
)
