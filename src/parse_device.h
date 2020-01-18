#include <upnp/device.h>
#include "xml.h"

namespace upnp {

optional<service> service_parse(const xml::tree& tree);

optional<device> device_parse_root(const std::string& xml_str);
optional<device> device_parse_root(const xml::tree& tree);
optional<device> device_parse(const xml::tree& tree);

} // namespace upnp
