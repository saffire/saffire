include $(top_srcdir)/Makefile.frag


SUBDIRS = objects modules compiler fastcgi general interpreter repl vm


#
# Target files
#
noinst_LIBRARIES = libcomponents.a
libcomponents_a_LIBADD = \
                $(top_srcdir)/components/compiler/libcompiler.a \
                $(top_srcdir)/components/fastcgi/libfastcgi.a \
                $(top_srcdir)/components/general/libgeneral.a \
                $(top_srcdir)/components/repl/librepl.a \
                $(top_srcdir)/components/interpreter/libinterpreter.a \
                $(top_srcdir)/components/vm/libvm.a \
                $(top_srcdir)/modules/libmodules.a \
                $(top_srcdir)/objects/libobjects.a
