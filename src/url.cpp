#include <upnp/url.h>
#include "str/consume_until.h"
#include "str/consume_number.h"

using namespace upnp;

/* static */
optional<url_t> url_t::parse(std::string url_) {
    using str::consume_until;

    url_t ret;

    ret._buf = std::move(url_);

    string_view state = ret._buf;

    if (auto opt = consume_until(state, "://")) {
        ret._scheme = *opt;
    }

    if (auto userinfo = consume_until(state, "@")) {
        ret._userinfo = *userinfo;
    }

    if (auto host = consume_until(state, ":")) {
        ret._host = *host;

        if (auto port = consume_until(state, "/", false)) {
            ret._port = *port;
        }
    } else if (auto host = consume_until(state, "/", false)) {
        ret._host = *host;
    } else {
        ret._host = state;
        return ret;
    }

    if (auto path = consume_until(state, "?")) {
        ret._path = *path;

        if (auto query = consume_until(state, "#")) {
            ret._query = *query;
            ret._fragment = state;
            return ret;
        } else {
            ret._query = state;
            return ret;
        }
    } else if (auto path = consume_until(state, "#")) {
        ret._path = *path;
        ret._fragment = state;
    } else {
        ret._path = state;
    }

    return ret;
}

optional<uint16_t> url_t::numeric_port() const {
    if (_port.empty()) return none;
    auto p = _port;
    return str::consume_number<uint16_t>(p);
}

std::ostream& upnp::operator<<(std::ostream& os, const url_t& u) {
    if (!u._scheme.empty())   os << u._scheme << "://";
    if (!u._userinfo.empty()) os << u._userinfo << "@";
    if (!u._host.empty())     os << u._host;
    if (!u._port.empty())     os << ":" << u._port;
    if (!u._path.empty())     os << u._path;
    if (!u._query.empty())    os << "?" << u._query;
    if (!u._fragment.empty()) os << "#" << u._fragment;
    return os;
}

