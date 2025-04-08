cmake_minimum_required(VERSION 3.5)

project(CPPUPnP VERSION 1.0.0 LANGUAGES CXX)

set(BOOST_COMPONENTS system context)
if ( Boost_VERSION LESS_EQUAL 1.79.0)
    set(BOOST_COMPONENTS ${BOOST_COMPONENTS} coroutine)
endif ()

if (NOT Boost_USE_STATIC_LIBS)
    set(THREAD_LIB "Threads::Threads")
else()
    # When linking with static Boost, we need to link with libboost_thread.
    set(BOOST_COMPONENTS thread ${BOOST_COMPONENTS})
    # Boost::thread adds Threads::Threads automatically
    set(THREAD_LIB "Boost::thread")
endif()

find_package(Threads REQUIRED)
find_package(Boost 1.71 REQUIRED COMPONENTS ${BOOST_COMPONENTS})

add_library(cpp_upnp
    STATIC
        ${CPPUPnP_DIR}/src/xml.cpp
        ${CPPUPnP_DIR}/src/igd.cpp
        ${CPPUPnP_DIR}/src/ssdp.cpp
        ${CPPUPnP_DIR}/src/url.cpp
        ${CPPUPnP_DIR}/src/parse_device.cpp
)

target_include_directories(cpp_upnp
    PUBLIC
        ${CPPUPnP_DIR}/include
)

target_link_libraries(cpp_upnp
    PUBLIC
        Boost::system
    PRIVATE
        ${THREAD_LIB}
        Boost::context
)

target_compile_features(cpp_upnp
    PUBLIC
        cxx_std_14
)

target_compile_options(cpp_upnp
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall>
)
