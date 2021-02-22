#ifndef EVENT_UTILS_H
#define EVENT_UTILS_H

#include <cinttypes>
#include "hhal.h"


namespace hhal {
    namespace events{
        void write(HHAL &hhal, int event_id, uint32_t value);
        uint32_t read(HHAL &hhal, int event_id);
        uint32_t lock(HHAL &hhal, int event_id);
        void wait(HHAL &hhal, int event_id, uint32_t state);
    }
}

#endif //EVENT_UTILS_H