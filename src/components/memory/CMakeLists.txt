set(sources
    smm.c
    printf/asprintf.c
)

add_library(memory STATIC ${sources})
