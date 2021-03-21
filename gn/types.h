#ifndef GN_TYPES_H
#define GN_TYPES_H

#include <vector>
#include <string>
#include <cinttypes>

namespace hhal {

struct gn_kernel {
    int id;
    int termination_event;
};

struct gn_buffer {
    int id;
    size_t size;
    int event;
    std::vector<int> kernels_in;
    std::vector<int> kernels_out;
};

struct gn_event {
    int id;
    std::vector<int> kernels_in;
    std::vector<int> kernels_out;
};

}

#endif