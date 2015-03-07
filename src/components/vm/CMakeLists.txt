
add_custom_target(
    generate_vm_opcodes
    COMMAND python
        ${CMAKE_SOURCE_DIR}/support/generators/generate_vm_opcodes.py
        ${CMAKE_SOURCE_DIR}/src/components/vm/vm_opcodes.dat
        ${CMAKE_SOURCE_DIR}/src/components/vm/_generated_vm_opcodes.c
        ${CMAKE_SOURCE_DIR}/include/saffire/vm/_generated_vm_opcodes.h
    COMMENT "Generating vm opcodes source files")

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/_generated_vm_opcodes.c
    COMMAND generate_vm_opcodes)

set_source_files_properties(_generated_vm_opcodes.c PROPERTIES GENERATED TRUE)

set(sources
    vm.c
    block.c
    stackframe.c
    codeblock.c
    context.c
    thread.c
    import.c
    _generated_vm_opcodes.c)

add_library(vm STATIC ${sources})

add_dependencies(vm generate_interfaces generate_exceptions generate_vm_opcodes)

target_link_libraries(
    vm
    modules
    compiler
    ${3rdparty_libs})
