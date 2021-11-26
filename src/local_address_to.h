#pragma once

#include <boost/asio/ip/address.hpp>
#include <boost/optional.hpp>
#include <boost/asio/spawn.hpp>
#include <upnp/third_party/error_code.h>
#include <upnp/third_party/net.h>

namespace upnp {

template<class Proto /* net::ip::{tcp,udp} */>
inline
boost::optional<net::ip::address> local_address_to(
        net::executor& exec,
        net::ip::basic_endpoint<Proto> ep,
        net::yield_context yield)
{
    auto ipv = ep.address().is_v4() ? Proto::v4() : Proto::v6();
    typename Proto::socket s(exec, ipv);
    error_code ec;
    s.async_connect(ep, yield[ec]);
    if (ec) return boost::none;
    return s.local_endpoint().address();
}

} // namespace
