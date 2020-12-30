#ifndef NVIDIA_TYPES_H
#define NVIDIA_TYPES_H

namespace hhal {

typedef struct nvidia_kernel_args_t {
    char *function_name;
    //int threads;
    //int blocks;
    int arg_count;
    char *arg_array;
} nvidia_kernel_args;

typedef struct nvidia_kernel_t {
    int id;
    int gpu_id;
    int mem_id;
    size_t size;
} nvidia_kernel;

typedef struct nvidia_buffer_t {
    int id;
    int gpu_id;
    int mem_id;
    size_t size;
} nvidia_buffer;

typedef struct nvidia_event_t {
    int id;
} nvidia_event;

}

#endif
