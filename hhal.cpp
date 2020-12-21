#include "hhal.h"

namespace hhal {

void HHAL::assign_kernel_to_gn(gn_kernel info) {
    kernel_to_unit[info.id] = Unit::GN;
    gn_manager.assign_kernel(info);
}

void HHAL::assign_kernel_to_hn(hn_kernel info) {
    kernel_to_unit[info.id] = Unit::HN;
    hn_manager.assign_kernel(info);
}

void HHAL::assign_kernel_to_nvidia(nvidia_kernel info) {
    kernel_to_unit[info.id] = Unit::NVIDIA;
    nvidia_manager.assign_kernel(info);
}

void HHAL::assign_buffer_to_gn(gn_buffer info) {
    buffer_to_unit[info.id] = Unit::GN;
    gn_manager.assign_buffer(info);
}

void HHAL::assign_buffer_to_hn(hn_buffer info) {
    buffer_to_unit[info.id] = Unit::HN;
    hn_manager.assign_buffer(info);
}

void HHAL::assign_buffer_to_nvidia(nvidia_buffer info) {
    buffer_to_unit[info.id] = Unit::NVIDIA;
    nvidia_manager.assign_buffer(info);
}

void HHAL::assign_event_to_gn(gn_event info) {
    event_to_unit[info.id] = Unit::GN;
    gn_manager.assign_event(info);
}

void HHAL::assign_event_to_hn(hn_event info) {
    event_to_unit[info.id] = Unit::HN;
    hn_manager.assign_event(info);
}

void HHAL::assign_event_to_nvidia(nvidia_event info) {
    event_to_unit[info.id] = Unit::NVIDIA;
    nvidia_manager.assign_event(info);
}

void HHAL::kernel_write(int kernel_id, std::string image_path) {
    switch (kernel_to_unit[kernel_id]) {
        case Unit::GN:
            gn_manager.kernel_write(kernel_id, image_path);
            break;
        case Unit::HN:
            hn_manager.kernel_write(kernel_id, image_path);
            break;
        case Unit::NVIDIA:
            nvidia_manager.kernel_write(kernel_id, image_path);
            break;
        default:
            break;
    }
}

void HHAL::kernel_start(int kernel_id, std::string arguments) {
    switch (kernel_to_unit[kernel_id]) {
        case Unit::GN:
            gn_manager.kernel_start(kernel_id, arguments);
            break;
        case Unit::HN:
            hn_manager.kernel_start(kernel_id, arguments);
            break;
        case Unit::NVIDIA:
            nvidia_manager.kernel_start(kernel_id, arguments);
            break;
        default:
            break;
    }
}

void HHAL::allocate_memory(int buffer_id) {
    switch (buffer_to_unit[buffer_id]) {
        case Unit::NVIDIA:
            nvidia_manager.allocate_memory(buffer_id);
            break;
        default:
            break;
    }
}

void HHAL::release_memory(int buffer_id) {
    switch (buffer_to_unit[buffer_id]) {
        case Unit::NVIDIA:
            nvidia_manager.release_memory(buffer_id);
            break;
        default:
            break;
    }
}

void HHAL::allocate_kernel(int kernel_id) {
    switch (kernel_to_unit[kernel_id]) {
        case Unit::NVIDIA:
            nvidia_manager.allocate_kernel(kernel_id);
            break;
        default:
            break;
    }
}

void HHAL::release_kernel(int kernel_id) {
    switch (kernel_to_unit[kernel_id]) {
        case Unit::NVIDIA:
            nvidia_manager.release_kernel(kernel_id);
            break;
        default:
            break;
    }
}

void HHAL::write_to_memory(int buffer_id, const void *source, size_t size) {
    switch (buffer_to_unit[buffer_id]) {
        case Unit::GN:
            gn_manager.write_to_memory(buffer_id, source, size);
            break;
        case Unit::HN:
            hn_manager.write_to_memory(buffer_id, source, size);
            break;
        case Unit::NVIDIA:
            nvidia_manager.write_to_memory(buffer_id, source, size);
            break;
        default:
            break;
    }
}

void HHAL::read_from_memory(int buffer_id, void *dest, size_t size) {
    switch (buffer_to_unit[buffer_id]) {
        case Unit::GN:
            gn_manager.read_from_memory(buffer_id, dest, size);
            break;
        case Unit::HN:
            hn_manager.read_from_memory(buffer_id, dest, size);
            break;
        case Unit::NVIDIA:
            nvidia_manager.read_from_memory(buffer_id, dest, size);
            break;
        default:
            break;
    }
}

void HHAL::write_sync_register(int event_id, uint8_t data) {
    switch (event_to_unit[event_id]) {
        case Unit::GN:
            gn_manager.write_sync_register(event_id, data);
            break;
        case Unit::HN:
            hn_manager.write_sync_register(event_id, data);
            break;
        case Unit::NVIDIA:
            nvidia_manager.write_sync_register(event_id, data);
            break;
        default:
            break;
    }
}

void HHAL::read_sync_register(int event_id, uint8_t *data) {
    switch (event_to_unit[event_id]) {
        case Unit::GN:
            gn_manager.read_sync_register(event_id, data);
            break;
        case Unit::HN:
            hn_manager.read_sync_register(event_id, data);
            break;
        case Unit::NVIDIA:
            nvidia_manager.read_sync_register(event_id, data);
            break;
        default:
            break;
    }
}

}