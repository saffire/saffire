
set(sources
    dbgp/dbgp.c
    dbgp/xml.c
    dbgp/sock.c
    dbgp/args.c
    dbgp/commands.c)

add_library(debugger STATIC ${sources})

add_dependencies(debugger generate_interfaces generate_exceptions)

target_link_libraries(
    debugger
    generic
    memory
    ${3rdparty_libs})
