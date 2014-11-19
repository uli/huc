all: sgx_test.iso sgx_test.sgx

CC = ../../bin/huc
SRC = sgx_test.c

CFLAGS ?= -sgx -fno-recursive -msmall
CDFLAGS = -scd

sgx_test.iso: $(SRC) $(CC)
	$(CC) $(CFLAGS) $(CDFLAGS) $(SRC) $(LIBS)
sgx_test.sgx: $(SRC) $(CC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
	mv sgx_test.pce $@

clean:
	rm -f *.pce *.sgx *.lst *.sym *.s *.iso
