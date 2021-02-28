add_custom_target (saxpy
    COMMAND ${MANGO_ROOT}/cuda_compiler/cuda_compiler ${CMAKE_CURRENT_SOURCE_DIR}/saxpy.cu
    COMMENT "Generating saxpy..."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BUILD_DIR}"
)

add_custom_target (saxpy_1
    COMMAND ${MANGO_ROOT}/cuda_compiler/cuda_compiler ${CMAKE_CURRENT_SOURCE_DIR}/saxpy_1.cu
    COMMENT "Generating saxpy_1..."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BUILD_DIR}"
)

add_custom_target (saxpy_2
    COMMAND ${MANGO_ROOT}/cuda_compiler/cuda_compiler ${CMAKE_CURRENT_SOURCE_DIR}/saxpy_2.cu
    COMMENT "Generating saxpy_2..."
    WORKING_DIRECTORY "${CMAKE_CURRENT_BUILD_DIR}"
)

add_dependencies(nvidia_launch_kernel saxpy)
add_dependencies(nvidia_multiple_kernels saxpy_1)
add_dependencies(nvidia_multiple_kernels saxpy_2)



