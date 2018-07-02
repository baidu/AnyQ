INCLUDE(ExternalProject)

ExternalProject_Add(
    proto_build
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DEPENDS               extern_protobuf
    DOWNLOAD_DIR          ""
    DOWNLOAD_COMMAND      ""
    DOWNLOAD_NO_PROGRESS  1
    PREFIX                ""
    BUILD_COMMAND         ""
    UPDATE_COMMAND        ""
    CONFIGURE_COMMAND     ""
    INSTALL_COMMAND       ""
    BUILD_IN_SOURCE       1

)

add_custom_command(TARGET proto_build POST_BUILD
            COMMAND ${PROTOC_BIN} --cpp_out=${PROTO_PATH} --proto_path=${PROTO_PATH} ${PROTO_PATH}/*.proto
            COMMAND mkdir -p ${PROTO_INC} ${PROTO_SRC}
            COMMAND mv ${PROTO_PATH}/*.h ${PROTO_INC}
            COMMAND mv ${PROTO_PATH}/*.cc ${PROTO_SRC}
)
