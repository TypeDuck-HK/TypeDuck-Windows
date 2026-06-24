project(SetupHelper)

add_executable(SetupHelper WIN32
    SetupHelper.cpp
    SetupHelper.rc
)

set_target_properties(SetupHelper PROPERTIES
    OUTPUT_NAME "TypeDuckSetupHelper"
)

target_include_directories(SetupHelper PRIVATE
    ${CMAKE_SOURCE_DIR}
)

target_link_libraries(SetupHelper
    Advapi32
    Shell32
)
