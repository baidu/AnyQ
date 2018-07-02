include(ExternalProject)
set(GLOG_PROJECT       "extern_glog")
set(GLOG_VER           "v0.3.5")
set(GLOG_SOURCES_DIR ${THIRD_PARTY_PATH}/glog)
set(GLOG_DOWNLOAD_DIR  "${GLOG_SOURCES_DIR}/src/")
set(GLOG_INSTALL_DIR    ${THIRD_PARTY_PATH})

ExternalProject_Add(
    ${GLOG_PROJECT}
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DOWNLOAD_DIR          ${GLOG_DOWNLOAD_DIR}
    DOWNLOAD_COMMAND      wget --no-check-certificate https://codeload.github.com/google/glog/zip/v0.3.5 -c -q -O glog-0.3.5.zip
                          && unzip glog-0.3.5.zip
    DOWNLOAD_NO_PROGRESS  1
    PREFIX                ${GLOG_SOURCES_DIR}
    CONFIGURE_COMMAND     ""
    BUILD_COMMAND         ""
    INSTALL_COMMAND       cd ${GLOG_DOWNLOAD_DIR}/glog-0.3.5/ && ./configure --prefix=${GLOG_INSTALL_DIR} && make && make install
    UPDATE_COMMAND        ""
)
