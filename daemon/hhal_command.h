#ifndef HHAL_COMMAND_H
#define HHAL_COMMAND_H

#include <cstring>
#include <cinttypes>

#include "hhal.h"

namespace hhal_daemon {

enum class command_type {
    // Kerner execution
    KERNEL_WRITE,
    KERNEL_START,
    WRITE_MEMORY,
    READ_MEMORY,
    WRITE_REGISTER,
    READ_REGISTER,

    // Resource management
    ASSIGN_KERNEL,
    ASSIGN_BUFFER,
    ASSIGN_EVENT,
    DEASSIGN_KERNEL,
    DEASSIGN_BUFFER,
    DEASSIGN_EVENT,
    ALLOCATE_MEMORY,
    ALLOCATE_KERNEL,
    ALLOCATE_EVENT,
    RELEASE_MEMORY,
    RELEASE_KERNEL,
    RELEASE_EVENT,
};

struct command_base {
    command_type type;
};

struct kernel_write_command {
    command_type type;
    int kernel_id;
    size_t sources_size;
};

struct kernel_start_command {
    command_type type;
    int kernel_id;
    size_t arguments_size;
};

struct write_memory_command {
    command_type type;
    int buffer_id;
    size_t size;
};

struct read_memory_command {
    command_type type;
    int buffer_id;
    size_t size;
};

struct write_register_command {
    command_type type;
    int event_id;
    uint32_t data;
};

struct read_register_command {
    command_type type;
    int event_id;
};

struct assign_kernel_command {
    command_type type;
    hhal::Unit unit;
    size_t size;
};

struct assign_buffer_command {
    command_type type;
    hhal::Unit unit;
    size_t size;
};

struct assign_event_command {
    command_type type;
    hhal::Unit unit;
    size_t size;
};

struct deassign_kernel_command {
    command_type type;
    int kernel_id;
};

struct deassign_buffer_command {
    command_type type;
    int buffer_id;
};

struct deassign_event_command {
    command_type type;
    int event_id;
};

struct allocate_kernel_command {
    command_type type;
    int kernel_id;
};

struct allocate_memory_command {
    command_type type;
    int buffer_id;
};

struct allocate_event_command {
    command_type type;
    int event_id;
};

struct release_kernel_command {
    command_type type;
    int kernel_id;
};

struct release_memory_command {
    command_type type;
    int buffer_id;
};

struct release_event_command {
    command_type type;
    int event_id;
};

inline void init_kernel_write_command(kernel_write_command &cmd, int kernel_id, size_t sources_size) {
    cmd.type = command_type::KERNEL_WRITE;
    cmd.kernel_id = kernel_id;
    cmd.sources_size = sources_size;
}

inline void init_kernel_start_command(kernel_start_command &cmd, int kernel_id, size_t arguments_size) {
    cmd.type = command_type::KERNEL_START;
    cmd.kernel_id = kernel_id;
    cmd.arguments_size = arguments_size;
}

inline void init_write_memory_command(write_memory_command &cmd, int buffer_id, size_t size) {
    cmd.type = command_type::WRITE_MEMORY;
    cmd.buffer_id = buffer_id;
    cmd.size = size;
}

inline void init_read_memory_command(read_memory_command &cmd, int buffer_id, size_t size) {
    cmd.type = command_type::READ_MEMORY;
    cmd.buffer_id = buffer_id;
    cmd.size = size;
}

inline void init_write_register_command(write_register_command &cmd, int event_id, uint32_t data) {
    cmd.type = command_type::WRITE_REGISTER;
    cmd.event_id = event_id;
    cmd.data = data;
}

inline void init_read_register_command(read_register_command &cmd, int event_id) {
    cmd.type = command_type::READ_REGISTER;
    cmd.event_id = event_id;
}

inline void init_assign_kernel_command(assign_kernel_command &cmd, hhal::Unit unit, size_t size) {
    cmd.type = command_type::ASSIGN_KERNEL;
    cmd.unit = unit;
    cmd.size = size;
}

inline void init_assign_buffer_command(assign_buffer_command &cmd, hhal::Unit unit, size_t size) {
    cmd.type = command_type::ASSIGN_BUFFER;
    cmd.unit = unit;
    cmd.size = size;
}

inline void init_assign_event_command(assign_event_command &cmd, hhal::Unit unit, size_t size) {
    cmd.type = command_type::ASSIGN_EVENT;
    cmd.unit = unit;
    cmd.size = size;
}

inline void init_deassign_kernel_command(deassign_kernel_command &cmd, int kernel_id) {
    cmd.type = command_type::DEASSIGN_KERNEL;
    cmd.kernel_id = kernel_id;
}

inline void init_deassign_buffer_command(deassign_buffer_command &cmd, int buffer_id) {
    cmd.type = command_type::DEASSIGN_BUFFER;
    cmd.buffer_id = buffer_id;
}

inline void init_deassign_event_command(deassign_event_command &cmd, int event_id) {
    cmd.type = command_type::DEASSIGN_EVENT;
    cmd.event_id = event_id;
}

inline void init_allocate_kernel_command(allocate_kernel_command &cmd, int kernel_id) {
    cmd.type = command_type::ALLOCATE_KERNEL;
    cmd.kernel_id = kernel_id;
}

inline void init_allocate_memory_command(allocate_memory_command &cmd, int buffer_id) {
    cmd.type = command_type::ALLOCATE_MEMORY;
    cmd.buffer_id = buffer_id;
}

inline void init_allocate_event_command(allocate_event_command &cmd, int event_id) {
    cmd.type = command_type::ALLOCATE_EVENT;
    cmd.event_id = event_id;
}

inline void init_release_kernel_command(release_kernel_command &cmd, int kernel_id) {
    cmd.type = command_type::RELEASE_KERNEL;
    cmd.kernel_id = kernel_id;
}

inline void init_release_memory_command(release_memory_command &cmd, int buffer_id) {
    cmd.type = command_type::RELEASE_MEMORY;
    cmd.buffer_id = buffer_id;
}

inline void init_release_event_command(release_event_command &cmd, int event_id) {
    cmd.type = command_type::RELEASE_EVENT;
    cmd.event_id = event_id;
}


} // namespace daemon

#endif
