all: scroll.pce

include ../Make_ex.inc

SRC = scroll.c

CFLAGS ?= -fno-recursive -msmall

scroll.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

clean:
	rm -f *.pce *.lst *.sym *.s
