#ifndef GN_TYPES_H
#define GN_TYPES_H

namespace hhal {

typedef struct gn_kernel_t {
    int id;
    int tlb;
    int virtual_addr;
    int physical_addr;
    int cluster_id;
    int mem_tile;
    size_t size;
    int unit_id;
} gn_kernel;

typedef struct gn_buffer_t {
    int id;
    int physical_addr;
    int cluster_id;
    int mem_tile;
    size_t size;
} gn_buffer;

typedef struct gn_event_t {
    int id;
    int physical_addr;
    int cluster_id;
} gn_event;

}

#endif