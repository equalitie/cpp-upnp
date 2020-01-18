#include "parse_device.h"

namespace upnp {

optional<service> service_parse(const xml::tree& tree) {
    service ret;

    auto opt_id = tree.get_optional<std::string>("serviceId");
    if (!opt_id) return none;
    ret.id = std::move(*opt_id);

    auto opt_type = tree.get_optional<std::string>("serviceType");
    if (!opt_type) return none;
    ret.type = std::move(*opt_type);

    auto opt_url_s = tree.get_optional<std::string>("controlURL");
    if (!opt_url_s) return none;
    auto opt_url = url_t::parse(*opt_url_s);
    if (!opt_url) return none;
    ret.control_url = std::move(*opt_url);

    return ret;
}

optional<device> device_parse_root(const std::string& xml_str) {
    auto tree = xml::parse(xml_str);
    if (!tree) return none;
    return device_parse_root(*tree);
}

optional<device> device_parse_root(const xml::tree& tree) {
    device ret;
    auto opt_dev = tree.get_child_optional("root.device");
    if (!opt_dev) return none;
    return device_parse(*opt_dev);
}

optional<device> device_parse(const xml::tree& tree) {
    device ret;

    auto opt_type = tree.get_optional<std::string>("deviceType");
    if (!opt_type) return none;
    ret.type = std::move(*opt_type);

    auto opt_udn = tree.get_optional<std::string>("UDN");
    if (!opt_udn) return none;
    ret.udn = std::move(*opt_udn);

    auto opt_name = tree.get_optional<std::string>("friendlyName");
    if (!opt_name) return none;
    ret.friendly_name = std::move(*opt_name);

    auto opt_services = tree.get_child_optional("serviceList");

    if (opt_services) {
        for (auto& v : *opt_services) {
            auto opt = service_parse(v.second);
            if (!opt) continue;
            ret.services.push_back(std::move(*opt));
        }
    }

    auto opt_devices = tree.get_child_optional("deviceList");

    if (opt_devices) {
        for (auto& v : *opt_devices) {
            auto opt = device_parse(v.second);
            if (!opt) continue;
            ret.devices.push_back(std::move(*opt));
        }
    }

    return ret;
}

} // namespace upnp
