find_package(Threads REQUIRED)

add_executable(add-port-mapping "./add-port-mapping.cpp")
target_compile_features(add-port-mapping PRIVATE cxx_std_17)
target_link_libraries(add-port-mapping upnp Threads::Threads ${Boost_LIBRARIES})



