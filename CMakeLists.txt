cmake_minimum_required(VERSION 2.8)

project (saffire)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

set (saffire_VERSION_MAJOR 0)
set (saffire_VERSION_MINOR 1)

include_directories(include)

########################################################################
# Generate custom files
########################################################################

# config.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h)

# Generate gitversion.h
execute_process(
    COMMAND git rev-parse HEAD
    OUTPUT_VARIABLE GITREV_SHA1
    OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gitversion.h.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/gitversion.h"
    @ONLY)

add_subdirectory(./src)
#add_subdirectory(./unittests/core)