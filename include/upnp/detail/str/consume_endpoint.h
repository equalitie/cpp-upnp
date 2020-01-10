#pragma once

#include <boost/asio/ip/address.hpp>
#include <upnp/core/string_view.h>
#include <upnp/core/optional.h>
#include <upnp/detail/str/consume_number.h>
#include <upnp/detail/namespaces.h>

namespace upnp { namespace str {

template<class Proto /* one of asio::ip::{tcp,udp} */>
inline
optional<typename Proto::endpoint>
consume_endpoint(string_view& s)
{
    string_view s_orig = s;

    using namespace std;
    auto pos = s.rfind(':');

    error_code ec;

    if (pos == string::npos) {
        return none;
    }

#if 0
    // TODO: There is some issue with Boost.Asio not defining the make_address
    // for string_view (Boost 1.71, not sure about 1.72) which results in
    // undefined reference.
    std::string_view std_s(s.data(), pos);
    auto addr = net::ip::make_address(std_s, ec);
#else
    auto addr = net::ip::make_address(s.substr(0, pos).to_string(), ec);
#endif

    if (ec) return none;

    s = s.substr(pos+1);

    auto opt_port = consume_number<uint16_t>(s);

    if (!opt_port) {
        s = s_orig;
        return none;
    }

    return typename Proto::endpoint{move(addr), *opt_port};
}

}} // namespaces
