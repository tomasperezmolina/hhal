#ifndef HN_MANAGER_H
#define HN_MANAGER_H

#include <map>
#include <string>
#include <cstdint>

#include "hn/types.h"

namespace hhal {

enum class HNManagerExitCode {
    OK,
    ERROR,
};

class HNManager {
    public:
        HNManagerExitCode assign_kernel(hn_kernel info);
        HNManagerExitCode assign_buffer(hn_buffer info);
        HNManagerExitCode assign_event(hn_event info);

        HNManagerExitCode kernel_write(int kernel_id, std::string image_path);
        HNManagerExitCode kernel_start(int kernel_id, std::string arguments);

        // BBQUE handles HN memory directly from now
        // HNManagerExitCode allocate_memory(int buffer_id);
        // HNManagerExitCode release_memory(int buffer_id);

        HNManagerExitCode write_to_memory(int buffer_id, const void *source, size_t size);
        HNManagerExitCode read_from_memory(int buffer_id, void *dest, size_t size);
        HNManagerExitCode write_sync_register(int event_id, uint8_t data);
        HNManagerExitCode read_sync_register(int event_id, uint8_t *data);

    private:
        std::map<int, hn_kernel> kernel_info;
        std::map<int, hn_buffer> buffer_info;
        std::map<int, hn_event> event_info;
};

}

#endif