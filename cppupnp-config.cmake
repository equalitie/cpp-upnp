cmake_minimum_required(VERSION 3.13)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.30)
    cmake_policy(SET CMP0167 NEW)
endif()

project(CPPUPnP VERSION 1.0.0 LANGUAGES CXX)

if (NOT BOOST_VERSION)
    set(BOOST_VERSION 1.87.0)
endif()

set(BOOST_COMPONENTS system context)
set(LINK_LIBRARIES_PUB Boost::system)
set(LINK_LIBRARIES_PRIV Boost::context)

if (BOOST_VERSION LESS_EQUAL 1.79.0)
    set(BOOST_COMPONENTS ${BOOST_COMPONENTS} coroutine)
    set(LINK_LIBRARIES_PUB ${LINK_LIBRARIES_PUB} "Boost::coroutine")
endif ()

if (NOT Boost_USE_STATIC_LIBS)
    set(LINK_LIBRARIES_PRIV ${LINK_LIBRARIES_PRIV} "Threads::Threads")
else()
    # When linking with static Boost, we need to link with libboost_thread.
    set(BOOST_COMPONENTS thread ${BOOST_COMPONENTS})
    # Boost::thread adds Threads::Threads automatically
    set(LINK_LIBRARIES_PRIV ${LINK_LIBRARIES_PRIV} "Boost::thread")
endif()

find_package(Threads REQUIRED)
find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS ${BOOST_COMPONENTS})

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
        ${LINK_LIBRARIES_PUB}
    PRIVATE
        ${LINK_LIBRARIES_PRIV}
)

target_compile_features(cpp_upnp
    PUBLIC
        cxx_std_14
)

target_compile_options(cpp_upnp
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall>
)
