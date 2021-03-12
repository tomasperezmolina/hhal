#include "hhal.h"

#include "event_utils.cpp"

namespace hhal { namespace events {

template<>
bool is_exit_code_OK<HHALExitCode>(HHALExitCode ec) {
    return ec == HHALExitCode::OK;
}

template void write<HHAL>(HHAL &hhal, int event_id, uint32_t value);
template uint32_t read<HHAL>(HHAL &hhal, int event_id);
template uint32_t lock<HHAL>(HHAL &hhal, int event_id);
template void wait<HHAL>(HHAL &hhal, int event_id, uint32_t state);

}}