
########### next target ###############

SET(tests_STAT_SRCS
   utmain.c
)

add_library(tests STATIC ${tests_STAT_SRCS})


########### next target ###############

SET(utmain_SRCS
   utmain.c
   hashtable/hashtable.c
   dll/dll.c
   bz2/bz2.c
   ini/ini.c
)

add_executable(utmain ${utmain_SRCS})

target_link_libraries(utmain pthread cunit)

########### install files ###############




#original Makefile.am contents follow:

## Complain about everything, except unused functions
#AM_CFLAGS = -Wall -Wno-unused-function
#
#LIB_DIR = ../../src
#
#AM_CFLAGS += -I../../src/include
#
## There are double archives here, this is because of resolving of objects.
#SAFFIRE_LIBS = \
#                 ${LIB_DIR}/libobjects.a \
#                 ${LIB_DIR}/libgeneral.a \
#                 ${LIB_DIR}/libobjects.a \
#                 ${LIB_DIR}/libgeneral.a \
#                 ${LIB_DIR}/libcompiler.a \
#                 ${LIB_DIR}/libfastcgi.a \
#                 ${LIB_DIR}/librepl.a \
#                 ${LIB_DIR}/libvm.a \
#                 ${LIB_DIR}/libcompiler.a \
#                 ${LIB_DIR}/libgc.a \
#                 ${LIB_DIR}/libgeneral.a \
#                 ${LIB_DIR}/libmodules.a \
#                 ${LIB_DIR}/libdebugger.a \
#                 ${LIB_DIR}/libgeneral.a
#
#
#########################################################################
## ../unittests/core
#########################################################################
#
#noinst_LIBRARIES = libtests.a
#libtests_a_SOURCES = utmain.c
#
#
#########################################################################
## unittests
#########################################################################
#
#
#bin_PROGRAMS =
#
#TESTS = utmain
#check_PROGRAMS = utmain
#
#utmain_LDADD = $(SAFFIRE_LIBS) ${libxml2_LIBS} ${ICU_LIBS} -lpthread -lcunit
#
#utmain_SOURCES = \
#                    utmain.c \
#                    hashtable/hashtable.c \
#                    dll/dll.c \
#                    bz2/bz2.c \
#                    ini/ini.c
#
