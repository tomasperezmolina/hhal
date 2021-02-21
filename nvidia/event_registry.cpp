#include <cstdio>

#include "nvidia/event_registry.h"

namespace hhal {
    EventRegistryExitCode EventRegistry::add_event(int event_id) {
        std::unique_lock<std::mutex> lck(registers_mtx);
        if (registers.find(event_id) != registers.end()) {
            printf("[Error] EventRegistry: Event %d already present\n", event_id);
            return EventRegistryExitCode::ERROR;
        }
        registers[event_id] = 0;
        return EventRegistryExitCode::OK;
    }
    EventRegistryExitCode EventRegistry::remove_event(int event_id) {
        std::unique_lock<std::mutex> lck(registers_mtx);
        if (registers.find(event_id) == registers.end()) {
            printf("[Error] EventRegistry: Event %d not present\n", event_id);
            return EventRegistryExitCode::ERROR;
        }
        registers.erase(event_id);
        return EventRegistryExitCode::OK;
    }

    EventRegistryExitCode EventRegistry::read_event(int event_id, uint32_t *data) {
        std::unique_lock<std::mutex> lck(registers_mtx);
        auto it = registers.find(event_id);
        if (it == registers.end()) {
            printf("[Error] EventRegistry: Event %d not present\n", event_id);
            return EventRegistryExitCode::ERROR;
        }
        *data = it->second;
        it->second = 0;
        return EventRegistryExitCode::OK;
    }

    EventRegistryExitCode EventRegistry::write_event(int event_id, uint32_t data) {
        std::unique_lock<std::mutex> lck(registers_mtx);
        auto it = registers.find(event_id);
        if (it == registers.end()) {
            printf("[Error] EventRegistry: Event %d not present\n", event_id);
            return EventRegistryExitCode::ERROR;
        }
        it->second = data;
        return EventRegistryExitCode::OK;
    }
}
