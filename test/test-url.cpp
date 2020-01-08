#define BOOST_TEST_MODULE url
#include <boost/test/included/unit_test.hpp>

#include <upnp/url.h>
#include <sstream>

using namespace std;

inline string to_str(const upnp::url_t& url) {
    stringstream ss;
    ss << url;
    return ss.str();
}

inline string to_str(const upnp::optional<upnp::url_t>& url) {
    if (!url) return {};
    return to_str(*url);
}

BOOST_AUTO_TEST_CASE(test_url) {
    auto test = [] (const string& url_s) {
        auto url = upnp::url_t::parse(url_s);
        BOOST_REQUIRE_EQUAL(to_str(url), url_s);
    };

    test("http://example.org/");
    test("http://example.org:1500/");
    test("http://alice@example.org/");
    test("http://alice@example.org");
    test("http://alice@example.org?foo=bar");
    test("http://alice@example.org?foo=bar#baz");
    test("http://alice@example.org#baz");
    test("http://alice@example.org/#baz");
    test("");
    test("/");
    test("/foo/bar");
}

BOOST_AUTO_TEST_CASE(test_url_replace) {
    {
        // Bigger
        auto url = upnp::url_t::parse("http://alice@example.org:123/foo?bar#baz");
        BOOST_REQUIRE(url);
        url->replace_path("/abcd");
        BOOST_REQUIRE_EQUAL(to_str(url), "http://alice@example.org:123/abcd?bar#baz");
    }
    {
        // Smaller
        auto url = upnp::url_t::parse("http://alice@example.org:123/foo?bar#baz");
        BOOST_REQUIRE(url);
        url->replace_path("/ab");
        BOOST_REQUIRE_EQUAL(to_str(url), "http://alice@example.org:123/ab?bar#baz");
    }
}
