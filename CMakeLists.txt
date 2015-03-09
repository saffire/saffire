cmake_minimum_required(VERSION 2.8.3)

project(saffire)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(saffire_VERSION_MAJOR 0)
set(saffire_VERSION_MINOR 1)

add_subdirectory(include/saffire)
add_subdirectory(src)
add_subdirectory(unittests/core)
