all: ac_test.iso

CC = ../../bin/huc
SRC = ac_test.c

CFLAGS ?= -acd -sgx -fno-recursive -msmall

ac_test.iso: $(SRC) $(CC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
	# inject pce_tile.bin at $E1000
	dd if=pce_tile.bin of=$@ bs=`printf %d 0xe1000` seek=1
	# inject pce_bat.bin at $E9000
	dd if=pce_bat.bin of=ac_test.iso bs=`printf %d 0xe9000` seek=1
	# inject pce_pal.bin at $E9800
	dd if=pce_pal.bin of=ac_test.iso bs=`printf %d 0xe9800` seek=1
	echo padding ISO track to $200000 bytes
	dd if=/dev/zero of=ac_test.iso bs=`printf %d 0x200000` seek=1 count=0

clean:
	rm -f *.pce *.lst *.sym *.s *.iso
