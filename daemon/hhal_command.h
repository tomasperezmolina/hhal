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
    ALLOCATE_MEMORY,
    ALLOCATE_KERNEL,
    ALLOCATE_EVENT,
    RELEASE_MEMORY,
    RELEASE_KERNEL,
    RELEASE_EVENT,
};

struct command_base {
    command_type cmd;
};

struct kernel_write_command {
    command_type cmd;
    int kernel_id;
    size_t images_size;
};

struct kernel_start_command {
    command_type cmd;
    int kernel_id;
    size_t arguments_size;
};

struct write_memory_command {
    command_type cmd;
    int buffer_id;
    size_t size;
};

struct read_memory_command {
    command_type cmd;
    int buffer_id;
    size_t size;
};

struct write_register_command {
    command_type cmd;
    int event_id;
    uint32_t data;
};

struct read_register_command {
    command_type cmd;
    int event_id;
};

struct assign_kernel_command {
    command_type cmd;
    hhal::Unit unit;
};

struct assign_buffer_command {
    command_type cmd;
    hhal::Unit unit;
};

struct assign_event_command {
    command_type cmd;
    hhal::Unit unit;
};

struct allocate_kernel_command {
    command_type cmd;
    int kernel_id;
};

struct allocate_memory_command {
    command_type cmd;
    int buffer_id;
};

struct allocate_event_command {
    command_type cmd;
    int event_id;
};

struct release_kernel_command {
    command_type cmd;
    int kernel_id;
};

struct release_memory_command {
    command_type cmd;
    int buffer_id;
};

struct release_event_command {
    command_type cmd;
    int event_id;
};

inline void init_kernel_write_command(kernel_write_command &cmd, int kernel_id, size_t images_size) {
  cmd.cmd = command_type::KERNEL_WRITE;
  cmd.kernel_id = kernel_id;
  cmd.images_size = images_size;
}

inline void init_kernel_start_command(kernel_start_command &cmd, int kernel_id, size_t arguments_size) {
    cmd.cmd = command_type::KERNEL_START;
    cmd.kernel_id = kernel_id;
    cmd.arguments_size = arguments_size;
}

inline void init_write_memory_command(write_memory_command &cmd, int buffer_id, size_t size) {
    cmd.cmd = command_type::WRITE_MEMORY;
    cmd.buffer_id = buffer_id;
    cmd.size = size;
}

inline void init_read_memory_command(read_memory_command &cmd, int buffer_id, size_t size) {
    cmd.cmd = command_type::READ_MEMORY;
    cmd.buffer_id = buffer_id;
    cmd.size = size;
}

inline void init_write_register_command(write_register_command &cmd, int event_id, uint32_t data) {
    cmd.cmd = command_type::WRITE_REGISTER;
    cmd.event_id = event_id;
    cmd.data = data;
}

inline void init_read_register_command(read_register_command &cmd, int event_id) {
    cmd.cmd = command_type::READ_REGISTER;
    cmd.event_id = event_id;
}

inline void init_assign_kernel_command(assign_kernel_command &cmd, hhal::Unit unit) {
    cmd.cmd = command_type::ASSIGN_KERNEL;
    cmd.unit = unit;
}

inline void init_assign_buffer_command(assign_buffer_command &cmd, hhal::Unit unit) {
    cmd.cmd = command_type::ASSIGN_BUFFER;
    cmd.unit = unit;
}

inline void init_assign_event_command(assign_event_command &cmd, hhal::Unit unit) {
    cmd.cmd = command_type::ASSIGN_EVENT;
    cmd.unit = unit;
}

inline void init_allocate_kernel_command(allocate_kernel_command &cmd, int kernel_id) {
    cmd.cmd = command_type::ALLOCATE_KERNEL;
    cmd.kernel_id = kernel_id;
}

inline void init_allocate_memory_command(allocate_memory_command &cmd, int buffer_id) {
    cmd.cmd = command_type::ALLOCATE_MEMORY;
    cmd.buffer_id = buffer_id;
}

inline void init_allocate_event_command(allocate_event_command &cmd, int event_id) {
    cmd.cmd = command_type::ALLOCATE_EVENT;
    cmd.event_id = event_id;
}

inline void init_release_kernel_command(release_kernel_command &cmd, int kernel_id) {
    cmd.cmd = command_type::RELEASE_KERNEL;
    cmd.kernel_id = kernel_id;
}

inline void init_release_memory_command(release_memory_command &cmd, int buffer_id) {
    cmd.cmd = command_type::RELEASE_MEMORY;
    cmd.buffer_id = buffer_id;
}

inline void init_release_event_command(release_event_command &cmd, int event_id) {
    cmd.cmd = command_type::RELEASE_EVENT;
    cmd.event_id = event_id;
}


} // namespace daemon

#endif
