project(gif_copy_kernel LANGUAGES C CXX)

set(Main_File main.c)

set_source_files_properties(${Main_File}
	PROPERTIES GENERATED TRUE)

add_custom_target (copy_kernel_main_gen
	COMMAND ${MANGO_ROOT}/usr/bin/mango_gen_kernel_entry.py GN ${CMAKE_CURRENT_SOURCE_DIR}/copy_kernel.c
	COMMENT "Generating main.c..."
	WORKING_DIRECTORY "${CMAKE_CURRENT_BUILD_DIR}"
)

add_executable(copy_kernel copy_kernel.c ${Main_File})
find_library(MANGODEV mango-dev-gn PATHS ${MANGO_ROOT}/usr/lib)
find_package(Threads)
target_link_libraries(copy_kernel ${MANGODEV} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(copy_kernel PUBLIC ${MANGO_ROOT}/usr/include/libmango/)

add_dependencies (copy_kernel copy_kernel_main_gen)
