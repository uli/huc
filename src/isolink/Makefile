# Makefile for ISOLINK
#
# Written for Linux developemnt version, August 20, 2001 by Dave Shadoff 
#

#
# Defines
#
BASEDIR=..d ..
include ../Make_src.inc

HDRS = main.h
OBJS = main.o 
EXE = isolink$(EXESUFFIX)

all: $(EXE)

main.o: main.h ../../include/overlay.h

$(EXE): $(OBJS) $(LIBS) $(HDRS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(CP) $(EXE) $(BINDIR)

#
# Targets
#
include $(MAKETARG)

