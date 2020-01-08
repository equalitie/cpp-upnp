#pragma once

#include <upnp/core/error_code.h>

namespace upnp {

enum class SsdpError {
    failed_to_parse = 1,
    non_ok_result
};

} // upnp namespace
