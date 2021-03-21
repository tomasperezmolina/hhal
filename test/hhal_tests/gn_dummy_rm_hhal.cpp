#include "gn_dummy_rm.cpp"

namespace gn_rm {

template void resource_allocation<hhal::HHAL>(
    hhal::HHAL &hhal, 
    const std::vector<registered_kernel> &kernels, 
    const std::vector<registered_buffer> &buffers, 
    const std::vector<mango_event> &events
);

template void resource_deallocation<hhal::HHAL>(
    hhal::HHAL &hhal, 
    const std::vector<mango_kernel> &kernels, 
    const std::vector<mango_buffer> &buffers, 
    const std::vector<mango_event> &events
);

}