#ifndef HHAL_ARGUMENTS_H
#define HHAL_ARGUMENTS_H

#include <vector>
#include "types.h"
#include "stdint.h"

namespace hhal {

enum class ArgumentType {
    BUFFER,
    EVENT,
    SCALAR,
};

typedef struct buffer_arg_t {
    int id;
} buffer_arg;

typedef struct event_arg_t {
    int id;
} event_arg;

enum class ScalarType {
    INT,    // any size integer value
    UINT,    // any size unsigned integer value
    FLOAT,  // any size floating point value
};

typedef struct scalar_arg_t {
    ScalarType type;
    size_t size;
    union {
       int8_t  aint8; 
       int16_t aint16; 
       int32_t aint32; 
       int64_t aint64; 
       uint8_t  uint8;
       uint16_t uint16;
       uint32_t uint32;
       uint64_t uint64;

       float   afloat;
       double   adouble;
    };
} scalar_arg;

void *get_scalar_ptr(const scalar_arg *scalar);

typedef struct arg_t {
    ArgumentType type;
    union { 
        buffer_arg buffer; 
        event_arg event; 
        scalar_arg scalar; 
    };
} arg;

class Arguments {
    public:
        Arguments() {};
        Arguments(std::vector<arg> args) {
            this->args = args;
        }

        inline void add_event(const event_arg &event) { 
            arg a;
            a.type = ArgumentType::EVENT;
            a.event = event;
            args.push_back(a);
        }

        inline void add_buffer(const buffer_arg &buffer) { 
            arg a;
            a.type = ArgumentType::BUFFER;
            a.buffer = buffer;
            args.push_back(a);
        }

        inline void add_scalar(const scalar_arg &scalar) {
            arg a;
            a.type = ArgumentType::SCALAR;
            a.scalar = scalar;
            args.push_back(a);
        }

        inline void add_arguments(const Arguments &other_args) {
            args.insert(args.end(), other_args.args.begin(), other_args.args.end());
        }

        inline const std::vector<arg>& get_args() const {
            return args;
    }
    
    private:
        std::vector<arg> args;
};
}

#endif
