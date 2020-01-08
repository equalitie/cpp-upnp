#pragma once

#include <boost/asio/ip/address.hpp>
#include <boost/optional.hpp>
#include <upnp/core/error_code.h>

namespace upnp {

template<class Proto /* net::ip::{tcp,udp} */>
inline
boost::optional<net::ip::address> local_address_to(net::ip::basic_endpoint<Proto> ep) {
    net::io_context ctx;
    auto ipv = ep.address().is_v4() ? Proto::v4() : Proto::v6();
    typename Proto::socket s(ctx, ipv);
    error_code ec;
    s.connect(ep, ec);
    if (ec) return boost::none;
    return s.local_endpoint().address();
}

} // namespace
