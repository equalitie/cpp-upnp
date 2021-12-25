#pragma once

#include <upnp/third_party/result.h>
#include <upnp/third_party/net.h>
#include <upnp/third_party/variant.h>
#include <upnp/third_party/error_code.h>
#include <upnp/url.h>

#include <boost/range/begin.hpp> // needed by spawn
#include <boost/range/end.hpp> // needed by spawn
#include <boost/asio/spawn.hpp>

namespace upnp { namespace ssdp {

class query {
public:
    struct error {
        struct http_status_line { std::string response; };
        struct http_result      { std::string response; };
        struct location_url     { std::string response; };

        struct parse {
            variant< http_status_line
                   , http_result
                   , location_url
                   > inner;
        };

        struct get_response {
            // get_response may continue working after reporting
            // `parse` error.
            variant< parse
                   , error_code
                   > inner;

            bool is_parse_error() const {
                return boost::get<parse>(&inner) != nullptr;
            }
            const error_code* as_error_code() const {
                return boost::get<error_code>(&inner);
            }
        };
    };

    struct response {
        std::string service_type;
        std::string usn; // usn stands for unique servie name
        std::string uuid; // uuid is part of usn
        url_t location;

        static result<response, error::parse> parse(string_view);

        friend std::ostream& operator<<(std::ostream& os, const response& r)
        {
            return os << "(uuid:" << r.uuid
                      << " location:" << r.location
                      << " service_type:" << r.service_type
                      << " usn:" << r.usn
                      << ")";
        }
    };

public:
    query(const query&)            = delete;
    query& operator=(const query&) = delete;

    query(query&&)            = default;
    query& operator=(query&&) = default;

    static result<query> start(net::any_io_executor, net::yield_context, const string_view &bind_ip);

    // May be called multiple times until error is of type error_code.
    // This let's callers of this function decide what to do when ssdp
    // received a response that we failed to parse.
    result<response, error::get_response>
    get_response(net::yield_context);

    void stop();

    ~query();

private:
    struct state_t;
    query(std::shared_ptr<state_t>);

private:
    std::shared_ptr<state_t> _state;
};

}} // namespace upnp::ssdp
