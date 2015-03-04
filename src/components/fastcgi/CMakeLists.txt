
set(sources
    fastcgi_srv.c
    scoreboard.c
    daemonize.c)

add_library(fastcgi STATIC ${sources})
