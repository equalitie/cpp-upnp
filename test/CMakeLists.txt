cmake_minimum_required(VERSION 3.15)

project(cpp-upnp-tests)

add_executable            (test-url ./test-url.cpp ../src/url.cpp)
target_include_directories(test-url PRIVATE ../include)
