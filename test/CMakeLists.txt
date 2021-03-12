cmake_minimum_required (VERSION 2.8.11)

project(hhal_test LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose debug or release" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(MANGO_ROOT /opt/mango)

find_package(hhal
    CONFIG REQUIRED HINTS /opt/mango/lib/cmake)
find_package(hhal_client 
    CONFIG REQUIRED HINTS /opt/mango/lib/cmake)

find_library(GIF_LIB NAMES gif)

add_subdirectory(gn_kernels)
add_subdirectory(cuda_kernels)
add_subdirectory(hhal_tests)
add_subdirectory(daemon_tests)
