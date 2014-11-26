#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#ifdef __linux__
#include <sys/io.h>
#endif
#include "develo.h"

// #define PC98	1	/* uncomment this for PC98 machines - UNTESTED!! */

/* globals
 * ----
 */
int develo;		/* develo box presence flag */
int develo_lpt;		/* parallel I/O port */
int develo_com;		/* serial I/O port */
int develo_wait1;	/* timeout delays */
int develo_wait2;
int develo_error;	/* latest error */
int develo_status;	/* transfer status */
int slave_mode;
int slave_wait;

/* protos
 * ----
 */
static int get_hex(char **p);
static int get_dec(char **p);


/* ----
 * dv_init()
 * ----
 */

int
dv_init(void)
{
	char *env;

	/* default value */
	develo_lpt = 0x378;
	develo_com = 0;
	develo_wait1 = 200000;
	develo = 0;

#ifdef __linux__
	if (iopl(3)) {
		perror("cannot access I/O ports");
		exit(1);
	}
#endif

	/* get environment variable */
	env = getenv("DEVELOPORT");

	/* ok found */
	develo = 1;

	/* parse string */
	while (env && *env != '\0') {
		/* LPT port address */
		if (strncasecmp(env, "LPT:", 4) == 0) {
			env += 4;
			develo_lpt = get_hex(&env);
		}

		/* COM port address */
		else if (strncasecmp(env, "COM:", 4) == 0) {
			env += 4;
			develo_com = get_hex(&env);
		}

		/* timeout delay 1 */
		else if (strncasecmp(env, "WAIT1:", 6) == 0) {
			env += 6;
			develo_wait1 = get_dec(&env);
		}

		/* timeout delay 2 */
		else if (strncasecmp(env, "WAIT2:", 6) == 0) {
			env += 6;
			develo_wait2 = get_dec(&env);
		}
		else {
			env++;
		}
	}

	/* ok */
	return (DV_OK);
}


/* ----
 * get_hex()
 * ----
 */

static int
get_hex(char **p)
{
	char c;
	int val = 0;

	for (;;) {
		c = tolower(*(*p)++);
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'f')
			c -= 'a' - 10;
		else {
			break;
		}
		val = (val << 4) + c;
	}

	/* return value */
	return (val);
}


/* ----
 * get_dec()
 * ----
 */

static int
get_dec(char **p)
{
	char c;
	int val = 0;

	for (;;) {
		c = *(*p)++;
		if (c >= '0' && c <= '9')
			c -= '0';
		else {
			break;
		}
		val = (10 * val) + c;
	}

	/* return value */
	return (val);
}

