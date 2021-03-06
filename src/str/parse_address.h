#pragma once

#include <boost/asio/ip/address.hpp>
#include <upnp/third_party/error_code.h>
#include <upnp/third_party/string_view.h>
#include <upnp/third_party/optional.h>
#include <upnp/third_party/net.h>

namespace upnp { namespace str {

inline
optional<net::ip::address>
parse_address(string_view s)
{
    error_code ec;
#if 0
    // TODO: There is some issue with Boost.Asio not defining the make_address
    // for string_view (Boost 1.71, not sure about 1.72) which results in
    // undefined reference.
    std::string_view std_s(s.data(), pos);
    auto addr = net::ip::make_address(std_s, ec);
#else
    auto addr = net::ip::make_address(s.to_string(), ec);
#endif

    if (ec) return none;
    return addr;
}

}} // namespaces
