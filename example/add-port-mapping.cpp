#include <upnp.h>
#include <iostream>

using namespace std;
namespace net = upnp::net;

void list_port_mappings(upnp::igd& igd, net::yield_context yield)
{
    cerr << "Getting list of port mappings\n";

    auto r = igd.get_list_of_port_mappings( upnp::igd::udp
                                          , 0
                                          , 65535
                                          , 100
                                          , yield);

    if (r) {
        cerr << "    Success, found " << r.value().size() << " entries:\n";
        for (auto e : r.value()) {
            cerr << "    "
                 << e.proto
                 << " EXT:" << e.ext_port
                 << " INT:" << e.int_port
                 << " ADDR:" << e.int_client
                 << " DURATION:" << e.lease_duration.count() << "s"
                 << " " << e.description << "\n";
        }
    } else {
        cerr << "    Error: " << r.error() << "\n";
    }
}

void delete_port_mapping(upnp::igd& igd, uint16_t port, net::yield_context yield)
{
    cerr << "Removing port mapping EXT:" << port << "\n";
    auto r = igd.delete_port_mapping(upnp::igd::udp, port, yield);
    if (r) {
        cerr << "    Success\n";
    } else {
        cerr << "    Error: " << r.error() << "\n";
    }
}

void get_external_address(upnp::igd& igd, net::yield_context yield)
{
    cerr << "Getting external address\n";

    auto r = igd.get_external_address(yield);

    if (r) {
        cerr << "    " << r.value() << "\n";
    } else {
        cerr << "    Error: " << r.error() << "\n";
    }
}

void add_port_mapping( upnp::igd& igd
                     , uint16_t ext_p
                     , uint16_t int_p
                     , net::yield_context yield)
{
    cerr << "Adding port mapping ext:" << ext_p << " int:" << int_p << "\n";

    auto r = igd.add_port_mapping( upnp::igd::udp
                                 , ext_p
                                 , int_p
                                 , "test"
                                 , chrono::minutes(1)
                                 , yield);

    if (r) {
        cerr << "    Success\n";
    } else {
        cerr << "    Error: " << r.error() << "\n";
    }
}

int main()
{
    net::io_context ctx;

    net::spawn(ctx, [&] (net::yield_context yield) {
        cerr << "Discovering IGDs\n";
        auto r_igds = upnp::igd::discover(ctx.get_executor(), yield);

        if (r_igds) {
            cerr << "    Success" << "\n";
        } else {
            cerr << "    Error: " << r_igds.error() << "\n";
            return;
        }

        auto igds = move(r_igds.value());

        net::ip::udp::socket
            socket(ctx, net::ip::udp::endpoint(net::ip::address_v4::any(), 0));

        for (auto& igd : igds) {
            get_external_address(igd, yield);
            add_port_mapping(igd, 7777, socket.local_endpoint().port(), yield);
            list_port_mappings(igd, yield);
            //delete_port_mapping(igd, 7777, yield);
            //list_port_mappings(igd, yield);
        }

        cerr << "Listening on UDP " << socket.local_endpoint() << "\n";

         while (true) {
             upnp::error_code ec;
             net::ip::udp::endpoint ep;
             std::array<char, 256> d;
             size_t size = socket.async_receive_from(net::buffer(d), ep, yield[ec]);
             if (ec) { cerr << "Bye\n"; break; }
             boost::string_view sv(d.data(), size);
             std::cerr << "received " << sv << "\n";
         }
    });

    ctx.run();
}
