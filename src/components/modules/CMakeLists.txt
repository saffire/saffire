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

########################################################################
# Add libcpuid
########################################################################
include_directories(${saffire_SOURCE_DIR}/lib/src)
add_library(libcpuid STATIC IMPORTED)
set_property(TARGET libcpuid PROPERTY IMPORTED_LOCATION ${saffire_SOURCE_DIR}/lib/src/libcpuid/libcpuid/.libs/libcpuid.a)



########################################################################
add_dependencies(modules generate_interfaces generate_exceptions)

target_link_libraries(
    modules
    generic
    dl
    libcpuid
    ${3rdparty_libs})
