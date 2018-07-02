include(ExternalProject)

SET(BOOST_PROJECT       "extern_boost")
SET(BOOST_VER           "1.41.0")
SET(BOOST_URL "https://jaist.dl.sourceforge.net/project/boost/boost/1.41.0/boost_1_41_0.tar.gz")
SET(BOOST_SOURCES_DIR ${THIRD_PARTY_PATH}/boost)
SET(BOOST_DOWNLOAD_DIR  "${BOOST_SOURCES_DIR}/src/")

ExternalProject_Add(
    ${BOOST_PROJECT}
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DOWNLOAD_DIR          ${BOOST_DOWNLOAD_DIR}
    DOWNLOAD_COMMAND      wget --no-check-certificate ${BOOST_URL} -c && tar -zxvf boost_1_41_0.tar.gz
    DOWNLOAD_NO_PROGRESS  1
    PREFIX                ${BOOST_SOURCES_DIR}
    CONFIGURE_COMMAND     ""
    BUILD_COMMAND         ""
    INSTALL_COMMAND       ""
    UPDATE_COMMAND        ""
)

add_custom_command(TARGET extern_boost POST_BUILD
    COMMAND mkdir -p ${THIRD_PARTY_PATH}/include/
    COMMAND cp -r ${BOOST_DOWNLOAD_DIR}/boost_1_41_0/boost ${THIRD_PARTY_PATH}/include/
)
