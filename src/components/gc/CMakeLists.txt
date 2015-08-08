
set(sources
    gc.c)

add_library(gc STATIC ${sources})

add_dependencies(gc generate_interfaces generate_exceptions)

target_link_libraries(
    gc
    generic
    memory
    ${3rdparty_libs})
