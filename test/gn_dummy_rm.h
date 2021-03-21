#ifndef GN_DUMMY_RM_H
#define GN_DUMMY_RM_H

#include <vector>

#include "mango_arguments.h"
#include "hhal.h"

namespace gn_rm {

struct registered_kernel {
    mango_kernel k;
    int kernel_termination_event;
};

struct registered_buffer {
    mango_buffer b;
    int event;
};

template<class H>
void resource_allocation(
    H &hhal, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<registered_buffer> &buffers, 
    const std::vector<mango_event> &events
);

template<class H>
void resource_deallocation(
    H &hhal, 
    const std::vector<mango_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
);

registered_kernel register_kernel(mango_kernel kernel);
registered_buffer register_buffer(mango_buffer buffer);
int get_new_event_id();
}

#endif