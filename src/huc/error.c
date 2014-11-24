/*	File error.c: 2.1 (83/03/20,16:02:00) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "defs.h"
#include "data.h"
#include "error.h"
#include "io.h"

void error (char *ptr)
{
	FILE *tempfile;

	if (output == NULL) {
		fprintf(stderr, "%s\n", ptr);
		exit(1);
	}

	tempfile = output;
	output = stderr;
	doerror(ptr, 0);
	output = tempfile;
	errcnt++;
	if (errcnt > 3) {
		errcnt = 0;
		error("too many errors, aborting");
		exit(1);
	}
}

void warning (int type, char *text)
{
	FILE *tfp;

	assert(type > 0);
	tfp = output;
	output = stderr;
	doerror(text, type);
	output = tfp;
}

void doerror (char *ptr, int type)
{
	long k;

	comment();
	if (!type)
		outstr("error: ");
	else
		outstr("warning: ");
	if (inclsp)
		outstr(inclstk_name[inclsp - 1]);
	else
		outstr(fname_copy);
	outbyte('(');
	outdec(line_number);
	outbyte(')');
	nl();
	comment();
	outstr(line);
	nl();
	comment();
	k = 0;
	while (k < lptr) {
		if (line[k] == 9)
			tab();
		else
			outbyte(' ');
		k++;
	}
	outbyte('^');
	nl();
	comment();
	outstr("******  ");
	outstr(ptr);
	outstr("  ******");
	nl();
}
