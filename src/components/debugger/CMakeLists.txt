
set(sources
    dbgp/dbgp.c
    dbgp/xml.c
    dbgp/sock.c
    dbgp/args.c
    dbgp/commands.c)

add_library(debugger STATIC ${sources})
