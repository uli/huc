# Makefile for HuC
#
# Written for Linux development version, March 4, 2001 by Dave Shadoff 
#

#
# Defines
#
BASEDIR=..d ..
include ../Make_src.inc


HDRS = code.h data.h defs.h error.h gen.h lex.h preproc.h pseudo.h sym.h while.h
OBJS = code.o const.o data.o error.o expr.o \
       function.o gen.o io.o lex.o main.o \
       optimize.o pragma.o preproc.o primary.o pseudo.o \
       stmt.o sym.o while.o struct.o enum.o initials.o
EXE = huc$(EXESUFFIX)

all: $(EXE)

$(EXE): $(OBJS) $(LIBS) $(HDRS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(CP) $(EXE) $(BINDIR)

$(OBJS): code.h data.h defs.h error.h io.h

code.o:  function.h main.h optimize.h
const.o: const.h lex.h primary.h sym.h
expr.o:  expr.h function.h gen.h lex.h primary.h
function.o: expr.h function.h gen.h lex.h optimize.h pragma.h pseudo.h stmt.h sym.h
gen.o:   primary.h sym.h
io.o:    optimize.h preproc.h
lex.o:   lex.h preproc.h
main.o:  const.h function.h gen.h lex.h main.h optimize.h pragma.h preproc.h \
	 pseudo.h sym.h
optimize.o: function.h
pragma.o:   lex.h pragma.h sym.h
preproc.o:  lex.h optimize.h preproc.h sym.h
primary.o:  expr.h gen.h lex.h primary.h sym.h
pseudo.o:   lex.h optimize.h primary.h pseudo.h sym.h
stmt.o:  expr.h gen.h lex.h preproc.h primary.h stmt.h sym.h while.h
sym.o:   const.h gen.h lex.h primary.h pragma.h sym.h
while.o: gen.h while.h

indent:	uncrustify.cfg
	uncrustify -l c -c $< --replace *.c *.h

#
# Targets
#
include $(MAKETARG)
