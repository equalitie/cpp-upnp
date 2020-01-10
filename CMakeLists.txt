cmake_minimum_required(VERSION 3.5)

project(CPPUPnP VERSION 1.0.0 LANGUAGES CXX)

option(CPP_UPNP_WITH_TESTS    "Build tests"    ON)
option(CPP_UPNP_WITH_EXAMPLES "Build examples" ON)

find_package(Threads REQUIRED)
find_package(Boost 1.71 REQUIRED COMPONENTS context coroutine system)

add_library(cpp_upnp INTERFACE)

target_include_directories(cpp_upnp
    INTERFACE
        include
        ${Boost_INCLUDE_DIR}
)

target_link_libraries(cpp_upnp
    INTERFACE
        ${Boost_LIBRARIES}
        Threads::Threads
)

target_compile_features(cpp_upnp
    INTERFACE
        cxx_std_14
)

target_compile_options(cpp_upnp
    INTERFACE
        $<$<CXX_COMPILER_ID:GNU>:-Wall>
)

if (CPP_UPNP_WITH_TESTS)
    add_subdirectory(test)
endif()

if (CPP_UPNP_WITH_EXAMPLES)
    add_subdirectory(example)
endif()
