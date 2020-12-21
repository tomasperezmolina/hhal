#include "hn/manager.h"

namespace hhal {
    HNManagerExitCode HNManager::assign_kernel(hn_kernel info) {
        return HNManagerExitCode::OK;
    }
    HNManagerExitCode HNManager::assign_buffer(hn_buffer info) {
        return HNManagerExitCode::OK;
    }
    HNManagerExitCode HNManager::assign_event(hn_event info) {
        return HNManagerExitCode::OK;
    }
    HNManagerExitCode HNManager::kernel_write(int kernel_id, std::string image_path) {
        return HNManagerExitCode::OK;
    }
    HNManagerExitCode HNManager::kernel_start(int kernel_id, std::string arguments) {
        return HNManagerExitCode::OK;
    }
    HNManagerExitCode HNManager::write_to_memory(int buffer_id, const void *source, size_t size) {
        return HNManagerExitCode::OK;
    }
    HNManagerExitCode HNManager::read_from_memory(int buffer_id, void *dest, size_t size) {
        return HNManagerExitCode::OK;
    }
    HNManagerExitCode HNManager::write_sync_register(int event_id, uint8_t data) {
        return HNManagerExitCode::OK;
    }
    HNManagerExitCode HNManager::read_sync_register(int event_id, uint8_t *data) {
        return HNManagerExitCode::OK;
    }
}