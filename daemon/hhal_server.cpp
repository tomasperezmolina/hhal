#include "hhal_server.h"
#include "utils/logger.h"
#include "serialization.h"
#include "hhal_response.h"

namespace hhal_daemon {

Server::message_t ack_message() {
    response_base *response = (response_base *) malloc(sizeof(response_base));
    init_ack_response(*response);
    return {response, sizeof(response_base)};
}

static Logger &logger = Logger::get_instance();

Server::message_result_t HHALServer::handle_command(int id, Server::message_t msg, Server &server) {
    logger.info("Handling command");
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
    logger.info("Received data, size: {}", packet.extra_data.size);
    // TODO: error handling everywhere
    command_base *base = (command_base *) packet.msg.buf;
    switch (base->type) {
    case command_type::KERNEL_START: {
        kernel_start_command *command = (kernel_start_command *) base;
        hhal::Arguments args = deserialize_arguments({packet.extra_data.buf, packet.extra_data.size});
        hhal.kernel_start(command->kernel_id, args);
        break;
    }
    case command_type::KERNEL_WRITE: {
        kernel_write_command *command = (kernel_write_command *) base;
        std::map<hhal::Unit, std::string> kernel_images = 
            deserialize_kernel_images({packet.extra_data.buf, packet.extra_data.size});
        hhal.kernel_write(command->kernel_id, kernel_images);
        break;
    }
    case command_type::WRITE_MEMORY: {
        write_memory_command *command = (write_memory_command *) base;
        hhal.write_to_memory(command->buffer_id, packet.extra_data.buf, packet.extra_data.size);
        break;
    }
    case command_type::ASSIGN_KERNEL: {
        assign_kernel_command *command = (assign_kernel_command *) base;
        hhal.assign_kernel(command->unit, (hhal::hhal_kernel *) packet.extra_data.buf);
        break;
    }
    case command_type::ASSIGN_BUFFER: {
        assign_buffer_command *command = (assign_buffer_command *) base;
        hhal.assign_buffer(command->unit, (hhal::hhal_buffer *) packet.extra_data.buf);
        break;
    }
    case command_type::ASSIGN_EVENT: {
        assign_event_command *command = (assign_event_command *) base;
        hhal.assign_event(command->unit, (hhal::hhal_event *) packet.extra_data.buf);
        break;
    }
    default: {
        logger.info("Data from unsupported command: {}", (char *)packet.extra_data.buf);
        break;
    }
    }

    free(packet.extra_data.buf);
    free(packet.msg.buf);
    server.send_on_socket(id, ack_message());
    return Server::DataListenerExitCode::OK;
}

// Kernel Execution
Server::message_result_t HHALServer::handle_kernel_start(int id, const kernel_start_command *cmd, Server &server) {
    logger.info("Received: kernel start command");
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(kernel_start_command), cmd->arguments_size};
}

Server::message_result_t HHALServer::handle_kernel_write(int id, const kernel_write_command *cmd, Server &server) {
    logger.info("Received: kernel write command");
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(kernel_write_command), cmd->images_size};
}

Server::message_result_t HHALServer::handle_write_to_memory(int id, const write_memory_command *cmd, Server &server) {
    logger.info("Received: write to memory command");
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(write_memory_command), cmd->size};
}

Server::message_result_t HHALServer::handle_read_from_memory(int id, const read_memory_command *cmd, Server &server) {
    logger.info("Received: read from memory command");
    void *buf = malloc(cmd->size);
    // TODO: error handling
    hhal.read_from_memory(cmd->buffer_id, buf, cmd->size);
    server.send_on_socket(id, ack_message());
    server.send_on_socket(id, {buf, cmd->size});
    return {Server::MessageListenerExitCode::OK, sizeof(read_memory_command), 0};
}

Server::message_result_t HHALServer::handle_write_sync_register(int id, const write_register_command *cmd, Server &server) {
    logger.info("Received: write sync register command");
    // TODO: error handling
    hhal.write_sync_register(cmd->event_id, cmd->data);
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(write_register_command), 0};
}

Server::message_result_t HHALServer::handle_read_sync_register(int id, const read_register_command *cmd, Server &server) {
    logger.info("Received: read sync register command");
    // TODO: error handling
    uint32_t val;
    hhal.read_sync_register(cmd->event_id, &val);
    register_data_response *res = (register_data_response *) malloc(sizeof(register_data_response));
    init_register_data_response(*res, val);
    server.send_on_socket(id, {res, sizeof(register_data_response)});
    return {Server::MessageListenerExitCode::OK, sizeof(read_register_command), 0};
}

// Resource management
Server::message_result_t HHALServer::handle_assign_kernel(int id, const assign_kernel_command *cmd, Server &server) {
    logger.info("Received: assign kernel command 2");
    size_t payload_size;
    bool error = false;
    switch (cmd->unit) {
        case hhal::Unit::NVIDIA:
            payload_size = sizeof(hhal::nvidia_kernel);
            break;
        case hhal::Unit::GN:
            payload_size = sizeof(hhal::gn_kernel);
            break;
        default:
            logger.error("Unknown unit type {}", cmd->unit);
            error = true;
            break;
    }
    if (!error) {
        logger.debug("Prepared to receive {} bytes of data", payload_size);
        server.send_on_socket(id, ack_message());
        return {Server::MessageListenerExitCode::OK, sizeof(assign_kernel_command), payload_size};
    } else {
        logger.error("ERROR", payload_size);
        // TODO: error handling
        server.send_on_socket(id, ack_message());
        return {Server::MessageListenerExitCode::OK, sizeof(assign_kernel_command), 0};
    }
}

Server::message_result_t HHALServer::handle_assign_buffer(int id, const assign_buffer_command *cmd, Server &server) {
    logger.info("Received: assign buffer command");
    size_t payload_size;
    bool error = false;
    switch (cmd->unit) {
        case hhal::Unit::NVIDIA:
            payload_size = sizeof(hhal::nvidia_buffer);
            break;
        case hhal::Unit::GN:
            payload_size = sizeof(hhal::gn_buffer);
            break;
        default:
            logger.error("Unknown unit type {}", cmd->unit);
            error = true;
            break;
    }
    if (!error) {
        server.send_on_socket(id, ack_message());
        return {Server::MessageListenerExitCode::OK, sizeof(assign_buffer_command), payload_size};
    } else {
        // TODO: error handling
        server.send_on_socket(id, ack_message());
        return {Server::MessageListenerExitCode::OK, sizeof(assign_buffer_command), 0};
    }
}

Server::message_result_t HHALServer::handle_assign_event(int id, const assign_event_command *cmd, Server &server) {
    logger.info("Received: assign event command");
    size_t payload_size;
    bool error = false;
    switch (cmd->unit) {
        case hhal::Unit::NVIDIA:
            payload_size = sizeof(hhal::nvidia_event);
            break;
        case hhal::Unit::GN:
            payload_size = sizeof(hhal::gn_event);
            break;
        default:
            logger.error("Unknown unit type {}", cmd->unit);
            error = true;
            break;
    }
    if (!error) {
        server.send_on_socket(id, ack_message());
        return {Server::MessageListenerExitCode::OK, sizeof(assign_event_command), payload_size};
    } else {
        // TODO: error handling
        server.send_on_socket(id, ack_message());
        return {Server::MessageListenerExitCode::OK, sizeof(assign_event_command), 0};
    }
}

Server::message_result_t HHALServer::handle_allocate_kernel(int id, const allocate_kernel_command *cmd, Server &server) {
    logger.info("Received: allocate kernel command");
    // TODO: error handling
    hhal.allocate_kernel(cmd->kernel_id);
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(allocate_kernel_command), 0};
}

Server::message_result_t HHALServer::handle_allocate_memory(int id, const allocate_memory_command *cmd, Server &server) {
    logger.info("Received: allocate buffer command");
    // TODO: error handling
    hhal.allocate_memory(cmd->buffer_id);
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(allocate_memory_command), 0};
}

Server::message_result_t HHALServer::handle_allocate_event(int id, const allocate_event_command *cmd, Server &server) {
    logger.info("Received: allocate event command");
    // TODO: error handling
    hhal.allocate_event(cmd->event_id);
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(allocate_event_command), 0};
}

Server::message_result_t HHALServer::handle_release_kernel(int id, const release_kernel_command *cmd, Server &server) {
    logger.info("Received: release kernel command");
    // TODO: error handling
    hhal.release_kernel(cmd->kernel_id);
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(release_kernel_command), 0};
}

Server::message_result_t HHALServer::handle_release_memory(int id, const release_memory_command *cmd, Server &server) {
    logger.info("Received: release memory command");
    // TODO: error handling
    hhal.release_memory(cmd->buffer_id);
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(release_memory_command), 0};
}

Server::message_result_t HHALServer::handle_release_event(int id, const release_event_command *cmd, Server &server) {
    logger.info("Received: release event command");
    // TODO: error handling
    hhal.release_event(cmd->event_id);
    server.send_on_socket(id, ack_message());
    return {Server::MessageListenerExitCode::OK, sizeof(release_event_command), 0};   
}

HHALServer::HHALServer(const char *socket_path): server(
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