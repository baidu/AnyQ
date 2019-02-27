cmake_minimum_required(VERSION 3.0)
project(ANYQ C CXX)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # require at least gcc 4.8
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
        message(FATAL_ERROR "GCC is too old, please install a newer version supporting C++11")
    endif()
else()
    message(WARNING "You are using an unsupported compiler! Compilation has only been tested with GCC.")
endif()

SET(THIRD_PARTY_PATH "${CMAKE_BINARY_DIR}/third_party" CACHE STRING
  "A path setting third party libraries download & build directories.")
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(USE_TENSORFLOW "use tensorflow rather than paddle" OFF)

if(USE_TENSORFLOW)
    set(TENSORFLOW_TAG "-DUSE_TENSORFLOW")
    set(TENSORFLOW_INC ${THIRD_PARTY_PATH}/include/tf)
    LIST(APPEND TENSORFLOW_LIB "${THIRD_PARTY_PATH}/lib/libtensorflow_cc.so" "${THIRD_PARTY_PATH}/lib/libtensorflow_framework.so")
else()
    set(TENSORFLOW_TAG "")
    set(TENSORFLOW_INC "")
    set(TENSORFLOW_LIB "")
endif()

SET(PROTOC_BIN "${THIRD_PARTY_PATH}/bin/protoc")
SET(PROTO_SRC ${CMAKE_SOURCE_DIR}/src/config)
SET(PROTO_INC ${CMAKE_SOURCE_DIR}/include/config)
SET(PROTO_PATH ${CMAKE_SOURCE_DIR}/proto)
execute_process(COMMAND mkdir -p ${PROTO_SRC})
execute_process(COMMAND mkdir -p ${PROTO_INC})

include(external/zlib)
include(external/curl)
include(external/protobuf)
include(external/glog)
include(external/gflags)
include(external/gtest)
include(external/boost)
include(external/jsoncpp)
include(external/leveldb)
include(external/openssl)
include(external/brpc)
include(external/xgboost)
include(external/eigen)
include(external/paddle)
include(external/lac)
include(proto_build)

if(USE_TENSORFLOW)
    include(external/jdk)
    include(external/bazel)
    include(external/tensorflow)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/include/config
    ${THIRD_PARTY_PATH}/include/
    ${THIRD_PARTY_PATH}/include/eigen
    ${TENSORFLOW_INC}
)

link_directories(
    ${THIRD_PARTY_PATH}/lib
)

set(CMAKE_CXX_FLAGS "-g -pipe -W -Wall -fPIC -std=c++11 -mfpmath=sse -msse -msse2 -msse3 -fopenmp -liconv") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-Bsymbolic -rdynamic -zmuldefs")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TENSORFLOW_TAG}")

file(GLOB SRCS "src/*/*.cpp" "src/*/*.cc" "src/*/*/*.cpp" "src/*/*/*.cc")

file(GLOB PROTOS "${PROTO_PATH}/*.proto")
foreach(PROTO_FILE ${PROTOS})
    GET_FILENAME_COMPONENT(FILE_NAME ${PROTO_FILE} NAME_WE)
    SET(PROTO_CPP ${PROTO_SRC}/${FILE_NAME}.pb.cc)
    LIST(APPEND SRCS ${PROTO_CPP})
endforeach()

SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES GENERATED TRUE)

add_library(anyq STATIC ${SRCS})

add_dependencies(anyq 
    extern_curl
    extern_boost 
    extern_eigen 
    extern_gtest 
    extern_brpc
    extern_jsoncpp
    extern_xgboost
    extern_paddle
    extern_lac
    proto_build)

if(USE_TENSORFLOW)
    add_dependencies(anyq extern_tensorflow)
endif()

LIST(APPEND LIBS_LIST anyq
    ${THIRD_PARTY_PATH}/lib/liblac.a
    ${THIRD_PARTY_PATH}/lib/libpaddle_fluid.so
    ${TENSORFLOW_LIB}
    ${THIRD_PARTY_PATH}/lib/libmklml_intel.so
    ${THIRD_PARTY_PATH}/lib/libiomp5.so
    ${THIRD_PARTY_PATH}/lib/libbrpc.a
    ${THIRD_PARTY_PATH}/lib/libgtest.a
    ${THIRD_PARTY_PATH}/lib/libgtest_main.a
    ${THIRD_PARTY_PATH}/lib/libcrypto.a
    ${THIRD_PARTY_PATH}/lib/libleveldb.a
    ${THIRD_PARTY_PATH}/lib/libssl.a
    ${THIRD_PARTY_PATH}/lib/libxgboost.a
    ${THIRD_PARTY_PATH}/lib/libdmlc.a
    ${THIRD_PARTY_PATH}/lib/librabit.a
    ${THIRD_PARTY_PATH}/lib/libjsoncpp.a
    ${THIRD_PARTY_PATH}/lib/libprotobuf-lite.a
    ${THIRD_PARTY_PATH}/lib/libprotobuf.a
    ${THIRD_PARTY_PATH}/lib/libprotoc.a
    ${THIRD_PARTY_PATH}/lib/libgmock.a
    ${THIRD_PARTY_PATH}/lib/libgmock_main.a
    ${THIRD_PARTY_PATH}/lib/libglog.so
    ${THIRD_PARTY_PATH}/lib/libgflags.a
    ${THIRD_PARTY_PATH}/lib/libcurl.so
    dl
    rt
    ssl
    crypt
    crypto
    z
    m)
    
add_executable(demo_anyq_multi ${CMAKE_SOURCE_DIR}/demo/demo_anyq_multi.cpp)
add_executable(demo_anyq ${CMAKE_SOURCE_DIR}/demo/demo_anyq.cpp)
add_executable(run_server ${CMAKE_SOURCE_DIR}/demo/run_server.cpp)
add_executable(annoy_index_build_tool ${CMAKE_SOURCE_DIR}/demo/annoy_index_build.cpp)
add_executable(feature_dump_tool ${CMAKE_SOURCE_DIR}/demo/feature_dump.cpp)

target_link_libraries(demo_anyq_multi ${LIBS_LIST})
target_link_libraries(demo_anyq ${LIBS_LIST})
target_link_libraries(run_server ${LIBS_LIST})
target_link_libraries(annoy_index_build_tool ${LIBS_LIST})
target_link_libraries(feature_dump_tool ${LIBS_LIST})
