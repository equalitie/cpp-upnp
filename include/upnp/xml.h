#pragma once

#include <upnp/core/optional.h>
#include <upnp/detail/str/consume_number.h>
#include <upnp/detail/str/parse_address.h>
#include <upnp/detail/str/consume_until.h>

#if defined(BOOST_PROPERTY_TREE_RAPIDXML_STATIC_POOL_SIZE) \
    && BOOST_PROPERTY_TREE_RAPIDXML_STATIC_POOL_SIZE > 512
#  error "The xml parser in Boost.PropertyTree reserves too static pool "
         "size causing problems when used with coroutines in Boost.Asio. "
         "For more information have a look at this SO post "
         "https://stackoverflow.com/a/45473980/273348"
#else
#  define BOOST_PROPERTY_TREE_RAPIDXML_STATIC_POOL_SIZE 512
#endif

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/iostreams/stream.hpp>

namespace upnp { namespace xml {

using tree = boost::property_tree::ptree;

inline optional<tree> parse(const std::string& xml_str) {
    try {
        namespace ios = boost::iostreams;
        namespace pt = boost::property_tree;
        // https://stackoverflow.com/a/37712933/273348
        ios::stream<ios::array_source> stream(xml_str.c_str(), xml_str.size());
        pt::ptree tree;
        pt::read_xml(stream, tree);
        return std::move(tree);
    } catch (std::exception& e) {
        return none;
    }

}

template<class Num>
inline
optional<Num> get_num(tree t, const char* tag) {
    auto s = t.get_optional<std::string>(tag);
    if (!s) return none;
    string_view sv(*s);
    return str::consume_number<Num>(sv);
}

inline
optional<net::ip::address> get_address(tree t, const char* tag) {
    auto s = t.get_optional<std::string>(tag);
    if (!s) return none;
    return str::parse_address(*s);
}

// This does the same as ptree::get_child_optional, but allows us to ignore
// the XML namespace prefix (e.g. the 's' in <s:Envelope/>) by using
// '*' in the query instead of the namespace name. e.g.:
//
// optional<string> os = get<string>(tree, "*:Envelope.*:Body");
inline
const tree* get_child(const tree& tr, string_view path)
{
    const tree* t = &tr;

    static const auto name_split = [] (string_view s)
        -> std::pair<string_view, string_view>
    {
        auto o = str::consume_until(s, ":");
        if (!o) return std::make_pair("", s);
        return std::make_pair(*o, s);
    };

    while (t) {
        if (path.empty()) return t;

        auto op = str::consume_until(path, ".");
        string_view p;
        if (op) { p = *op; } else { p = path; path = ""; }
        auto ns = name_split(p);

        auto t_ = t;
        t = nullptr;

        if (ns.first == "*") {
            for (auto& e : *t_) {
                auto name = e.first;
                auto ns_ = name_split(name);
                if (ns.second == ns_.second) {
                    t = &e.second;
                    break;
                }
            }
        } else {
            for (auto& e : *t_) {
                if (e.first == p) {
                    t = &e.second;
                    break;
                }
            }
        }
    }

    return t;
}

template<class T>
inline
optional<T> get(const tree& tr, string_view path)
{
    auto t = get_child(tr, path);
    if (!t) return boost::none;
    return t->get_value_optional<T>();
}

}} // upnp::xml namespace

namespace boost { namespace property_tree {

inline
std::ostream& operator<<(std::ostream& os, const ptree& xml) {
    write_xml(os, xml);
    return os;
}

}} // boost::property_tree namespace
