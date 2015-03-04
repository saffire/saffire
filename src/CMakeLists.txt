
# @TODO: Only set this when not doing debug
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wno-unused-function -Wno-unused-result")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wno-unused-function -ggdb  -O0 -D__DEBUG")

# Add include directory so include files are found
include_directories(${saffire_SOURCE_DIR}/include)

########################################################################
# additional 3rd party libraries
########################################################################

#
# Packages
#

# Use pkg-config
find_package(PkgConfig REQUIRED)

# ICU
pkg_check_modules(PC_ICUI18N REQUIRED icu-i18n)
set(3rdparty_LIBS ${3rdparty_LIBS} ${PC_ICUI18N_LIBRARIES})
include_directories(${PC_ICUI18N_INCLUDE_DIR})
add_definitions(${PC_ICUI18N_CFLAGS})

# LibXml2
pkg_check_modules(PC_LIBXML2 REQUIRED libxml-2.0)
set(3rdparty_LIBS ${3rdparty_LIBS} ${PC_LIBXML2_LIBRARIES})
include_directories(${PC_LIBXML2_INCLUDE_DIR})
add_definitions(${PC_LIBXML2_CFLAGS})

# LibEdit
pkg_check_modules(PC_LIBEDIT REQUIRED libedit)
set(3rdparty_LIBS ${3rdparty_LIBS} ${PC_LIBEDIT_LIBRARIES})
include_directories(${PC_LIBEDIT_INCLUDE_DIR})
add_definitions(${PC_LIBEDIT_CFLAGS})

## LibICU
#find_package(ICU 4.4 REQUIRED)
#include_directories(${ICU_INCLUDE_DIR})
#set(3rdparty_LIBS ${3rdparty_LIBS} ${ICU_LIBRARIES})

# LibPCRE
pkg_check_modules(PC_PCRE REQUIRED libpcre)
set(3rdparty_LIBS ${3rdparty_LIBS} ${PC_PCRE_LIBRARIES})
include_directories(${PC_PCRE_INCLUDE_DIR})
add_definitions(${PC_PCRE_CFLAGS})


#
# Libraries
#
find_library(m_LIBRARY m REQUIRED)
set(3rdparty_LIBS ${3rdparty_LIBS} ${m_LIBRARY})

find_library(bz2_LIBRARY bz2 REQUIRED)
set(3rdparty_LIBS ${3rdparty_LIBS} ${bz2_LIBRARY})

find_library(fcgi_LIBRARY fcgi REQUIRED)
set(3rdparty_LIBS ${3rdparty_LIBS} ${fcgi_LIBRARY})

find_library(pthread_LIBRARY pthread REQUIRED)
set(3rdparty_LIBS ${3rdparty_LIBS} ${pthread_LIBRARY})


########################################################################
# components/compiler
########################################################################

#SET_SOURCE_FILES_PROPERTIES({CMAKE_SOURCE_DIR}/include/saffire/objects/_generated_interfaces.h GENERATED)
#SET_SOURCE_FILES_PROPERTIES(components/vm/_generated_vm_opcodes.c GENERATED)


# parser.tab.c
find_package(BISON REQUIRED)
set(BisonOutput ${saffire_SOURCE_DIR}/src/components/compiler/parser.tab.c)
if (BISON_FOUND)
    add_custom_command(
        OUTPUT ${BisonOutput}
        COMMAND ${BISON_EXECUTABLE}
            -d
            --defines=${CMAKE_SOURCE_DIR}/include/saffire/compiler/parser.tab.h
            --output=${BisonOutput}
            ${CMAKE_SOURCE_DIR}/src/components/compiler/saffire.y
        IMPLICIT_DEPENDS C components/vm/_generated_vm_opcodes.c
        IMPLICIT_DEPENDS C components/objects/_generated_exceptions.inc
        IMPLICIT_DEPENDS C components/objects/_generated_interfaces.inc
        COMMENT "Generating parser.tab.c"
    )
endif()

# lex.yy.c
find_package(FLEX REQUIRED)
set(FlexOutput ${saffire_SOURCE_DIR}/src/components/compiler/lex.yy.c)
set(FlexInput ${saffire_SOURCE_DIR}/src/components/compiler/saffire.l)
if (FLEX_FOUND)
    add_custom_command(
        OUTPUT ${FlexOutput}
        SOURCE ${FlexInput}
        COMMAND ${FLEX_EXECUTABLE}
            --outfile=${FlexOutput}
            --header-file=${saffire_SOURCE_DIR}/include/saffire/compiler/lex.yy.h
            ${FlexInput}
        COMMENT "Generating lex.yy.c"
    )
endif()

set(compiler_SRCS
    components/compiler/parser.tab.c
    components/compiler/lex.yy.c
    components/compiler/parser_helpers.c
    components/compiler/ast_nodes.c
    components/compiler/bytecode/marshal.c
    components/compiler/bytecode/io.c
    components/compiler/ast_to_asm.c
    components/compiler/output/dot.c
    components/compiler/output/asm.c
)
add_library(compiler STATIC ${compiler_SRCS})


########################################################################
# components/fastcgi
########################################################################

set(fastcgi_SRCS
    components/fastcgi/fastcgi_srv.c
    components/fastcgi/scoreboard.c
    components/fastcgi/daemonize.c
)
add_library(fastcgi STATIC ${fastcgi_SRCS})


########################################################################
# components/general
########################################################################

set(general_SRCS
    components/general/hashtable.c
    components/general/hash/chained.c
    components/general/hash/hash_funcs.c
    components/general/smm.c
    components/general/smm/asprintf.c
    components/general/md5.c
    components/general/dll.c
    components/general/stack.c
    components/general/parse_options.c
    components/general/popen2.c
    components/general/gpg.c
    components/general/path_handling.c
    components/general/bzip2.c
    components/general/config.c
    components/general/output.c
    components/general/mutex.c
    components/general/printf/arg_printf.c
    components/general/ini.c
    components/general/base64.c
    components/general/string.c
    components/general/unicode.c
)
# Note that the "general" library is called libgeneric.a.
# 'general' is a reserved keyword
add_library(generic STATIC ${general_SRCS})


########################################################################
# components/modules
########################################################################

set(modules_SRCS
    components/modules/module_api.c
    components/modules/io.c
    components/modules/saffire.c
    components/modules/sapi/fastcgi.c
)
add_library(modules STATIC ${modules_SRCS})


########################################################################
# components/debugger
########################################################################

set(debugger_SRCS
    components/debugger/dbgp/dbgp.c
    components/debugger/dbgp/xml.c
    components/debugger/dbgp/sock.c
    components/debugger/dbgp/args.c
    components/debugger/dbgp/commands.c
)
add_library(debugger STATIC ${debugger_SRCS})


########################################################################
# components/objects
########################################################################
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/components/objects/_generated_exceptions.inc ${CMAKE_SOURCE_DIR}/include/saffire/objects/_generated_exceptions.h
    COMMAND php
        ${CMAKE_SOURCE_DIR}/support/generators/generate_exceptions.php
        ${CMAKE_SOURCE_DIR}/src/components/objects/exceptions.dat
        ${CMAKE_SOURCE_DIR}/src/components/objects/_generated_exceptions.inc
        ${CMAKE_SOURCE_DIR}/include/saffire/objects/_generated_exceptions.h
    COMMENT "Generating exception objects"
)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/components/objects/_generated_interfaces.inc ${CMAKE_SOURCE_DIR}/include/saffire/objects/_generated_interfaces.h
    COMMAND php
        ${CMAKE_SOURCE_DIR}/support/generators/generate_interfaces.php
        ${CMAKE_SOURCE_DIR}/src/components/objects/interfaces.dat
        ${CMAKE_SOURCE_DIR}/src/components/objects/_generated_interfaces.inc
        ${CMAKE_SOURCE_DIR}/include/saffire/objects/_generated_interfaces.h
    COMMENT "Generating interface objects"
)

set(objects_SRCS
    components/objects/object.c
    components/objects/base.c
    components/objects/null.c
    components/objects/boolean.c
    components/objects/numerical.c
    components/objects/string.c
    components/objects/regex.c
    components/objects/callable.c
    components/objects/attrib.c
    components/objects/hash.c
    components/objects/tuple.c
    components/objects/list.c
    components/objects/user.c
    components/objects/exception.c
)
add_library(objects STATIC ${objects_SRCS})


########################################################################
# components/repl
########################################################################

set(repl_SRCS
    components/repl/repl.c
    components/repl/prompt.c
)
add_library(repl STATIC ${repl_SRCS})


########################################################################
# components/vm
########################################################################

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/components/vm/_generated_vm_opcodes.c ${CMAKE_SOURCE_DIR}/include/saffire/vm/_generated_vm_opcodes.h
    COMMAND php
        ${CMAKE_SOURCE_DIR}/support/generators/generate_vm_opcodes.php
        ${CMAKE_SOURCE_DIR}/src/components/vm/vm_opcodes.dat
        ${CMAKE_SOURCE_DIR}/src/components/vm/_generated_vm_opcodes.c
        ${CMAKE_SOURCE_DIR}/include/saffire/vm/_generated_vm_opcodes.h
    COMMENT "Generating vm opcodes source files"
)

set(vm_SRCS
    components/vm/vm.c
    components/vm/block.c
    components/vm/stackframe.c
    components/vm/codeblock.c
    components/vm/context.c
    components/vm/thread.c
    components/vm/import.c
    components/vm/_generated_vm_opcodes.c
)
add_library(vm STATIC ${vm_SRCS})


########################################################################
# components/gc
########################################################################

set(gc_SRCS
    components/gc/gc.c
)
add_library(gc STATIC ${gc_SRCS})


########################################################################
# main/saffire
########################################################################

set(saffire_SRCS
   main/saffire.c
   main/commands/config.c
   main/commands/fastcgi.c
   main/commands/lint.c
   main/commands/exec.c
   main/commands/repl.c
   main/commands/help.c
   main/commands/version.c
   main/commands/bytecode.c
)

set(saffire_LIBS
    objects
    generic
    compiler
    fastcgi
    repl
    vm
    gc
    modules
    debugger
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_executable(saffire ${saffire_SRCS})

# Link saffire_LIBS twice, as there are dependencies on libraries
target_link_libraries(saffire ${saffire_LIBS} ${saffire_LIBS} ${3rdparty_LIBS})

install(TARGETS saffire DESTINATION bin)

