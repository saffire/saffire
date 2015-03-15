list(APPEND 3rdparty_libs rt)

set(sources
    module_api.c
    io.c
    saffire.c
    standard/math.c
    sapi/fastcgi.c)

add_library(modules STATIC ${sources})

add_dependencies(modules generate_interfaces generate_exceptions)

target_link_libraries(
    modules
    generic
    ${3rdparty_libs})
