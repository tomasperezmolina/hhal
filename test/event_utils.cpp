#include "event_utils.h"
#include "hhal_client.h"

namespace hhal { namespace events {

bool is_exit_code_OK(HHALExitCode ec) {
    return ec == HHALExitCode::OK;
}

bool is_exit_code_OK(hhal_daemon::HHALClientExitCode ec) {
    return ec == hhal_daemon::HHALClientExitCode::OK;
}

template<class H>
void write(H &hhal, int event_id, uint32_t value) {
    if (is_exit_code_OK(hhal.write_sync_register(event_id, value))) {
        printf("Writing to sync register failed.\n");
    }
}

template void write<HHAL>(HHAL &hhal, int event_id, uint32_t value);
template void write<hhal_daemon::HHALClient>(hhal_daemon::HHALClient &hhal, int event_id, uint32_t value);

template<class H>
uint32_t read(H &hhal, int event_id) {
    uint32_t value;
    if (is_exit_code_OK(hhal.read_sync_register(event_id, &value))) {
        printf("Read synch register failed!\n");
    }
    return value;
}

template uint32_t read<HHAL>(HHAL &hhal, int event_id);
template uint32_t read<hhal_daemon::HHALClient>(hhal_daemon::HHALClient &hhal, int event_id);

template<class H>
uint32_t lock(H &hhal, int event_id) {
    uint32_t value;
    do {
        value = read(hhal, event_id);
    } while (value == 0);
    return value;
}

template uint32_t lock<HHAL>(HHAL &hhal, int event_id);
template uint32_t lock<hhal_daemon::HHALClient>(hhal_daemon::HHALClient &hhal, int event_id);

template<class H>
void wait(H &hhal, int event_id, uint32_t state) {
    uint32_t value;
    do {
        value = lock(hhal, event_id);

        if (value != state) {
            // printf("Expected %d, instead it's %d\n", state, value);
            //Rewrite the value so it is not lost
            write(hhal, event_id, value);
        }

    } while (value != state);
}

template void wait<HHAL>(HHAL &hhal, int event_id, uint32_t state);
template void wait<hhal_daemon::HHALClient>(hhal_daemon::HHALClient &hhal, int event_id, uint32_t state);

}}
