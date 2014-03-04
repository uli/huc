# Makefile for mod2mml
#
# Written for Linux development version, September 27, 2001 by Dave Shadoff 
#

#
# Defines
#
BASEDIR=..d ..d ..
include ../../Make_src.inc


HDRS = 
OBJS = mod2mml.o
EXE = mod2mml$(EXESUFFIX)

all: $(EXE)


$(EXE): $(OBJS) $(LIBS) $(HDRS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(CP) $(EXE) $(BINDIR)

#
# Targets
#
include $(MAKETARG)
