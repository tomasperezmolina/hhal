project(saxpy2_kernel LANGUAGES C CXX)

set(Main_File main.c)

set_source_files_properties(${Main_File}
    PROPERTIES GENERATED TRUE)

add_custom_target (gn_saxpy_2_main_gen
    COMMAND ${MANGO_ROOT}/usr/bin/mango_gen_kernel_entry.py GN ${CMAKE_CURRENT_SOURCE_DIR}/saxpy2.c
    COMMENT "Generating main.c..."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BUILD_DIR}"
)

add_executable(gn_saxpy_2 saxpy2.c ${Main_File})
find_library(MANGODEV mango-dev-gn PATHS ${MANGO_ROOT}/usr/lib)
find_package(Threads)
target_link_libraries(gn_saxpy_2 ${MANGODEV} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(gn_saxpy_2 PUBLIC ${MANGO_ROOT}/usr/include/libmango/)

add_dependencies (gn_saxpy_2 gn_saxpy_2_main_gen)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/saxpy2_source.c
	DESTINATION ./
	FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
