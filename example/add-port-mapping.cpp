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
    });

    ctx.run();
}
