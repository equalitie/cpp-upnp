#pragma once

#include <upnp/xml.h>
#include <upnp/url.h>

namespace upnp {

struct service {
    std::string id;
    std::string type;
    url_t control_url;

    static optional<service> parse(const xml::tree& tree) {
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
};

struct device {
    std::string type;
    std::string udn;
    std::string friendly_name;
    std::vector<device> devices;
    std::vector<service> services;

    static optional<device> parse_root(const std::string& xml_str) {
        auto tree = xml::parse(xml_str);
        if (!tree) return none;
        return parse_root(*tree);
    }

    static optional<device> parse_root(const xml::tree& tree) {
        device ret;
        auto opt_dev = tree.get_child_optional("root.device");
        if (!opt_dev) return none;
        return parse(*opt_dev);
    }

    static optional<device> parse(const xml::tree& tree) {
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
                auto opt = service::parse(v.second);
                if (!opt) continue;
                ret.services.push_back(std::move(*opt));
            }
        }

        auto opt_devices = tree.get_child_optional("deviceList");

        if (opt_devices) {
            for (auto& v : *opt_devices) {
                auto opt = device::parse(v.second);
                if (!opt) continue;
                ret.devices.push_back(std::move(*opt));
            }
        }

        return ret;
    }

    //std::vector<igd_service> filter_igd
};


template<class T> struct pretty_printer {
    const T& v;
    unsigned level;

    pretty_printer(const T& v) : v(v), level(0) {}
    pretty_printer(const T& v, unsigned level) : v(v), level(level) {}
};

inline
std::ostream& operator<<(std::ostream& os, const pretty_printer<service>& pd) {
    std::string pad(pd.level * 2, ' ');
    os << pad << pd.v.id << "\n";
    os << pad << pd.v.type << "\n";
    os << pad << pd.v.control_url << "\n";
    return os;
}

inline
std::ostream& operator<<(std::ostream& os, const pretty_printer<device>& pd) {
    std::string pad(pd.level * 2, ' ');
    os << pad << "type: " << pd.v.type << "\n";
    os << pad << "friendly_name: " << pd.v.friendly_name << "\n";
    os << pad << "udn: " << pd.v.udn << "\n";

    if (!pd.v.devices.empty()) {
        os << pad << "devices:\n";
        for (auto& d : pd.v.devices) {
            os << pretty_printer<device>(d, pd.level+1);
        }
    }

    if (!pd.v.services.empty()) {
        os << pad << "services:\n";
        for (auto& s : pd.v.services) {
            os << pretty_printer<service>(s, pd.level+1);
        }
    }

    return os;
}

} // namespace upnp
