#ifndef NVIDIA_DUMMY_RM_H
#define NVIDIA_DUMMY_RM_H

#include <vector>

#include "hhal.h"

#include "mango_arguments.h"

namespace nvidia_rm {
struct registered_kernel {
    mango_kernel k;
    int kernel_termination_event;
};

void resource_allocation(hhal::HHAL &hhal, const std::vector<registered_kernel> &kernels, const std::vector<mango_buffer> &buffers, const std::vector<mango_event> &events);
void resource_deallocation(hhal::HHAL &hhal, const std::vector<registered_kernel> &kernels, const std::vector<mango_buffer> &buffers, const std::vector<mango_event> &events);
registered_kernel register_kernel(mango_kernel kernel);
}

#endif