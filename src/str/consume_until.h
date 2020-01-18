#pragma once

#include <upnp/core/string_view.h>
#include <upnp/core/optional.h>

namespace upnp { namespace str {

inline
optional<string_view>
consume_until( string_view& s
             , std::initializer_list<string_view> tokens
             , bool consume_tokens = true) {
    string_view c = s;

    size_t consumed = 0;

    while (c.size()) {
        for (auto token : tokens) {
            if (c.starts_with(token)) {
                if (consume_tokens) {
                    c.remove_prefix(token.size());
                }
                auto ret = s.substr(0, consumed);
                s = c;
                return ret;
            }
        }
        ++consumed;
        c.remove_prefix(1);
    }

    return none;
}

inline
optional<string_view>
consume_until(string_view& s, string_view token, bool consume_tokens = true) {
    return consume_until(s, {token}, consume_tokens);
}

}}
