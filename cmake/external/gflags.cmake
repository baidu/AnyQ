INCLUDE(ExternalProject)

SET(GFLAGS_SOURCES_DIR ${THIRD_PARTY_PATH}/gflags)
SET(GFLAGS_INSTALL_DIR ${THIRD_PARTY_PATH}/)

ExternalProject_Add(
    extern_gflags
    ${EXTERNAL_PROJECT_LOG_ARGS}
    GIT_REPOSITORY  "https://github.com/gflags/gflags.git"
    GIT_TAG         "v2.2.1"
    PREFIX          ${GFLAGS_SOURCES_DIR}
    UPDATE_COMMAND  ""
    CMAKE_ARGS      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
                    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                    -DCMAKE_PREFIX_PATH=${GFLAGS_SOURCES_DIR}
                    -DCMAKE_INSTALL_PREFIX=${GFLAGS_INSTALL_DIR}
                    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                    -DBUILD_SHARED_LIBS=ON 
                    -DBUILD_STATIC_LIBS=ON
                    -DBUILD_TESTING=OFF
                    -DCMAKE_BUILD_TYPE=${THIRD_PARTY_BUILD_TYPE}
                    ${EXTERNAL_OPTIONAL_ARGS}
    CMAKE_CACHE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${GFLAGS_INSTALL_DIR}
                     -DBUILD_SHARED_LIBS:BOOL=ON 
                     -DBUILD_STATIC_LIBS:BOOL=ON
                     -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
                     -DCMAKE_BUILD_TYPE:STRING=${THIRD_PARTY_BUILD_TYPE}
)

