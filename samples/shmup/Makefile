CC = huc
CFLAGS = -msmall -fno-recursive
LIBS = -lst

all: shmup.pce

shmup.pce: shmup.c bgm.asm
	$(CC) -scd $(CFLAGS) $(LIBS) $<
	$(CC) $(CFLAGS) $(LIBS) $<

bgm.asm: ackerlight.mod
	mod2mml -d 11=4,12=3,13=1,15=2 -a -n -v 3=28,1=28 -f 3=.25 -o $@ $<

clean:
	rm -f *.sym *.lst *.s *.pce bgm.asm mod2mml.log *.iso


