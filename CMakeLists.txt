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
set(CONFIG_DIR ${MANGO_ROOT}/etc)

set(DINAMIC_COMPILER_SOURCES
    dynamic_compiler/config_reader.cpp
    dynamic_compiler/compiler.cpp
	dynamic_compiler/mango_gen_kernel_entry.cpp
)

if (CONFIG_DYNAMIC_COMPILER_LLVM_ENABLE)
	add_definitions(-DLLVM_ENABLED)
	set(DINAMIC_COMPILER_SOURCES ${DINAMIC_COMPILER_SOURCES} dynamic_compiler/LLVMInstanceManager.cpp)
endif (CONFIG_DYNAMIC_COMPILER_LLVM_ENABLE)

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
    arguments.cpp
    hhal.cpp
)

set(SOURCES ${SOURCES} ${NVIDIA_SOURCES} ${GN_SOURCES} ${DINAMIC_COMPILER_SOURCES})

set(HEADERS 
    hhal.h 
    arguments.h 
    types.h
)

add_library(hhal SHARED ${SOURCES} ${HEADERS})

target_link_libraries(hhal PRIVATE pthread)

if (CONFIG_DYNAMIC_COMPILER_LLVM_ENABLE)
	# LLVM library
	find_package(LLVM REQUIRED CONFIG)
	message("LLVM_INCLUDE_DIRS: ${LLVM_INCLUDE_DIRS}")
	message("LLVM_LIBRARY_DIRS: ${LLVM_LIBRARY_DIRS}")

	find_library(CLANG_BASIC clangBasic HINTS ${LLVM_LIBRARY_DIRS})
	find_library(CLANG_DRIVER clangDriver HINTS ${LLVM_LIBRARY_DIRS})

	target_include_directories (hhal PUBLIC ${LLVM_INCLUDE_DIRS})

	target_link_libraries (hhal PRIVATE LLVM ${CLANG_DRIVER} ${CLANG_BASIC})
endif (CONFIG_DYNAMIC_COMPILER_LLVM_ENABLE)

target_include_directories(hhal PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${INCLUDE_DIR}>
)

find_package(cuda_compiler CONFIG REQUIRED HINTS /opt/mango/lib/cmake)
find_package(cuda_manager CONFIG REQUIRED HINTS /opt/mango/lib/cmake)
target_link_libraries(hhal PRIVATE cuda_compiler cuda_manager)

install(TARGETS hhal EXPORT hhalConfig LIBRARY DESTINATION ${LIB_DIR})

install(FILES ${HEADERS} DESTINATION ${INCLUDE_DIR})

install(
    EXPORT hhalConfig 
    NAMESPACE hhal::
    DESTINATION ${EXPORT_DIR}
)

# Dynamic compiler kernels
set (COMPILED_KERNELS_DIR $ENV{HOME}/.mango/compiled_kernels)
add_definitions(-DKERNEL_CACHE_BASE_DIR="${COMPILED_KERNELS_DIR}")
install (DIRECTORY DESTINATION ${COMPILED_KERNELS_DIR} DIRECTORY_PERMISSIONS
	OWNER_WRITE OWNER_READ OWNER_EXECUTE
	GROUP_WRITE GROUP_READ GROUP_EXECUTE
	WORLD_WRITE WORLD_READ WORLD_EXECUTE)
	
set(DYNAMIC_COMPILER_CONFIG_FILENAME dynamic_compiler.conf)
set(DYNAMIC_COMPILER_CONFIG_FILE_PATH "dynamic_compiler/${DYNAMIC_COMPILER_CONFIG_FILENAME}")
set(DYNAMIC_COMPILER_CONFIG_INSTALL_PATH "${CONFIG_DIR}/${DYNAMIC_COMPILER_CONFIG_FILENAME}")

add_definitions(-DDYNAMIC_COMPILER_CONFIG="${DYNAMIC_COMPILER_CONFIG_INSTALL_PATH}")

configure_file("${DYNAMIC_COMPILER_CONFIG_FILE_PATH}.in" ${DYNAMIC_COMPILER_CONFIG_FILE_PATH} COPYONLY)
install(FILES "${PROJECT_BINARY_DIR}/${DYNAMIC_COMPILER_CONFIG_FILE_PATH}" DESTINATION ${CONFIG_DIR})

add_subdirectory(gn)
add_subdirectory(nvidia)
add_subdirectory(daemon)