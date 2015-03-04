
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
find_package(ICU 4.4 REQUIRED)
include_directories(${ICU_INCLUDE_DIRS})
set(3rdparty_LIBS ${3rdparty_LIBS} ${ICU_LIBRARIES})

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


# parser.tab.c
find_package(BISON REQUIRED)
#set(BisonOutput ${saffire_SOURCE_DIR}/src/components/compiler/parser.tab.c)
#if (BISON_FOUND)
#    add_custom_target(
#        run_bison
#        COMMAND ${BISON_EXECUTABLE}
#            -d
#            --defines=${CMAKE_SOURCE_DIR}/include/compiler/parser.tab.h
#            --output=${BisonOutput}
#            ${CMAKE_SOURCE_DIR}/src/components/compiler/saffire.y
#        COMMENT "Generating parser.tab.c"
#    )
#endif()

# lex.yy.c
find_package(FLEX REQUIRED)
#set(FlexOutput ${saffire_SOURCE_DIR}/src/components/compiler/lex.yy.c)
#set(FlexInput ${saffire_SOURCE_DIR}/src/components/compiler/saffire.l)
#if (FLEX_FOUND)
#
#    add_custom_target(
#        run_flex
#        COMMAND ${FLEX_EXECUTABLE}
#            --outfile=${FlexOutput}
#            --header-file=${saffire_SOURCE_DIR}/include/compiler/lex.yy.h
#            ${FlexInput}
#        COMMENT "Generating lex.yy.c")
#
#endif()

add_subdirectory(components)

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

