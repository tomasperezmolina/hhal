#include "arguments.h"
#include <assert.h>

namespace hhal {

void *get_scalar_ptr(const scalar_arg *scalar) {
    switch (scalar->type) {
        case ScalarType::INT: {
            switch (scalar->size) {
                case sizeof(int8_t):
                    return (void*)&scalar->aint8;
                case sizeof(int16_t):
                    return (void*)&scalar->aint16;
                case sizeof(int32_t):
                    return (void*)&scalar->aint32;
                case sizeof(int64_t):
                    return (void*)&scalar->aint64;
                default:
                    assert(false && "Unknown scalar int size");
            }
        }
        case ScalarType::UINT: {
            switch (scalar->size) {
                case sizeof(uint8_t):
                    return (void*)&scalar->uint8;
                case sizeof(uint16_t):
                    return (void*)&scalar->uint16;
                case sizeof(uint32_t):
                    return (void*)&scalar->uint32;
                case sizeof(uint64_t):
                    return (void*)&scalar->uint64;
                default:
                    assert(false && "Unknown scalar uint size");
            }
        }
        case ScalarType::FLOAT: {
            switch (scalar->size) {
                case sizeof(float):
                    return (void*)&scalar->afloat;
                case sizeof(double):
                    return (void*)&scalar->adouble;
                default:
                    assert(false && "Unknown scalar float size");
            }
        }
    }
}

}
