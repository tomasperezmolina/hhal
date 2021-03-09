#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "hhal.h"

namespace hhal_daemon {
    struct serialized_object {
        void *buf;
        size_t size;
    };

    serialized_object serialize(const hhal::Arguments &arguments);
    serialized_object serialize(const std::map<hhal::Unit, std::string> &kernel_images);
    serialized_object serialize(const hhal::gn_kernel &kernel);
    serialized_object serialize(const hhal::gn_buffer &buffer);
    serialized_object serialize(const hhal::gn_event &event);
    serialized_object serialize(const hhal::nvidia_buffer &buffer);

    hhal::Arguments deserialize_arguments(const serialized_object &obj);
    std::map<hhal::Unit, std::string> deserialize_kernel_images(const serialized_object &obj);
    hhal::gn_kernel deserialize_gn_kernel(const serialized_object &obj);
    hhal::gn_buffer deserialize_gn_buffer(const serialized_object &obj);
    hhal::gn_event deserialize_gn_event(const serialized_object &obj);
    hhal::nvidia_buffer deserialize_nvidia_buffer(const serialized_object &obj);
}

#endif