INCLUDE(ExternalProject)

SET(BAZEL_SOURCES_DIR ${THIRD_PARTY_PATH}/bazel)
SET(BAZEL_INSTALL_DIR ${THIRD_PARTY_PATH}/)
SET(BAZEL_DOWNLOAD_DIR  "${BAZEL_SOURCES_DIR}/src/")
SET(BAZEL_URL "https://github.com/bazelbuild/bazel/releases/download/0.10.0/bazel-0.10.0-dist.zip")
ExternalProject_Add(
    extern_bazel
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DEPENDS extern_jdk
    DOWNLOAD_DIR ${BAZEL_DOWNLOAD_DIR}
    DOWNLOAD_COMMAND wget --no-check-certificate ${BAZEL_URL} -c -O bazel-0.10.0-dist.zip && mkdir -p bazel_build 
                     && unzip bazel-0.10.0-dist.zip -d bazel_build
    DOWNLOAD_NO_PROGRESS  1
    PREFIX                ${BAZEL_SOURCES_DIR}
    BUILD_COMMAND         export JAVA_HOME=${THIRD_PARTY_PATH}/jdk-1.8 && cd ${BAZEL_DOWNLOAD_DIR}/bazel_build && sh compile.sh 
    UPDATE_COMMAND        ""
    CONFIGURE_COMMAND     ""
    INSTALL_COMMAND       mkdir -p ${BAZEL_INSTALL_DIR}/bin 
                          && cp ${BAZEL_DOWNLOAD_DIR}/bazel_build/output/bazel ${BAZEL_INSTALL_DIR}/bin
    BUILD_IN_SOURCE 1
)

