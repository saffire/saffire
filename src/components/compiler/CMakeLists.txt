find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

flex_target(
    run_flex
    ${CMAKE_CURRENT_SOURCE_DIR}/saffire.l
    ${CMAKE_CURRENT_SOURCE_DIR}/lex.yy.c)

bison_target(
    run_bison
    ${CMAKE_CURRENT_SOURCE_DIR}/saffire.y
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.tab.c)

add_flex_bison_dependency(run_flex run_bison)

add_custom_target(
    parser
    DEPENDS
        ${FLEX_run_flex_OUTPUTS}
        ${BISON_run_bison_OUTPUTS})

set(sources
    ${FLEX_run_flex_OUTPUTS}
    ${BISON_run_bison_OUTPUTS}
    parser_helpers.c
    ast_nodes.c
    bytecode/marshal.c
    bytecode/io.c
    ast_to_asm.c
    output/dot.c
    output/asm.c)

set_source_files_properties(
    ${FLEX_run_flex_OUTPUTS}
    ${BISON_run_bison_OUTPUTS}
    PROPERTIES
        GENERATED TRUE)

add_library(compiler STATIC ${sources})

add_dependencies(compiler generate_interfaces generate_exceptions parser)
