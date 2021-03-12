#include "nvidia_dummy_rm.cpp"

namespace nvidia_rm {

template void resource_allocation<hhal::HHAL>(
    hhal::HHAL &hhal, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
);

template void resource_deallocation<hhal::HHAL>(
    hhal::HHAL &hhal, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
);

}