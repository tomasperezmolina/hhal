add_custom_target (nvidia_saxpy
    COMMAND ${MANGO_ROOT}/usr/bin/cuda_compiler/cuda_compiler_tool ${CMAKE_CURRENT_SOURCE_DIR}/saxpy.cu
    COMMENT "Generating saxpy..."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BUILD_DIR}"
)

add_custom_target (nvidia_saxpy_1
    COMMAND ${MANGO_ROOT}/usr/bin/cuda_compiler/cuda_compiler_tool ${CMAKE_CURRENT_SOURCE_DIR}/saxpy_1.cu
    COMMENT "Generating saxpy_1..."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BUILD_DIR}"
)

add_custom_target (nvidia_saxpy_2
    COMMAND ${MANGO_ROOT}/usr/bin/cuda_compiler/cuda_compiler_tool ${CMAKE_CURRENT_SOURCE_DIR}/saxpy_2.cu
    COMMENT "Generating saxpy_2..."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BUILD_DIR}"
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/saxpy.cu
	DESTINATION ./
	FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
