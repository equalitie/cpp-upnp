#pragma once

#include <upnp/core/optional.h>

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

}} // upnp::xml namespace
