cmake_minimum_required(VERSION 2.8.3)

project(libcpuid)

set(libcpuid_dir ${CMAKE_SOURCE_DIR}/../../../lib/libcpuid)

add_custom_target(
    TARGET libcpuid
    COMMAND           libtoolize
    COMMAND           autoreconfig --install
    COMMAND           ./configure
    COMMAND           make
    WORKING_DIRECTORY ${libcpuid_dir}
)

install(TARGETS libcpuid DESTINATION ../compiled/libcpuid)

#add_library(
#install(TARGETS libcpuid DESTINATION src/libcpuid/.libs/libcpuid.a EXPORT
#set_property(TARGET libcpuid PROPERTY IMPORTED_LOCATION ${saffire_SOURCE_DIR}/lib/compiled/libcpuid/libcpuid.a)
