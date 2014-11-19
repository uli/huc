all: design.pce

include ../Make_ex.inc

SRC = design.c pointer.c waves.c
LIBS = -lst
CFLAGS ?= -fno-recursive -msmall

design.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

clean:
	rm -f *.pce *.lst *.sym *.s
