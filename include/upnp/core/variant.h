#pragma once

#include <boost/variant.hpp>

namespace upnp {
    template<typename... T>
    using variant = boost::variant<T...>;
}
