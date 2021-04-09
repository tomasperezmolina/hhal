#ifndef HHAL_TYPES_H
#define HHAL_TYPES_H

#include <string>

namespace hhal {

enum class Unit {
    GN,
    NVIDIA
};

enum class source_type {
    BINARY,
    SOURCE,
    STRING
};

typedef struct hhal_kernel_source_t {
    source_type type;
    std::string path_or_string;
} hhal_kernel_source;

/*
 * Base kernel, buffer and event structs.
 * These are later casted into architecture specific structs, 
 * so they need to respect data order.
 */

typedef struct hhal_kernel_t {
    int id;
} hhal_kernel;

typedef struct hhal_buffer_t {
    int id;
} hhal_buffer;

typedef struct hhal_event_t {
    int id;
} hhal_event;

}

#endif
