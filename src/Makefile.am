bin_PROGRAMS = saffire

if DEBUG
  AM_CFLAGS = -ggdb  -O0 -D__DEBUG
  AM_CXXFLAGS = -ggdb -O0 -D__DEBUG
else
  AM_CFLAGS = -O2
  AM_CXXFLAGS = -O2
endif

BUILT_SOURCES=compiler/lex.yy.c
AM_LFLAGS= --outfile=compiler/$(LEX_OUTPUT_ROOT).c --header-file=compiler/$(LEX_OUTPUT_ROOT).h
AM_YFLAGS = -d

CLEANFILES = compiler/parser.tab.c compiler/parser.tab.h compiler/lex.yy.c compiler/lex.yy.h \
             compiler/*.o \
             dot/*.o \
             general/*.o \
             interpreter/*.o \
             object/*.o

compiler/parser.tab.h: compiler/parser.tab.c

compiler/parser.tab.c: compiler/saffire.y
	$(YACC) $(AM_YFLAGS) -o $@ $<

compiler/lex.yy.c: compiler/saffire.l
	$(LEX) $(AM_LFLAGS) $<

saffire_SOURCES = compiler/parser.tab.c compiler/lex.yy.c \
                  main.c general/svar.c compiler/ast.c dot/dot.c compiler/saffire_compiler.c \
                  interpreter/saffire_interpreter.c general/hashtable.c general/smm.c \
                  general/md5.c \
                  object/object.c object/base.c object/string.c object/numerical.c object/boolean.c \
                  object/null.c object/regex.c


saffire_LDADD = -lpcre

