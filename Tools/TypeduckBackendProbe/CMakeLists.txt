add_executable(TypeduckBackendProbe
    main.cpp
    ${MOQI_PROTO_CPP}
)

add_dependencies(TypeduckBackendProbe moqi_proto_cpp_gen)

target_include_directories(TypeduckBackendProbe
    PRIVATE
        "${MOQI_GENERATED_PROTO_ROOT}"
        "${PROJECT_SOURCE_DIR}"
        "${MOQI_PROTOBUF_INCLUDE_DIR}"
        "${MOQI_PROTOBUF_ABSL_INCLUDE_DIR}"
)

target_link_libraries(TypeduckBackendProbe
    PRIVATE
        ${MOQI_PROTOBUF_TARGET}
)
