
add_custom_target(
    generate_interfaces
    COMMAND python
        ${CMAKE_SOURCE_DIR}/support/generators/generate_interfaces.py
        ${CMAKE_SOURCE_DIR}/src/components/objects/interfaces.dat
        ${CMAKE_SOURCE_DIR}/src/components/objects/_generated_interfaces.inc
        ${CMAKE_SOURCE_DIR}/include/saffire/objects/_generated_interfaces.h
    COMMENT "Generating interface objects")

add_custom_target(
    generate_exceptions
    COMMAND python
        ${CMAKE_SOURCE_DIR}/support/generators/generate_exceptions.py
        ${CMAKE_SOURCE_DIR}/src/components/objects/exceptions.dat
        ${CMAKE_SOURCE_DIR}/src/components/objects/_generated_exceptions.inc
        ${CMAKE_SOURCE_DIR}/include/saffire/objects/_generated_exceptions.h
    COMMENT "Generating exception objects")

set(sources
    object.c
    base.c
    null.c
    boolean.c
    numerical.c
    string.c
    regex.c
    callable.c
    attrib.c
    hash.c
    tuple.c
    list.c
    user.c
    exception.c)

add_library(objects STATIC ${sources})

add_dependencies(objects generate_interfaces generate_exceptions)
