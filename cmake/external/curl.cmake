include(ExternalProject)

SET(CURL_PROJECT  "extern_curl")
SET(CURL_URL      "https://curl.haxx.se/download/curl-7.60.0.tar.gz")
SET(CURL_SOURCES_DIR ${THIRD_PARTY_PATH}/curl)
SET(CURL_DOWNLOAD_DIR  "${CURL_SOURCES_DIR}/src/")

ExternalProject_Add(
    ${CURL_PROJECT}
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DOWNLOAD_DIR          ${CURL_DOWNLOAD_DIR}
    DOWNLOAD_COMMAND      wget --no-check-certificate ${CURL_URL} -c && tar -zxvf curl-7.60.0.tar.gz
    DOWNLOAD_NO_PROGRESS  1
    PREFIX                ${CURL_SOURCES_DIR}
    CONFIGURE_COMMAND     cd ${CURL_DOWNLOAD_DIR}/curl-7.60.0 && ./configure --prefix=${THIRD_PARTY_PATH} --without-ssl 
    BUILD_COMMAND         cd ${CURL_DOWNLOAD_DIR}/curl-7.60.0 && make
    INSTALL_COMMAND       cd ${CURL_DOWNLOAD_DIR}/curl-7.60.0 && make install
    UPDATE_COMMAND        ""
)
