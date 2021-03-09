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

    hhal::Arguments deserialize_arguments(const serialized_object &obj);
    std::map<hhal::Unit, std::string> deserialize_kernel_images(const serialized_object &obj);
}

#endif