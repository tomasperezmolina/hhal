#ifndef EVENT_REGISTRY_H
#define EVENT_REGISTRY_H

#include <mutex>
#include <cinttypes>
#include <map>

namespace hhal {

enum class EventRegistryExitCode {
    OK,
    ERROR,
};

class EventRegistry {
    public:
        EventRegistryExitCode add_event(int event_id);
        EventRegistryExitCode remove_event(int event_id);

        EventRegistryExitCode read_event(int event_id, uint8_t *data);
        EventRegistryExitCode write_event(int event_id, uint8_t data);

    private:
        std::mutex registers_mtx;
        std::map<int, uint8_t> registers;

};
}

#endif