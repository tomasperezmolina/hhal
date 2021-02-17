#ifndef NVIDIA_MANAGER_H
#define NVIDIA_MANAGER_H

#include <map>
#include <string>
#include <cstdint>
#include <mutex>

#include "arguments.h"
#include "nvidia/types.h"
#include "nvidia/event_registry.h"
#include "nvidia/thread_pool.h"

// There should be a cleaner solution to enable/disable a manager.
// If its not done here it would have to be done at the hhal.cpp for each function.
// For now this seems like the lowest effort approach.
#ifdef ENABLE_NVIDIA
    #include "cuda_api.h"
#endif

namespace hhal {

enum class NvidiaManagerExitCode {
    OK,
    ERROR,
};

class NvidiaManager {
    public:
#ifdef ENABLE_NVIDIA
        NvidiaManagerExitCode assign_kernel(nvidia_kernel *info);
        NvidiaManagerExitCode assign_buffer(nvidia_buffer *info);
        NvidiaManagerExitCode assign_event(nvidia_event *info);

        NvidiaManagerExitCode kernel_write(int kernel_id, std::string image_path);

        NvidiaManagerExitCode kernel_start(int kernel_id, const Arguments &arguments);

        NvidiaManagerExitCode allocate_memory(int buffer_id);
        NvidiaManagerExitCode allocate_kernel(int kernel_id);
        NvidiaManagerExitCode release_memory(int buffer_id);
        NvidiaManagerExitCode release_kernel(int kernel_id);
        NvidiaManagerExitCode write_to_memory(int buffer_id, const void *source, size_t size);
        NvidiaManagerExitCode read_from_memory(int buffer_id, void *dest, size_t size);
        NvidiaManagerExitCode write_sync_register(int event_id, uint8_t data);
        NvidiaManagerExitCode read_sync_register(int event_id, uint8_t *data);

        NvidiaManagerExitCode allocate_event(int event_id);
        NvidiaManagerExitCode release_event(int event_id);
#else
        inline NvidiaManagerExitCode assign_kernel(nvidia_kernel *info) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode assign_buffer(nvidia_buffer *info) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode assign_event(nvidia_event *info) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode kernel_write(int kernel_id, std::string image_path) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode kernel_start(int kernel_id, const Arguments &arguments) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode allocate_memory(int buffer_id) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode allocate_kernel(int kernel_id) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode release_memory(int buffer_id) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode release_kernel(int kernel_id) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode write_to_memory(int buffer_id, const void *source, size_t size) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode read_from_memory(int buffer_id, void *dest, size_t size) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode write_sync_register(int event_id, uint8_t data) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode read_sync_register(int event_id, uint8_t *data) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode allocate_event(int event_id) {
            return NvidiaManagerExitCode::ERROR;
        }
        inline NvidiaManagerExitCode release_event(int event_id) {
            return NvidiaManagerExitCode::ERROR;
        }
#endif

    private:
        std::map<int, nvidia_kernel> kernel_info;
        std::map<int, nvidia_buffer> buffer_info;
        std::map<int, nvidia_event> event_info;
        
        ThreadPool thread_pool;
        EventRegistry registry;

        void launch_kernel(int kernel_id, char *arg_array, int arg_count);

#ifdef ENABLE_NVIDIA
        CudaApi cuda_api;
#endif
        uint8_t data;
};

}

#endif
