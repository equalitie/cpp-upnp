#pragma once

#include <upnp/xml.h>
#include <upnp/url.h>

namespace upnp {

struct service {
    std::string id;
    std::string type;
    url_t control_url;

    static optional<service> parse(const xml::tree& tree);
};

struct device {
    std::string type;
    std::string udn;
    std::string friendly_name;
    std::vector<device> devices;
    std::vector<service> services;

    static optional<device> parse_root(const std::string& xml_str);

    static optional<device> parse_root(const xml::tree& tree);

    static optional<device> parse(const xml::tree& tree);
};


//template<class T> struct pretty_printer {
//    const T& v;
//    unsigned level;
//
//    pretty_printer(const T& v) : v(v), level(0) {}
//    pretty_printer(const T& v, unsigned level) : v(v), level(level) {}
//};
//
//inline
//std::ostream& operator<<(std::ostream& os, const pretty_printer<service>& pd) {
//    std::string pad(pd.level * 2, ' ');
//    os << pad << pd.v.id << "\n";
//    os << pad << pd.v.type << "\n";
//    os << pad << pd.v.control_url << "\n";
//    return os;
//}
//
//inline
//std::ostream& operator<<(std::ostream& os, const pretty_printer<device>& pd) {
//    std::string pad(pd.level * 2, ' ');
//    os << pad << "type: " << pd.v.type << "\n";
//    os << pad << "friendly_name: " << pd.v.friendly_name << "\n";
//    os << pad << "udn: " << pd.v.udn << "\n";
//
//    if (!pd.v.devices.empty()) {
//        os << pad << "devices:\n";
//        for (auto& d : pd.v.devices) {
//            os << pretty_printer<device>(d, pd.level+1);
//        }
//    }
//
//    if (!pd.v.services.empty()) {
//        os << pad << "services:\n";
//        for (auto& s : pd.v.services) {
//            os << pretty_printer<service>(s, pd.level+1);
//        }
//    }
//
//    return os;
//}

} // namespace upnp
