# Makefile for mml
#

#
# Defines
#
BASEDIR=..d ..d ..
include ../../Make_src.inc
CFLAGS += -Wno-char-subscripts -D_GNU_SOURCE

HDRS = 
LIBS = -lm
OBJS = mml.o
EXE = mml$(EXESUFFIX)

all: $(EXE)


$(EXE): $(OBJS) $(HDRS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(CP) $(EXE) $(BINDIR)

#
# Targets
#
include $(MAKETARG)
