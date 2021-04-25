cmake_minimum_required(VERSION 3.13)
project(he_nn_exploration)

set(CMAKE_CXX_STANDARD 17)
find_package(SEAL 3.6 REQUIRED)
add_executable(he_nn_poc main.cpp src/Matrix.h)
target_link_libraries(he_nn_poc SEAL::seal)
