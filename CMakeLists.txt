cmake_minimum_required(VERSION 3.21)
# Satisfy older FetchContent deps (e.g. jsoncpp) under CMake 4.x; avoids fragile -D on command line.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Explicitly control the MSVC runtime before project() so generated VS projects
# keep the legacy PIME behavior: /MDd for Debug, /MD for RelWithDebInfo,
# and /MT for Release/MinSizeRel.
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
set(CMAKE_MSVC_RUNTIME_LIBRARY
    "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:DLL>")

# Override default c/c++ flags to keep MSVC release builds self-contained.
set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_flag_overrides.cmake")
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake")

project(MoqiIMWindows LANGUAGES C CXX)

include(FetchContent)
set(MOQI_PROTOBUF_ROOT "" CACHE PATH "Optional local protobuf install root (contains bin/include)")
set(MOQI_PROTOBUF_SOURCE_DIR "" CACHE PATH "Optional local protobuf source tree")
set(MOQI_PROTOC_EXECUTABLE "" CACHE FILEPATH "Optional local protoc executable")

# Load semantic version from version.txt.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" MOQI_VERSION)
string(STRIP "${MOQI_VERSION}" MOQI_VERSION)
string(REGEX MATCHALL "[0-9]+" MOQI_VERSION_PARTS "${MOQI_VERSION}")
list(GET MOQI_VERSION_PARTS 0 MOQI_VERSION_MAJOR)
list(GET MOQI_VERSION_PARTS 1 MOQI_VERSION_MINOR)
list(GET MOQI_VERSION_PARTS 2 MOQI_VERSION_PATCH)

add_definitions(
    /D_UNICODE=1
    /DUNICODE=1
    /D_CRT_SECURE_NO_WARNINGS
    /GR-
)

add_definitions(
    -DWIN32_LEAN_AND_MEAN
    -D_WIN32_WINNT=0x0602
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_definitions(
        _HAS_DEPRECATED_RESULT_OF=1
        _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING
    )
    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:/utf-8>
        $<$<COMPILE_LANGUAGE:CXX>:/Zc:char8_t->
        $<$<COMPILE_LANGUAGE:CXX>:/FS>
    )
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)

set(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_STRICT_ISO OFF CACHE BOOL "" FORCE)

set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE_HO OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_WARNINGS OFF CACHE BOOL "" FORCE)

set(LIBUV_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(LIBUV_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LIBUV_BUILD_BENCH OFF CACHE BOOL "" FORCE)

set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_LIBUPB ON CACHE BOOL "" FORCE)
set(protobuf_WITH_ZLIB OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_PROTOBUF_BINARIES ON CACHE BOOL "" FORCE)
if(MOQI_PROTOC_EXECUTABLE)
    set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE BOOL "" FORCE)
else()
    set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "" FORCE)
endif()
set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "" FORCE)
set(ABSL_PROPAGATE_CXX_STD ON CACHE BOOL "" FORCE)
set(ABSL_MSVC_STATIC_RUNTIME OFF CACHE BOOL "" FORCE)

function(moqi_apply_project_msvc_runtime Dir)
    if(NOT MSVC OR NOT IS_DIRECTORY "${Dir}")
        return()
    endif()

    get_property(MOQI_RUNTIME_TARGETS DIRECTORY "${Dir}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(MOQI_RUNTIME_TARGET IN LISTS MOQI_RUNTIME_TARGETS)
        get_target_property(MOQI_RUNTIME_TARGET_TYPE "${MOQI_RUNTIME_TARGET}" TYPE)
        if(NOT MOQI_RUNTIME_TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
            set_property(TARGET "${MOQI_RUNTIME_TARGET}" PROPERTY MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}")
        endif()
    endforeach()

    get_property(MOQI_RUNTIME_SUBDIRS DIRECTORY "${Dir}" PROPERTY SUBDIRECTORIES)
    foreach(MOQI_RUNTIME_SUBDIR IN LISTS MOQI_RUNTIME_SUBDIRS)
        moqi_apply_project_msvc_runtime("${MOQI_RUNTIME_SUBDIR}")
    endforeach()
endfunction()

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.2.1
)

FetchContent_MakeAvailable(spdlog)

if(NOT MOQI_PROTOC_EXECUTABLE AND MOQI_PROTOBUF_ROOT AND EXISTS "${MOQI_PROTOBUF_ROOT}/bin/protoc.exe")
    set(MOQI_PROTOC_EXECUTABLE "${MOQI_PROTOBUF_ROOT}/bin/protoc.exe")
endif()

if(MOQI_PROTOBUF_SOURCE_DIR)
    # Local protobuf source trees used by developers may already vendor abseil.
    # Keep protobuf in local-only mode for that case.
    set(protobuf_LOCAL_DEPENDENCIES_ONLY ON CACHE BOOL "" FORCE)
    if(EXISTS "${MOQI_PROTOBUF_SOURCE_DIR}/third_party/abseil-cpp/CMakeLists.txt" AND NOT TARGET absl::strings)
        add_subdirectory("${MOQI_PROTOBUF_SOURCE_DIR}/third_party/abseil-cpp" "${CMAKE_BINARY_DIR}/abseil")
    endif()
    add_subdirectory("${MOQI_PROTOBUF_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/protobuf")
    set(MOQI_PROTOBUF_INCLUDE_DIR "${MOQI_PROTOBUF_SOURCE_DIR}/src")
    set(MOQI_PROTOBUF_ABSL_INCLUDE_DIR "${MOQI_PROTOBUF_SOURCE_DIR}/third_party/abseil-cpp")
else()
    # CI often provides only protoc. Let protobuf fetch its own CMake deps.
    set(protobuf_LOCAL_DEPENDENCIES_ONLY OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        protobuf
        URL https://github.com/protocolbuffers/protobuf/releases/download/v33.5/protobuf-33.5.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(protobuf)
    set(MOQI_PROTOBUF_INCLUDE_DIR "${protobuf_SOURCE_DIR}/src")
    set(MOQI_PROTOBUF_ABSL_INCLUDE_DIR "${protobuf_SOURCE_DIR}/third_party/abseil-cpp")
endif()

if(MSVC)
    foreach(MOQI_DEP_RUNTIME_DIR IN ITEMS
        "${CMAKE_BINARY_DIR}/abseil"
        "${CMAKE_BINARY_DIR}/protobuf"
        "${CMAKE_BINARY_DIR}/_deps/absl-build"
        "${CMAKE_BINARY_DIR}/_deps/protobuf-build")
        moqi_apply_project_msvc_runtime("${MOQI_DEP_RUNTIME_DIR}")
    endforeach()
endif()

if(MSVC)
    foreach(MOQI_PROTOBUF_TOOL_TARGET IN ITEMS libprotoc protoc)
        if(TARGET ${MOQI_PROTOBUF_TOOL_TARGET})
            target_compile_options(${MOQI_PROTOBUF_TOOL_TARGET} PRIVATE /MP1)
        endif()
    endforeach()
endif()

set(MOQI_GENERATED_PROTO_ROOT "${CMAKE_BINARY_DIR}/generated")
set(MOQI_GENERATED_PROTO_DIR "${MOQI_GENERATED_PROTO_ROOT}/proto")
set(MOQI_PROTO_SCHEMA "${CMAKE_CURRENT_SOURCE_DIR}/proto/moqi.proto")
set(MOQI_PROTO_CPP "${MOQI_GENERATED_PROTO_DIR}/moqi.pb.cc")
set(MOQI_PROTO_HDR "${MOQI_GENERATED_PROTO_DIR}/moqi.pb.h")
file(TO_CMAKE_PATH "${MOQI_GENERATED_PROTO_ROOT}" MOQI_GENERATED_PROTO_ROOT_PROTOC)

if(NOT MOQI_PROTOC_EXECUTABLE)
    find_program(MOQI_PROTOC_EXECUTABLE NAMES protoc)
endif()

set(MOQI_PROTOC_COMMAND "")
set(MOQI_PROTOC_DEPENDS "")

if(MOQI_PROTOC_EXECUTABLE)
    set(MOQI_PROTOC_COMMAND "${MOQI_PROTOC_EXECUTABLE}")
elseif(TARGET protobuf::protoc)
    set(MOQI_PROTOC_COMMAND "$<TARGET_FILE:protobuf::protoc>")
    set(MOQI_PROTOC_DEPENDS protoc)
endif()

if(MOQI_PROTOC_COMMAND)
    add_custom_command(
        OUTPUT "${MOQI_PROTO_CPP}" "${MOQI_PROTO_HDR}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${MOQI_GENERATED_PROTO_DIR}"
        COMMAND "${MOQI_PROTOC_COMMAND}" --proto_path=. "--cpp_out=${MOQI_GENERATED_PROTO_ROOT_PROTOC}" "proto/moqi.proto"
        DEPENDS "${MOQI_PROTO_SCHEMA}" ${MOQI_PROTOC_DEPENDS}
        COMMENT "Generating C++ protobuf sources for moqi.proto"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        VERBATIM
    )
    add_custom_target(moqi_proto_cpp_gen DEPENDS "${MOQI_PROTO_CPP}" "${MOQI_PROTO_HDR}")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/proto/moqi.pb.cc" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/proto/moqi.pb.h")
    message(WARNING "Cannot find protoc. Falling back to checked-in proto/moqi.pb.* for local proof builds.")
    set(MOQI_GENERATED_PROTO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
    set(MOQI_GENERATED_PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
    set(MOQI_PROTO_CPP "${CMAKE_CURRENT_SOURCE_DIR}/proto/moqi.pb.cc")
    set(MOQI_PROTO_HDR "${CMAKE_CURRENT_SOURCE_DIR}/proto/moqi.pb.h")
    add_custom_target(moqi_proto_cpp_gen DEPENDS "${MOQI_PROTO_CPP}" "${MOQI_PROTO_HDR}")
else()
    message(FATAL_ERROR "Cannot find protoc. Set MOQI_PROTOC_EXECUTABLE or MOQI_PROTOBUF_ROOT to a local protoc installation.")
endif()

if(TARGET protobuf::libprotobuf)
    set(MOQI_PROTOBUF_TARGET protobuf::libprotobuf)
else()
    set(MOQI_PROTOBUF_TARGET libprotobuf)
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/jsoncpp")

add_subdirectory("${PROJECT_SOURCE_DIR}/libIME2")
add_subdirectory("${PROJECT_SOURCE_DIR}/libuv")
add_subdirectory("${PROJECT_SOURCE_DIR}/Tests/TypeDuckCandidateData")
add_subdirectory("${PROJECT_SOURCE_DIR}/Tests/TypeDuckSettings")
add_subdirectory("${PROJECT_SOURCE_DIR}/Tests/TypeDuckProtocol")
add_subdirectory("${PROJECT_SOURCE_DIR}/MoqiTextService")
add_subdirectory("${PROJECT_SOURCE_DIR}/Preview")
add_subdirectory("${PROJECT_SOURCE_DIR}/TypeDuckSettings")

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_subdirectory("${PROJECT_SOURCE_DIR}/Tools/TypeduckBackendProbe")
    add_subdirectory("${PROJECT_SOURCE_DIR}/MoqLauncher")
    add_subdirectory("${PROJECT_SOURCE_DIR}/SetupHelper")
endif()
