# Note that the "general" library is called libgeneric.a.
# 'general' is a reserved keyword

set(sources
    hashtable.c
    hash/chained.c
    hash/hash_funcs.c
    smm.c
    smm/asprintf.c
    md5.c
    dll.c
    stack.c
    parse_options.c
    popen2.c
    gpg.c
    path_handling.c
    bzip2.c
    config.c
    output.c
    mutex.c
    printf/arg_printf.c
    ini.c
    base64.c
    string.c
    unicode.c)

add_library(generic STATIC ${sources})

add_dependencies(generic generate_interfaces generate_exceptions)

target_link_libraries(
    generic
    debugger
    objects
    vm
    ${3rdparty_libs})
