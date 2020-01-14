#include <upnp.h>
#include <iostream>

using namespace std;
namespace net = upnp::net;

int main()
{
    net::io_context ctx;

    net::spawn(ctx, [&] (net::yield_context yield) {
        auto r_igds = upnp::igd::discover(ctx.get_executor(), yield);

        if (!r_igds) {
            cerr << "::: Failure " << r_igds.error().message() << "\n";
            return;
        }

        auto igds = move(r_igds.value());

        for (auto& igd : igds) {
            {
                auto r = igd.get_external_address(yield);

                if (!r) {
                    cerr << "Failed to get external IP address " << r.error()
                         << "\n";
                } else {
                    cerr << "Got external IP address: " << r.value() << "\n";
                }
            }

            {
                auto r = igd.add_port_mapping( upnp::igd::udp
                                             , 9998
                                             , 9998
                                             , "test"
                                             , chrono::minutes(1)
                                             , yield);

                if (r) {
                    cerr << "::: Success\n";
                } else {
                    cerr << "::: Error: " << r.error() << "\n";
                }
            }

            {
                auto r = igd.get_list_of_port_mappings( upnp::igd::udp
                                                      , 0
                                                      , 65535
                                                      , 100
                                                      , yield);

                if (r) {
                    cerr << "::: Success " << r.value().size() << "\n";
                    for (auto e : r.value()) {
                        cerr << e.proto
                             << " EXT:" << e.ext_port
                             << " INT:" << e.int_port
                             << " ADDR:" << e.int_client
                             << " DURATION:" << e.lease_duration.count() << "s"
                             << " " << e.description << "\n";
                    }
                } else {
                    cerr << "::: Error: " << r.error() << "\n";
                }
            }
        }
    });

    ctx.run();
}
