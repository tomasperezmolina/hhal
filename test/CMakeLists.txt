cmake_minimum_required (VERSION 2.8.11)

project(hhal_test LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    message(STATUS "Has a parent scope.")
else(HAS_PARENT)
    set(MANGO_ROOT /opt/mango)
    set(BIN_DIR ${MANGO_ROOT}/bin/hhal_test)

    find_package(hhal CONFIG REQUIRED HINTS /opt/mango/lib/cmake)
    find_package(cuda_manager CONFIG REQUIRED HINTS /opt/mango/lib/cmake)
endif(HAS_PARENT)

add_executable(nvidia_launch_kernel 
    event_utils.cpp 
    nvidia_launch_kernel.cpp
)

add_executable(gn_launch_kernel 
    event_utils.cpp 
    gn_launch_kernel.cpp
)


target_include_directories(nvidia_launch_kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(gn_launch_kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(HAS_PARENT)
    target_link_libraries(gn_launch_kernel PRIVATE hhal)

    target_link_libraries(nvidia_launch_kernel PRIVATE hhal)
    configure_file(saxpy saxpy COPYONLY)
else(HAS_PARENT)
    target_link_libraries(nvidia_launch_kernel PRIVATE cuda_manager)
    target_link_libraries(gn_launch_kernel PRIVATE hhal::hhal)

    target_link_libraries(nvidia_launch_kernel PRIVATE cuda_manager)
    target_link_libraries(nvidia_launch_kernel PRIVATE hhal::hhal)
    configure_file(saxpy saxpy COPYONLY)
endif(HAS_PARENT)

add_subdirectory(kernel)
