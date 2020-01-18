#pragma once

#include <upnp/third_party/string_view.h>
#include <boost/algorithm/string.hpp>

namespace upnp { namespace str {

inline
bool istarts_with(string_view s, string_view prefix)
{
    return boost::iequals(s.substr(0, prefix.size()), prefix);
}

}} // namespaces
