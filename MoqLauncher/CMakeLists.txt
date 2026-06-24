project(MoqiLauncher)

# Legacy scaffold source compatibility: keep the internal target/source names
# while deploying the launcher executable under the TypeDuck product name.

# generate the resource file containing version info
configure_file("version.rc.in" "version.rc" @ONLY)

set(LIBUV_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/libuv/include
    ${CMAKE_SOURCE_DIR}/libuv/src
)

set(SPDLOG_INCLUDE_DIRS
    ${spdlog_SOURCE_DIR}/include
)
add_definitions(
    # Enable using wchar_t & wstring for filenames
    -DSPDLOG_WCHAR_FILENAMES=1
    # Prevent the fd of log files from being inherited by child processes
    # This is required. Otherwise the file rotation will fail since the file is kept open by the child process.
    -DSPDLOG_PREVENT_CHILD_FD=1

    # Enable our own patched libuv in UvPipe.h
    -DHAVE_UV_NAMED_PIPE
)

include_directories(
    ${MOQI_GENERATED_PROTO_ROOT}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${jsoncpp_SOURCE_DIR}/include
    ${MOQI_PROTOBUF_INCLUDE_DIR}
    ${MOQI_PROTOBUF_ABSL_INCLUDE_DIR}
    ${LIBUV_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIRS}
)

add_executable(MoqiLauncher WIN32
    ${MOQI_PROTO_CPP}
    UvPipe.h
    MoqiLauncher.cpp
    PipeServer.cpp
    PipeServer.h
    PipeClient.cpp
    PipeClient.h
    PipeSecurity.cpp
    PipeSecurity.h
    BackendServer.cpp
    BackendServer.h
    TypeDuckPreferences.cpp
    TypeDuckPreferences.h
    Utils.cpp
    Utils.h
    # resources
    MoqiLauncher.rc
)

set_target_properties(MoqiLauncher PROPERTIES
    OUTPUT_NAME "TypeDuckLauncher"
)

set_source_files_properties(${MOQI_PROTO_CPP} ${MOQI_PROTO_HDR} PROPERTIES GENERATED TRUE)
add_dependencies(MoqiLauncher moqi_proto_cpp_gen)

target_link_libraries(MoqiLauncher 
    jsoncpp_lib_static
    ${MOQI_PROTOBUF_TARGET}
    Rpcrt4 # for uuid stuff
    libuv
)
