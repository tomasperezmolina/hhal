#include "gn/manager.h"

namespace hhal {
    GNManagerExitCode GNManager::assign_kernel(gn_kernel info) {
        return GNManagerExitCode::OK;
    }
    GNManagerExitCode GNManager::assign_buffer(gn_buffer info) {
        return GNManagerExitCode::OK;
    }
    GNManagerExitCode GNManager::assign_event(gn_event info) {
        return GNManagerExitCode::OK;
    }
    GNManagerExitCode GNManager::kernel_write(int kernel_id, std::string image_path) {
        return GNManagerExitCode::OK;
    }
    GNManagerExitCode GNManager::kernel_start(int kernel_id, std::string arguments) {
        return GNManagerExitCode::OK;
    }
    GNManagerExitCode GNManager::write_to_memory(int buffer_id, const void *source, size_t size) {
        return GNManagerExitCode::OK;
    }
    GNManagerExitCode GNManager::read_from_memory(int buffer_id, void *dest, size_t size) {
        return GNManagerExitCode::OK;
    }
    GNManagerExitCode GNManager::write_sync_register(int event_id, uint8_t data) {
        return GNManagerExitCode::OK;
    }
    GNManagerExitCode GNManager::read_sync_register(int event_id, uint8_t *data) {
        return GNManagerExitCode::OK;
    }
}