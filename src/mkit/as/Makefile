# MAGICKIT MAKEFILE
#

#
# Defines
#
BASEDIR=..d ..d ..
include ../../Make_src.inc

OBJS   = main.o input.o assemble.o expr.o code.o command.o\
         macro.o func.o proc.o symbol.o pcx.o output.o crc.o\
         pce.o map.o mml.o nes.o

TARGPCE  = pceas$(EXESUFFIX)
TARGNES  = nesasm$(EXESUFFIX)
TARGETS  = $(TARGPCE) $(TARGNES)

#
#

all: $(TARGETS)


# ASSEMBLER
#

$(OBJS): defs.h externs.h protos.h
main.o: inst.h vars.h
expr.o: expr.h
pce.o: pce.h
nes.o: nes.h

# EXE
#

$(TARGPCE) : $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	$(CP) $(TARGPCE) $(BINDIR)

$(TARGNES) : $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	$(CP) $(TARGNES) $(BINDIR)

indent:	uncrustify.cfg
	uncrustify -l c -q -c uncrustify.cfg --replace *.c *.h

#
# Targets
#
include $(MAKETARG)

clean::
	rm -f $(TARGETS) $(BINDIR)/$(TARGPCE) $(BINDIR)/$(TARGNES)
