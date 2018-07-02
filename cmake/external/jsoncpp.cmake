INCLUDE(ExternalProject)

SET(JSONCPP_SOURCES_DIR ${THIRD_PARTY_PATH}/jsoncpp)
SET(JSONCPP_INSTALL_DIR ${THIRD_PARTY_PATH}/install/jsoncpp)
SET(JSONCPP_TARGET_VERSION "1.8.4")

ExternalProject_Add(extern_jsoncpp
    GIT_REPOSITORY      https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG             1.8.4
    PREFIX              ${JSONCPP_SOURCES_DIR}
    CONFIGURE_COMMAND   cd <SOURCE_DIR> && ${CMAKE_COMMAND} -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_PATH} CMakeLists.txt
    BUILD_COMMAND       cd <SOURCE_DIR> && make
    INSTALL_COMMAND     cd <SOURCE_DIR> && make install 
    UPDATE_COMMAND      ""
)

LIST(APPEND external_project_dependencies jsoncpp)
