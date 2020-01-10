#pragma once

#include <upnp/detail/namespaces.h>
#include <upnp/detail/cancel.h>
#include <upnp/core/result.h>
#include <upnp/core/string_view.h>
#include <upnp/core/variant.h>
#include <upnp/core/beast.h>
#include <upnp/device.h>
#include <boost/asio/spawn.hpp>

namespace upnp {

// Internet Gateway Device
class igd final {
public:
    struct error {
        struct aborted {};
        struct igd_host_parse_failed {};
        struct no_endpoint_to_igd {};
        struct cant_connect {};
        struct cant_send {};
        struct cant_receive {};
        struct bad_response_status {
            beast::http::status status;
        };

        using add_port_mapping = variant<
            aborted,
            igd_host_parse_failed,
            no_endpoint_to_igd,
            cant_connect,
            cant_send,
            cant_receive,
            bad_response_status
        >;

        friend std::ostream& operator<<(std::ostream& os, const add_port_mapping& e) {
            using boost::get;
            if (get<aborted>(&e)) {
                os << "operation aborted";
            } else
            if (get<igd_host_parse_failed>(&e)) {
                os << "failed to parse IGD host";
            } else
            if (get<no_endpoint_to_igd>(&e)) {
                os << "no suitable endpoint to IGD";
            } else
            if (get<cant_connect>(&e)) {
                os << "can't connect to IGD";
            } else
            if (get<cant_send>(&e)) {
                os << "can't send request to IGD";
            } else
            if (get<cant_receive>(&e)) {
                os << "can't receive response from IGD";
            } else
            if (auto p = get<bad_response_status>(&e)) {
                os << "IGD resonded with non OK status " << p->status;
            } else {
                os << "Unknown error";
            }
            return os;
        }
    };

public:
    igd(igd&&)            = default;
    igd& operator=(igd&&) = default;

    static
    result<std::vector<igd>> discover(net::executor, net::yield_context);

    /*
     *
     * This text https://tools.ietf.org/html/rfc6886#section-9.5 states that
     * setting @duration to != 0 may be a bad idea, although there seem to be
     * projects that use non zero values as a default and fall back to zero
     * (meaning maximum) if that fails. e.g.
     *
     * https://github.com/syncthing/syncthing/blob/119d76d0/lib/upnp/igd_service.go#L75-L77
     *
     */
    result< void
          , error::add_port_mapping
          >
    add_port_mapping( uint16_t external_port
                    , uint16_t internal_port
                    , string_view description
                    , std::chrono::seconds duration
                    , net::yield_context yield) noexcept;

    void stop();

    ~igd() { stop(); }

private:
    igd( std::string   uuid
       , device        upnp_device
       , std::string   service_id
       , url_t         url
       , std::string   urn
       , net::executor exec);

    static
    result<device>
    query_root_device(net::executor, const url_t&, net::yield_context) noexcept;

private:
    std::string   _uuid;
    device        _upnp_device;
    std::string   _service_id;
    url_t         _url;
    std::string   _urn;
    net::executor _exec;
    Cancel        _cancel;
};

} // namespace upnp

#include <upnp/impl/igd.ipp>
