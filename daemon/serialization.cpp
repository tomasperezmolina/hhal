#include "serialization.h"
#include <vector>
#include <cstring>
#include <assert.h>
#include <stdlib.h>

// All these POD structs should have the same initial section as their regular counterparts.
struct gn_kernel_POD {
    int id;
    int termination_event;
};

struct gn_buffer_POD {
    int id;
    size_t size;
    int event;
};

struct gn_event_POD {
    int id;
};

struct nvidia_buffer_POD {
    int id;
    int gpu_id;
    int mem_id;
    size_t size;
};

namespace hhal_daemon {
serialized_object serialize(const hhal::Arguments &arguments) {
    auto &args = arguments.get_args();
    size_t size = 0;
    for (int i = 0; i < args.size(); i++) {
        size += sizeof(hhal::ArgumentType);
        switch (args[i].type) {
            case hhal::ArgumentType::SCALAR: {
                auto &scalar = args[i].scalar;
                size += sizeof(hhal::ScalarType);
                size += sizeof(size_t);
                size += scalar.size;
                break;
            }
            case hhal::ArgumentType::BUFFER: {
                auto &buffer = args[i].buffer;
                size += sizeof(decltype(buffer.id));
                break;
            }
            case hhal::ArgumentType::EVENT: {
                auto &event = args[i].event;
                size += sizeof(decltype(event.id));
                break;
            }
        }
    }
    void *buf = malloc(size);
    char *curr = (char*) buf;
    for (int i = 0; i < args.size(); i++) {
        memcpy(curr, &args[i].type, sizeof(hhal::ArgumentType));
        curr += sizeof(hhal::ArgumentType);
        switch (args[i].type) {
            case hhal::ArgumentType::SCALAR: {
                const hhal::scalar_arg scalar = args[i].scalar;
                memcpy(curr, &scalar.type, sizeof(hhal::ScalarType));
                curr += sizeof(hhal::ScalarType);
                memcpy(curr, &scalar.size, sizeof(size_t));
                curr += sizeof(size_t);
                //memcpy(curr, scalar.address, scalar.size);

                void *scalar_ptr = get_scalar_ptr(&scalar);
                memcpy(curr, scalar_ptr, scalar.size);
                curr += scalar.size;
                break;
            }
            case hhal::ArgumentType::BUFFER: {
                auto &buffer = args[i].buffer;
                memcpy(curr, &buffer.id, sizeof(decltype(buffer.id)));
                curr += sizeof(decltype(buffer.id));
                break;
            }
            case hhal::ArgumentType::EVENT: {
                auto &event = args[i].event;
                memcpy(curr, &event.id, sizeof(decltype(event.id)));
                curr += sizeof(decltype(event.id));
                break;
            }
        }
    }
    assert(curr - (char*) buf == size && "Allocated space is different from the written data size");

    return {buf, size};
}

serialized_object serialize(const std::map<hhal::Unit, std::string> &kernel_images) {
    size_t unit_size = sizeof(hhal::Unit);

    size_t size = 0;
    auxiliary_allocations allocs;
    std::vector<size_t> full_path_sizes;
    for(auto it = kernel_images.cbegin(); it != kernel_images.cend(); ++it) {
        size += unit_size;
        char *full_path = realpath(it->second.c_str(), nullptr);
        size_t full_path_size = strlen(full_path) + 1;
        size += full_path_size;

        allocs.allocations.push_back(full_path);
        full_path_sizes.push_back(full_path_size);
    }
    void *buf = malloc(size);
    char *curr = (char*) buf;
    int i = 0;
    for(auto it = kernel_images.cbegin(); it != kernel_images.cend(); ++it) {
        hhal::Unit unit = it->first;
        memcpy(curr, &unit, unit_size);
        curr += unit_size;
        size_t str_size = full_path_sizes[i];
        memcpy(curr, allocs.allocations[i], str_size);
        curr += str_size;
        i++;
    }
    assert(curr - (char*) buf == size && "Allocated space is different from the written data size");
    return {buf, size};
}

serialized_object serialize(const hhal::gn_kernel &kernel) {
    typedef decltype(kernel.task_events)::value_type t_ev_type;

    size_t t_ev_amount = kernel.task_events.size();
    size_t t_ev_size = sizeof(t_ev_type) * t_ev_amount;

    // Size of POD + indicator for size of vector + vector values
    size_t size = sizeof(gn_kernel_POD) + sizeof(size_t) + t_ev_size;

    void *buf = malloc(size);
    char *curr = (char *) buf;
    memcpy(curr, &kernel, sizeof(gn_kernel_POD));
    curr += sizeof(gn_kernel_POD);
    memcpy(curr, &t_ev_amount, sizeof(size_t));
    curr += sizeof(size_t);
    memcpy(curr, kernel.task_events.data(), t_ev_size);
    curr += t_ev_size;
    assert(curr - (char*) buf == size && "Allocated space is different from the written data size");
    return {buf, size};
}

serialized_object serialize(const hhal::gn_buffer &buffer) {
    typedef decltype(buffer.kernels_in)::value_type k_in_type;
    typedef decltype(buffer.kernels_out)::value_type k_out_type;


    size_t k_in_amount = buffer.kernels_in.size();
    size_t k_out_amount = buffer.kernels_out.size();
    size_t k_in_size = sizeof(k_in_type) * k_in_amount;
    size_t k_out_size = sizeof(k_out_type) * k_out_amount;

    // Size of POD + indicators for size of vectors + vector values
    size_t size = sizeof(gn_buffer_POD) + sizeof(size_t) * 2 + k_in_size + k_out_size;

    void *buf = malloc(size);
    char *curr = (char *) buf;
    memcpy(curr, &buffer, sizeof(gn_buffer_POD));
    curr += sizeof(gn_buffer_POD);
    memcpy(curr, &k_in_amount, sizeof(size_t));
    curr += sizeof(size_t);
    memcpy(curr, &k_out_amount, sizeof(size_t));
    curr += sizeof(size_t);
    memcpy(curr, buffer.kernels_in.data(), k_in_size);
    curr += k_in_size;
    memcpy(curr, buffer.kernels_out.data(), k_out_size);
    curr += k_out_size;
    assert(curr - (char*) buf == size && "Allocated space is different from the written data size");
    return {buf, size};
}

serialized_object serialize(const hhal::gn_event &event) {
    typedef decltype(event.kernels_in)::value_type k_in_type;
    typedef decltype(event.kernels_out)::value_type k_out_type;

    size_t k_in_amount = event.kernels_in.size();
    size_t k_out_amount = event.kernels_out.size();
    size_t k_in_size = sizeof(k_in_type) * k_in_amount;
    size_t k_out_size = sizeof(k_out_type) * k_out_amount;

    // Size of POD + indicators for size of vectors + vector values
    size_t size = sizeof(gn_event_POD) + sizeof(size_t) * 2 + k_in_size + k_out_size;

    void *buf = malloc(size);
    char *curr = (char *) buf;
    memcpy(curr, &event, sizeof(gn_event_POD));
    curr += sizeof(gn_event_POD);
    memcpy(curr, &k_in_amount, sizeof(size_t));
    curr += sizeof(size_t);
    memcpy(curr, &k_out_amount, sizeof(size_t));
    curr += sizeof(size_t);
    memcpy(curr, event.kernels_in.data(), k_in_size);
    curr += k_in_size;
    memcpy(curr, event.kernels_out.data(), k_out_size);
    curr += k_out_size;
    assert(curr - (char*) buf == size && "Allocated space is different from the written data size");
    return {buf, size};
}

serialized_object serialize(const hhal::nvidia_buffer &buffer) {
    typedef decltype(buffer.kernels_in)::value_type k_in_type;
    typedef decltype(buffer.kernels_out)::value_type k_out_type;

    size_t k_in_amount = buffer.kernels_in.size();
    size_t k_out_amount = buffer.kernels_out.size();
    size_t k_in_size = sizeof(k_in_type) * k_in_amount;
    size_t k_out_size = sizeof(k_out_type) * k_out_amount;

    // Size of POD + indicators for size of vectors + vector values
    size_t size = sizeof(nvidia_buffer_POD) + sizeof(size_t) * 2 + k_in_size + k_out_size;

    void *buf = malloc(size);
    char *curr = (char *) buf;
    memcpy(curr, &buffer, sizeof(nvidia_buffer_POD));
    curr += sizeof(nvidia_buffer_POD);
    memcpy(curr, &k_in_amount, sizeof(size_t));
    curr += sizeof(size_t);
    memcpy(curr, &k_out_amount, sizeof(size_t));
    curr += sizeof(size_t);
    memcpy(curr, buffer.kernels_in.data(), k_in_size);
    curr += k_in_size;
    memcpy(curr, buffer.kernels_out.data(), k_out_size);
    curr += k_out_size;
    assert(curr - (char*) buf == size && "Allocated space is different from the written data size");
    return {buf, size};
}

hhal::Arguments deserialize_arguments(const serialized_object &obj, auxiliary_allocations &allocs) {
    hhal::Arguments result;
    char *curr = (char*) obj.buf;
    char *end = (char*) obj.buf + obj.size;
    while (curr != end) {
        hhal::ArgumentType arg_type;
        memcpy(&arg_type, curr, sizeof(hhal::ArgumentType));
        curr += sizeof(hhal::ArgumentType);
        switch (arg_type) {
            case hhal::ArgumentType::SCALAR: {
                hhal::scalar_arg scalar;
                memcpy(&scalar.type, curr, sizeof(hhal::ScalarType));                
                curr += sizeof(hhal::ScalarType);
                memcpy(&scalar.size, curr, sizeof(size_t));
                curr += sizeof(size_t);
                //scalar.address = malloc(scalar.size);
                //allocs.allocations.push_back(scalar.address);
                //memcpy(scalar.address, curr, scalar.size);

                void *scalar_ptr = get_scalar_ptr(&scalar);
                memcpy(scalar_ptr, curr, scalar.size);
                curr += scalar.size;
                result.add_scalar(scalar);
                break;
            }
            case hhal::ArgumentType::BUFFER: {
                hhal::buffer_arg buffer;
                memcpy(&buffer.id, curr, sizeof(decltype(buffer.id)));
                curr += sizeof(decltype(buffer.id));
                result.add_buffer(buffer);
                break;
            }
            case hhal::ArgumentType::EVENT: {
                hhal::event_arg event;
                memcpy(&event.id, curr, sizeof(decltype(event.id)));
                curr += sizeof(decltype(event.id));
                result.add_event(event);
                break;
            }
        }
    }
    return result;
}

std::map<hhal::Unit, std::string> deserialize_kernel_images(const serialized_object &obj) {
    std::map<hhal::Unit, std::string> res;
    char *curr = (char*) obj.buf;
    char *end = (char*) obj.buf + obj.size;
    while (curr != end) {
        hhal::Unit unit;
        memcpy(&unit, curr, sizeof(hhal::Unit));
        curr += sizeof(hhal::Unit);
        std::string path(curr);
        curr += path.size() + 1;
        res[unit] = path;
    }
    assert(curr - (char*) obj.buf == obj.size && "Allocated space is different from the read data size");
    return res;
}

hhal::gn_kernel deserialize_gn_kernel(const serialized_object &obj) {
    hhal::gn_kernel res;

    typedef decltype(res.task_events)::value_type t_ev_type;

    size_t t_ev_amount;
    char *curr = (char *) obj.buf;
    memcpy(&res, curr, sizeof(gn_kernel_POD));
    curr += sizeof(gn_kernel_POD);
    memcpy(&t_ev_amount, curr, sizeof(size_t));
    curr += sizeof(size_t);
    res.task_events = std::vector<t_ev_type>(
        (t_ev_type *) curr,
        (t_ev_type *) curr + t_ev_amount
    );
    curr += sizeof(t_ev_type) * t_ev_amount;
    assert(curr - (char*) obj.buf == obj.size && "Allocated space is different from the read data size");
    return res;
}

hhal::gn_buffer deserialize_gn_buffer(const serialized_object &obj) {
    hhal::gn_buffer res;

    typedef decltype(res.kernels_in)::value_type k_in_type;
    typedef decltype(res.kernels_out)::value_type k_out_type;

    size_t k_in_amount;
    size_t k_out_amount;

    char *curr = (char *) obj.buf;
    memcpy(&res, curr, sizeof(gn_buffer_POD));
    curr += sizeof(gn_buffer_POD);
    memcpy(&k_in_amount, curr, sizeof(size_t));
    curr += sizeof(size_t);
    memcpy(&k_out_amount, curr, sizeof(size_t));
    curr += sizeof(size_t);
    res.kernels_in = std::vector<k_in_type>(
        (k_in_type *) curr,
        (k_in_type *) curr + k_in_amount
    );
    curr += sizeof(k_in_type) * k_in_amount;
    res.kernels_out = std::vector<k_out_type>(
        (k_out_type *) curr,
        (k_out_type *) curr + k_out_amount
    );
    curr += sizeof(k_out_type) * k_out_amount;
    assert(curr - (char*) obj.buf == obj.size && "Allocated space is different from the read data size");
    return res;
}

hhal::gn_event deserialize_gn_event(const serialized_object &obj) {
    hhal::gn_event res;

    typedef decltype(res.kernels_in)::value_type k_in_type;
    typedef decltype(res.kernels_out)::value_type k_out_type;

    size_t k_in_amount;
    size_t k_out_amount;

    char *curr = (char *) obj.buf;
    memcpy(&res, curr, sizeof(gn_event_POD));
    curr += sizeof(gn_event_POD);
    memcpy(&k_in_amount, curr, sizeof(size_t));
    curr += sizeof(size_t);
    memcpy(&k_out_amount, curr, sizeof(size_t));
    curr += sizeof(size_t);
    res.kernels_in = std::vector<k_in_type>(
        (k_in_type *) curr,
        (k_in_type *) curr + k_in_amount
    );
    curr += sizeof(k_in_type) * k_in_amount;
    res.kernels_out = std::vector<k_out_type>(
        (k_out_type *) curr,
        (k_out_type *) curr + k_out_amount
    );
    curr += sizeof(k_out_type) * k_out_amount;
    assert(curr - (char*) obj.buf == obj.size && "Allocated space is different from the read data size");
    return res;
}

hhal::nvidia_buffer deserialize_nvidia_buffer(const serialized_object &obj) {
    hhal::nvidia_buffer res;

    typedef decltype(res.kernels_in)::value_type k_in_type;
    typedef decltype(res.kernels_out)::value_type k_out_type;

    size_t k_in_amount;
    size_t k_out_amount;

    char *curr = (char *) obj.buf;
    memcpy(&res, curr, sizeof(nvidia_buffer_POD));
    curr += sizeof(nvidia_buffer_POD);
    memcpy(&k_in_amount, curr, sizeof(size_t));
    curr += sizeof(size_t);
    memcpy(&k_out_amount, curr, sizeof(size_t));
    curr += sizeof(size_t);
    res.kernels_in = std::vector<k_in_type>(
        (k_in_type *) curr,
        (k_in_type *) curr + k_in_amount
    );
    curr += sizeof(k_in_type) * k_in_amount;
    res.kernels_out = std::vector<k_out_type>(
        (k_out_type *) curr,
        (k_out_type *) curr + k_out_amount
    );
    curr += sizeof(k_out_type) * k_out_amount;
    assert(curr - (char*) obj.buf == obj.size && "Allocated space is different from the read data size");
    return res;
}

}
