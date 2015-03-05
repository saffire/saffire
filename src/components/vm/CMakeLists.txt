
add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/_generated_vm_opcodes.c
    COMMAND python
        ${CMAKE_SOURCE_DIR}/support/generators/generate_vm_opcodes.py
        ${CMAKE_SOURCE_DIR}/src/components/vm/vm_opcodes.dat
        ${CMAKE_SOURCE_DIR}/src/components/vm/_generated_vm_opcodes.c
        ${CMAKE_SOURCE_DIR}/include/vm/_generated_vm_opcodes.h
    COMMENT "Generating vm opcodes source files")

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
