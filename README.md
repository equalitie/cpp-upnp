[![CircleCI](https://circleci.com/gh/equalitie/cpp-upnp/tree/master.svg?style=shield)](https://circleci.com/gh/equalitie/cpp-upnp/tree/master)

# CPPUPnP

Boost.Asio based C++ library to implement parts of the UPnP standard.

## Status

This library is under development and the API will very likely change in a
backward incompatible way.

The current functionality includes listing, creating and deleting of TCP and
UDP port mappings on IGD (v1 and v2) devices.

## Build instructions

### Include in your project

If you have sources of CPPUPnP somewhere on your disk, simply set the
`CPPUPnP_DIR` variable to point to that CPPUPnP's directory and call
`find_package(CPPUPnP REQUIRED)` as is done in the
[example/CMakeLists.txt](example/CMakeLists.txt)

If you don't have CPPUPnP on your disk, have cmake download it prior to calling
`find_package(CPPUPnP REQUIRED)` like so:

    include(FetchContent)
    fetchcontent_populate(CPPUPnP GIT_REPOSITORY <CPPUPnP-GIT-Repository>)
    set(CPPUPnP_DIR ${CMAKE_BINARY_DIR}/cppupnp-src)
    find_package(CPPUPnP REQUIRED)

### Build examples and tests

Please have a look into [.circleci/config.yml](.circleci/config.yml)

## Useful links/documents

[UPnP standards](https://openconnectivity.org/developer/specifications/upnp-resources/upnp/#standards)<br>
[RFC6970](https://tools.ietf.org/html/rfc6970)<br>
[UPnP IGD "A Fox in the Hen House" (pdf)](https://www.blackhat.com/presentations/bh-usa-08/Squire/BH_US_08_Squire_A_Fox_in_the_Hen_House%20White%20Paper.pdf)<br>
[Golang code that inspired this library](https://github.com/syncthing/syncthing/tree/master/lib/upnp)
