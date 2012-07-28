.SILENT :

# Standard applications
BISON=bison
LEX=lex
CC=gcc
LD=gcc
RM=rm

# Additional inker libraries
LIBS =

# Standard flags for C compiler
INCLUDES  =
USR_FLAGS =
C_FLAGS   = -Wall -std=gnu99 $(CC_DEBUG_FLAGS) $(INCLUDES) $(USR_FLAGS)

# Bison flags
BISON_FLAGS = -d $(BISON_DEBUG_FLAGS)

# Lex flags
LEX_FLAGS = $(LEX_DEBUG_FLAGS)

# Linker flags
LD_FLAGS = $(LIBS) $(LD_DEBUG_FLAGS)


# Main target application
TARGET = saffire

# Default make target
all:	$(TARGET)


# Additional debug flags (will be added when "make debug")
debug: CC_DEBUG_FLAGS = -ggdb
debug: BISON_DEBUG_FLAGS = -v
debug: LEX_DEBUG_FLAGS = -d
debug: all


# Our objects that needs to be compiled
OBJS =	main.o \
	svar.o \
	saffire_parser.o \
	lex.yy.o \
	parser.tab.o

# Main target
saffire: $(OBJS)
	$(LD) $(LD_FLAGS) -o $(TARGET) $(OBJS)

# depends on parser.tab.c (actually, parser.tab.h)
main.o:	parser.tab.c

# Bison the parser from our saffire.y
parser.tab.c:
	echo [BISON] $<
	$(BISON) $(BISON_FLAGS) -o $@ saffire.y

# Lex our lexer from saffire.l
lex.yy.c: saffire.l
	echo [LEX] $<
	$(LEX) $(LEX_FLAGS) $<

# Compile .c files into .o objects
.c.o:
	echo [CC] $<
	$(CC) $(C_FLAGS) -c -o $@ $<

# Clean up old data
clean:
	echo [RM] $@
	$(RM) parser.tab.c parser.tab.h lex.yy.c || true
	$(RM) *.o || true
	$(RM) $(TARGET) || true

.PHONY:	all debug clean