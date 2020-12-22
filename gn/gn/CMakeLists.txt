cmake_minimum_required (VERSION 2.8.11)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(MANGO_ROOT "/opt/mango" CACHE STRING "MANGO installation root directory")
#message(STATUS "MANGO_ROOT=" ${MANGO_ROOT})

set(CMAKE_COMMON_FLAGS "-DMANGO_ROOT=\\\"${MANGO_ROOT}\\\" -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_COMMON_FLAGS} -Wall --pedantic -std=c++14 -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_COMMON_FLAGS} -Wall --pedantic -Wno-deprecated-declarations")
# Set DEBUG compilation
set(CMAKE_BUILD_TYPE Debug)

# List of source files of HN library
set(LIBMANGO_HHAL_GN_SOURCES    gn.cpp
                                gn_return.c
                                hnemu/hnemu.cpp
                                hnemu/logger.cpp)

add_library (mango_hhal_gn SHARED ${LIBMANGO_HHAL_GN_SOURCES})


target_include_directories (mango_hhal_gn PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# add_executable(gn_return gn_return.c)
# set_target_properties(gn_return PROPERTIES COMPILE_FLAGS ${CMAKE_C_FLAGS})

# install(
#         FILES "${CMAKE_CURRENT_SOURCE_DIR}/config.xml"
#         DESTINATION "usr/local/share/"
# )
# install (TARGETS gn_return DESTINATION bin)
