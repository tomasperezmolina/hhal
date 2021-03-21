#include <map>
#include <vector>
#include <cstdio>
#include <exception>

#include "arguments.h"

#include "mango_arguments.h"
#include "gn_dummy_rm.h"
#include "rm_common.h"

using namespace hhal;

#define GN_DEFAULT_CLUSTER 0

namespace gn_rm {

template <class H>
void resource_allocation(
    H &hhal,
    const std::vector<registered_kernel> &kernels, 
    const std::vector<registered_buffer> &buffers, 
    const std::vector<mango_event> &events
) {
	printf("[DummyRM] resource_allocation\n");

    for (auto &k : kernels) {
        gn_kernel kernel_info;
        kernel_info.id = k.k.id;
        kernel_info.size = k.k.image_size;
        kernel_info.task_events = k.task_events;
        kernel_info.termination_event = k.kernel_termination_event;

        hhal.assign_kernel(hhal::Unit::GN, (hhal_kernel *) &kernel_info);
        hhal.allocate_kernel(kernel_info.id);
    }

    printf("[DummyRM] resource_allocation: %zu tiles reserved\n", kernels.size());

	for(auto &et : events) {
        gn_event info;
        info.id = et.id;
        info.kernels_in = et.kernels_in;
        info.kernels_out = et.kernels_out;

        hhal.assign_event(hhal::Unit::GN, (hhal_event *) &info);
        hhal.allocate_event(info.id);
	}

	for(auto &bt : buffers) {
        gn_buffer info;
        info.id = bt.b.id;
        info.size = bt.b.size;
        info.event = bt.event;
        info.kernels_in = bt.b.kernels_in;
        info.kernels_out = bt.b.kernels_out;
        
        hhal.assign_buffer(hhal::Unit::GN, (hhal_buffer *) &info);
        hhal.allocate_memory(info.id);
	}
}

template <class H>
void resource_deallocation(
    H &hhal, 
    const std::vector<mango_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
) {
    for (auto &k : kernels) {
        hhal.release_kernel(k.id);
    }

    for(auto &et : events) {
        hhal.release_event(et.id);
    }

    for(auto &bt : buffers) {
        hhal.release_memory(bt.id);
    }

}

int get_new_event_id() {
    return rm_common::get_new_event_id();
}

registered_kernel register_kernel(mango_kernel kernel) {
    return {
        kernel,
        rm_common::get_new_event_id(),
        {
            rm_common::get_new_event_id(), // Task event
            rm_common::get_new_event_id(), // Barrier event
            rm_common::get_new_event_id()  // Release event
        },
    };
}

registered_buffer register_buffer(mango_buffer buffer) {
    return {
        buffer,
        rm_common::get_new_event_id(),
    };
};

}
