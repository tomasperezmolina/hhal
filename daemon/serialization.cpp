#include "serialization.h"
#include <vector>
#include <cstring>

struct kernel_image_pair {
    hhal::Unit unit;
    const char *str;
};


// All these POD structs should have the same initial section as their regular counterparts.
struct gn_kernel_POD {
    int id;
    uint32_t physical_addr;
    int cluster_id;
    int mem_tile;
    size_t size;
    int event;
};

struct gn_buffer_POD {
    int id;
    uint32_t physical_addr;
    int cluster_id;
    int mem_tile;
    size_t size;
    int unit_id;
    int termination_event;
};

struct gn_event_POD {
    int id;
    uint32_t physical_addr;
    int cluster_id;
};

struct nvidia_buffer_POD {
    int id;
    int gpu_id;
    int mem_id;
    size_t size;
};

namespace hhal_daemon {
serialized_object serialize(const hhal::Arguments &arguments) {
    std::vector<hhal::arg> args = arguments.get_args();
    size_t size = sizeof(hhal::arg) * args.size();
    hhal::arg *buf = (hhal::arg *) malloc(size);
    memcpy(buf, args.data(), size);
    return {buf, size};
}

serialized_object serialize(const std::map<hhal::Unit, std::string> &kernel_images) {
    size_t size = sizeof(kernel_image_pair) * kernel_images.size();
    kernel_image_pair *buf = (kernel_image_pair *) malloc(size);
    unsigned int curr = 0;
    for(auto it = kernel_images.cbegin(); it != kernel_images.cend(); ++it) {
        buf[curr++] = {it->first, it->second.c_str()};
    }
    return {buf, size};
}

serialized_object serialize(const hhal::gn_kernel &kernel) {
    typedef decltype(kernel.task_events)::value_type t_ev_type;

    size_t t_ev_size = sizeof(t_ev_type) * kernel.task_events.size();

    // Size of POD + indicator for size of vector + vector values
    size_t size = sizeof(gn_kernel_POD) + sizeof(size_t) + t_ev_size;

    void *buf = malloc(size);
    char *curr = (char *) buf;
    memcpy(buf, &kernel, sizeof(gn_kernel_POD));
    curr += sizeof(gn_kernel_POD);
    *((size_t*) curr) = kernel.task_events.size();
    curr += sizeof(size_t);
    memcpy(curr, kernel.task_events.data(), t_ev_size);
    curr += t_ev_size;
    return {buf, size};
}

serialized_object serialize(const hhal::gn_buffer &buffer) {
    typedef decltype(buffer.kernels_in)::value_type k_in_type;
    typedef decltype(buffer.kernels_out)::value_type k_out_type;

    size_t k_in_size = sizeof(k_in_type) * buffer.kernels_in.size();
    size_t k_out_size = sizeof(k_out_type) * buffer.kernels_out.size();

    // Size of POD + indicators for size of vectors + vector values
    size_t size = sizeof(gn_buffer_POD) + sizeof(size_t) * 2 + k_in_size + k_out_size;

    void *buf = malloc(size);
    char *curr = (char *) buf;
    memcpy(buf, &buffer, sizeof(gn_buffer_POD));
    curr += sizeof(gn_buffer_POD);
    *((size_t*) curr) = buffer.kernels_in.size();
    curr += sizeof(size_t);
    *((size_t*) curr) = buffer.kernels_out.size();
    curr += sizeof(size_t);
    memcpy(curr, buffer.kernels_in.data(), k_in_size);
    curr += k_in_size;
    memcpy(curr, buffer.kernels_out.data(), k_out_size);
    curr += k_out_size;
    return {buf, size};
}

serialized_object serialize(const hhal::gn_event &event) {
    typedef decltype(event.kernels_in)::value_type k_in_type;
    typedef decltype(event.kernels_out)::value_type k_out_type;

    size_t k_in_size = sizeof(k_in_type) * event.kernels_in.size();
    size_t k_out_size = sizeof(k_out_type) * event.kernels_out.size();

    // Size of POD + indicators for size of vectors + vector values
    size_t size = sizeof(gn_event_POD) + sizeof(size_t) * 2 + k_in_size + k_out_size;

    void *buf = malloc(size);
    char *curr = (char *) buf;
    memcpy(buf, &event, sizeof(gn_event_POD));
    curr += sizeof(gn_event_POD);
    *((size_t*) curr) = event.kernels_in.size();
    curr += sizeof(size_t);
    *((size_t*) curr) = event.kernels_out.size();
    curr += sizeof(size_t);
    memcpy(curr, event.kernels_in.data(), k_in_size);
    curr += k_in_size;
    memcpy(curr, event.kernels_out.data(), k_out_size);
    curr += k_out_size;
    return {buf, size};
}

serialized_object serialize(const hhal::nvidia_buffer &buffer) {
    typedef decltype(buffer.kernels_in)::value_type k_in_type;
    typedef decltype(buffer.kernels_out)::value_type k_out_type;

    size_t k_in_size = sizeof(k_in_type) * buffer.kernels_in.size();
    size_t k_out_size = sizeof(k_out_type) * buffer.kernels_out.size();

    // Size of POD + indicators for size of vectors + vector values
    size_t size = sizeof(nvidia_buffer_POD) + sizeof(size_t) * 2 + k_in_size + k_out_size;

    void *buf = malloc(size);
    char *curr = (char *) buf;
    memcpy(buf, &buffer, sizeof(nvidia_buffer_POD));
    curr += sizeof(nvidia_buffer_POD);
    *((size_t*) curr) = buffer.kernels_in.size();
    curr += sizeof(size_t);
    *((size_t*) curr) = buffer.kernels_out.size();
    curr += sizeof(size_t);
    memcpy(curr, buffer.kernels_in.data(), k_in_size);
    curr += k_in_size;
    memcpy(curr, buffer.kernels_out.data(), k_out_size);
    curr += k_out_size;
    return {buf, size};
}

hhal::Arguments deserialize_arguments(const serialized_object &obj) {
    std::vector<hhal::arg> args((hhal::arg *) obj.buf, (hhal::arg *) obj.buf + obj.size);
    return hhal::Arguments(args);
}

std::map<hhal::Unit, std::string> deserialize_kernel_images(const serialized_object &obj) {
    std::map<hhal::Unit, std::string> res;
    kernel_image_pair *buf = (kernel_image_pair *) obj.buf;
    unsigned int size = obj.size / sizeof(kernel_image_pair);
    for(unsigned int i = 0; i < size; i++) {
        res[buf[i].unit] = std::string(buf[i].str);
    }
    return res;
}

hhal::gn_kernel deserialize_gn_kernel(const serialized_object &obj) {
    hhal::gn_kernel res;

    typedef decltype(res.task_events)::value_type t_ev_type;

    char *curr = (char *) obj.buf;
    memcpy(&res, curr, sizeof(gn_kernel_POD));
    curr += sizeof(gn_kernel_POD);
    size_t k_in_size = *((size_t* )curr);
    curr += sizeof(size_t);
    res.task_events = std::vector<t_ev_type>(
        (t_ev_type *) curr,
        (t_ev_type *) curr + k_in_size
    );
    curr += sizeof(t_ev_type) * k_in_size;
    return res;
}

hhal::gn_buffer deserialize_gn_buffer(const serialized_object &obj) {
    hhal::gn_buffer res;

    typedef decltype(res.kernels_in)::value_type k_in_type;
    typedef decltype(res.kernels_out)::value_type k_out_type;

    char *curr = (char *) obj.buf;
    memcpy(&res, curr, sizeof(gn_buffer_POD));
    curr += sizeof(gn_buffer_POD);
    size_t k_in_size = *((size_t* )curr);
    curr += sizeof(size_t);
    size_t k_out_size = *((size_t* )curr);
    curr += sizeof(size_t);
    res.kernels_in = std::vector<k_in_type>(
        (k_in_type *) curr,
        (k_in_type *) curr + k_in_size
    );
    curr += sizeof(k_in_type) * k_in_size;
    res.kernels_out = std::vector<k_out_type>(
        (k_out_type *) curr,
        (k_out_type *) curr + k_in_size
    );
    return res;
}

hhal::gn_event deserialize_gn_event(const serialized_object &obj) {
    hhal::gn_event res;

    typedef decltype(res.kernels_in)::value_type k_in_type;
    typedef decltype(res.kernels_out)::value_type k_out_type;

    char *curr = (char *) obj.buf;
    memcpy(&res, curr, sizeof(gn_event_POD));
    curr += sizeof(gn_event_POD);
    size_t k_in_size = *((size_t* )curr);
    curr += sizeof(size_t);
    size_t k_out_size = *((size_t* )curr);
    curr += sizeof(size_t);
    res.kernels_in = std::vector<k_in_type>(
        (k_in_type *) curr,
        (k_in_type *) curr + k_in_size
    );
    curr += sizeof(k_in_type) * k_in_size;
    res.kernels_out = std::vector<k_out_type>(
        (k_out_type *) curr,
        (k_out_type *) curr + k_in_size
    );
    return res;
}

hhal::nvidia_buffer deserialize_nvidia_buffer(const serialized_object &obj) {
    hhal::nvidia_buffer res;

    typedef decltype(res.kernels_in)::value_type k_in_type;
    typedef decltype(res.kernels_out)::value_type k_out_type;

    char *curr = (char *) obj.buf;
    memcpy(&res, curr, sizeof(nvidia_buffer_POD));
    curr += sizeof(nvidia_buffer_POD);
    size_t k_in_size = *((size_t* )curr);
    curr += sizeof(size_t);
    size_t k_out_size = *((size_t* )curr);
    curr += sizeof(size_t);
    res.kernels_in = std::vector<k_in_type>(
        (k_in_type *) curr,
        (k_in_type *) curr + k_in_size
    );
    curr += sizeof(k_in_type) * k_in_size;
    res.kernels_out = std::vector<k_out_type>(
        (k_out_type *) curr,
        (k_out_type *) curr + k_in_size
    );
    return res;
}

}