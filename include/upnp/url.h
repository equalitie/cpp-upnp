#pragma once

#include <upnp/core/optional.h>
#include <upnp/core/string_view.h>
#include <upnp/detail/str/consume_until.h>
#include <upnp/detail/str/consume_number.h>

namespace upnp {

// https://en.wikipedia.org/wiki/URL

class url_t {
public:
    url_t() = default;

    url_t(const url_t& other)
        : _buf(other._buf)
        , _scheme  (transfer(other._scheme,   other._buf.data()))
        , _userinfo(transfer(other._userinfo, other._buf.data()))
        , _host    (transfer(other._host,     other._buf.data()))
        , _port    (transfer(other._port,     other._buf.data()))
        , _path    (transfer(other._path,     other._buf.data()))
        , _query   (transfer(other._query,    other._buf.data()))
        , _fragment(transfer(other._fragment, other._buf.data()))
    {
    }

    url_t(url_t&& other)
    {
        *this = std::move(other);
    }

    url_t& operator=(url_t&& other)
    {
        auto old_ptr = other._buf.data();
        _buf = std::move(other._buf);

        _scheme   = transfer(other._scheme,   old_ptr);
        _userinfo = transfer(other._userinfo, old_ptr);
        _host     = transfer(other._host,     old_ptr);
        _port     = transfer(other._port,     old_ptr);
        _path     = transfer(other._path,     old_ptr);
        _query    = transfer(other._query,    old_ptr);
        _fragment = transfer(other._fragment, old_ptr);

        return *this;
    }

    static optional<url_t> parse(std::string url);

    const string_view scheme()   const { return _scheme;   }
    const string_view userinfo() const { return _userinfo; }
    const string_view host()     const { return _host;     }
    const string_view port()     const { return _port;     }
    const string_view path()     const { return _path;     }
    const string_view query()    const { return _query;    }
    const string_view fragment() const { return _fragment; }

    const string_view host_and_port() const {
        string_view ret;

        if (_port.empty()) return _host;
        return string_view(_host.data(), _port.end() - _host.begin());
    }

    optional<uint16_t> numeric_port() const {
        if (_port.empty()) return none;
        auto p = _port;
        return str::consume_number<uint16_t>(p);
    }

    void replace_path(string_view p) {
        auto old_ptr = _buf.data();

        _buf.replace(_path.data() - _buf.data(), _path.size()
                     , p.data(), p.size());

        ptrdiff_t delta = p.size() - _path.size();

        _path     = {_path.data(),             _path.size() + delta };
        _query    = {_query.data()    + delta, _query.size()        };
        _fragment = {_fragment.data() + delta, _fragment.size()     };

        _scheme   = transfer(_scheme,   old_ptr);
        _userinfo = transfer(_userinfo, old_ptr);
        _host     = transfer(_host,     old_ptr);
        _port     = transfer(_port,     old_ptr);
        _path     = transfer(_path,     old_ptr);
        _query    = transfer(_query,    old_ptr);
        _fragment = transfer(_fragment, old_ptr);
    }

    operator string_view() const {
        return _buf;
    }

private:
    string_view transfer(string_view sv, const char* from) {
        const char* to = _buf.data();
        return {sv.data() - from + to, sv.size()};
    }

private:
    friend std::ostream& operator<<(std::ostream&, const url_t&);

    std::string _buf;

    string_view _scheme;
    string_view _userinfo;
    string_view _host;
    string_view _port;
    string_view _path;
    string_view _query;
    string_view _fragment;
};

/* static */
inline
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

std::ostream& operator<<(std::ostream& os, const url_t& u) {
    if (!u._scheme.empty())   os << u._scheme << "://";
    if (!u._userinfo.empty()) os << u._userinfo << "@";
    if (!u._host.empty())     os << u._host;
    if (!u._port.empty())     os << ":" << u._port;
    if (!u._path.empty())     os << u._path;
    if (!u._query.empty())    os << "?" << u._query;
    if (!u._fragment.empty()) os << "#" << u._fragment;
    return os;
}

} // namespace upnp
