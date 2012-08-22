.SILENT :

# Standard applications
BISON=bison
LEX=lex
CC=gcc
LD=gcc
RM=rm

# Additional inker libraries
LIBS = -lm

# Standard flags for C compiler
INCLUDES  = -I.
USR_FLAGS =
CFLAGS   = -Wall -std=gnu99 $(CC_DEBUG_FLAGS) $(INCLUDES) $(USR_FLAGS)

# Bison flags
BISON_FLAGS = -d $(BISON_DEBUG_FLAGS)

# Lex flags
LEX_FLAGS = $(LEX_DEBUG_FLAGS) --header-file=compiler/lex.yy.h

# Linker flags
LDFLAGS = $(LIBS) $(LD_DEBUG_FLAGS)


# Main target application
TARGET = saffire


all:
	echo "Usage: make <target>"
	echo 
	echo "Targets:"
	echo "  saffire   Build a complete build"
	echo "  debug     Build a debug build"
	echo "  clean     Clean temporary objects and files"

## Default make target
#saffire:	$(TARGET)

# Additional debug flags (will be added when "make debug")
debug: CC_DEBUG_FLAGS = -ggdb -D__DEBUG
debug: BISON_DEBUG_FLAGS = -v
debug: LEX_DEBUG_FLAGS = -d
debug: saffire


# Our objects that needs to be compiled
OBJS =	main.o \
	general/svar.o \
	compiler/ast.o \
	dot/dot.o \
	compiler/saffire_compiler.o \
	interpreter/saffire_interpreter.o \
	compiler/lex.yy.o \
	compiler/parser.tab.o \
	general/hashtable.o

# Main target
saffire: $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)
	echo "Saffire has been build in  $(CURDIR)/$(TARGET)"

# depends on parser.tab.c (actually, parser.tab.h)
main.o:	compiler/lex.yy.c compiler/parser.tab.c

# Bison the parser from our saffire.y
compiler/parser.tab.c:
	echo [BISON] $@
	$(BISON) $(BISON_FLAGS) -o $@ compiler/saffire.y

# Lex our lexer from saffire.l
compiler/lex.yy.c: compiler/saffire.l
	echo [LEX] $<
	$(LEX) $(LEX_FLAGS) -o $@ $<

# Compile .c files into .o objects
.c.o:
	echo [CC] $<
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up old data
clean:
	echo [RM] $@
	$(RM) -f compiler/parser.tab.c compiler/parser.tab.h compiler/lex.yy.c compiler/parser.output || true
	$(RM) -f $(OBJS) || true
	$(RM) -f $(TARGET) || true

.PHONY:	all debug clean
