#ifndef GN_DUMMY_RM_H
#define GN_DUMMY_RM_H

#include <vector>

#include "hhal.h"

#include "mango_arguments.h"

namespace gn_rm {

typedef struct registered_kernel_t {
    mango_kernel k;
    int kernel_termination_event;
    std::vector<int> task_events;
} registered_kernel;

typedef struct registered_buffer_t {
    mango_buffer b;
    int event;
} registered_buffer;

void resource_allocation(
    hhal::HHAL &hhal, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<registered_buffer> &buffers, 
    const std::vector<mango_event> &events
);
void resource_deallocation(
    hhal::HHAL &hhal, 
    const std::vector<mango_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
);
registered_kernel register_kernel(mango_kernel kernel);
registered_buffer register_buffer(mango_buffer buffer);
int get_new_event_id();
}

#endif