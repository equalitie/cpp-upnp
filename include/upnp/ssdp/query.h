#pragma once

#include <upnp/core/result.h>
#include <upnp/core/optional.h>
#include <upnp/url.h>
#include <upnp/detail/namespaces.h>
#include <upnp/detail/condition_variable.h>
#include <upnp/detail/str/consume_until.h>
#include <upnp/detail/str/consume_endpoint.h>
#include <upnp/detail/str/istarts_with.h>
#include <upnp/detail/str/trim.h>
#include <chrono>
#include <boost/asio/ip/multicast.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>

namespace upnp { namespace ssdp {

class query {
public:
    struct response {
        std::string uuid;
        url_t location;

        static result<response> parse(string_view);

        friend std::ostream& operator<<(std::ostream& os, const response& r)
        {
            return os << "(uuid:" << r.uuid << " location:" << r.location << ")";
        }
    };

public:
    query(net::executor e) : _exec(e) {}

    result<response> get_one(net::yield_context);

private:
    net::executor _exec;
};

inline
result<query::response> query::get_one(net::yield_context yield)
{
    using udp = net::ip::udp;

    udp::socket s(_exec, udp::v4());
    s.set_option(udp::socket::reuse_address(true));

    error_code ec;

    response ret;

    // https://www.grc.com/port_1900.htm
    udp::endpoint multicast_ep(net::ip::address_v4({239, 255, 255, 250}), 1900);

    s.bind(udp::endpoint(net::ip::address_v4::any(), 0), ec);

    if (ec) return ec;

    std::stringstream ss;

    //const char* device = "urn:schemas-upnp-org:device:InternetGatewayDevice:1";
    const char* device = "urn:schemas-upnp-org:device:InternetGatewayDevice:2";

    ss << "M-SEARCH * HTTP/1.1\r\n"
       << "HOST: " << multicast_ep << "\r\n"
       << "ST: " << device << "\r\n"
       << "MAN: \"ssdp:discover\"\r\n"
       << "MX: 60\r\n"
       << "USER-AGENT: asio-upnp/1.0\r\n";

    auto sss = ss.str();

    ConditionVariable cv(_exec);
    bool done = false;

    s.set_option(net::ip::multicast::join_group(multicast_ep.address()));

    error_code rx_ec;

    net::spawn(_exec, [&] (auto y) {
        std::array<char, 32*1024> rx;
        size_t size = s.async_receive_from(net::buffer(rx.data(), rx.size()), multicast_ep, y[rx_ec]);
        done = true;
        cv.notify();
        if (rx_ec) return;
        auto r = response::parse(string_view(rx.data(), size));
        if (r) {
            ret = std::move(r.value());
        } else {
            rx_ec = r.error();
        }
        return;
    });

    error_code tx_ec;
    s.async_send_to(net::buffer(sss.data(), sss.size()), multicast_ep, yield[tx_ec]);

    if (!done) cv.wait(yield[ec]);

    if (tx_ec) return tx_ec;
    if (rx_ec) return rx_ec;

    return std::move(ret);
}

/* static */
inline
result<query::response> query::response::parse(string_view lines)
{
    size_t line_n = 0;

    response ret;

    while (auto opt_line = str::consume_until(lines, {"\r\n", "\n"})) {
        auto line = *opt_line;

        if (line_n++ == 0) {
            if (!str::istarts_with(line, "http")) {
                return std::move(ret);
            }
            str::consume_until(line, " ");
            str::trim_space_prefix(line);
            auto result = str::consume_until(line, " ");
            if (!result || *result != "200") {
                return boost::system::errc::invalid_argument;
            }
            continue;
        }

        auto opt_key = str::consume_until(line, ":");
        if (!opt_key) break;

        auto key = *opt_key;
        auto val = line;
    
        str::trim_space_prefix(val);
        str::trim_space_suffix(val);

        if (boost::iequals(key, "USN")) {
            while (auto opt_token = str::consume_until(val, ":")) {
                if (boost::iequals(*opt_token, "uuid")) {
                    auto opt_uuid = str::consume_until(val, ":");
                    if (!opt_uuid) {
                        return boost::system::errc::invalid_argument;
                    }
                    ret.uuid = opt_uuid->to_string();
                }
            }
        } 
        if (boost::iequals(key, "LOCATION")) {
            auto location = url_t::parse(val.to_string());
            if (!location) return sys::errc::invalid_argument;
            ret.location = std::move(*location);
        }
    }

    return std::move(ret);
}

}} // namespace upnp::ssdp
