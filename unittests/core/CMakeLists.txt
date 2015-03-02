include_directories(../../include)
link_directories(../../src)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-function")

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

target_link_libraries(utmain ${saffire_LIBS} ${saffire_LIBS} pthread cunit)
