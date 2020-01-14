#pragma once

#include <upnp/core/optional.h>
#include <upnp/detail/str/consume_number.h>
#include <upnp/detail/str/parse_address.h>

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

}} // upnp::xml namespace
