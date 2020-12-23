#ifndef NVIDIA_MANAGER_H
#define NVIDIA_MANAGER_H

#include <map>
#include <string>
#include <cstdint>

#include "nvidia/types.h"
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
        // TODO kernel_start(kernel_id, ..., char *args)
        NvidiaManagerExitCode kernel_start_string_args(int kernel_id, std::string arguments);
        NvidiaManagerExitCode allocate_memory(int buffer_id);
        NvidiaManagerExitCode allocate_kernel(int kernel_id);
        NvidiaManagerExitCode release_memory(int buffer_id);
        NvidiaManagerExitCode release_kernel(int kernel_id);
        NvidiaManagerExitCode write_to_memory(int buffer_id, const void *source, size_t size);
        NvidiaManagerExitCode read_from_memory(int buffer_id, void *dest, size_t size);
        NvidiaManagerExitCode write_sync_register(int event_id, uint8_t data);
        NvidiaManagerExitCode read_sync_register(int event_id, uint8_t *data);

    private:
        std::map<int, nvidia_kernel> kernel_info;
        std::map<int, nvidia_buffer> buffer_info;
        std::map<int, nvidia_event> event_info;

        CudaApi cuda_api;
        uint8_t data;
};

}

#endif
