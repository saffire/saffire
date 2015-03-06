
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
set(3rdparty_libs ${3rdparty_libs} ${ICU_LIBRARIES})

# LibXml2
pkg_check_modules(PC_LIBXML2 REQUIRED libxml-2.0)
set(3rdparty_libs ${3rdparty_libs} ${PC_LIBXML2_LIBRARIES})
include_directories(${PC_LIBXML2_INCLUDE_DIR})
add_definitions(${PC_LIBXML2_CFLAGS})

# LibEdit
pkg_check_modules(PC_LIBEDIT REQUIRED libedit)
set(3rdparty_libs ${3rdparty_libs} ${PC_LIBEDIT_LIBRARIES})
include_directories(${PC_LIBEDIT_INCLUDE_DIR})
add_definitions(${PC_LIBEDIT_CFLAGS})

# LibPCRE
pkg_check_modules(PC_PCRE REQUIRED libpcre)
set(3rdparty_libs ${3rdparty_libs} ${PC_PCRE_LIBRARIES})
include_directories(${PC_PCRE_INCLUDE_DIR})
add_definitions(${PC_PCRE_CFLAGS})


#
# Libraries
#
find_library(m_LIBRARY m REQUIRED)
set(3rdparty_libs ${3rdparty_libs} ${m_LIBRARY})

find_library(bz2_LIBRARY bz2 REQUIRED)
set(3rdparty_libs ${3rdparty_libs} ${bz2_LIBRARY})

find_library(fcgi_LIBRARY fcgi REQUIRED)
set(3rdparty_libs ${3rdparty_libs} ${fcgi_LIBRARY})

find_library(pthread_LIBRARY pthread REQUIRED)
set(3rdparty_libs ${3rdparty_libs} ${pthread_LIBRARY})

add_subdirectory(components)

########################################################################
# main/saffire
########################################################################

set(sources
   main/saffire.c
   main/commands/config.c
   main/commands/fastcgi.c
   main/commands/lint.c
   main/commands/exec.c
   main/commands/repl.c
   main/commands/help.c
   main/commands/version.c
   main/commands/bytecode.c)

set(libs
    objects
    generic
    compiler
    fastcgi
    repl
    vm
    gc
    modules
    debugger)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(saffire ${sources})

target_link_libraries(saffire ${libs})

## !! need to re-link `generic' due recursive refs !! ##
target_link_libraries(saffire generic)

target_link_libraries(saffire ${3rdparty_libs})

install(TARGETS saffire DESTINATION bin)
