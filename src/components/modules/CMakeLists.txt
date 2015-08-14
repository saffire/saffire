########################################################################
#
########################################################################


if (UNIX AND NOT APPLE)
    list(APPEND 3rdparty_libs rt)
endif()

set(sources
    module_api.c
    saffire.c
    standard/io.c
    standard/io/file.c
    standard/io/socket.c
    standard/os.c
    standard/cpu.c
    standard/math.c
    standard/datetime.c
    sapi/fastcgi.c)

add_library(modules STATIC ${sources})

add_dependencies(modules generate_interfaces generate_exceptions)

include(${CMAKE_SOURCE_DIR}/lib/CPUID.cmake)

target_link_libraries(
    modules
    generic
    memory
    dl
    cpuid
    ${3rdparty_libs})
