#include <upnp.hpp>
#include <iostream>

using namespace std;
namespace net = upnp::net;

int main()
{
    net::io_context ctx;

    namespace pt = boost::property_tree;

    net::spawn(ctx, [&] (net::yield_context yield) {
        auto r_igds = upnp::igd::discover(ctx.get_executor(), yield);

        if (!r_igds) {
            cerr << "::: Failure " << r_igds.error().message() << "\n";
            return;
        }

        auto igds = move(r_igds.value());

        for (auto& igd : igds) {
            auto r = igd.add_port_mapping(9999, 9999, "test", chrono::minutes(5), yield);
        }
    });

    ctx.run();
}
