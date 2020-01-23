cmake_minimum_required(VERSION 3.5)

project(CPPUPnP VERSION 1.0.0 LANGUAGES CXX)

find_package(Threads REQUIRED)
find_package(Boost 1.71 REQUIRED COMPONENTS thread coroutine system)

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
        Boost::coroutine
        Boost::thread
    PRIVATE
        Threads::Threads
)

target_compile_features(cpp_upnp
    PUBLIC
        cxx_std_14
)

target_compile_options(cpp_upnp
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall>
)
