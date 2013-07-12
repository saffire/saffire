# Complain about everything, except unused functions
AM_CFLAGS = -Wall -Wno-unused-function

LIB_DIR = ../../src

# Add GNUSOURCE define to ignore warning for aprintf
AM_CFLAGS += -D_GNU_SOURCE
AM_CFLAGS += -I../../src/include

# There are double archives here, this is because of resolving of objects.
SAFFIRE_LIBS = \
                 ${LIB_DIR}/libobjects.a \
                 ${LIB_DIR}/libgeneral.a \
                 ${LIB_DIR}/libobjects.a \
                 ${LIB_DIR}/libgeneral.a \
                 ${LIB_DIR}/libcompiler.a \
                 ${LIB_DIR}/libfastcgi.a \
                 ${LIB_DIR}/librepl.a \
                 ${LIB_DIR}/libvm.a \
                 ${LIB_DIR}/libcompiler.a \
                 ${LIB_DIR}/libgc.a \
                 ${LIB_DIR}/libgeneral.a \
                 ${LIB_DIR}/libmodules.a \
                 ${LIB_DIR}/libdebugger.a \
                 ${LIB_DIR}/libgeneral.a


########################################################################
# ../unittests/core
########################################################################

noinst_LIBRARIES = libtests.a
libtests_a_SOURCES = utmain.c


########################################################################
# unittests
########################################################################


bin_PROGRAMS =

TESTS = utmain
check_PROGRAMS = utmain

utmain_LDADD = $(SAFFIRE_LIBS) ${libxml2_LIBS} -lpthread -lcunit

utmain_SOURCES = \
                    utmain.c \
                    hashtable/hashtable.c \
                    dll/dll.c \
                    bz2/bz2.c \
                    ini/ini.c

