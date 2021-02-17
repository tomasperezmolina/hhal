#ifndef MANGO_ARGUMENTS_H
#define MANGO_ARGUMENTS_H

#include <vector>

// High level structs not tied to any architecture which would be used by libmango.

typedef struct kernel {
    int id;
    size_t image_size;
} mango_kernel;

typedef struct buffer {
    int id;
    size_t size;
    std::vector<int> kernels_in;
    std::vector<int> kernels_out;
} mango_buffer;

typedef struct event {
    int id;
    std::vector<int> kernels_in;
    std::vector<int> kernels_out;
} mango_event;

#endif // MANGO_ARGUMENTS_H