#ifndef HHAL_RESPONSE_H
#define HHAL_RESPONSE_H

#include <cstring>
#include <cinttypes>

#include "hhal.h"

namespace hhal_daemon {

enum class response_type {
    ACK,
    REGISTER_DATA,
};

struct response_base {
    response_type res;
};

struct register_data_response {
    response_type res;
    uint32_t data;
};

inline void init_ack_response(response_base &res) {
    res.res = response_type::ACK;
}

inline void init_register_data_response(register_data_response &res, uint32_t data) {
    res.res = response_type::REGISTER_DATA;
    res.data = data;
}
}

#endif