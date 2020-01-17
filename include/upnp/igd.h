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
private:
    using os_t = std::ostream;

public:
    enum protocol { tcp, udp };

    friend os_t& operator<<(os_t& os, const protocol& p) {
        return os << (p == tcp ? "TCP" : "UDP");
    }

public:
    struct error {
        struct igd_host_parse_failed {};
        struct no_endpoint_to_igd {};
        struct invalid_xml_body {};
        struct invalid_response {};
        struct bad_address {};

        struct tcp_connect{};
        struct http_request{};
        struct http_response{};
        struct http_status{
            beast::http::status status;
        };

        struct soap_request {
            variant<
                igd_host_parse_failed,
                tcp_connect,
                http_request,
                http_response,
                http_status
            > inner;
        };

        friend os_t& operator<<(os_t& os, const igd_host_parse_failed&) {
            return os << "failed to parse IGD host";
        }
        friend os_t& operator<<(os_t& os, const soap_request& r) {
            return os << "failed to do soap request: " << r.inner;
        }
        friend os_t& operator<<(os_t& os, const no_endpoint_to_igd&) {
            return os << "no suitable endpoint to IGD";
        }
        friend os_t& operator<<(os_t& os, const invalid_xml_body&) {
            return os << "failed to parse xml body";
        }
        friend os_t& operator<<(os_t& os, const invalid_response&) {
            return os << "bad result";
        }
        friend os_t& operator<<(os_t& os, const bad_address&) {
            return os << "bad address";
        }
        friend os_t& operator<<(os_t& os, const tcp_connect&) {
            return os << "tcp connect";
        }
        friend os_t& operator<<(os_t& os, const http_request&) {
            return os << "http request";
        }
        friend os_t& operator<<(os_t& os, const http_response&) {
            return os << "http response";
        }
        friend os_t& operator<<(os_t& os, const http_status& e) {
            return os << "IGD resonded with non OK status " << e.status;
        }

        using add_port_mapping = variant<
            igd_host_parse_failed,
            no_endpoint_to_igd,
            soap_request
        >;

        using get_external_address = variant<
            soap_request,
            invalid_xml_body,
            invalid_response,
            bad_address
        >;

        using get_list_of_port_mappings = variant<
            soap_request,
            invalid_xml_body,
            invalid_response
        >;

        using delete_port_mapping = variant<
            soap_request
        >;
    };

public:
    igd(igd&&)            = default;
    igd& operator=(igd&&) = default;

    const std::string& friendly_name() const { return _upnp_device.friendly_name; }

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
    result<void, error::add_port_mapping>
    add_port_mapping( protocol
                    , uint16_t external_port
                    , uint16_t internal_port
                    , string_view description
                    , std::chrono::seconds duration
                    , net::yield_context yield) noexcept;

    result<net::ip::address , error::get_external_address>
    get_external_address(net::yield_context yield) noexcept;

    struct map_entry {
        // TODO: There are others
        std::string description;
        uint16_t ext_port;
        uint16_t int_port;
        std::chrono::seconds lease_duration;
        protocol proto;
        net::ip::address int_client;
    };

    result<std::vector<map_entry>, error::get_list_of_port_mappings>
    get_list_of_port_mappings( protocol
                             , uint16_t min_port
                             , uint16_t max_port
                             , uint16_t max_count
                             , net::yield_context yield) noexcept;

    result<void, error::delete_port_mapping>
    delete_port_mapping( protocol
                       , uint16_t ext_port
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

    using soap_response = beast::http::response<beast::http::string_body>;

    result<soap_response, error::soap_request>
    soap_request( string_view command
                , string_view message
                , net::yield_context) noexcept;

private:
    std::string   _uuid;
    device        _upnp_device;
    std::string   _service_id;
    url_t         _url;
    std::string   _urn;
    net::executor _exec;
    cancel_t      _cancel;
};

} // namespace upnp

#include <upnp/impl/igd.ipp>
