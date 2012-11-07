include $(top_srcdir)/Makefile.frag

# Define our programs
bin_PROGRAMS = saffire
#dnl check_PROGRAMS = test

saffire_LDADD = $(top_srcdir)/repl/librepl.a $(top_srcdir)/fastcgi/libfastcgi.a $(top_srcdir)/compile/libcompile.a

# Define different sources for our targets
saffire_SOURCES = saffire.c \
                  commands/config.c commands/fastcgi.c commands/lint.c \
                  commands/exec.c commands/cli.c \
                  commands/help.c commands/version.c commands/compile.c 

#dnl test_SOURCES = test.c
