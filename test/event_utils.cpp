#include "event_utils.h"

namespace hhal { namespace events {

template<class EC>
bool is_exit_code_OK(EC ec);

template<class H>
void write(H &hhal, int event_id, uint32_t value) {
    if (!is_exit_code_OK(hhal.write_sync_register(event_id, value))) {
        printf("Writing to sync register failed.\n");
    }
}


template<class H>
uint32_t read(H &hhal, int event_id) {
    uint32_t value;
    if (!is_exit_code_OK(hhal.read_sync_register(event_id, &value))) {
        printf("Read synch register failed!\n");
    }
    return value;
}

template<class H>
uint32_t lock(H &hhal, int event_id) {
    uint32_t value;
    do {
        value = read(hhal, event_id);
    } while (value == 0);
    return value;
}

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

}}
