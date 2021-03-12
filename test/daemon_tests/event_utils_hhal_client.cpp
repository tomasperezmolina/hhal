#include "hhal_client.h"

#include "event_utils.cpp"

namespace hhal { namespace events {

using namespace hhal_daemon;

template<>
bool is_exit_code_OK<HHALClientExitCode>(HHALClientExitCode ec) {
    return ec == HHALClientExitCode::OK;
}

template void write<HHALClient>(HHALClient &hhal, int event_id, uint32_t value);
template uint32_t read<HHALClient>(HHALClient &hhal, int event_id);
template uint32_t lock<HHALClient>(HHALClient &hhal, int event_id);
template void wait<HHALClient>(HHALClient &hhal, int event_id, uint32_t state);

}}