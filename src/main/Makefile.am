include $(top_srcdir)/Makefile.frag


# Define our programs
bin_PROGRAMS = saffire


saffire_LDADD = \
                $(top_srcdir)/components/compiler/libcompiler.a \
                $(top_srcdir)/components/fastcgi/libfastcgi.a \
                $(top_srcdir)/components/general/libgeneral.a \
                $(top_srcdir)/components/repl/librepl.a \
                $(top_srcdir)/components/interpreter/libinterpreter.a \
                $(top_srcdir)/components/vm/libvm.a \
                $(top_srcdir)/modules/libmodules.a \
                $(top_srcdir)/objects/libobjects.a


# Force the creation of the libcomponents prior
$(top_srcdir)/components/libcomponents.a: FORCE
	(cd $(top_srcdir)/components && $(MAKE) $(AM_MAKEFLAGS) libcomponents.a )


saffire_LIBDADD = $(top_srcdir)/components/libcomponents.a -lpcre -lfcgi


# Define different sources for our targets
saffire_SOURCES = saffire.c \
                  commands/config.c commands/fastcgi.c commands/lint.c \
                  commands/exec.c commands/cli.c \
                  commands/help.c commands/version.c commands/compile.c 

