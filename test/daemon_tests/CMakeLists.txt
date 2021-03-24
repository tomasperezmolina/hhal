set(DAEMON_PATH "/tmp/mango_hhal_daemon")

add_definitions(-DDAEMON_PATH="${DAEMON_PATH}")

add_executable(daemon_gn_launch_kernel 
    event_utils_hhal_client.cpp 
    gn_dummy_rm_hhal_client.cpp
    ../rm_common.cpp
    gn_launch_kernel.cpp
)

add_executable(daemon_nvidia_launch_kernel 
    event_utils_hhal_client.cpp 
    nvidia_dummy_rm_hhal_client.cpp
    ../rm_common.cpp
    nvidia_launch_kernel.cpp
)

add_executable(daemon_gn_serial_saxpy 
    event_utils_hhal_client.cpp 
    gn_dummy_rm_hhal_client.cpp
    ../rm_common.cpp
    gn_serial_saxpy.cpp
)

add_executable(daemon_nvidia_multiple_kernels 
    event_utils_hhal_client.cpp 
    nvidia_dummy_rm_hhal_client.cpp
    ../rm_common.cpp
    nvidia_multiple_kernels.cpp
)

add_executable(daemon_gn_gif_animation
    event_utils_hhal_client.cpp 
    gn_dummy_rm_hhal_client.cpp
    ../rm_common.cpp
    ../AnimatedGifSaver.cpp
    gn_gif_animation.cpp
)

add_executable(daemon_gn_nvidia_saxpy
    event_utils_hhal_client.cpp 
    gn_dummy_rm_hhal_client.cpp
    nvidia_dummy_rm_hhal_client.cpp
    ../rm_common.cpp
    gn_nvidia_saxpy.cpp
)

target_include_directories(daemon_gn_launch_kernel PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(daemon_nvidia_launch_kernel PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(daemon_gn_serial_saxpy PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(daemon_nvidia_multiple_kernels PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(daemon_gn_nvidia_saxpy PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(daemon_gn_gif_animation PRIVATE ${PROJECT_SOURCE_DIR})

target_link_libraries(daemon_gn_launch_kernel PRIVATE hhal_client::hhal_client)
target_link_libraries(daemon_nvidia_launch_kernel PRIVATE hhal_client::hhal_client)
target_link_libraries(daemon_gn_serial_saxpy PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(daemon_nvidia_multiple_kernels PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(daemon_gn_nvidia_saxpy PRIVATE hhal::hhal hhal_client::hhal_client)
target_link_libraries(daemon_gn_gif_animation PRIVATE hhal::hhal ${GIF_LIB} hhal_client::hhal_client)

add_dependencies(daemon_gn_launch_kernel matrix_multiplication_dev)
add_dependencies(daemon_nvidia_launch_kernel nvidia_saxpy)
add_dependencies(daemon_gn_serial_saxpy gn_saxpy_1 gn_saxpy_2)
add_dependencies(daemon_nvidia_multiple_kernels nvidia_saxpy_1 nvidia_saxpy_2)
add_dependencies(daemon_gn_nvidia_saxpy gn_saxpy_1 nvidia_saxpy_2)
add_dependencies(daemon_gn_gif_animation copy_kernel smooth_kernel scale_kernel)