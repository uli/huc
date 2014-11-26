#include <stdio.h>
#include <string.h>
#include "lib/develo.h"


/* ----
 * main()
 * ----
 */

int
main(int argc, char **argv)
{
	char *ptr;
	char fname[128];
	int addr, page, bank;
	int i;

	/* check args */
	if ((argc != 2) || (!strcmp(argv[1], "-?"))) {
		printf("PERUN (v1.00)\n\n");
		printf("perun [-? (for help)] infile[.mx]\n\n");
		printf("infile : file to be transfered and executed\n");
		return (0);
	}

	/* add file extension */
	strcpy(fname, argv[1]);

	if ((ptr = strrchr(fname, '.')) == NULL)
		strcat(fname, ".mx");
	else {
		if (strchr(ptr, PATH_SEPARATOR))
			strcat(fname, ".mx");
	}

	/* init the develo box */
	if (dv_init() != DV_OK) {
		printf("can not initialize the develo box!\n");
		exit(1);
	}

	/* load the mx file */
	if (dv_load_mx(fname, &addr, &bank, 1) == DV_ERR)
		exit(1);

	/* init slave mode */
	if (dv_slave_init() != DV_OK) {
		printf("can not initialize slave mode!\n");
		exit(1);
	}

	/* execute message */
	printf("execute... ");
	fflush(stdout);

	/* check start address */
	if (addr > 0xFFFF) {
		printf("incorrect address, %X!\n", addr);
		exit(1);
	}

	/* get page */
	page = (addr >> 13);

	/* select bank */
	for (i = page; i < 7; i++) {
		if (dv_set_bank(i, bank++) != DV_OK) {
			printf("%s, can not set bank!\n", dv_get_errmsg());
			exit(1);
		}
	}

	/* go */
	if (dv_exec(addr, 0) != DV_OK) {
		printf("%s, can not execute program!\n", dv_get_errmsg());
		exit(1);
	}
	printf(" OK\n");

	/* enter slave mode */
	printf("entering slave mode -- press ESC to exit\n\n");
	dv_slave(1);

	/* ok */
	return (0);
}

