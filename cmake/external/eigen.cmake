INCLUDE(ExternalProject)

SET(EIGEN_SOURCES_DIR ${THIRD_PARTY_PATH}/eigen)
SET(EIGEN_INSTALL_DIR ${THIRD_PARTY_PATH}/)
set(EIGEN_DOWNLOAD_DIR ${EIGEN_SOURCES_DIR}/src/)
ExternalProject_Add(
    extern_eigen
    ${EXTERNAL_PROJECT_LOG_ARGS}
    DOWNLOAD_DIR          ${EIGEN_DOWNLOAD_DIR}
    DOWNLOAD_COMMAND      git clone https://github.com/PX4/eigen.git && cd eigen && git checkout 3.3.4
    DOWNLOAD_NO_PROGRESS  1
    PREFIX                ${EIGEN_SOURCES_DIR}
    BUILD_COMMAND         ""
    UPDATE_COMMAND        ""
    CONFIGURE_COMMAND     ""
    INSTALL_COMMAND       ""
    BUILD_IN_SOURCE       1
)

add_custom_command(TARGET extern_eigen POST_BUILD
    COMMAND mkdir -p third_party/lib/
    COMMAND mkdir -p third_party/include/
    COMMAND cp -rf ${EIGEN_DOWNLOAD_DIR}/eigen/ ${EIGEN_INSTALL_DIR}/include/
)
