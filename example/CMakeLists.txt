cmake_minimum_required(VERSION 3.15)

project(cpp-upnp-example)

list(INSERT CMAKE_PREFIX_PATH 0 ${CMAKE_SOURCE_DIR}/..)
find_package(CPPUPnP REQUIRED)

add_executable            (add-port-mapping ./add-port-mapping.cpp)
target_compile_features   (add-port-mapping PRIVATE cxx_std_17)
target_include_directories(add-port-mapping PRIVATE cpp_upnp)
target_link_libraries     (add-port-mapping cpp_upnp)
