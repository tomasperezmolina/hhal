#ifndef HHAL_H
#define HHAL_H

#include <map>

#include "arguments.h"
#include "types.h"

#include "gn/types.h"
#include "gn/manager.h"

#include "hn/types.h"
#include "hn/manager.h"

#include "nvidia/types.h"
#include "nvidia/manager.h"

namespace hhal {

enum class HHALExitCode {
    OK,
    ERROR,
};

class HHAL {
    public:
        HHAL();

        HHALExitCode assign_kernel(Unit unit, hhal_kernel *info);
        HHALExitCode assign_buffer(Unit unit, hhal_buffer *info);
        HHALExitCode assign_event (Unit unit, hhal_event *info);

        HHALExitCode kernel_write(int kernel_id, std::string image_path);

        HHALExitCode kernel_start(int kernel_id, const Arguments &arguments);

        HHALExitCode allocate_memory(int buffer_id);
        HHALExitCode release_memory(int buffer_id);
        HHALExitCode allocate_kernel(int kernel_id);
        HHALExitCode release_kernel(int kernel_id);
        HHALExitCode write_to_memory(int buffer_id, const void *source, size_t size);
        HHALExitCode read_from_memory(int buffer_id, void *dest, size_t size);
        HHALExitCode write_sync_register(int event_id, uint8_t data);
        HHALExitCode read_sync_register(int event_id, uint8_t *data);

        GNManager gn_manager;

    private:
        std::map<int, Unit> kernel_to_unit;
        std::map<int, Unit> buffer_to_unit;
        std::map<int, Unit> event_to_unit;

        HNManager hn_manager;
        NvidiaManager nvidia_manager;
};

}

#endif //HHAL_H
