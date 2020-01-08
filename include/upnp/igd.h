#pragma once

#include <upnp/detail/namespaces.h>
#include <upnp/core/result.h>
#include <upnp/core/string_view.h>
#include <upnp/device.h>
#include <boost/asio/spawn.hpp>

namespace upnp {

// Internet Gateway Device
class igd {
public:
    static
    result<std::vector<igd>> discover(net::executor, net::yield_context);

    void add_port_mapping( uint16_t external_port
                         , uint16_t internal_port
                         , string_view description
                         , std::chrono::seconds duration
                         , net::yield_context yield);

private:
    igd( std::string   uuid
       , device        upnp_device
       , std::string   service_id
       , url_t         url
       , std::string   urn
       , net::executor exec);

private:
    std::string   _uuid;
    device        _upnp_device;
    std::string   _service_id;
    url_t         _url;
    std::string   _urn;
    net::executor _exec;
};

} // namespace upnp

#include <upnp/impl/igd.ipp>
