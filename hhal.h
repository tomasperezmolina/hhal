#ifndef HHAL_H
#define HHAL_H

#include <map>

#include "arguments.h"
#include "types.h"

#include "gn/types.h"
#include "nvidia/types.h"

namespace hhal {

enum class HHALExitCode {
    OK,
    ERROR,
};

class HHAL {
    public:
        HHAL();
        ~HHAL();

        // Kernel execution
        HHALExitCode kernel_write(int kernel_id, const std::map<Unit, hhal_kernel_source> &kernel_sources);
        HHALExitCode kernel_start(int kernel_id, const Arguments &arguments);

        HHALExitCode write_to_memory(int buffer_id, const void *source, size_t size);
        HHALExitCode read_from_memory(int buffer_id, void *dest, size_t size);

        HHALExitCode write_sync_register(int event_id, uint32_t data);
        HHALExitCode read_sync_register(int event_id, uint32_t *data);
        // -----------------------

        // Resource management
        HHALExitCode assign_kernel(Unit unit, hhal_kernel *info);
        HHALExitCode assign_buffer(Unit unit, hhal_buffer *info);
        HHALExitCode assign_event (Unit unit, hhal_event *info);

        HHALExitCode deassign_kernel(int kernel_id);
        HHALExitCode deassign_buffer(int buffer_id);
        HHALExitCode deassign_event(int event_id);

        HHALExitCode allocate_memory(int buffer_id);
        HHALExitCode release_memory(int buffer_id);

        HHALExitCode allocate_kernel(int kernel_id);
        HHALExitCode release_kernel(int kernel_id);

        HHALExitCode allocate_event(int event_id);
        HHALExitCode release_event(int event_id);
        // -----------------------

    private:
        class Impl;

        // Pointer to forward declared class to avoid including implementation dependent headers
        Impl *impl;

        std::map<int, Unit> kernel_to_unit;
        std::map<int, Unit> buffer_to_unit;
        std::map<int, Unit> event_to_unit;
};

}

#endif //HHAL_H
