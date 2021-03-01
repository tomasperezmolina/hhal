#include <map>
#include <vector>
#include <cstdio>
#include <exception>

#include "hhal.h"

#include "arguments.h"

#include "mango_arguments.h"
#include "nvidia_dummy_rm.h"

using namespace hhal;

int event_id_gen = 0;

namespace nvidia_rm {

// This should be handled in the RM side, it lives here for now as there are not two libraries at the moment.
// In the future there would be a library for RM which handles this,
// and a library for kernel execution which handles whats in the main function.
void resource_allocation(HHAL &hhal, const std::vector<registered_kernel> &kernels, const std::vector<mango_buffer> &buffers, const std::vector<mango_event> &events) {
    for (auto &kernel: kernels) {
        printf("Allocating kernel %d\n", kernel.k.id);
        nvidia_kernel k = { kernel.k.id, 0, kernel.k.id, kernel.k.image_size, "", kernel.kernel_termination_event };
        hhal.assign_kernel(hhal::Unit::NVIDIA, (hhal_kernel *) &k);
        hhal.allocate_kernel(kernel.k.id);
    }

    for (auto &buf: buffers) {
        nvidia_buffer buffer = { buf.id, 0, buf.id, buf.size, buf.kernels_in, buf.kernels_out };
        hhal.assign_buffer(hhal::Unit::NVIDIA, (hhal_buffer *) &buffer);
        hhal.allocate_memory(buffer.id);
    }

    for (auto &ev: events) {
        nvidia_event event = { ev.id };
        hhal.assign_event(hhal::Unit::NVIDIA, (hhal_event *) &event);
        hhal.allocate_event(event.id);
    }
}

void resource_deallocation(hhal::HHAL &hhal, const std::vector<registered_kernel> &kernels, const std::vector<mango_buffer> &buffers, const std::vector<mango_event> &events) {
    for (auto &k : kernels) {
        hhal.release_kernel(k.k.id);
    }
    
    for (auto &b : buffers) {
        hhal.release_memory(b.id);
    }

    for (auto &e : events) {
        hhal.release_event(e.id);
    }
}

registered_kernel register_kernel(mango_kernel kernel) {
    return {
        kernel,
        event_id_gen++,
    };
}

}