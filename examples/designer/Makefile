all: design.pce

CC = ../../bin/huc
SRC = design.c pointer.c waves.c
LIBS = -lst
CFLAGS ?= -fno-recursive -msmall

design.pce: $(SRC) $(CC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

clean:
	rm -f *.pce *.lst *.sym *.s
