include $(top_srcdir)/Makefile.frag


# Define our programs
bin_PROGRAMS = saffire
#dnl check_PROGRAMS = test

SAFFIRE_LIBS=general/libgeneral.a interactive/libinteractive.a modules/libmodules.a object/libobjects.a

# Don't install libsaffire. It's only for static linking
noinst_LIBRARIES = libsaffire.a $(SAFFIRE_LIBS)

# Libsaffire is the main core of saffire
libsaffire_a_SOURCES = \
                  repl/repl.c \
                  commands/config.c commands/fastcgi.c commands/lint.c commands/exec.c commands/cli.c \
                  commands/help.c commands/version.c \
                  fastcgi/fastcgi_srv.c dot/dot.c

# Link all sources against these libraries
saffire_LIBDADD = libsaffire.a $(SAFFIRE_LIBS) -lpcre -lfcgi

# Define different sources for our targets
saffire_SOURCES = main.c
#dnl test_SOURCES = test.c

