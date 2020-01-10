#pragma once

#include <boost/outcome/result.hpp>

namespace upnp {

    namespace outcome = boost::outcome_v2;

    using outcome::success;

#ifndef NDEBUG // if debug
    template<class V, class E = boost::system::error_code>
    using result = outcome::result<
                     V,
                     E,
                     outcome::policy::terminate>;
#else
    template<class V, class E = boost::system::error_code>
    using result = outcome::result<
                     V,
                     E,
                     outcome::policy::throw_bad_result_access<E, void>>;
#endif

} // namespace upnp
