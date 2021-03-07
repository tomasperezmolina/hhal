#include "serialization.h"
#include <vector>
#include <cstring>

struct kernel_image_pair {
    hhal::Unit unit;
    const char *str;
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
}