
if (UNIX AND NOT APPLE)
    list(APPEND 3rdparty_libs rt)
endif()

set(sources
    module_api.c
    io.c
    saffire.c
    standard/os.c
    standard/math.c
    standard/file.c
    sapi/fastcgi.c)

add_library(modules STATIC ${sources})

add_dependencies(modules generate_interfaces generate_exceptions)

target_link_libraries(
    modules
    generic
    dl
    ${3rdparty_libs})
