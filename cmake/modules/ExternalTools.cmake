
set(AUTORECONF autoreconf)
set(MAKE make)

if(APPLE)
    set(LIBTOOLIZE glibtoolize)
elseif(UNIX)
    set(LIBTOOLIZE libtoolize)
endif()
