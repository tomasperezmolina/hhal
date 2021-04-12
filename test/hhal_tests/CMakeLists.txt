add_executable(nvidia_launch_kernel 
    event_utils_hhal.cpp 
    nvidia_dummy_rm_hhal.cpp
    ../rm_common.cpp
    nvidia_launch_kernel.cpp
)

add_executable(gn_launch_kernel 
    event_utils_hhal.cpp 
    gn_dummy_rm_hhal.cpp
    ../rm_common.cpp
    gn_launch_kernel.cpp
)

add_executable(gn_serial_saxpy 
    event_utils_hhal.cpp 
    gn_dummy_rm_hhal.cpp
    ../rm_common.cpp
    gn_serial_saxpy.cpp
)

add_executable(nvidia_multiple_kernels 
    event_utils_hhal.cpp 
    nvidia_dummy_rm_hhal.cpp
    ../rm_common.cpp
    nvidia_multiple_kernels.cpp
)

add_executable(gn_gif_animation
    event_utils_hhal.cpp 
    gn_dummy_rm_hhal.cpp
    ../rm_common.cpp
    ../AnimatedGifSaver.cpp
    gn_gif_animation.cpp
)

add_executable(gn_nvidia_saxpy
    event_utils_hhal.cpp 
    gn_dummy_rm_hhal.cpp
    nvidia_dummy_rm_hhal.cpp
    ../rm_common.cpp
    gn_nvidia_saxpy.cpp
)

add_executable(gn_serial_saxpy_bin_source
    event_utils_hhal.cpp 
    gn_dummy_rm_hhal.cpp
    ../rm_common.cpp
    gn_serial_saxpy_bin_source.cpp
)

add_executable(gn_serial_saxpy_bin_string
    event_utils_hhal.cpp 
    gn_dummy_rm_hhal.cpp
    ../rm_common.cpp
    gn_serial_saxpy_bin_string.cpp
)

add_executable(nvidia_launch_kernel_source
    event_utils_hhal.cpp 
    nvidia_dummy_rm_hhal.cpp
    ../rm_common.cpp
    nvidia_launch_kernel_source.cpp
)

target_include_directories(gn_launch_kernel PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(nvidia_launch_kernel PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(gn_serial_saxpy PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(nvidia_multiple_kernels PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(gn_nvidia_saxpy PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(gn_gif_animation PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(gn_serial_saxpy_bin_source PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(gn_serial_saxpy_bin_string PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(nvidia_launch_kernel_source PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(gn_launch_kernel PRIVATE hhal::hhal)
target_link_libraries(nvidia_launch_kernel PRIVATE hhal::hhal)
target_link_libraries(gn_serial_saxpy PRIVATE hhal::hhal)
target_link_libraries(nvidia_multiple_kernels PRIVATE hhal::hhal)
target_link_libraries(gn_nvidia_saxpy PRIVATE hhal::hhal)
target_link_libraries(gn_gif_animation PRIVATE hhal::hhal ${GIF_LIB})
target_link_libraries(gn_serial_saxpy_bin_source PRIVATE hhal::hhal)
target_link_libraries(gn_serial_saxpy_bin_string PRIVATE hhal::hhal)
target_link_libraries(nvidia_launch_kernel_source PRIVATE hhal::hhal)

add_dependencies(gn_launch_kernel matrix_multiplication_dev)
add_dependencies(nvidia_launch_kernel nvidia_saxpy)
add_dependencies(gn_serial_saxpy gn_saxpy_1 gn_saxpy_2)
add_dependencies(nvidia_multiple_kernels nvidia_saxpy_1 nvidia_saxpy_2)
add_dependencies(gn_nvidia_saxpy gn_saxpy_1 nvidia_saxpy_2)
add_dependencies(gn_gif_animation copy_kernel smooth_kernel scale_kernel)
add_dependencies(gn_serial_saxpy_bin_source gn_saxpy_1)
add_dependencies(gn_serial_saxpy_bin_string gn_saxpy_1)
