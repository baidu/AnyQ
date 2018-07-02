INCLUDE(ExternalProject)

SET(LAC_SOURCES_DIR ${THIRD_PARTY_PATH}/lac)
SET(LAC_INSTALL_DIR ${THIRD_PARTY_PATH}/)
SET(LAC_DOWNLOAD_DIR ${LAC_SOURCES_DIR}/src/)
ExternalProject_Add(
    extern_lac
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DEPENDS                 extern_paddle
    DOWNLOAD_DIR            ${LAC_DOWNLOAD_DIR}
    DOWNLOAD_COMMAND        git clone https://github.com/baidu/lac.git
    DOWNLOAD_NO_PROGRESS    1
    PREFIX                  ${LAC_SOURCES_DIR}
    BUILD_COMMAND           cd ${LAC_DOWNLOAD_DIR}/lac && cmake -DPADDLE_ROOT=${THIRD_PARTY_PATH}/install/paddle/fluid_install_dir/ ./
    UPDATE_COMMAND          ""
    CONFIGURE_COMMAND       ""
    INSTALL_COMMAND         cd ${LAC_DOWNLOAD_DIR}/lac && make -j8 && make install
    BUILD_IN_SOURCE         1
)

add_custom_command(TARGET extern_lac POST_BUILD
    COMMAND mkdir -p ${LAC_INSTALL_DIR}/include/
    COMMAND mkdir -p ${LAC_INSTALL_DIR}/lib/
    COMMAND cp -r ${LAC_DOWNLOAD_DIR}/lac/output/include/* ${LAC_INSTALL_DIR}/include/
    COMMAND cp -r ${LAC_DOWNLOAD_DIR}/lac/output/lib/* ${LAC_INSTALL_DIR}/lib/
)
