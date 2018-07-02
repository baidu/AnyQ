INCLUDE(ExternalProject)

SET(TENSORFLOW_SOURCES_DIR ${THIRD_PARTY_PATH}/tensorflow)
SET(TENSORFLOW_INSTALL_DIR ${THIRD_PARTY_PATH}/)
SET(TENSORFLOW_DOWNLOAD_DIR  "${TENSORFLOW_SOURCES_DIR}/src/")
SET(TENSORFLOW_URL "https://codeload.github.com/tensorflow/tensorflow/zip/v1.8.0")
ExternalProject_Add(
    extern_tensorflow
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DEPENDS extern_bazel
    DOWNLOAD_DIR ${TENSORFLOW_DOWNLOAD_DIR}
    DOWNLOAD_COMMAND wget --no-check-certificate ${TENSORFLOW_URL} -c -O tensorflow-1.8.0.zip
                     && unzip tensorflow-1.8.0.zip
    DOWNLOAD_NO_PROGRESS  1
    PREFIX                ${TENSORFLOW_SOURCES_DIR}
    BUILD_COMMAND         export JAVA_HOME=${THIRD_PARTY_PATH}/jdk-1.8 
                          && cd ${TENSORFLOW_DOWNLOAD_DIR}/tensorflow-1.8.0
                          && ${THIRD_PARTY_PATH}/bin/bazel build //tensorflow:libtensorflow_cc.so
    UPDATE_COMMAND        ""
    CONFIGURE_COMMAND     ""
    INSTALL_COMMAND       ""
    BUILD_IN_SOURCE 1
)

add_custom_command(TARGET extern_tensorflow POST_BUILD
    COMMAND mkdir -p ${TENSORFLOW_INSTALL_DIR}/include/tf
    COMMAND mkdir -p ${TENSORFLOW_INSTALL_DIR}/lib
    COMMAND cp -rf ${TENSORFLOW_DOWNLOAD_DIR}/tensorflow-1.8.0/bazel-bin/tensorflow/lib*.so ${TENSORFLOW_INSTALL_DIR}/lib
    COMMAND cp -rf ${TENSORFLOW_DOWNLOAD_DIR}/tensorflow-1.8.0/bazel-genfiles/* ${TENSORFLOW_INSTALL_DIR}/include/tf
    COMMAND cp -rf ${TENSORFLOW_DOWNLOAD_DIR}/tensorflow-1.8.0/tensorflow ${TENSORFLOW_INSTALL_DIR}/include/tf
    COMMAND cp -rf ${TENSORFLOW_DOWNLOAD_DIR}/tensorflow-1.8.0/third_party ${TENSORFLOW_INSTALL_DIR}/include/tf
)
