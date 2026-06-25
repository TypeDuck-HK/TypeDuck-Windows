project(TypeDuckSettings)

configure_file("TypeDuckSettingsVersion.rc.in" "TypeDuckSettingsVersion.rc" @ONLY)
configure_file("TypeDuckSettingsVersion.h.in" "TypeDuckSettingsVersion.h" @ONLY)
configure_file("TypeDuckAboutVersion.rc.in" "TypeDuckAboutVersion.rc" @ONLY)

add_executable(TypeDuckSettings WIN32
  ${MOQI_PROTO_CPP}
  main.cpp
  TypeDuckSettingsWindow.cpp
  TypeDuckSettingsWindow.h
  "${CMAKE_SOURCE_DIR}/MoqLauncher/TypeDuckPreferences.cpp"
  TypeDuckSettings.rc
)

target_include_directories(TypeDuckSettings PRIVATE
  "${MOQI_GENERATED_PROTO_ROOT}"
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/MoqLauncher"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${jsoncpp_SOURCE_DIR}/include"
  "${MOQI_PROTOBUF_INCLUDE_DIR}"
  "${MOQI_PROTOBUF_ABSL_INCLUDE_DIR}"
)

set_source_files_properties(${MOQI_PROTO_CPP} ${MOQI_PROTO_HDR} PROPERTIES GENERATED TRUE)
add_dependencies(TypeDuckSettings moqi_proto_cpp_gen)

target_link_libraries(TypeDuckSettings
  jsoncpp_lib_static
  ${MOQI_PROTOBUF_TARGET}
  Advapi32
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
