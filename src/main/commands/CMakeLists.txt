
set(sources
    config.c
    fastcgi.c
    lint.c
    exec.c
    repl.c
    help.c
    version.c
    bytecode.c)

add_library(commands STATIC ${sources})

add_dependencies(commands generate_interfaces generate_exceptions)

target_link_libraries(
    commands
    fastcgi
    repl
    ${3rdparty_libs})
