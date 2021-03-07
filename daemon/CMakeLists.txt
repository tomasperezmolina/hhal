cmake_minimum_required (VERSION 2.8.11)

set(MANGO_ROOT /opt/mango)

project(daemon LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose debug or release" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Specify rpath to avoid having to add the path to LD_LIBRARY_PATH
# Necessary because cuda_manager is in a non-standard location
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH}:${MANGO_ROOT}/lib/hhal)

set (CMAKE_CXX_STANDARD 14)

set(INCLUDE_DIRS 
  ${PROJECT_SOURCE_DIR}
  external
  base
  /opt/mango/include/hhal
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
)

find_package(hhal CONFIG REQUIRED HINTS /opt/mango/lib/cmake)

add_executable(hhal_daemon ${SERVER_SOURCES})
add_library(hhal_client ${CLIENT_SOURCES})

target_link_libraries(hhal_daemon PRIVATE hhal::hhal spdlog::spdlog)
target_link_libraries(hhal_client PUBLIC hhal::hhal)

target_include_directories(hhal_daemon PRIVATE ${INCLUDE_DIRS})
target_include_directories(hhal_client PUBLIC ${INCLUDE_DIRS})

configure_file(daemon.conf.in daemon.conf COPYONLY)

install(TARGETS hhal_daemon 
  RUNTIME DESTINATION ${MANGO_ROOT}/bin)

add_subdirectory(external/spdlog)
