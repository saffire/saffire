
set(sources
    repl.c
    prompt.c)

add_library(repl STATIC ${sources})

add_dependencies(compiler parser)
