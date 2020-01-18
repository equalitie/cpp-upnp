#pragma once

#include <upnp/core/result.h>
#include <upnp/url.h>
#include <upnp/namespaces.h>

#include <boost/range/begin.hpp> // needed by spawn
#include <boost/range/end.hpp> // needed by spawn
#include <boost/asio/spawn.hpp>

namespace upnp { namespace ssdp {

class query {
public:
    struct response {
        std::string service_type;
        std::string usn; // unique servie name
        std::string uuid;
        url_t location;

        static result<response> parse(string_view);

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

    static result<query> start(net::executor, net::yield_context);

    // May be called multiple times until error is returned.
    result<response> get_response(net::yield_context);

    void stop();

    ~query();

private:
    struct state_t;
    query(std::shared_ptr<state_t>);

private:
    std::shared_ptr<state_t> _state;
};

}} // namespace upnp::ssdp
