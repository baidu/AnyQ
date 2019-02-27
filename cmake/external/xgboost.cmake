INCLUDE(ExternalProject)

SET(XGBOOST_SOURCES_DIR ${THIRD_PARTY_PATH}/xgboost)
SET(XGBOOST_INSTALL_DIR ${XGBOOST_SOURCES_DIR}/src/xgboost)

ExternalProject_Add(
    extern_xgboost
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DOWNLOAD_DIR ${XGBOOST_SOURCES_DIR}/src/
    DOWNLOAD_COMMAND git clone --recursive https://github.com/dmlc/xgboost.git
    DOWNLOAD_NO_PROGRESS  1
    PREFIX          ${XGBOOST_SOURCES_DIR}
    BUILD_COMMAND         ""
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND  cd ${XGBOOST_INSTALL_DIR} && make -j4
    BUILD_IN_SOURCE 1
)

add_custom_command(TARGET extern_xgboost POST_BUILD
    COMMAND mkdir -p third_party/lib/
    COMMAND mkdir -p third_party/include/
    COMMAND cp -r ${XGBOOST_INSTALL_DIR}/include/* third_party/include/
    COMMAND cp -r ${XGBOOST_INSTALL_DIR}/lib/* third_party/lib
    COMMAND cp ${XGBOOST_INSTALL_DIR}/rabit/lib/librabit.a ${XGBOOST_INSTALL_DIR}/dmlc-core/libdmlc.a third_party/lib/
    COMMAND cp -r ${XGBOOST_INSTALL_DIR}/src third_party/
    COMMAND cp -r ${XGBOOST_INSTALL_DIR}/dmlc-core/include/* ${XGBOOST_INSTALL_DIR}/rabit/include/rabit third_party/include/
)
