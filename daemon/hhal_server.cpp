#include "hhal_server.h"
#include "utils/logger.h"
#include "serialization.h"
#include "hhal_response.h"

namespace hhal_daemon {

static Logger &logger = Logger::get_instance();

Server::message_t ack_message() {
    response_base *response = (response_base *) malloc(sizeof(response_base));
    init_ack_response(*response);
    return {response, sizeof(response_base)};
}

Server::message_t error_message(hhal::HHALExitCode ec) {
    error_response *response = (error_response *) malloc(sizeof(error_response));
    init_error_response(*response, ec);
    return {response, sizeof(error_response)};
}

Server::message_result_t HHALServer::handle_command(int id, Server::message_t msg, Server &server) {
    logger.trace("Handling command");
    if (msg.size < sizeof(command_base)) {
        return {Server::MessageListenerExitCode::INSUFFICIENT_DATA, 0, 0}; // Need to read more data to determine a command
    }
    command_base *base = (command_base *) msg.buf;
    switch (base->type) {
    case command_type::KERNEL_START:
        if (msg.size >= sizeof(kernel_start_command)) {
            return handle_kernel_start(id, (kernel_start_command *)msg.buf, server);
        }
        break;
    case command_type::KERNEL_WRITE:
        if (msg.size >= sizeof(kernel_write_command)) {
            return handle_kernel_write(id, (kernel_write_command *)msg.buf, server);
        }
        break;
    case command_type::WRITE_MEMORY:
        if (msg.size >= sizeof(write_memory_command)) {
            return handle_write_to_memory(id, (write_memory_command *)msg.buf, server);
        }
        break;
    case command_type::READ_MEMORY:
        if (msg.size >= sizeof(read_memory_command)) {
            return handle_read_from_memory(id, (read_memory_command *)msg.buf, server);
        }
        break;
    case command_type::WRITE_REGISTER:
        if (msg.size >= sizeof(write_register_command)) {
            return handle_write_sync_register(id, (write_register_command *)msg.buf, server);
        }
        break;
    case command_type::READ_REGISTER:
        if (msg.size >= sizeof(read_register_command)) {
            return handle_read_sync_register(id, (read_register_command *)msg.buf, server);
        }
        break;
    case command_type::ASSIGN_KERNEL:
        if (msg.size >= sizeof(assign_kernel_command)) {
            return handle_assign_kernel(id, (assign_kernel_command *)msg.buf, server);
        }
        break;
    case command_type::ASSIGN_EVENT:
        if (msg.size >= sizeof(assign_event_command)) {
            return handle_assign_event(id, (assign_event_command *)msg.buf, server);
        }
        break;
    case command_type::ASSIGN_BUFFER:
        if (msg.size >= sizeof(assign_buffer_command)) {
            return handle_assign_buffer(id, (assign_buffer_command *)msg.buf, server);
        }
        break;
    case command_type::ALLOCATE_KERNEL:
        if (msg.size >= sizeof(allocate_kernel_command)) {
            return handle_allocate_kernel(id, (allocate_kernel_command *)msg.buf, server);
        }
        break;
    case command_type::ALLOCATE_MEMORY:
        if (msg.size >= sizeof(allocate_memory_command)) {
            return handle_allocate_memory(id, (allocate_memory_command *)msg.buf, server);
        }
        break;
    case command_type::ALLOCATE_EVENT:
        if (msg.size >= sizeof(allocate_event_command)) {
            return handle_allocate_event(id, (allocate_event_command *)msg.buf, server);
        }
        break;
    case command_type::RELEASE_KERNEL:
        if (msg.size >= sizeof(release_kernel_command)) {
            return handle_release_kernel(id, (release_kernel_command *)msg.buf, server);
        }
        break;
    case command_type::RELEASE_MEMORY:
        if (msg.size >= sizeof(release_memory_command)) {
            return handle_release_memory(id, (release_memory_command *)msg.buf, server);
        }
        break;
    case command_type::RELEASE_EVENT:
        if (msg.size >= sizeof(release_event_command)) {
            return handle_release_event(id, (release_event_command *)msg.buf, server);
        }
        break;
    default:
        logger.info("Received: unknown command");
        return {Server::MessageListenerExitCode::UNKNOWN_MESSAGE, 0, 0};
        break;
    }
    return {Server::MessageListenerExitCode::INSUFFICIENT_DATA, 0, 0};
}

Server::DataListenerExitCode HHALServer::handle_data(int id, Server::packet_t packet, Server &server) {
    logger.trace("Received data, size: {}", packet.extra_data.size);
    command_base *base = (command_base *) packet.msg.buf;
    switch (base->type) {
        case command_type::KERNEL_START: {
            return handle_kernel_start_data(id, (kernel_start_command *) base, packet.extra_data, server);
        }
        case command_type::KERNEL_WRITE: {
            return handle_kernel_write_data(id, (kernel_write_command *) base, packet.extra_data, server);
        }
        case command_type::WRITE_MEMORY: {
            return handle_write_to_memory_data(id, (write_memory_command *) base, packet.extra_data, server); 
        }
        case command_type::ASSIGN_KERNEL: {
            return handle_assign_kernel_data(id, (assign_kernel_command *) base, packet.extra_data, server);
        }
        case command_type::ASSIGN_BUFFER: {
            return handle_assign_buffer_data(id, (assign_buffer_command *) base, packet.extra_data, server);
        }
        case command_type::ASSIGN_EVENT: {
            return handle_assign_event_data(id, (assign_event_command *) base, packet.extra_data, server);
        }
        default: {
            logger.info("Data from unsupported command: {}", (char *)packet.extra_data.buf);
            free(packet.msg.buf);
            free(packet.extra_data.buf);
            return Server::DataListenerExitCode::OPERATION_ERROR;
        }
    }
}

Server::DataListenerExitCode HHALServer::handle_kernel_start_data(int id, kernel_start_command *cmd, Server::message_t data, Server &server) {
    auxiliary_allocations aux;
    hhal::Arguments args = deserialize_arguments({data.buf, data.size}, aux);
    auto ec = hhal.kernel_start(cmd->kernel_id, args);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
    }
    free(cmd);
    return Server::DataListenerExitCode::OK;
}  

Server::DataListenerExitCode HHALServer::handle_kernel_write_data(int id, kernel_write_command *cmd, Server::message_t data, Server &server) {
    std::map<hhal::Unit, std::string> kernel_images = 
        deserialize_kernel_images({data.buf, data.size});
    auto ec = hhal.kernel_write(cmd->kernel_id, kernel_images);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
    }
    free(cmd);
    return Server::DataListenerExitCode::OK;
}

Server::DataListenerExitCode HHALServer::handle_write_to_memory_data(int id, write_memory_command *cmd, Server::message_t data, Server &server) {
    auto ec = hhal.write_to_memory(cmd->buffer_id, data.buf, data.size);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
    }
    free(cmd);
    free(data.buf);
    return Server::DataListenerExitCode::OK;
}

Server::DataListenerExitCode HHALServer::handle_assign_kernel_data(int id, assign_kernel_command *cmd, Server::message_t data, Server &server) {
    switch (cmd->unit) {
        case hhal::Unit::GN: {
            hhal::gn_kernel k = deserialize_gn_kernel({data.buf, data.size});
            logger.debug("Received kernel data id: {}", k.id);
            auto ec = hhal.assign_kernel(cmd->unit, (hhal::hhal_kernel *) &k);
            if (ec != hhal::HHALExitCode::OK) {
                server.send_on_socket(id, error_message(ec));
            } else {
                server.send_on_socket(id, ack_message());
            }
            free(cmd);
            return Server::DataListenerExitCode::OK;
        }
        case hhal::Unit::NVIDIA: {
            // Already a POD
            auto ec = hhal.assign_kernel(cmd->unit, (hhal::hhal_kernel *) data.buf);
            if (ec != hhal::HHALExitCode::OK) {
                server.send_on_socket(id, error_message(ec));
            } else {
                server.send_on_socket(id, ack_message());
            }
            free(cmd);
            free(data.buf);
            return Server::DataListenerExitCode::OK;
        }
        default:
            logger.error("Received assign kernel command with unknown unit {}", cmd->unit);
            free(cmd);
            free(data.buf);
            return Server::DataListenerExitCode::OPERATION_ERROR;
    }
}

Server::DataListenerExitCode HHALServer::handle_assign_buffer_data(int id, assign_buffer_command *cmd, Server::message_t data, Server &server) {
    switch (cmd->unit) {
        case hhal::Unit::GN: {
            hhal::gn_buffer b = deserialize_gn_buffer({data.buf, data.size});
            logger.debug("Received buffer data id: {}", b.id);
            auto ec = hhal.assign_buffer(cmd->unit, (hhal::hhal_buffer *) &b);
            if (ec != hhal::HHALExitCode::OK) {
                server.send_on_socket(id, error_message(ec));
            } else {
                server.send_on_socket(id, ack_message());
            }
            free(cmd);
            return Server::DataListenerExitCode::OK;
        }
        case hhal::Unit::NVIDIA: {
            hhal::nvidia_buffer b = deserialize_nvidia_buffer({data.buf, data.size});
            auto ec = hhal.assign_buffer(cmd->unit, (hhal::hhal_buffer *) &b);
            if (ec != hhal::HHALExitCode::OK) {
                server.send_on_socket(id, error_message(ec));
            } else {
                server.send_on_socket(id, ack_message());
            }
            free(cmd);
            return Server::DataListenerExitCode::OK;
        }
        default:
            logger.error("Received assign buffer command with unknown unit {}", cmd->unit);
            free(cmd);
            free(data.buf);
            return Server::DataListenerExitCode::OPERATION_ERROR;
    }
}

Server::DataListenerExitCode HHALServer::handle_assign_event_data(int id, assign_event_command *cmd, Server::message_t data, Server &server) {
    switch (cmd->unit) {
        case hhal::Unit::GN: {
            hhal::gn_event e = deserialize_gn_event({data.buf, data.size});
            logger.debug("Received event data id: {}", e.id);
            auto ec = hhal.assign_event(cmd->unit, (hhal::hhal_event *) &e);
            if (ec != hhal::HHALExitCode::OK) {
                server.send_on_socket(id, error_message(ec));
            } else {
                server.send_on_socket(id, ack_message());
            }
            free(cmd);
            return Server::DataListenerExitCode::OK;
        }
        case hhal::Unit::NVIDIA: {
            // Already a POD
            auto ec = hhal.assign_event(cmd->unit, (hhal::hhal_event *) data.buf);
            if (ec != hhal::HHALExitCode::OK) {
                server.send_on_socket(id, error_message(ec));
            } else {
                server.send_on_socket(id, ack_message());
            }
            free(cmd);
            free(data.buf);
            return Server::DataListenerExitCode::OK;
        }
        default:
            logger.error("Received assign event command with unknown unit {}", cmd->unit);
            free(cmd);
            free(data.buf);
            return Server::DataListenerExitCode::OPERATION_ERROR;
    }
}

// Kernel Execution
Server::message_result_t HHALServer::handle_kernel_start(int id, const kernel_start_command *cmd, Server &server) {
    logger.trace("Received: kernel start command");
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(kernel_start_command), cmd->arguments_size};
}

Server::message_result_t HHALServer::handle_kernel_write(int id, const kernel_write_command *cmd, Server &server) {
    logger.trace("Received: kernel write command");
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(kernel_write_command), cmd->images_size};
}

Server::message_result_t HHALServer::handle_write_to_memory(int id, const write_memory_command *cmd, Server &server) {
    logger.trace("Received: write to memory command");
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(write_memory_command), cmd->size};
}

Server::message_result_t HHALServer::handle_read_from_memory(int id, const read_memory_command *cmd, Server &server) {
    logger.trace("Received: read from memory command");
    void *buf = malloc(cmd->size);
    auto ec = hhal.read_from_memory(cmd->buffer_id, buf, cmd->size);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
        server.send_on_socket(id, {buf, cmd->size});
    }
    return {Server::MessageListenerExitCode::OK, sizeof(read_memory_command), 0};
}

Server::message_result_t HHALServer::handle_write_sync_register(int id, const write_register_command *cmd, Server &server) {
    logger.trace("Received: write sync register command");
    auto ec = hhal.write_sync_register(cmd->event_id, cmd->data);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
    }
    return {Server::MessageListenerExitCode::OK, sizeof(write_register_command), 0};
}

Server::message_result_t HHALServer::handle_read_sync_register(int id, const read_register_command *cmd, Server &server) {
    logger.trace("Received: read sync register command");
    uint32_t val;
    auto ec = hhal.read_sync_register(cmd->event_id, &val);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        logger.trace("Read register, got value {}", val);
        register_data_response *res = (register_data_response *) malloc(sizeof(register_data_response));
        init_register_data_response(*res, val);
        server.send_on_socket(id, {res, sizeof(register_data_response)});
    }
    return {Server::MessageListenerExitCode::OK, sizeof(read_register_command), 0};
}

// Resource management
Server::message_result_t HHALServer::handle_assign_kernel(int id, const assign_kernel_command *cmd, Server &server) {
    logger.trace("Received: assign kernel command");
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(assign_kernel_command), cmd->size};
}

Server::message_result_t HHALServer::handle_assign_buffer(int id, const assign_buffer_command *cmd, Server &server) {
    logger.trace("Received: assign buffer command");
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(assign_buffer_command), cmd->size};
}

Server::message_result_t HHALServer::handle_assign_event(int id, const assign_event_command *cmd, Server &server) {
    logger.trace("Received: assign event command");
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(assign_event_command), cmd->size};
}

Server::message_result_t HHALServer::handle_allocate_kernel(int id, const allocate_kernel_command *cmd, Server &server) {
    logger.trace("Received: allocate kernel command");
    auto ec = hhal.allocate_kernel(cmd->kernel_id);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
    }
    return {Server::MessageListenerExitCode::OK, sizeof(allocate_kernel_command), 0};
}

Server::message_result_t HHALServer::handle_allocate_memory(int id, const allocate_memory_command *cmd, Server &server) {
    logger.trace("Received: allocate buffer command");
    auto ec = hhal.allocate_memory(cmd->buffer_id);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
    }
    return {Server::MessageListenerExitCode::OK, sizeof(allocate_memory_command), 0};
}

Server::message_result_t HHALServer::handle_allocate_event(int id, const allocate_event_command *cmd, Server &server) {
    logger.trace("Received: allocate event command");
    auto ec = hhal.allocate_event(cmd->event_id);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
    }
    return {Server::MessageListenerExitCode::OK, sizeof(allocate_event_command), 0};
}

Server::message_result_t HHALServer::handle_release_kernel(int id, const release_kernel_command *cmd, Server &server) {
    logger.trace("Received: release kernel command");
    auto ec = hhal.release_kernel(cmd->kernel_id);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
    }
    return {Server::MessageListenerExitCode::OK, sizeof(release_kernel_command), 0};
}

Server::message_result_t HHALServer::handle_release_memory(int id, const release_memory_command *cmd, Server &server) {
    logger.trace("Received: release memory command");
    auto ec = hhal.release_memory(cmd->buffer_id);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
    }
    return {Server::MessageListenerExitCode::OK, sizeof(release_memory_command), 0};
}

Server::message_result_t HHALServer::handle_release_event(int id, const release_event_command *cmd, Server &server) {
    logger.trace("Received: release event command");
    auto ec = hhal.release_event(cmd->event_id);
    if (ec != hhal::HHALExitCode::OK) {
        server.send_on_socket(id, error_message(ec));
    } else {
        server.send_on_socket(id, ack_message());
    }
    return {Server::MessageListenerExitCode::OK, sizeof(release_event_command), 0};   
}

HHALServer::HHALServer(std::string socket_path): server(
    socket_path, 10,
    [this](int id, Server::message_t msg, Server &server) { return this->handle_command(id, msg, server); },
    [this](int id, Server::packet_t packet, Server &server) { return this->handle_data(id, packet, server); }
) {
    logger.info("HHAL server starting...");
    Server::InitExitCode err = server.initialize();
    if (err != Server::InitExitCode::OK) {
        logger.error("HHAL server initialization error");
        exit(EXIT_FAILURE);
    }
    server.start();
}

HHALServer::~HHALServer() {}
} // namespace daemon