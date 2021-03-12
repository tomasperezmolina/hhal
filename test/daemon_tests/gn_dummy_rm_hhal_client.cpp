#include "gn_dummy_rm.cpp"
#include "hhal_client.h"

namespace gn_rm {

using namespace hhal_daemon;

template void resource_allocation<HHALClient>(
    HHALClient &hhal, 
    GNManager &gn_manager, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<registered_buffer> &buffers, 
    const std::vector<mango_event> &events
);

template void resource_deallocation<HHALClient>(
    HHALClient &hhal, 
    GNManager &gn_manager, 
    const std::vector<mango_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
);

}