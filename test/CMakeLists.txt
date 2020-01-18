cmake_minimum_required(VERSION 3.15)

project(cpp-upnp-tests)

list(INSERT CMAKE_PREFIX_PATH 0 ${CMAKE_SOURCE_DIR}/..)
find_package(CPPUPnP REQUIRED)

add_executable            (test-url ./test-url.cpp)
target_compile_features   (test-url PRIVATE cxx_std_17)
target_include_directories(test-url PRIVATE cpp_upnp)
target_link_libraries     (test-url cpp_upnp)
