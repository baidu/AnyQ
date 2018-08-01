include(ExternalProject)
set(GLOG_PROJECT       "extern_glog")
set(GLOG_VER           "v0.3.5")
set(GLOG_SOURCES_DIR ${THIRD_PARTY_PATH}/glog)
set(GLOG_DOWNLOAD_DIR  "${GLOG_SOURCES_DIR}/src/")
set(GLOG_INSTALL_DIR    ${THIRD_PARTY_PATH})

ExternalProject_Add(
    ${GLOG_PROJECT}
    ${EXTERNAL_PROJECT_LOG_ARGS}
    GIT_REPOSITORY  "https://github.com/google/glog.git"
    GIT_TAG         "v0.3.5"
    PREFIX          ${GLOG_SOURCES_DIR}
    UPDATE_COMMAND  ""
    CMAKE_ARGS      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
                    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                    -DCMAKE_PREFIX_PATH=${GLOG_SOURCES_DIR}
                    -DCMAKE_INSTALL_PREFIX=${GLOG_INSTALL_DIR}
                    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                    -DWITH_GFLAGS=OFF
                    ${EXTERNAL_OPTIONAL_ARGS}
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${GLOG_INSTALL_DIR}
                     -DBUILD_SHARED_LIBS:BOOL=ON 
                     -DBUILD_STATIC_LIBS:BOOL=ON
                     -DWITH_GFLAGS:BOOL=OFF 
                     -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
)
