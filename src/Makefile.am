bin_PROGRAMS = saffire

if DEBUG
AM_CFLAGS = -ggdb  -O0 -D__DEBUG
AM_CXXFLAGS = -ggdb -O0 -D__DEBUG
else
AM_CFLAGS = -O2
AM_CXXFLAGS = -O2
endif


saffire_SOURCES = main.c general/svar.c compiler/ast.c dot/dot.c compiler/saffire_compiler.c \
                  interpreter/saffire_interpreter.c compiler/lex.yy.c compiler/parser.tab.c \
                  general/hashtable.c general/smm.c
saffire_LDADD = $(INIT_LIBS) -lm

