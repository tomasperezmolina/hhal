cmake_minimum_required (VERSION 2.8.11)

project(hhal LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(MANGO_ROOT /opt/mango)
set(LIB_DIR ${MANGO_ROOT}/lib/hhal)
set(INCLUDE_DIR ${MANGO_ROOT}/include/hhal)
set(EXPORT_DIR ${MANGO_ROOT}/lib/cmake/hhal)

set(ENABLE_NVIDIA true)

set(NVIDIA_SOURCES nvidia/manager.cpp)

set(SOURCES 
    hhal.cpp
    gn/manager.cpp 
    gn/mm.cpp
    gn/gn/hnemu/hnemu.cpp
    gn/gn/hnemu/logger.cpp
    hn/manager.cpp
)

if(ENABLE_NVIDIA)
    add_definitions(-DENABLE_NVIDIA) 
    set(SOURCES ${SOURCES} ${NVIDIA_SOURCES})
endif(ENABLE_NVIDIA)

set(HEADERS hhal.h)

add_library(hhal SHARED ${SOURCES} ${HEADERS})

if(ENABLE_NVIDIA)
    add_executable(nvidia_launch_kernel test/nvidia_launch_kernel.cpp)
endif(ENABLE_NVIDIA)

add_executable(gn_launch_kernel test/gn_launch_kernel.cpp)

target_link_libraries(hhal PRIVATE pthread)

target_include_directories(hhal PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${INCLUDE_DIR}>)

if(ENABLE_NVIDIA)
    target_include_directories(nvidia_launch_kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif(ENABLE_NVIDIA)

target_include_directories(gn_launch_kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(gn_launch_kernel PRIVATE hhal)

if(ENABLE_NVIDIA)
    find_package(cuda_manager CONFIG REQUIRED HINTS /opt/mango/lib/cmake)
    target_link_libraries(hhal PUBLIC cuda_manager)
    target_link_libraries(nvidia_launch_kernel PRIVATE hhal)
    configure_file(test/saxpy saxpy COPYONLY)
endif(ENABLE_NVIDIA)


add_subdirectory(test)