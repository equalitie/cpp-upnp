#pragma once

#include <upnp/core/optional.h>
#include <upnp/core/string_view.h>

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
        if (_port.empty()) return _host;
        return string_view(_host.data(), _port.end() - _host.begin());
    }

    optional<uint16_t> numeric_port() const;

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

std::ostream& operator<<(std::ostream&, const url_t&);

} // namespace upnp
