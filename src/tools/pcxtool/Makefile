# Makefile for pcxtool
#
# Written for Linux development version, September 27, 2001 by Dave Shadoff 
#

#
# Defines
#
BASEDIR=..d ..d ..
include ../../Make_src.inc

HDRS = 
OBJS = pcxtool.o
EXE = pcxtool$(EXESUFFIX)

all: $(EXE)


$(EXE): $(OBJS) $(LIBS) $(HDRS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(CP) $(EXE) $(BINDIR)

#
# Targets
#
include $(MAKETARG)

