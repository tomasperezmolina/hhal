#ifndef HN_TYPES_H
#define HN_TYPES_H

namespace hhal {

typedef struct hn_kernel_t {
    int id;
    int tlb;
    int virtual_addr;
    int physical_addr;
    int cluster_id;
    int mem_tile;
    size_t size;
} hn_kernel;

typedef struct hn_buffer_t {
    int id;
    int physical_addr;
    int cluster_id;
    int mem_tile;
    size_t size;
} hn_buffer;

typedef struct hn_event_t {
    int id;
    int physical_addr;
    int cluster_id;
} hn_event;

}

#endif