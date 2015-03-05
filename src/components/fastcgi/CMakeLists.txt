
set(sources
    fastcgi_srv.c
    scoreboard.c
    daemonize.c)

add_library(fastcgi STATIC ${sources})

add_dependencies(fastcgi generate_interfaces generate_exceptions)
