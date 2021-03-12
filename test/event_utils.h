#ifndef EVENT_UTILS_H
#define EVENT_UTILS_H

#include <cinttypes>

namespace hhal {
    namespace events{
        template<class H>
        void write(H &hhal, int event_id, uint32_t value);
        template<class H>
        uint32_t read(H &hhal, int event_id);
        template<class H>
        uint32_t lock(H &hhal, int event_id);
        template<class H>
        void wait(H &hhal, int event_id, uint32_t state);
    }
}

#endif //EVENT_UTILS_H