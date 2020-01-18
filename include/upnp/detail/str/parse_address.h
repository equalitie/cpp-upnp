#pragma once

#include <boost/asio/ip/address.hpp>
#include <upnp/core/error_code.h>
#include <upnp/core/string_view.h>
#include <upnp/core/optional.h>
#include <upnp/detail/namespaces.h>

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
    return std::move(addr);
}

}} // namespaces
