
# Used for all targets
set(DEFAULT_C_FLAGS "-std=gnu99 -Wall -Wno-unused-function -Wno-unused-result")

# Additional fields for targets
set(CMAKE_C_FLAGS             "${DEFAULT_C_FLAGS} -O3 ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG       "${DEFAULT_C_FLAGS} -ggdb -O0 -D__DEBUG ${CMAKE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_COVERAGE    "${DEFAULT_C_FLAGS} -ggdb -O0 -fprofile-arcs -ftest-coverage ${CMAKE_C_FLAGS_COVERAGE}")

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
find_package(ICU 4.2 REQUIRED)
include_directories(${ICU_INCLUDE_DIRS})
list(APPEND 3rdparty_libs ${ICU_LIBRARIES})

# LibXml2
pkg_check_modules(PC_LIBXML2 REQUIRED libxml-2.0)
list(APPEND 3rdparty_libs ${PC_LIBXML2_LIBRARIES})
include_directories(${PC_LIBXML2_INCLUDE_DIR})
add_definitions(${PC_LIBXML2_CFLAGS})

# LibEdit
pkg_check_modules(PC_LIBEDIT REQUIRED libedit)
list(APPEND 3rdparty_libs ${PC_LIBEDIT_LIBRARIES})
include_directories(${PC_LIBEDIT_INCLUDE_DIR})
add_definitions(${PC_LIBEDIT_CFLAGS})

# LibPCRE
pkg_check_modules(PC_PCRE REQUIRED libpcre)
list(APPEND 3rdparty_libs ${PC_PCRE_LIBRARIES})
include_directories(${PC_PCRE_INCLUDE_DIR})
add_definitions(${PC_PCRE_CFLAGS})


#
# Libraries
#
find_library(M_LIBRARY m REQUIRED)
list(APPEND 3rdparty_libs ${M_LIBRARY})

find_library(BZ2_LIBRARY bz2 REQUIRED)
list(APPEND 3rdparty_libs ${BZ2_LIBRARY})

find_library(FCGI_LIBRARY fcgi REQUIRED)
list(APPEND 3rdparty_libs ${FCGI_LIBRARY})

find_library(PTHREAD_LIBRARY pthread REQUIRED)
list(APPEND 3rdparty_libs ${PTHREAD_LIBRARY})


add_subdirectory(components)
add_subdirectory(main)

########################################################################
# main/saffire
########################################################################

set(sources
    main/saffire.c)

add_executable(saffire ${sources})

target_link_libraries(saffire commands ${3rdparty_libs})

install(TARGETS saffire DESTINATION bin)
