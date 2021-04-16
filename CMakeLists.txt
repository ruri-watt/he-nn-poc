cmake_minimum_required(VERSION 3.17)
project(he_nn_exploration)

set(CMAKE_CXX_STANDARD 17)
include_directories(${SEAL_ROOT}/include/SEAL-3.6/)
add_executable(he_nn_exploration main.cpp src/Matrix.h)
target_link_libraries(he_nn_exploration ${SEAL_ROOT}/lib64/libseal-3.6.a)
