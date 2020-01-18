#pragma once

#include "parse_address.h"
#include "consume_number.h"
#include <upnp/third_party/net.h>

namespace upnp { namespace str {

template<class Proto /* one of asio::ip::{tcp,udp} */>
inline
optional<typename Proto::endpoint>
consume_endpoint(string_view& s)
{
    string_view s_orig = s;

    using namespace std;
    auto pos = s.rfind(':');

    if (pos == string::npos) {
        return none;
    }

    auto addr = parse_address(s.substr(0, pos));

    if (!addr) return none;

    s = s.substr(pos+1);

    auto opt_port = consume_number<uint16_t>(s);

    if (!opt_port) {
        s = s_orig;
        return none;
    }

    return typename Proto::endpoint{move(*addr), *opt_port};
}

}} // namespaces
