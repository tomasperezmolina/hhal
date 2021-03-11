#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "hhal.h"

namespace hhal_daemon {
    
    struct serialized_object {
        void *buf;
        size_t size;

        serialized_object(): buf(nullptr), size(0) {}
        serialized_object(void *buf, size_t size): buf(buf), size(size) {}

        ~serialized_object() noexcept {
            if (buf != nullptr) free(buf);
        }

        serialized_object(const serialized_object &) noexcept = delete;

        serialized_object(serialized_object&& other) noexcept {
            buf = other.buf;
            size = other.size;
            other.buf = nullptr;
            other.size = 0;
        }

        serialized_object& operator=(serialized_object&& other) noexcept {
            if (this != &other) {
                free(buf);

                buf = other.buf;
                size = other.size;
                other.buf = nullptr;
                other.size = 0;
            }
            return *this;
        }
    };

    struct auxiliary_allocations {
        std::vector<void *> allocations;

        auxiliary_allocations() {}

        ~auxiliary_allocations() {
            for(auto &alloc: allocations) {
                free(alloc);
            }
        }
    };

    serialized_object serialize(const hhal::Arguments &arguments);
    serialized_object serialize(const std::map<hhal::Unit, std::string> &kernel_images);
    serialized_object serialize(const hhal::gn_kernel &kernel);
    serialized_object serialize(const hhal::gn_buffer &buffer);
    serialized_object serialize(const hhal::gn_event &event);
    serialized_object serialize(const hhal::nvidia_buffer &buffer);

    hhal::Arguments deserialize_arguments(const serialized_object &obj, auxiliary_allocations &allocs);
    std::map<hhal::Unit, std::string> deserialize_kernel_images(const serialized_object &obj);
    hhal::gn_kernel deserialize_gn_kernel(const serialized_object &obj);
    hhal::gn_buffer deserialize_gn_buffer(const serialized_object &obj);
    hhal::gn_event deserialize_gn_event(const serialized_object &obj);
    hhal::nvidia_buffer deserialize_nvidia_buffer(const serialized_object &obj);
}

#endif