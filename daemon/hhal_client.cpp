#include "hhal_client.h"
#include "client/socket_client.h"
#include "serialization.h"
#include "hhal_command.h"
#include "hhal_response.h"

#define NO_SOCKET -1

#define CHECK_OPEN_SOCKET                           \
    if (socket_fd == NO_SOCKET)                     \
        return HHALClientExitCode::SEVERE_ERROR;

#define TRY_OR_CLOSE(x)                             \
    if (!x) {                                       \
        close_socket();                             \
        return HHALClientExitCode::SEVERE_ERROR;    \
    }

namespace hhal_daemon {

HHALClient::HHALClient(const std::string socket_path) {
    socket_fd = initialize(socket_path.c_str());
    if (socket_fd == NO_SOCKET) {
        printf("CudaClient: Socket initialization failure\n");
        exit(EXIT_FAILURE);
    }
}

HHALClient::~HHALClient() {
    close_socket();
}

void HHALClient::close_socket() {
    if (socket_fd == NO_SOCKET) return;

    end(socket_fd);
    socket_fd = NO_SOCKET;
}

// Kernel execution
HHALClientExitCode HHALClient::kernel_write(int kernel_id, const std::map<hhal::Unit, std::string> &kernel_images) {
    CHECK_OPEN_SOCKET

    serialized_object serialized = serialize(kernel_images);

    kernel_write_command cmd;
    init_kernel_write_command(cmd, kernel_id, serialized.size);

}

HHALClientExitCode HHALClient::kernel_start(int kernel_id, const hhal::Arguments &arguments) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::write_to_memory(int buffer_id, const void *source, size_t size) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::read_from_memory(int buffer_id, void *dest, size_t size) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::write_sync_register(int event_id, uint32_t data) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::read_sync_register(int event_id, uint32_t *data) {
    CHECK_OPEN_SOCKET
}
// -----------------------

// Resource management
HHALClientExitCode HHALClient::assign_kernel(hhal::Unit unit, hhal::hhal_kernel *info) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::assign_buffer(hhal::Unit unit, hhal::hhal_buffer *info) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::assign_event (hhal::Unit unit, hhal::hhal_event *info) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::allocate_memory(int buffer_id) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::release_memory(int buffer_id) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::allocate_kernel(int kernel_id) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::release_kernel(int kernel_id) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::allocate_event(int event_id) {
    CHECK_OPEN_SOCKET
}

HHALClientExitCode HHALClient::release_event(int event_id) {
    CHECK_OPEN_SOCKET
}

}