#include <boost/asio/ip/tcp.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/beast.hpp>
#include "local_address_to.h"
#include "str/consume_endpoint.h"
#include <upnp/ssdp.h>
#include <upnp/device.h>
#include <upnp/config.h>
#include <upnp/igd.h>
#include "xml.h"
#include "parse_device.h"
#include <set>

namespace upnp {

namespace sys = boost::system;

igd::igd( std::string   uuid
        , device        upnp_device
        , std::string   service_id
        , url_t         url
        , std::string   urn
        , net::executor exec)
    : _uuid(std::move(uuid))
    , _upnp_device(std::move(upnp_device))
    , _service_id(std::move(service_id))
    , _url(std::move(url))
    , _urn(std::move(urn))
    , _exec(exec)
{}

result<void, igd::error::add_port_mapping>
igd::add_port_mapping( protocol proto
                     , uint16_t external_port
                     , uint16_t internal_port
                     , string_view description
                     , std::chrono::seconds duration
                     , net::yield_context yield) noexcept
{
    auto host_port = _url.host_and_port();
    auto opt_remote_ep = str::consume_endpoint<net::ip::tcp>(host_port);
    if (!opt_remote_ep)
        return error::igd_host_parse_failed{_url};

    auto opt_local_ip = local_address_to(*opt_remote_ep);
    if (!opt_local_ip)
        return error::no_endpoint_to_igd{};

    net::ip::address local_ip = *opt_local_ip;

    std::stringstream body;
    body << "<u:AddPortMapping xmlns:u=\""<< _urn <<"\">"
            "<NewRemoteHost></NewRemoteHost>"
            "<NewEnabled>1</NewEnabled>"
            "<NewExternalPort>"           << external_port     << "</NewExternalPort>"
            "<NewProtocol>"               << proto             << "</NewProtocol>"
            "<NewInternalPort>"           << internal_port     << "</NewInternalPort>"
            "<NewInternalClient>"         << local_ip          << "</NewInternalClient>"
            "<NewPortMappingDescription>" << description       << "</NewPortMappingDescription>"
            "<NewLeaseDuration>"          << duration.count()  << "</NewLeaseDuration>"
            "</u:AddPortMapping>";

    auto rs = soap_request("AddPortMapping", body.str(), yield);
    if (!rs) return rs.error();

    return success();
}

result<net::ip::address, igd::error::get_external_address>
igd::get_external_address(net::yield_context yield) noexcept
{
    std::string body = "<u:GetExternalIPAddress xmlns:u=\"" + _urn + "\"/>";

    auto rs = soap_request("GetExternalIPAddress", body, yield);
    if (!rs) return rs.error();

    auto opt_xml = xml::parse(rs.value().body());
    if (!opt_xml) return error::invalid_xml_body{};
    auto& xml_rs = *opt_xml;

    const char* path = "*:Envelope.*:Body.u:GetExternalIPAddressResponse."
                       "NewExternalIPAddress";

    auto opt_ip_s = xml::get<std::string>(xml_rs, path);
    if (!opt_ip_s) return error::invalid_response{};
    auto& ip_s = *opt_ip_s;

    error_code ec;
    auto addr = net::ip::make_address(ip_s, ec);
    if (ec) return error::bad_address{};

    return std::move(addr);
}

result<igd::map_entry, igd::error::get_generic_port_mapping_entry>
igd::get_generic_port_mapping_entry( uint16_t index
                                   , net::yield_context yield) noexcept
{
    std::stringstream body;
    body <<
        "<u:GetGenericPortMappingEntry xmlns:u=\"" << _urn << "\">"
        "<NewPortMappingIndex>" << index << "</NewPortMappingIndex>"
        "</u:GetGenericPortMappingEntry>";

    auto rs = soap_request("GetGenericPortMappingEntry", body.str(), yield);
    if (!rs) return rs.error();

    auto b = std::move(rs.value().body());
    auto opt_xml = xml::parse(b);
    if (!opt_xml) return error::invalid_xml_body{};
    auto& xml_rs = *opt_xml;

    const char* path = "*:Envelope.*:Body.u:GetGenericPortMappingEntryResponse";
    auto o = xml::get_child(xml_rs, path);

    auto odes = o->get_optional<std::string>("NewPortMappingDescription");
    auto oext = xml::get_num<uint16_t>(*o, "NewExternalPort");
    auto oint = xml::get_num<uint16_t>(*o, "NewInternalPort");
    auto odur = xml::get_num<uint32_t>(*o, "NewLeaseDuration");
    auto opro = o->get_optional<std::string>("NewProtocol");
    auto ocli = xml::get_address      (*o, "NewInternalClient");
    auto oena = xml::get_num<uint16_t>(*o, "NewEnabled");

    if (!oext || !oint || !ocli || !oena || !odur || !odes || !opro)
        return error::invalid_response{};

    protocol proto;
    if (boost::iequals(*opro, "UDP")) {
        proto = udp;
    } else if (boost::iequals(*opro, "TCP")) {
        proto = tcp;
    } else {
        return error::invalid_response{};
    }

    return map_entry {
        std::move(*odes),
        *oext,
        *oint,
        std::chrono::seconds(*odur),
        proto,
        *ocli,
        bool(*oena)
    };
}

result<std::vector<igd::map_entry>, igd::error::get_list_of_port_mappings>
igd::get_list_of_port_mappings( protocol proto
                              , uint16_t min_port
                              , uint16_t max_port
                              , uint16_t max_count
                              , net::yield_context yield) noexcept
{
    std::stringstream body;
    body <<
        "<u:GetListOfPortMappings xmlns:u=\"" << _urn << "\">"
        "<NewStartPort>" << min_port << "</NewStartPort>"
        "<NewEndPort>" << max_port << "</NewEndPort>"
        "<NewProtocol>" << proto << "</NewProtocol>"
        "<NewNumberOfPorts>" << max_count << "</NewNumberOfPorts>"
        "</u:GetListOfPortMappings>";

    auto rs = soap_request("GetListOfPortMappings", body.str(), yield);
    if (!rs) return rs.error();

    auto b = std::move(rs.value().body());

    auto opt_xml = xml::parse(b);
    if (!opt_xml) return error::invalid_xml_body{};
    auto& xml_rs = *opt_xml;

    const char* path = "s:Envelope.s:Body.u:GetListOfPortMappingsResponse.NewPortListing";

    auto o1 = xml_rs.get_optional<std::string>(path);
    if (!o1) return error::invalid_response{};
    auto o2 = xml::parse(*o1);
    if (!o2) return error::invalid_response{};
    auto o3 = o2->get_child_optional("p:PortMappingList");
    if (!o3) return error::invalid_response{};

    std::vector<map_entry> entries;

    for (auto& e : *o3) {
        if (e.first != "p:PortMappingEntry") continue;
        auto& child = e.second;

        auto oext = xml::get_num<uint16_t>(child, "p:NewExternalPort");
        auto oint = xml::get_num<uint16_t>(child, "p:NewInternalPort");
        auto ocli = xml::get_address      (child, "p:NewInternalClient");
        auto oena = xml::get_num<uint16_t>(child, "p:NewEnabled");
        auto olea = xml::get_num<uint32_t>(child, "p:NewLeaseTime");
        auto odes = child.get_optional<std::string>("p:NewDescription");
        auto opro = child.get_optional<std::string>("p:NewProtocol");

        if (!oext || !oint || !ocli || !oena || !olea || !odes || !opro)
            continue;

        protocol proto = tcp;

        if      (*opro == "UDP") proto = udp;
        else if (*opro == "TCP") proto = tcp;
        else continue;

        entries.push_back({ std::move(*odes)
                          , *oext
                          , *oint
                          , std::chrono::seconds(*olea)
                          , proto
                          , std::move(*ocli)
                          , bool(*oena)});
    }

    return std::move(entries);
}

result<void, igd::error::delete_port_mapping>
igd::delete_port_mapping( protocol proto
                        , uint16_t ext_port
                        , net::yield_context yield) noexcept
{
    std::stringstream body;
    body <<
        "<u:DeletePortMapping xmlns:u=\"" + _urn + "\"/>"
        "<NewProtocol>" << proto << "</NewProtocol>"
        "<NewExternalPort>" << ext_port << "</NewExternalPort>"
        "<NewRemoteHost></NewRemoteHost>"
        "</u:DeletePortMapping>";

    auto rs = soap_request("DeletePortMapping", body.str(), yield);
    if (!rs) return rs.error();
    return success();
}

result<igd::soap_response, igd::error::soap_request>
igd::soap_request( string_view command
                 , string_view message
                 , net::yield_context yield) noexcept
{
    namespace http = beast::http;
    using E = error::soap_request;

    auto host_port = _url.host_and_port();
    auto opt_remote_ep = str::consume_endpoint<net::ip::tcp>(host_port);
    if (!opt_remote_ep)
        return E{error::igd_host_parse_failed{_url}};

    std::string body =
        "<?xml version=\"1.0\" ?>"
        "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" "
                    "s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
        "<s:Body>" + message.to_string() + "</s:Body>"
        "</s:Envelope>";

    http::request<http::string_body> rq{http::verb::post, _url, 11};
    rq.set(http::field::host, _url.host_and_port());
    rq.set(http::field::user_agent, CPP_UPNP_HTTP_USER_AGENT);
    rq.set(http::field::content_type, "text/xml; charset=\"utf-8\"");
    rq.set(http::field::connection, "Close");
    rq.set(http::field::cache_control, "no-cache");
    rq.set(http::field::pragma, "no-cache");
    rq.set("SOAPAction", _urn + "#" + command.to_string());

    rq.body() = std::move(body);
    rq.prepare_payload();

    //std::cerr << rq;

    error_code ec;

    beast::tcp_stream stream(_exec);
    stream.expires_after(std::chrono::seconds(5));

    auto cancelled = _cancel.connect([&] { stream.close(); });

    stream.async_connect(*opt_remote_ep, yield[ec]);
    if (ec) return E{error::tcp_connect{}};

    http::async_write(stream, rq, yield[ec]);
    if (ec) return E{error::http_request{}};

    beast::flat_buffer b;
    soap_response rs;

    http::async_read(stream, b, rs, yield[ec]);
    if (ec) return E{error::http_response{}};

    if (rs.result() != beast::http::status::ok) {
        return E{error::http_status{rs.result()}};
    }

    return std::move(rs);
}

/* static */
result<std::vector<igd>> igd::discover(net::executor exec, net::yield_context yield)
{
    using namespace std;

    auto q = ssdp::query::start(exec, yield);
    if (!q) return q.error();
    auto& query = q.value();

    std::set<std::string> already_seen;
    std::vector<igd> igds;

    while (true) {
        auto qr = query.get_response(yield);
        if (!qr) {
            if (qr.error() == net::error::timed_out && !igds.empty()) {
                break;
            }
            return qr.error();
        }
        auto& rsp = qr.value();

        auto res_root_dev = query_root_device(exec, rsp.location, yield);
        if (!res_root_dev) return sys::errc::io_error;
        auto& root_dev = res_root_dev.value();

        string v;

        if (root_dev.type == "urn:schemas-upnp-org:device:InternetGatewayDevice:1") {
            v = "1";
        } else
        if (root_dev.type == "urn:schemas-upnp-org:device:InternetGatewayDevice:2") {
            v = "2";
        } else {
            continue;
        }

        string device_urn     = "urn:schemas-upnp-org:device:WANDevice:"           + v;
        string connection_urn = "urn:schemas-upnp-org:device:WANConnectionDevice:" + v;
        string con_ip         = "urn:schemas-upnp-org:service:WANIPConnection:"    + v;
        string con_ppp        = "urn:schemas-upnp-org:service:WANPPPConnection:"   + v;

        for (const auto& device : root_dev.devices) {
            // No duplicates
            if (!already_seen.insert(device.udn).second) continue;

            if (device.type != device_urn) continue;

            for (const auto& connection : device.devices) {
                if (connection.type != connection_urn) continue;

                for (const auto& service : connection.services) {
                    if (service.type != con_ip && service.type != con_ppp)continue;

                    url_t url = rsp.location;
                    url.replace_path(service.control_url.path());

                    igds.push_back({
                        rsp.uuid,
                        device,
                        service.id,
                        url,
                        service.type,
                        exec
                    });
                }
            }
        }
    }

    return std::move(igds);
}

/* static */
result<device>
igd::query_root_device( net::executor exec
                      , const url_t& url
                      , net::yield_context yield) noexcept
{
    namespace http = beast::http;
    using request  = http::request<http::empty_body>;
    using response = http::response<http::string_body>;

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

    if (rs.result() != beast::http::status::ok) {
        return sys::errc::protocol_error;
    }

    auto opt_root_dev = device_parse_root(rs.body());
    if (!opt_root_dev) return sys::errc::io_error;

    return std::move(*opt_root_dev);
}

void igd::stop() {
    _cancel();
}

} // upnp namespace
