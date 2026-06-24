cmake_minimum_required(VERSION 3.0)

project(MoqiCandidatePreview)

add_executable(MoqiCandidatePreview WIN32
    main.cpp
    "${PROJECT_SOURCE_DIR}/MoqiTextService/TypeDuckCandidateInfo.cpp"
)

target_include_directories(MoqiCandidatePreview PRIVATE
    "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(MoqiCandidatePreview
    gdi32
    user32
)
