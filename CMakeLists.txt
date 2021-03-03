cmake_minimum_required (VERSION 2.8.11)

project(hhal-project LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose debug or release" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(MANGO_ROOT /opt/mango)
set(LIB_DIR ${MANGO_ROOT}/lib/hhal)
set(INCLUDE_DIR ${MANGO_ROOT}/include/hhal)
set(EXPORT_DIR ${MANGO_ROOT}/lib/cmake/hhal)

set(NVIDIA_SOURCES 
    nvidia/manager.cpp 
    nvidia/thread_pool.cpp 
    nvidia/event_registry.cpp
)

set(GN_SOURCES
    gn/manager.cpp 
    gn/hnemu/hnemu.cpp
    gn/hnemu/logger.cpp
)

set(SOURCES 
    hhal.cpp
)

set(SOURCES ${SOURCES} ${NVIDIA_SOURCES} ${GN_SOURCES})

set(HEADERS 
    hhal.h 
    arguments.h 
    types.h
)

add_library(hhal SHARED ${SOURCES} ${HEADERS})

target_link_libraries(hhal PRIVATE pthread)

target_include_directories(hhal PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${INCLUDE_DIR}>)


find_package(cuda_manager CONFIG REQUIRED HINTS /opt/mango/lib/cmake)
target_link_libraries(hhal PRIVATE cuda_manager)

install(TARGETS hhal EXPORT hhalConfig LIBRARY DESTINATION ${LIB_DIR})

install(FILES ${HEADERS} DESTINATION ${INCLUDE_DIR})

install(
    EXPORT hhalConfig 
    NAMESPACE hhal::
    DESTINATION ${EXPORT_DIR}
)

add_subdirectory(gn)
add_subdirectory(nvidia)
