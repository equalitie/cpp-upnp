cmake_minimum_required(VERSION 3.5)
project(UPnP VERSION 1.0.0 LANGUAGES CXX)

find_package(Boost 1.71 REQUIRED COMPONENTS system context coroutine)

add_library(upnp INTERFACE)

add_library(UPnP::upnp ALIAS upnp)

target_include_directories(upnp INTERFACE include ${Boost_INCLUDE_DIR})
target_compile_features(upnp INTERFACE cxx_std_14)
target_compile_options(upnp INTERFACE $<$<CXX_COMPILER_ID:GNU>:-Wall>)

add_subdirectory("./example")

#######################
# Tests
add_subdirectory(test)
#######################
