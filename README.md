[![CircleCI](https://circleci.com/gh/equalitie/cpp-upnp/tree/master.svg?style=shield)](https://circleci.com/gh/equalitie/cpp-upnp/tree/master)

# CPPUPnP

Boost.Asio based C++ library to communicate with Internet Gateway Devices (IGDs).

## Status

This library is currently under development and the API will very likely change
in a backward incompatible way.

## Build examples and tests

### Examples

    mkdir build.examples
    cd build.examples
    cmake <PATH_TO_CPP_UPNP>/example
    cmake --build .

### Tests

    mkdir build.tests
    cd build.tests
    cmake <PATH_TO_CPP_UPNP>/tests
    cmake --build .

For more info please have a look into .circleci/config.yml

## Include in your project

Please have a look into example/CMakeLists.txt

## Useful links/documents

[RFC6970](https://tools.ietf.org/html/rfc6970)<br>
[UPnP IGD "A Fox in the Hen House" (pdf)](https://www.blackhat.com/presentations/bh-usa-08/Squire/BH_US_08_Squire_A_Fox_in_the_Hen_House%20White%20Paper.pdf)<br>
[Golang code that inspired this library](https://github.com/syncthing/syncthing/tree/master/lib/upnp)
