bin_PROGRAMS = saffire

AM_CXXFLAGS = $(INIT_CFLAGS) -Wall

AM_DEBUGFLAGS = -O0 -D _DEBUG
AM_RELEASEFAGS = -O2

saffire_SOURCES = main.c general/svar.c compiler/ast.c dot/dot.c compiler/saffire_compiler.c \
                  interpreter/saffire_interpreter.c compiler/lex.yy.c compiler/parser.tab.c \
                  general/hashtable.c
saffire_LDADD = $(INIT_LIBS) -lm

