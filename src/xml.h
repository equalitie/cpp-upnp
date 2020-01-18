#pragma once

#include <upnp/third_party/optional.h>
#include <upnp/third_party/string_view.h>
#include <upnp/third_party/net.h>
#include <boost/asio/ip/address.hpp>

#if defined(BOOST_PROPERTY_TREE_RAPIDXML_STATIC_POOL_SIZE) \
    && BOOST_PROPERTY_TREE_RAPIDXML_STATIC_POOL_SIZE > 512
#  error "The xml parser in Boost.PropertyTree reserves too big static pool "
         "size causing problems when used with coroutines in Boost.Asio. "
         "For more information have a look at this SO post "
         "https://stackoverflow.com/a/45473980/273348"
#else
#  define BOOST_PROPERTY_TREE_RAPIDXML_STATIC_POOL_SIZE 512
#endif

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/iostreams/stream.hpp>
#include "str/consume_number.h"

namespace upnp { namespace xml {

using tree = boost::property_tree::ptree;

optional<tree> parse(const std::string& xml_str);

template<class Num>
inline
optional<Num> get_num(tree t, const char* tag) {
    auto s = t.get_optional<std::string>(tag);
    if (!s) return none;
    string_view sv(*s);
    return str::consume_number<Num>(sv);
}

optional<net::ip::address> get_address(tree t, const char* tag);

// This does the same as ptree::get_child_optional, but allows us to ignore
// the XML namespace prefix (e.g. the 's' in <s:Envelope/>) by using
// '*' in the query instead of the namespace name. e.g.:
//
// optional<string> os = get<string>(tree, "*:Envelope.*:Body");
const tree* get_child(const tree& tr, string_view path);

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

std::ostream& operator<<(std::ostream& os, const ptree& xml);

}} // boost::property_tree namespace
