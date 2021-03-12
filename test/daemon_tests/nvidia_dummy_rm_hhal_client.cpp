#include "nvidia_dummy_rm.cpp"
#include "hhal_client.h"

namespace nvidia_rm {

using namespace hhal_daemon;

template void resource_allocation<HHALClient>(
    HHALClient &hhal, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
);

template void resource_deallocation<HHALClient>(
    HHALClient &hhal, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
);
}