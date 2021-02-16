#ifndef GN_TYPES_H
#define GN_TYPES_H

#include <vector>
#include <string>

namespace hhal {

typedef struct gn_kernel_t {
    int id;
    int tlb;
    // virtual address moved to TLB like the rest, in the future it probably comes back here though
    // int virtual_addr; 
    int physical_addr;
    int cluster_id;
    int mem_tile;
    size_t size;
    int unit_id;
    std::vector<int> task_events;
    int termination_event;
    std::string image_path;
} gn_kernel;

typedef struct gn_buffer_t {
    int id;
    int physical_addr;
    int cluster_id;
    int mem_tile;
    size_t size;
    std::vector<int> kernels_in;
    std::vector<int> kernels_out;
    int event;
} gn_buffer;

typedef struct gn_event_t {
    int id;
    int physical_addr;
    int cluster_id;
    std::vector<int> kernels_in;
    std::vector<int> kernels_out;
} gn_event;

}

#endif