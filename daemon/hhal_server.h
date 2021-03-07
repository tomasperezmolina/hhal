#ifndef HHAL_SERVER_H
#define HHAL_SERVER_H

#include "server/server.h"
#include "hhal.h"
#include "hhal_command.h"

namespace hhal_daemon {

class HHALServer {

public:
    HHALServer(const char *socket_path);
    ~HHALServer();

private:
    hhal::HHAL hhal;
    Server server;

    Server::message_result_t handle_command(int id, Server::message_t msg, Server &server);

    Server::DataListenerExitCode handle_data(int id, Server::packet_t packet, Server &server);

    // Kernel Execution
    Server::message_result_t handle_kernel_start(int id, const kernel_start_command *cmd, Server &server);
    Server::message_result_t handle_kernel_write(int id, const kernel_write_command *cmd, Server &server);

    Server::message_result_t handle_write_to_memory(int id, const write_memory_command *cmd, Server &server);
    Server::message_result_t handle_read_from_memory(int id, const read_memory_command *cmd, Server &server);

    Server::message_result_t handle_write_sync_register(int id, const write_register_command *cmd, Server &server);
    Server::message_result_t handle_read_sync_register(int id, const read_register_command *cmd, Server &server);

    // Resource management
    Server::message_result_t handle_assign_kernel(int id, const assign_kernel_command *cmd, Server &server);
    Server::message_result_t handle_assign_buffer(int id, const assign_buffer_command *cmd, Server &server);
    Server::message_result_t handle_assign_event(int id, const assign_event_command *cmd, Server &server);

    Server::message_result_t handle_allocate_kernel(int id, const allocate_kernel_command *cmd, Server &server);
    Server::message_result_t handle_allocate_memory(int id, const allocate_memory_command *cmd, Server &server);
    Server::message_result_t handle_allocate_event(int id, const allocate_event_command *cmd, Server &server);

    Server::message_result_t handle_release_kernel(int id, const release_kernel_command *cmd, Server &server);
    Server::message_result_t handle_release_memory(int id, const release_memory_command *cmd, Server &server);
    Server::message_result_t handle_release_event(int id, const release_event_command *cmd, Server &server);

};

} // namespace daemon

#endif // HHAL_SERVER_H