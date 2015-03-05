
set(sources
    module_api.c
    io.c
    saffire.c
    sapi/fastcgi.c)

add_library(modules STATIC ${sources})

add_dependencies(modules generate_interfaces generate_exceptions)
