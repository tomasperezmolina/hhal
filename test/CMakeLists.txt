cmake_minimum_required (VERSION 2.8.11)

project(hhal_test LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose debug or release" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(MANGO_ROOT /opt/mango)
set(BIN_DIR ${MANGO_ROOT}/bin/hhal_test)

find_package(hhal CONFIG REQUIRED HINTS /opt/mango/lib/cmake)
find_package(cuda_manager CONFIG REQUIRED HINTS /opt/mango/lib/cmake)

add_executable(nvidia_launch_kernel 
    event_utils.cpp 
    nvidia_launch_kernel.cpp
)

add_executable(gn_launch_kernel 
    event_utils.cpp 
    gn_launch_kernel.cpp
    gn_dummy_rm.cpp
)

add_executable(gn_serial_saxpy 
    event_utils.cpp 
    gn_serial_saxpy.cpp
    gn_dummy_rm.cpp
)

add_executable(nvidia_multiple_kernels 
    event_utils.cpp 
    nvidia_multiple_kernels.cpp
)

add_executable(gn_gif_animation
    event_utils.cpp 
    gn_dummy_rm.cpp
    AnimatedGifSaver.cpp
    gn_gif_animation.cpp
)

find_library(GIF_LIB NAMES gif)

target_include_directories(nvidia_launch_kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(gn_launch_kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(gn_serial_saxpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(nvidia_multiple_kernels PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gn_launch_kernel PRIVATE hhal::hhal)
target_link_libraries(gn_serial_saxpy PRIVATE hhal::hhal)
target_link_libraries(gn_gif_animation PRIVATE hhal::hhal ${GIF_LIB})

target_link_libraries(nvidia_launch_kernel PRIVATE hhal::hhal)
target_link_libraries(nvidia_multiple_kernels PRIVATE hhal::hhal)

add_subdirectory(gn_kernels)
add_subdirectory(cuda_kernels)
