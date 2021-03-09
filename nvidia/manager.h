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

#include "cuda_api.h"

namespace hhal {

enum class NvidiaManagerExitCode {
    OK,
    ERROR,
};

class NvidiaManager {

    public:
        NvidiaManagerExitCode assign_kernel(nvidia_kernel *info);
        NvidiaManagerExitCode assign_buffer(nvidia_buffer *info);
        NvidiaManagerExitCode assign_event(nvidia_event *info);

        NvidiaManagerExitCode kernel_write(int kernel_id, std::string image_path);

        NvidiaManagerExitCode kernel_start(int kernel_id, const Arguments &arguments);

        NvidiaManagerExitCode allocate_memory(int buffer_id);
        NvidiaManagerExitCode allocate_kernel(int kernel_id);
        NvidiaManagerExitCode allocate_event(int event_id);
        
        NvidiaManagerExitCode release_memory(int buffer_id);
        NvidiaManagerExitCode release_kernel(int kernel_id);
        NvidiaManagerExitCode release_event(int event_id);

        NvidiaManagerExitCode write_to_memory(int buffer_id, const void *source, size_t size);
        NvidiaManagerExitCode read_from_memory(int buffer_id, void *dest, size_t size);
        NvidiaManagerExitCode write_sync_register(int event_id, uint32_t data);
        NvidiaManagerExitCode read_sync_register(int event_id, uint32_t *data);

       
    private:
        std::map<int, nvidia_kernel> kernel_info;
        std::map<int, nvidia_buffer> buffer_info;
        std::map<int, nvidia_event> event_info;

        std::map<int, std::string> kernel_function_names;
        
        ThreadPool thread_pool;
        EventRegistry registry;

        void launch_kernel(int kernel_id, char *arg_array, int arg_count);

        CudaApi cuda_api;

};

}

#endif
