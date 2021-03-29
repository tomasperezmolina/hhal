#ifndef HHAL_SERVER_H
#define HHAL_SERVER_H

#include <string>

#include "server/server.h"
#include "hhal.h"
#include "hhal_command.h"

namespace hhal_daemon {

class HHALServer {

public:
    HHALServer(std::string socket_path);
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

    Server::message_result_t handle_deassign_kernel(int id, const deassign_kernel_command *cmd, Server &server);
    Server::message_result_t handle_deassign_buffer(int id, const deassign_buffer_command *cmd, Server &server);
    Server::message_result_t handle_deassign_event(int id, const deassign_event_command *cmd, Server &server);


    Server::message_result_t handle_allocate_kernel(int id, const allocate_kernel_command *cmd, Server &server);
    Server::message_result_t handle_allocate_memory(int id, const allocate_memory_command *cmd, Server &server);
    Server::message_result_t handle_allocate_event(int id, const allocate_event_command *cmd, Server &server);

    Server::message_result_t handle_release_kernel(int id, const release_kernel_command *cmd, Server &server);
    Server::message_result_t handle_release_memory(int id, const release_memory_command *cmd, Server &server);
    Server::message_result_t handle_release_event(int id, const release_event_command *cmd, Server &server);


    Server::DataListenerExitCode handle_kernel_start_data(int id, kernel_start_command *cmd, Server::message_t data, Server &server);
    Server::DataListenerExitCode handle_kernel_write_data(int id, kernel_write_command *cmd, Server::message_t data, Server &server);
    Server::DataListenerExitCode handle_write_to_memory_data(int id, write_memory_command *cmd, Server::message_t data, Server &server);
    Server::DataListenerExitCode handle_assign_kernel_data(int id, assign_kernel_command *cmd, Server::message_t data, Server &server);
    Server::DataListenerExitCode handle_assign_buffer_data(int id, assign_buffer_command *cmd, Server::message_t data, Server &server);
    Server::DataListenerExitCode handle_assign_event_data(int id, assign_event_command *cmd, Server::message_t data, Server &server);
};

} // namespace daemon

#endif // HHAL_SERVER_H
