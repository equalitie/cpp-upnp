#pragma once

#include <upnp/url.h>
#include <vector>

namespace upnp {

struct service {
    std::string id;
    std::string type;
    url_t control_url;
};

struct device {
    std::string type;
    std::string udn;
    std::string friendly_name;
    std::vector<device> devices;
    std::vector<service> services;
};

} // namespace upnp
