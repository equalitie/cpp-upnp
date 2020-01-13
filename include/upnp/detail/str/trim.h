#pragma once

#include <upnp/core/string_view.h>

namespace upnp { namespace str {

inline
void trim_space_prefix(string_view& s) {
    while (!s.empty() && isspace(s[0])) {
        s.remove_prefix(1);
    }
}

inline
void trim_space_suffix(string_view& s) {
    while (!s.empty() && isspace(s[s.size() - 1])) {
        s.remove_suffix(1);
    }
}

}} // namespaces
