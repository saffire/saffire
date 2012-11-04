include $(top_srcdir)/Makefile.frag

#
# Some additional rules for flex/bison file handling
#
BUILT_SOURCES=lex.yy.c
AM_LFLAGS = --outfile=$(LEX_OUTPUT_ROOT).c --header-file=$(LEX_OUTPUT_ROOT).h
AM_YFLAGS += -d

AM_CFLAGS += -I$(top_srcdir)/src/include


#
# parser.tab.h cannot be used before parser.tab.c
#
$(top_srcdir)/src/include/compiler/parser.tab.h: parser.tab.c

# parser.tab.c is created by bison
parser.tab.c: saffire.y
	$(YACC) $(AM_YFLAGS) -o $@ $<

# lex.yy.c is created by lex
lex.yy.c: saffire.l
	$(LEX) $(AM_LFLAGS) $<

# Cleanup these files on a "make clean"
CLEANFILES = parser.tab.c $(top_srcdir)/src/include/compiler/parser.tab.h lex.yy.c lex.yy.h


#
# Target files
#
noinst_LIBRARIES = libcompiler.a
libcompiler_a_SOURCES = parser.tab.c lex.yy.c ast.c saffire_compiler.c


