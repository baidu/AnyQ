INCLUDE(ExternalProject)

SET(OPENSSL_SOURCES_DIR ${THIRD_PARTY_PATH}/openssl)
SET(OPENSSL_INSTALL_DIR ${THIRD_PARTY_PATH}/)
set(OPENSSL_DOWNLOAD_DIR  "${OPENSSL_SOURCES_DIR}/src/")
ExternalProject_Add(
    extern_openssl
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DOWNLOAD_DIR ${OPENSSL_DOWNLOAD_DIR}
    DOWNLOAD_COMMAND        git clone https://github.com/openssl/openssl.git && cd openssl && git checkout OpenSSL_1_1_0
    DOWNLOAD_NO_PROGRESS    1
    PREFIX                  ${OPENSSL_SOURCES_DIR}
    BUILD_COMMAND           ""
    UPDATE_COMMAND          ""
    CONFIGURE_COMMAND       ""
    INSTALL_COMMAND         cd ${OPENSSL_DOWNLOAD_DIR}/openssl/ && pwd
                            && ./config --prefix=${OPENSSL_INSTALL_DIR} --libdir=lib && pwd
                            && make -j32
                            && make install
    BUILD_IN_SOURCE 1
)

