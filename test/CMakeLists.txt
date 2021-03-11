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

add_executable(nvidia_launch_kernel 
    event_utils.cpp 
    nvidia_dummy_rm.cpp
    rm_common.cpp
    hhal_tests/nvidia_launch_kernel.cpp
)

add_executable(gn_launch_kernel 
    event_utils.cpp 
    gn_dummy_rm.cpp
    rm_common.cpp
    hhal_tests/gn_launch_kernel.cpp
)

add_executable(gn_serial_saxpy 
    event_utils.cpp 
    gn_dummy_rm.cpp
    rm_common.cpp
    hhal_tests/gn_serial_saxpy.cpp
)

add_executable(nvidia_multiple_kernels 
    event_utils.cpp 
    nvidia_dummy_rm.cpp
    rm_common.cpp
    hhal_tests/nvidia_multiple_kernels.cpp
)

add_executable(gn_gif_animation
    event_utils.cpp 
    gn_dummy_rm.cpp
    rm_common.cpp
    AnimatedGifSaver.cpp
    hhal_tests/gn_gif_animation.cpp
)

add_executable(gn_nvidia_saxpy
    event_utils.cpp 
    gn_dummy_rm.cpp
    nvidia_dummy_rm.cpp
    rm_common.cpp
    hhal_tests/gn_nvidia_saxpy.cpp
)

# Daemon

add_executable(daemon_gn_launch_kernel 
    event_utils.cpp 
    gn_dummy_rm.cpp
    rm_common.cpp
    daemon_tests/gn_launch_kernel.cpp
)

add_executable(daemon_nvidia_launch_kernel 
    event_utils.cpp 
    nvidia_dummy_rm.cpp
    rm_common.cpp
    daemon_tests/nvidia_launch_kernel.cpp
)

add_executable(daemon_gn_serial_saxpy 
    event_utils.cpp 
    gn_dummy_rm.cpp
    rm_common.cpp
    daemon_tests/gn_serial_saxpy.cpp
)

add_executable(daemon_nvidia_multiple_kernels 
    event_utils.cpp 
    nvidia_dummy_rm.cpp
    rm_common.cpp
    daemon_tests/nvidia_multiple_kernels.cpp
)

add_executable(daemon_gn_gif_animation
    event_utils.cpp 
    gn_dummy_rm.cpp
    rm_common.cpp
    AnimatedGifSaver.cpp
    daemon_tests/gn_gif_animation.cpp
)

add_executable(daemon_gn_nvidia_saxpy
    event_utils.cpp 
    gn_dummy_rm.cpp
    nvidia_dummy_rm.cpp
    rm_common.cpp
    daemon_tests/gn_nvidia_saxpy.cpp
)

find_library(GIF_LIB NAMES gif)

target_include_directories(gn_launch_kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(nvidia_launch_kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(gn_serial_saxpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(nvidia_multiple_kernels PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(gn_nvidia_saxpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(gn_gif_animation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(daemon_gn_launch_kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(daemon_nvidia_launch_kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(daemon_gn_serial_saxpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(daemon_nvidia_multiple_kernels PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(daemon_gn_nvidia_saxpy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(daemon_gn_gif_animation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gn_launch_kernel PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(nvidia_launch_kernel PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(gn_serial_saxpy PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(nvidia_multiple_kernels PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(gn_nvidia_saxpy PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(gn_gif_animation PRIVATE hhal::hhal ${GIF_LIB} hhal_client::hhal_client)

target_link_libraries(daemon_gn_launch_kernel PRIVATE hhal_client::hhal_client)
target_link_libraries(daemon_nvidia_launch_kernel PRIVATE hhal_client::hhal_client)
target_link_libraries(daemon_gn_serial_saxpy PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(daemon_nvidia_multiple_kernels PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(daemon_gn_nvidia_saxpy PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(daemon_gn_gif_animation PRIVATE hhal::hhal ${GIF_LIB} hhal_client::hhal_client)

add_subdirectory(gn_kernels)
add_subdirectory(cuda_kernels)
