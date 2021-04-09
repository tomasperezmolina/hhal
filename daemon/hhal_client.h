#ifndef HHAL_CLIENT_H
#define HHAL_CLIENT_H

#include <map>
#include <cinttypes>

#include "hhal.h"

namespace hhal_daemon {

enum class HHALClientExitCode {
    OK,           // successful operation
    ERROR,        // generic error in the request
    SEVERE_ERROR, // error that leaves the client unusable
};

class HHALClient {
    public:
    HHALClient(const std::string socket_path);
    ~HHALClient();

    // Kernel execution
    HHALClientExitCode kernel_write(int kernel_id, const std::map<hhal::Unit, hhal::hhal_kernel_source> &kernel_sources);
    HHALClientExitCode kernel_start(int kernel_id, const hhal::Arguments &arguments);

    HHALClientExitCode write_to_memory(int buffer_id, const void *source, size_t size);
    HHALClientExitCode read_from_memory(int buffer_id, void *dest, size_t size);

    HHALClientExitCode write_sync_register(int event_id, uint32_t data);
    HHALClientExitCode read_sync_register(int event_id, uint32_t *data);
    // -----------------------

    // Resource management
    HHALClientExitCode assign_kernel(hhal::Unit unit, hhal::hhal_kernel *info);
    HHALClientExitCode assign_buffer(hhal::Unit unit, hhal::hhal_buffer *info);
    HHALClientExitCode assign_event (hhal::Unit unit, hhal::hhal_event *info);

    HHALClientExitCode deassign_kernel(int kernel_id);
    HHALClientExitCode deassign_buffer(int buffer_id);
    HHALClientExitCode deassign_event(int event_id);

    HHALClientExitCode allocate_memory(int buffer_id);
    HHALClientExitCode release_memory(int buffer_id);

    HHALClientExitCode allocate_kernel(int kernel_id);
    HHALClientExitCode release_kernel(int kernel_id);

    HHALClientExitCode allocate_event(int event_id);
    HHALClientExitCode release_event(int event_id);

    private:

    int socket_fd;

    void close_socket();
};

}

#endif
