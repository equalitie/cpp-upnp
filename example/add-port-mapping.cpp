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
                                             , 9999
                                             , 9999
                                             , "test"
                                             , chrono::minutes(5)
                                             , yield);

                if (r) {
                    cerr << "::: Success\n";
                } else {
                    cerr << "::: Error: " << r.error() << "\n";
                }
            }

            {
                auto r = igd.get_list_of_port_mappings( upnp::igd::udp
                                                      , 0 // 9998
                                                      , 65535 // 10000
                                                      , 100
                                                      , yield);

                if (r) {
                    cerr << "::: Success " << r.value().size() << "\n";
                    for (auto e : r.value()) {
                        cerr << "  > " << e.ext_port << " " << e.int_port << " " << e.proto << " " << e.int_client << " " << e.lease_duration.count() << " " << e.description << "\n";
                    }
                } else {
                    cerr << "::: Error: " << r.error() << "\n";
                }
            }
        }
    });

    ctx.run();
}
