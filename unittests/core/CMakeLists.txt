include_directories(../../include)
link_directories(../../src)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-function")

# Use pkg-config
find_package(PkgConfig REQUIRED)

# Cunit
pkg_check_modules(PC_CUNIT REQUIRED cunit)
list(APPEND 3rdparty_libs ${PC_CUNIT_LIBRARIES})
include_directories(${PC_CUNIT_INCLUDE_DIR})
link_directories(${PC_CUNIT_LIBRARY_DIRS})
add_definitions(${PC_CUNIT_CFLAGS})



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

SET(utmain_SRCS
   utmain.c
   hashtable/hashtable.c
   dll/dll.c
   bz2/bz2.c
   ini/ini.c
)

add_executable(utmain ${utmain_SRCS})

target_link_libraries(utmain ${saffire_LIBS} ${saffire_LIBS} ${3rdparty_libs} pthread)
