# Makefile for mod2mml
#
# Written for Linux development version, September 27, 2001 by Dave Shadoff 
#

#
# Defines
#
BASEDIR=..d ..d ..
include ../../Make_src.inc
CFLAGS += -Wno-char-subscripts

HDRS = 
OBJS = mod2mml.o analyze.o waves.o
EXE = mod2mml$(EXESUFFIX)

all: $(EXE)


$(EXE): $(OBJS) $(LIBS) $(HDRS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@ -lm
	$(CP) $(EXE) $(BINDIR)

#
# Targets
#
include $(MAKETARG)
