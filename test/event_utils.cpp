#include "event_utils.h"

namespace hhal { namespace events {

void write(HHAL &hhal, int event_id, uint32_t value) {
    if (hhal.write_sync_register(event_id, value) != HHALExitCode::OK) {
        printf("Writing to sync register failed.\n");
    }
}

uint32_t read(HHAL &hhal, int event_id) {
    uint32_t value;
    if (hhal.read_sync_register(event_id, &value) != HHALExitCode::OK) {
        printf("Read synch register failed!\n");
    }
    return value;
}


uint32_t lock(HHAL &hhal, int event_id) {
    uint32_t value;
    do {
        value = read(hhal, event_id);
    } while (value == 0);
    return value;
}


void wait(HHAL &hhal, int event_id, uint32_t state) {
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
