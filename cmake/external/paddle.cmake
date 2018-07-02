INCLUDE(ExternalProject)

SET(PADDLE_SOURCES_DIR ${THIRD_PARTY_PATH}/paddle)
SET(PADDLE_INSTALL_DIR ${THIRD_PARTY_PATH}/install/paddle)

ExternalProject_Add(
    extern_paddle
    ${EXTERNAL_PROJECT_LOG_ARGS}
    GIT_REPOSITORY       "https://github.com/PaddlePaddle/Paddle.git"
    GIT_TAG              "release/0.14.0"
    PREFIX               ${PADDLE_SOURCES_DIR}
    CONFIGURE_COMMAND    mkdir -p ${PADDLE_INSTALL_DIR} && cd ${PADDLE_INSTALL_DIR} && ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${PADDLE_INSTALL_DIR} 
                         -DCMAKE_BUILD_TYPE=Release -DWITH_PYTHON=OFF -DWITH_MKL=ON -DWITH_MKLDNN=OFF -DWITH_GPU=OFF -DWITH_FLUID_ONLY=ON <SOURCE_DIR>
    BUILD_COMMAND        cd ${PADDLE_INSTALL_DIR} && make -j16
    INSTALL_COMMAND      cd ${PADDLE_INSTALL_DIR} && make inference_lib_dist
    UPDATE_COMMAND       ""
)

add_custom_command(TARGET extern_paddle POST_BUILD
    COMMAND mkdir -p third_party/include/paddle/ third_party/lib
    COMMAND cp -rf ${PADDLE_INSTALL_DIR}/fluid_install_dir/paddle/fluid third_party/include/paddle
    COMMAND cp -rf ${PADDLE_INSTALL_DIR}/fluid_install_dir/paddle/fluid/inference/lib* third_party/lib
    COMMAND cp -rf ${PADDLE_INSTALL_DIR}/fluid_install_dir/third_party/install/mklml/include/* ${THIRD_PARTY_PATH}/include/
    COMMAND cp -rf ${PADDLE_INSTALL_DIR}/fluid_install_dir/third_party/install/mklml/lib/* ${THIRD_PARTY_PATH}/lib/
    COMMAND cp -rf  ${PADDLE_INSTALL_DIR}/fluid_install_dir/third_party/boost ${PADDLE_INSTALL_DIR}/fluid_install_dir/third_party/install/boost_1_41_0
)
