cmake_minimum_required(VERSION 2.8)

project (saffire)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set (saffire_VERSION_MAJOR 0)
set (saffire_VERSION_MINOR 1)

add_subdirectory(./src)
add_subdirectory(./unittests/core)