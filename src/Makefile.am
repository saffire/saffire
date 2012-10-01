include $(top_srcdir)/Makefile.frag

SUBDIRS = object general interpreter modules compiler

# Define our programs
bin_PROGRAMS = saffire
#dnl check_PROGRAMS = test

SAFFIRE_LIBS=interactive/libinteractive.a \
             modules/libmodules.a \
             objects/libobjects.a \
             src/general/libgeneral.a

# Don't install libsaffire. It's only for static linking
noinst_LIBRARIES = libsaffire.a $(SAFFIRE_LIBS)

# Libsaffire is the main core of saffire

libsaffire_a_SOURCES = \
                  repl/repl.c \
                  commands/config.c commands/fastcgi.c commands/lint.c commands/exec.c commands/cli.c \
                  commands/help.c commands/version.c commands/compile.c \
                  general/ini.c general/parse_options.c \
                  fastcgi/fastcgi_srv.c

# Link all sources against these libraries
libsaffire_a_LIBADD = $(SAFFIRE_LIBS)

saffire_LDADD = libsaffire.a -lpcre -lfcgi

# Define different sources for our targets
saffire_SOURCES = main.c
#dnl test_SOURCES = test.c
