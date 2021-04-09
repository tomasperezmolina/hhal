#ifndef NVIDIA_TYPES_H
#define NVIDIA_TYPES_H

namespace hhal {

typedef struct nvidia_kernel_t {
    int id;
    int gpu_id;
    int mem_id;
    uint32_t grid_dim_x;
    uint32_t grid_dim_y;
    uint32_t grid_dim_z;
    uint32_t block_dim_x;
    uint32_t block_dim_y;
    uint32_t block_dim_z;
    int termination_event;
} nvidia_kernel;

typedef struct nvidia_buffer_t {
    int id;
    int gpu_id;
    int mem_id;
    size_t size;
    std::vector<int> kernels_in;    // Id of kernels which write into the buffer
    std::vector<int> kernels_out;   // Id of kernels which read from the buffer
} nvidia_buffer;

typedef struct nvidia_event_t {
    int id;
} nvidia_event;

}

#endif
