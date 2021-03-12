#include <map>
#include <vector>
#include <cstdio>
#include <exception>

#include "hhal.h"

#include "arguments.h"

#include "mango_arguments.h"
#include "nvidia_dummy_rm.h"
#include "rm_common.h"

using namespace hhal;

namespace nvidia_rm {

// This should be handled in the RM side, it lives here for now as there are not two libraries at the moment.
// In the future there would be a library for RM which handles this,
// and a library for kernel execution which handles whats in the main function.
template<class H>
void resource_allocation(
    H &hhal, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events) 
{
    for (auto &kernel: kernels) {
        printf("[Nvidia_Dummy] Allocating kernel %d\n", kernel.k.id);
        nvidia_kernel k;
        k.id = kernel.k.id;
        k.gpu_id = 0;
        k.mem_id = kernel.k.id;
        k.size = kernel.k.image_size;
        k.termination_event = kernel.kernel_termination_event;
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

template<class H>
void resource_deallocation(
    H &hhal, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events) 
{
    for (auto &k : kernels) {
        printf("[Nvidia_Dummy] Releasing %d\n", k.k.id);
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
        rm_common::get_new_event_id(),
    };
}

}