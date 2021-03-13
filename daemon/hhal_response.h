#ifndef HHAL_RESPONSE_H
#define HHAL_RESPONSE_H

#include <cstring>
#include <cinttypes>

#include "hhal.h"

namespace hhal_daemon {

enum class response_type {
    ACK,
    REGISTER_DATA,
    ERROR,
};

struct response_base {
    response_type type;
};

struct register_data_response {
    response_type type;
    uint32_t data;
};

struct error_response {
    response_type type;
    hhal::HHALExitCode error_code;
};

inline void init_ack_response(response_base &res) {
    res.type = response_type::ACK;
}

inline void init_register_data_response(register_data_response &res, uint32_t data) {
    res.type = response_type::REGISTER_DATA;
    res.data = data;
}

inline void init_error_response(error_response &res, hhal::HHALExitCode error_code) {
    res.type = response_type::ERROR;
    res.error_code = error_code;
}
}

#endif