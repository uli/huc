all: scroll.pce

CC = ../../bin/huc
SRC = scroll.c

CFLAGS ?= -fno-recursive -msmall

scroll.pce: $(SRC) $(CC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

clean:
	rm -f *.pce *.lst *.sym *.s
