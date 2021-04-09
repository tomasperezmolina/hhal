#include "hhal.h"
#include "nvidia/manager.h"
#include "gn/manager.h"
#include "dynamic_compiler/compiler.h"

#define GN_MANAGER      managers->gn_manager
#define NVIDIA_MANAGER  managers->nvidia_manager

#define MAP_GN_EXIT_CODE(x)                     \
        do {                                    \
            if (x != GNManagerExitCode::OK) {   \
                return HHALExitCode::ERROR;     \
            }                                   \
            else {                              \
                return HHALExitCode::OK;        \
            }                                   \
        } while(0);

#define MAP_NVIDIA_EXIT_CODE(x)                     \
        do {                                        \
            if (x != NvidiaManagerExitCode::OK) {   \
                return HHALExitCode::ERROR;         \
            }                                       \
            else {                                  \
                return HHALExitCode::OK;            \
            }                                       \
        } while(0)

namespace hhal {

static dynamic_compiler::Compiler compiler;

class HHAL::Managers {
    public:
        NvidiaManager nvidia_manager;
        GNManager gn_manager;
};

HHAL::HHAL() {
    managers = new HHAL::Managers;
    GNManagerExitCode ec = GN_MANAGER.initialize();
    if (ec != GNManagerExitCode::OK) {
        printf("[Error] Could not initialize GNManager\n");
    }
    printf("HHAL: Initialized\n");
}

HHAL::~HHAL() {
    GNManagerExitCode ec = GN_MANAGER.finalize();
    if (ec != GNManagerExitCode::OK) {
        printf("[Error] Could not finalize GNManager properly\n");
    }
    delete managers;
    printf("HHAL: Destroyed\n");
}

HHALExitCode HHAL::assign_kernel(Unit unit, hhal_kernel *info) {
    kernel_to_unit[info->id] = unit;
    switch (unit) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.assign_kernel((gn_kernel *) info));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.assign_kernel((nvidia_kernel *)info));
            break;
        default:
            return HHALExitCode::ERROR;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::assign_buffer(Unit unit, hhal_buffer *info) {
    buffer_to_unit[info->id] = unit;
    switch (unit) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.assign_buffer((gn_buffer *) info));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.assign_buffer((nvidia_buffer *) info));
            break;
        default:
            return HHALExitCode::ERROR;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::assign_event(Unit unit, hhal_event *info) {
    event_to_unit[info->id] = unit;
    switch (unit) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.assign_event((gn_event *) info));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.assign_event((nvidia_event *) info));
            break;
        default:
            return HHALExitCode::ERROR;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::deassign_kernel(int kernel_id) {
    printf("Kernel id %d deassigned\n", kernel_id);
    Unit unit = kernel_to_unit[kernel_id];
    kernel_to_unit.erase(kernel_id);
    switch (unit) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.deassign_kernel(kernel_id));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.deassign_kernel(kernel_id));
            break;
        default:
            return HHALExitCode::ERROR;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::deassign_buffer(int buffer_id) {
    printf("Buffer id %d deassigned\n", buffer_id);
    Unit unit = buffer_to_unit[buffer_id];
    buffer_to_unit.erase(buffer_id);
    switch (unit) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.deassign_buffer(buffer_id));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.deassign_buffer(buffer_id));
            break;
        default:
            return HHALExitCode::ERROR;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::deassign_event(int event_id) {
    printf("Event id %d deassigned\n", event_id);
    Unit unit = event_to_unit[event_id];
    event_to_unit.erase(event_id);
    switch (unit) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.deassign_event(event_id));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.deassign_event(event_id));
            break;
        default:
            return HHALExitCode::ERROR;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::kernel_write(int kernel_id, const std::map<Unit, hhal_kernel_source> &kernel_sources) {
    std::string kernel_path; // path to the final kernel binary file
    std::map<Unit, hhal_kernel_source>::const_iterator it;
    switch (kernel_to_unit[kernel_id]) {
        case Unit::GN:
        {
            it = kernel_sources.find(Unit::GN);
            if (it == kernel_sources.end()) {
                printf("No kernel source for GN\n");
                return HHALExitCode::ERROR;
            }

            hhal_kernel_source source = it->second;

            switch (source.type) {
                case source_type::BINARY:
                    kernel_path = source.path_or_string;
                    break;
                case source_type::SOURCE: 
                    kernel_path = compiler.get_binary(source.path_or_string, Unit::GN);
                    if (kernel_path == "") {
                        return HHALExitCode::ERROR;
                    }
                    break;
                case source_type::STRING:
                {
                    std::string saved_file = dynamic_compiler::save_to_file(source.path_or_string);
                    kernel_path = compiler.get_binary(saved_file, Unit::GN);
                    if (kernel_path == "") {
                        return HHALExitCode::ERROR;
                    }
                    break;
                }
                default:
                    return HHALExitCode::ERROR;
            } 
            MAP_GN_EXIT_CODE(GN_MANAGER.kernel_write(kernel_id, kernel_path));
            break;
        }
        case Unit::NVIDIA:
        {
            it = kernel_sources.find(Unit::NVIDIA);
            if (it == kernel_sources.end()) {
                printf("No kernel source for NVIDIA\n");
                return HHALExitCode::ERROR;
            }

            hhal_kernel_source source = it->second;

            switch (source.type) {
                case source_type::BINARY:
                    kernel_path = source.path_or_string;
                    break;
                case source_type::SOURCE:
                    return HHALExitCode::ERROR;
                    break;
                case source_type::STRING:
                    return HHALExitCode::ERROR;
                    break;
                default:
                    return HHALExitCode::ERROR;
            }
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.kernel_write(kernel_id, kernel_path));
            break;
        }
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::kernel_start(int kernel_id, const Arguments &arguments) {
    printf("Starting kernel\n");
    switch (kernel_to_unit[kernel_id]) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.kernel_start(kernel_id, arguments));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.kernel_start(kernel_id, arguments));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::allocate_memory(int buffer_id) {
    switch (buffer_to_unit[buffer_id]) {
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.allocate_memory(buffer_id));
            break;
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.allocate_memory(buffer_id));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::release_memory(int buffer_id) {
    switch (buffer_to_unit[buffer_id]) {
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.release_memory(buffer_id));
            break;
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.release_memory(buffer_id));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::allocate_kernel(int kernel_id) {
    switch (kernel_to_unit[kernel_id]) {
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.allocate_kernel(kernel_id));
            break;
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.allocate_kernel(kernel_id));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::release_kernel(int kernel_id) {
    switch (kernel_to_unit[kernel_id]) {
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.release_kernel(kernel_id));
            break;
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.release_kernel(kernel_id));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::write_to_memory(int buffer_id, const void *source, size_t size) {
    switch (buffer_to_unit[buffer_id]) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.write_to_memory(buffer_id, source, size));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.write_to_memory(buffer_id, source, size));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::read_from_memory(int buffer_id, void *dest, size_t size) {
    switch (buffer_to_unit[buffer_id]) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.read_from_memory(buffer_id, dest, size));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.read_from_memory(buffer_id, dest, size));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::write_sync_register(int event_id, uint32_t data) {
    switch (event_to_unit[event_id]) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.write_sync_register(event_id, data));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.write_sync_register(event_id, data));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::read_sync_register(int event_id, uint32_t *data) {
    switch (event_to_unit[event_id]) {
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.read_sync_register(event_id, data));
            break;
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.read_sync_register(event_id, data));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::allocate_event(int event_id) {
    switch (event_to_unit[event_id]) {
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.allocate_event(event_id));
            break;
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.allocate_event(event_id));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

HHALExitCode HHAL::release_event(int event_id) {
    switch (event_to_unit[event_id]) {
        case Unit::NVIDIA:
            MAP_NVIDIA_EXIT_CODE(NVIDIA_MANAGER.release_event(event_id));
            break;
        case Unit::GN:
            MAP_GN_EXIT_CODE(GN_MANAGER.release_event(event_id));
            break;
        default:
            break;
    }
    return HHALExitCode::ERROR;
}

}
