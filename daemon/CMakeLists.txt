cmake_minimum_required (VERSION 2.8.11)

set(MANGO_ROOT /opt/mango)

project(daemon LANGUAGES CXX)

set(MANGO_ROOT /opt/mango)
set(CLIENT_LIB_DIR ${MANGO_ROOT}/lib/hhal_client)
set(CLIENT_INCLUDE_DIR ${MANGO_ROOT}/include/hhal_client)
set(CLIENT_EXPORT_DIR ${MANGO_ROOT}/lib/cmake/hhal_client)
set(DAEMON_BIN_DIR ${MANGO_ROOT}/bin)
set(CONFIG_DIR ${MANGO_ROOT}/etc)

set(DAEMON_CONFIG_FILENAME hhal_daemon.conf)
set(DAEMON_CONFIG_INSTALL_PATH "${CONFIG_DIR}/${DAEMON_CONFIG_FILENAME}")

add_definitions(-DDAEMON_CONFIG="${DAEMON_CONFIG_INSTALL_PATH}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose debug or release" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set (CMAKE_CXX_STANDARD 14)

set (PUBLIC_HEADERS
    hhal_client.h
)

set(INCLUDE_DIRS 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/external
    ${PROJECT_SOURCE_DIR}/base
    ${PROJECT_SOURCE_DIR}/..
)

set(SERVER_SOURCES
    base/server/server.cpp
    utils/config_reader.cpp
    utils/logger.cpp
    hhal_server.cpp
    run_daemon.cpp
    serialization.cpp
)

set(CLIENT_SOURCES
    base/client/socket_client.cpp
    serialization.cpp
    hhal_client.cpp
)

add_library(hhal_client SHARED ${CLIENT_SOURCES})

# Specify rpath to avoid having to add the path to LD_LIBRARY_PATH
# Necessary because hhal is in a non-standard location
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}:${MANGO_ROOT}/lib/hhal)
add_executable(hhal_daemon ${SERVER_SOURCES})

target_link_libraries(hhal_daemon PRIVATE hhal spdlog::spdlog)
target_link_libraries(hhal_client PUBLIC hhal)

target_include_directories(hhal_daemon PRIVATE ${INCLUDE_DIRS})
target_include_directories(hhal_client PUBLIC 
    "$<BUILD_INTERFACE:${INCLUDE_DIRS}>"
    $<INSTALL_INTERFACE:${CLIENT_INCLUDE_DIR}>
)

configure_file("${DAEMON_CONFIG_FILENAME}.in" ${DAEMON_CONFIG_FILENAME} COPYONLY)

install(FILES "${PROJECT_BINARY_DIR}/${DAEMON_CONFIG_FILENAME}" DESTINATION ${CONFIG_DIR})

install(TARGETS hhal_daemon RUNTIME DESTINATION ${DAEMON_BIN_DIR})

install(TARGETS hhal_client EXPORT hhal_clientConfig LIBRARY DESTINATION ${CLIENT_LIB_DIR})

install(FILES ${PUBLIC_HEADERS} DESTINATION ${CLIENT_INCLUDE_DIR})

install(
    EXPORT hhal_clientConfig 
    NAMESPACE hhal_client::
    DESTINATION ${CLIENT_EXPORT_DIR}
)

add_subdirectory(external/spdlog)

# Test
add_executable(serialization_test test/serialization_test.cpp)
target_include_directories(serialization_test PRIVATE ${INCLUDE_DIRS})
target_link_libraries(serialization_test hhal_client)
