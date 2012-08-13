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
INCLUDES  =
USR_FLAGS =
CFLAGS   = -Wall -std=gnu99 $(CC_DEBUG_FLAGS) $(INCLUDES) $(USR_FLAGS)

# Bison flags
BISON_FLAGS = -d $(BISON_DEBUG_FLAGS)

# Lex flags
LEX_FLAGS = $(LEX_DEBUG_FLAGS) --header-file=lex.yy.h

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
	echo "  test      Execute tests"

## Default make target
#saffire:	$(TARGET)

# Additional debug flags (will be added when "make debug")
debug: CC_DEBUG_FLAGS = -ggdb -D__DEBUG
debug: BISON_DEBUG_FLAGS = -v
debug: LEX_DEBUG_FLAGS = -d
debug: saffire


# Our objects that needs to be compiled
OBJS =	main.o \
	svar.o \
	ast.o \
	saffire_parser.o \
	saffire_compiler.o \
	saffire_interpreter.o \
	lex.yy.o \
	parser.tab.o \
	hashtable/hashtable.o

# Main target
saffire: $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)
	echo "Saffire has been build in  $(CURDIR)/$(TARGET)"

# depends on parser.tab.c (actually, parser.tab.h)
main.o:	lex.yy.c parser.tab.c

# Bison the parser from our saffire.y
parser.tab.c:
	echo [BISON] $@
	$(BISON) $(BISON_FLAGS) -o $@ saffire.y

# Lex our lexer from saffire.l
lex.yy.c: saffire.l
	echo [LEX] $<
	$(LEX) $(LEX_FLAGS) $<

# Compile .c files into .o objects
.c.o:
	echo [CC] $<
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up old data
clean:
	echo [RM] $@
	$(RM) parser.tab.c parser.tab.h lex.yy.c parser.output || true
	$(RM) $(OBJS) || true
	$(RM) $(TARGET) || true

.PHONY:	all debug clean
