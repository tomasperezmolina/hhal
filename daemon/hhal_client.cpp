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

bool receive_rest_of_response(int socket_fd, const response_base &res, void *bigger_res, size_t size) {
    memcpy(bigger_res, &res, sizeof(res));
    return receive_on_socket(socket_fd, ((char *) bigger_res) + sizeof(res), size - sizeof(res));
}

HHALClient::HHALClient(const std::string socket_path) {
    socket_fd = initialize(socket_path.c_str());
    if (socket_fd == NO_SOCKET) {
        printf("HHALClient: Socket initialization failure\n");
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
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    TRY_OR_CLOSE(send_on_socket(socket_fd, serialized.buf, serialized.size))
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::kernel_start(int kernel_id, const hhal::Arguments &arguments) {
    CHECK_OPEN_SOCKET

    serialized_object serialized = serialize(arguments);

    kernel_start_command cmd;
    init_kernel_start_command(cmd, kernel_id, serialized.size);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    TRY_OR_CLOSE(send_on_socket(socket_fd, serialized.buf, serialized.size))
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::write_to_memory(int buffer_id, const void *source, size_t size) {
    CHECK_OPEN_SOCKET

    write_memory_command cmd;
    init_write_memory_command(cmd, buffer_id, size);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    TRY_OR_CLOSE(send_on_socket(socket_fd, source, size))
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::read_from_memory(int buffer_id, void *dest, size_t size) {
    CHECK_OPEN_SOCKET

    read_memory_command cmd;
    init_read_memory_command(cmd, buffer_id, size);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    TRY_OR_CLOSE(receive_on_socket(socket_fd, dest, size))

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::write_sync_register(int event_id, uint32_t data) {
    CHECK_OPEN_SOCKET

    write_register_command cmd;
    init_write_register_command(cmd, event_id, data);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::read_sync_register(int event_id, uint32_t *data) {
    CHECK_OPEN_SOCKET

    read_register_command cmd;
    init_read_register_command(cmd, event_id);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    } else if (res.type == response_type::REGISTER_DATA) {
        register_data_response rd_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &rd_res, sizeof(rd_res)));
    } else {
        // Unknown response type
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}
// -----------------------

// Resource management
HHALClientExitCode HHALClient::assign_kernel(hhal::Unit unit, hhal::hhal_kernel *info) {
    printf("HHALClient: Assigning kernel id %d\n", info->id);
    CHECK_OPEN_SOCKET

    serialized_object serialized;
    switch (unit) {
        case hhal::Unit::GN:
            printf("HHALClient: Address of info: %p\n", info);
            serialized = serialize(*(hhal::gn_kernel *) info);
            break;
        case hhal::Unit::NVIDIA: {
            // Already a POD
            void *info_buf = malloc(sizeof(hhal::nvidia_kernel));
            memcpy(info_buf, info, sizeof(hhal::nvidia_kernel));
            serialized = {info_buf, sizeof(hhal::nvidia_kernel)};
            break;
        }
        default:
            printf("Unknown unit type %d", static_cast<int>(unit));
            return HHALClientExitCode::ERROR;
    }

    int id = *(int *) serialized.buf;
    printf("HHALClient: (After serialization) Assigning kernel id %d\n", id);

    assign_kernel_command cmd;
    init_assign_kernel_command(cmd, unit, serialized.size);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    TRY_OR_CLOSE(send_on_socket(socket_fd, serialized.buf, serialized.size))
    printf("Done sending serialized kernel\n");
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::assign_buffer(hhal::Unit unit, hhal::hhal_buffer *info) {
    CHECK_OPEN_SOCKET

    serialized_object serialized;
    switch (unit) {
        case hhal::Unit::GN:
            serialized = serialize(*(hhal::gn_buffer *) info);
            break;
        case hhal::Unit::NVIDIA:
            serialized = serialize(*(hhal::nvidia_buffer *) info);
            break;
        default:
            printf("Unknown unit type %d", static_cast<int>(unit));
            return HHALClientExitCode::ERROR;
    }

    assign_buffer_command cmd;
    init_assign_buffer_command(cmd, unit, serialized.size);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    TRY_OR_CLOSE(send_on_socket(socket_fd, serialized.buf, serialized.size))

    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::assign_event(hhal::Unit unit, hhal::hhal_event *info) {
    CHECK_OPEN_SOCKET

    serialized_object serialized;
    switch (unit) {
        case hhal::Unit::GN:
            serialized = serialize(*(hhal::gn_event *) info);
            break;
        case hhal::Unit::NVIDIA: {
            // Already a POD
            void *info_buf = malloc(sizeof(hhal::nvidia_event));
            memcpy(info_buf, info, sizeof(hhal::nvidia_event));
            serialized = {info_buf, sizeof(hhal::nvidia_event)};
            break;
        }
        default:
            printf("Unknown unit type %d", static_cast<int>(unit));
            return HHALClientExitCode::ERROR;
    }

    assign_event_command cmd;
    init_assign_event_command(cmd, unit, serialized.size);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    TRY_OR_CLOSE(send_on_socket(socket_fd, serialized.buf, serialized.size))
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::allocate_memory(int buffer_id) {
    CHECK_OPEN_SOCKET

    allocate_memory_command cmd;
    init_allocate_memory_command(cmd, buffer_id);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::release_memory(int buffer_id) {
    CHECK_OPEN_SOCKET

    release_memory_command cmd;
    init_release_memory_command(cmd, buffer_id);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::allocate_kernel(int kernel_id) {
    CHECK_OPEN_SOCKET

    allocate_kernel_command cmd;
    init_allocate_kernel_command(cmd, kernel_id);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::release_kernel(int kernel_id) {
    CHECK_OPEN_SOCKET

    release_kernel_command cmd;
    init_release_kernel_command(cmd, kernel_id);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::allocate_event(int event_id) {
    CHECK_OPEN_SOCKET

    allocate_event_command cmd;
    init_allocate_event_command(cmd, event_id);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

HHALClientExitCode HHALClient::release_event(int event_id) {
    CHECK_OPEN_SOCKET

    release_event_command cmd;
    init_release_event_command(cmd, event_id);
    TRY_OR_CLOSE(send_on_socket(socket_fd, &cmd, sizeof(cmd)))

    response_base res;
    TRY_OR_CLOSE(receive_on_socket(socket_fd, &res, sizeof(res)))

    if (res.type == response_type::ERROR) {
        error_response error_res;
        TRY_OR_CLOSE(receive_rest_of_response(socket_fd, res, &error_res, sizeof(error_res)));
        return HHALClientExitCode::ERROR;
    }

    return HHALClientExitCode::OK;
}

}