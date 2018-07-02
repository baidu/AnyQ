INCLUDE(ExternalProject)

SET(JDK_SOURCES_DIR ${THIRD_PARTY_PATH}/jdk)
SET(JDK_INSTALL_DIR ${THIRD_PARTY_PATH}/)
SET(JDK_DOWNLOAD_DIR  "${JDK_SOURCES_DIR}/src/")
SET(JDK_URL "http://anyq.bj.bcebos.com/tools/jdk-8u171-linux-x64.tar.gz")
ExternalProject_Add(
    extern_jdk
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DOWNLOAD_DIR ${JDK_DOWNLOAD_DIR}
    DOWNLOAD_COMMAND wget --no-check-certificate ${JDK_URL} -c
    DOWNLOAD_NO_PROGRESS  1
    PREFIX                ${JDK_SOURCES_DIR}
    BUILD_COMMAND         cd ${JDK_DOWNLOAD_DIR}/ && tar -zxvf jdk-8u171-linux-x64.tar.gz
    UPDATE_COMMAND        ""
    CONFIGURE_COMMAND     ""
    INSTALL_COMMAND cd ${JDK_DOWNLOAD_DIR}/ &&  cp -rf jdk1.8.0_171/ ${JDK_INSTALL_DIR}/jdk-1.8
    BUILD_IN_SOURCE 1
)

