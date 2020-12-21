#include <map>

#include "gn/types.h"
#include "gn/manager.h"

#include "hn/types.h"
#include "hn/manager.h"

#include "nvidia/types.h"
#include "nvidia/manager.h"

namespace hhal {

enum class Unit {
    GN,
    HN,
    NVIDIA
};

class HHAL {
    public:
        void assign_kernel_to_gn(gn_kernel info);
        void assign_kernel_to_hn(hn_kernel info);
        void assign_kernel_to_nvidia(nvidia_kernel info);

        void assign_buffer_to_gn(gn_buffer info);
        void assign_buffer_to_hn(hn_buffer info);
        void assign_buffer_to_nvidia(nvidia_buffer info);

        void assign_event_to_gn(gn_event info);
        void assign_event_to_hn(hn_event info);
        void assign_event_to_nvidia(nvidia_event info);

        void kernel_write(int kernel_id, std::string image_path);
        void kernel_start(int kernel_id, std::string arguments);
        void allocate_memory(int buffer_id);
        void release_memory(int buffer_id);
        void allocate_kernel(int kernel_id);
        void release_kernel(int kernel_id);
        void write_to_memory(int buffer_id, const void *source, size_t size);
        void read_from_memory(int buffer_id, void *dest, size_t size);
        void write_sync_register(int event_id, uint8_t data);
        void read_sync_register(int event_id, uint8_t *data);

    private:
        std::map<int, Unit> kernel_to_unit;
        std::map<int, Unit> buffer_to_unit;
        std::map<int, Unit> event_to_unit;

        GNManager gn_manager;
        HNManager hn_manager;
        NvidiaManager nvidia_manager;
};

}