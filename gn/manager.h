#ifndef GN_MANAGER_H
#define GN_MANAGER_H

#include <map>
#include <string>
#include <cstdint>

#include "gn/types.h"

namespace hhal {

enum class GNManagerExitCode {
    OK,
    ERROR,
};

class GNManager {
    public:
        GNManagerExitCode assign_kernel(gn_kernel info);
        GNManagerExitCode assign_buffer(gn_buffer info);
        GNManagerExitCode assign_event(gn_event info);

        GNManagerExitCode kernel_write(int kernel_id, std::string image_path);
        GNManagerExitCode kernel_start(int kernel_id, std::string arguments);

        // BBQUE handles GN memory directly from now
        // GNManagerExitCode allocate_memory(int buffer_id);
        // GNManagerExitCode release_memory(int buffer_id);

        GNManagerExitCode write_to_memory(int buffer_id, const void *source, size_t size);
        GNManagerExitCode read_from_memory(int buffer_id, void *dest, size_t size);
        GNManagerExitCode write_sync_register(int event_id, uint8_t data);
        GNManagerExitCode read_sync_register(int event_id, uint8_t *data);

    private:
        std::map<int, gn_kernel> kernel_info;
        std::map<int, gn_buffer> buffer_info;
        std::map<int, gn_event> event_info;
};

}

#endif