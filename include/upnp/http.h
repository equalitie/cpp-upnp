#pragma once

#include <upnp/core/beast.h>
#include <upnp/url.h>
#include <upnp/config.h>
#include <upnp/detail/str/consume_endpoint.h>

namespace upnp { namespace http {

using request  = beast::http::request<beast::http::empty_body>;
using response = beast::http::response<beast::http::string_body>;

inline
result<response>
get(net::executor exec, const url_t& url, net::yield_context yield)
{
    namespace http = beast::http;

    error_code ec;
    net::ip::tcp::resolver resolver(exec);

    auto hp = url.host_and_port();
    auto ep = str::consume_endpoint<net::ip::tcp>(hp);

    if (!ep) return sys::errc::invalid_argument;

    beast::tcp_stream stream(exec);
    stream.expires_after(std::chrono::seconds(5));

    stream.async_connect(*ep, yield[ec]);
    if (ec) return ec;

    request rq{http::verb::get, url.path(), 11};

    rq.set(http::field::host, url.host_and_port());
    rq.set(http::field::user_agent, CPP_UPNP_HTTP_USER_AGENT);

    http::async_write(stream, rq, yield[ec]);
    if (ec) return ec;

    beast::flat_buffer b;
    response rs;

    http::async_read(stream, b, rs, yield[ec]);
    if (ec) return ec;

    return rs;
}

}} // namespaces
