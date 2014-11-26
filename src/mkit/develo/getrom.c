#include <stdio.h>
#include <string.h>
#include "lib/develo.h"


int
get_block(unsigned char *buf, int start, int length)
{
	int retry;

	retry = 0;
	while (dv_get_ram(buf, start, length) != DV_OK) {
		if (++retry == 3) {
			return (DV_ERR);
		}
	}
	return (DV_OK);
}

/* ----
 * main()
 * ----
 */

int
main(int argc, char **argv)
{
	FILE *ofile;
	char fname[128];
	char bankbuf[8];
	unsigned char buf[0x2000];
	int i, j, offset, savebank;

	/* check args */
	if (((argc != 2) && (argc != 1)) ||
	    ((argc == 2) && !strcmp(argv[1], "-?"))) {
		printf("GETROM (v1.00)\n\n");
		printf("getrom [-? (for help)] [outfile]\n\n");
		printf("outfile : file to write ROM contents into\n");
		printf("          (default is 'system.rom')\n");
		return (0);
	}

	/* filename */
	if (argc == 1) {
		strcpy(fname, "system.rom");
	}
	else {
		strcpy(fname, argv[1]);
	}

	/* init the develo box */
	if (dv_init() != DV_OK) {
		printf("can not initialize the develo box!\n");
		exit(1);
	}

	ofile = fopen(fname, "wb");

	if (dv_get_bank(bankbuf) != DV_OK) {
		printf("can't get banks\n");
		exit(1);
	}

	savebank = bankbuf[4];

	for (i = 0; i < 32; i++) {
		if (dv_set_bank(4, i) != DV_OK) {
			printf("can't set bank\n");
			exit(1);
		}

		printf("Reading bank $%2.2X", i);
		fflush(stdout);

		for (j = 0; j < 4; j++) {
			offset = j * 0x0800;

			if (get_block(buf + offset, 0x8000 + offset, 0x0800) != DV_OK) {
				printf("can't get RAM\n");
				exit(1);
			}
			printf(".");
			fflush(stdout);
		}
		printf("\n");

		for (j = 0; j < 0x2000; j++) {
			fputc(buf[j], ofile);
		}
	}

	if (dv_set_bank(4, savebank) != DV_OK) {
		printf("can't set bank back to original\n");
		exit(1);
	}

	/* ok */
	printf(" OK\n");
	return (0);
}

