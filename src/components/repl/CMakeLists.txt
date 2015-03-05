
set(sources
    repl.c
    prompt.c)

add_library(repl STATIC ${sources})

add_dependencies(repl generate_interfaces generate_exceptions parser)

target_link_libraries(
    repl
    compiler
    generic
    vm
    ${3rdparty_libs})
