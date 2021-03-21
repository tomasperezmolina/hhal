#ifndef GN_TYPES_H
#define GN_TYPES_H

#include <vector>
#include <string>
#include <cinttypes>

namespace hhal {

typedef struct gn_kernel_t {
    int id;
    int cluster_id;
    size_t size;
    uint32_t unit_id;
    int termination_event;
    std::vector<int> task_events;
} gn_kernel;

typedef struct gn_buffer_t {
    int id;
    uint32_t physical_addr;
    int cluster_id;
    int mem_tile;
    size_t size;
    int event;
    std::vector<int> kernels_in;
    std::vector<int> kernels_out;
} gn_buffer;

typedef struct gn_event_t {
    int id;
    uint32_t physical_addr;
    int cluster_id;
    std::vector<int> kernels_in;
    std::vector<int> kernels_out;
} gn_event;

}

#endif