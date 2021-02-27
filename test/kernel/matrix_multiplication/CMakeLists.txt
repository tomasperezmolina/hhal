project(matrix_multiplication_kernel LANGUAGES C CXX)

set(Main_File main.c)

set_source_files_properties(${Main_File}
    PROPERTIES GENERATED TRUE)

add_custom_target (matrix_multiplication_main_gen
    COMMAND ${MANGO_ROOT}/usr/bin/mango_gen_kernel_entry.py GN ${CMAKE_CURRENT_SOURCE_DIR}/matrix_multiplication_dev.c
    COMMENT "Generating main.c..."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BUILD_DIR}"
)

add_executable(matrix_multiplication_dev matrix_multiplication_dev.c ${Main_File})
find_library(MANGODEV mango-dev-gn PATHS ${MANGO_ROOT}/usr/lib)
find_package(Threads)
target_link_libraries(matrix_multiplication_dev ${MANGODEV} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(matrix_multiplication_dev PUBLIC ${MANGO_ROOT}/usr/include/libmango)

add_dependencies (matrix_multiplication_dev matrix_multiplication_main_gen)