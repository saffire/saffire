# Set our optimization / debug flags
if DEBUG
  AM_CFLAGS = -ggdb  -O0 -D__DEBUG
  AM_CXXFLAGS = -ggdb -O0 -D__DEBUG
else
  AM_CFLAGS = -O2
  AM_CXXFLAGS = -O2
endif

if PARSEDEBUG
  AM_CFLAGS += -D__PARSEDEBUG
  AM_YFLAGS = -t -v
else
  AM_YFLAGS =
endif

# Add top include dir
AM_CFLAGS += -I$(top_srcdir)/src/include

# Complain about everything, except unused functions
AM_CFLAGS += -Wall -Wno-unused-function
