/*	File io.c: 2.1 (83/03/20,16:02:07) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "data.h"
#include "io.h"
#include "optimize.h"
#include "preproc.h"
#include "main.h"
#include "code.h"

/*
 *	open input file
 * Input : char* p
 * Output : long error code
 *
 * Try to open the file whose filename is p, return YES if opened, else NO
 * Updates fname with the actual opened name
 * input is the handle of the opened file
 *
 */
long openin (char *p)
{
	strcpy(fname, p);
	strcpy(fname_copy, fname);
	fixname(fname);
	if (!checkname(fname)) {
		fprintf(stderr, "%s: unknown file type\n", fname);
		return (NO);
	}
	if ((input = fopen(fname, "r")) == NULL) {
		perror(fname);
		return (NO);
	}
	kill();
	return (YES);
}

/*
 *	open output file
 * Input : nothing but uses global fname
 * Output : nothing but fname contain the name of the out file now
 *
 * Guess the name of the outfile thanks to the input one and try to open it
 * In case of succes returns YES and output is the handle of the opened file
 * else returns NO
 *
 */
long openout (void)
{
	if (user_outfile[0])
		output = fopen(user_outfile, "w");
	else {
		outfname(fname);
		output = fopen(fname, "w");
	}
	if (output == NULL) {
		pl("Open failure : ");
		if (user_outfile[0])
			pl(user_outfile);
		else
			pl(fname);
		pl("\n");
		return (NO);
	}
	kill();
	return (YES);
}

/*
 *	change input filename to output filename
 * Input : char* s
 * Output : char* s is updated
 *
 * Simply replace the last letter of s by 's'
 * Used to return "file.s" from "file.c"
 *
 */
void outfname (char *s)
{
	while (*s)
		s++;
	*--s = 's';
}

/*
 *	remove NL from filenames
 * Input : char* s
 * Output : char* s is updated
 *
 * if any, remove the trailing newline char from the s string
 *
 */
void fixname (char *s)
{
	while (*s && *s++ != EOL) ;
	if (!*s) return;

	*(--s) = 0;
}

/*
 *	check that filename is "*.c"
 * Input : char* s
 * Output : long
 *
 * verify that the 2 last letter of s are ".c", returns YES in this case,
 * else NO
 *
 */
long checkname (char *s)
{
	while (*s)
		s++;
	if (*--s != 'c' && *s != 'C')
		return (NO);

	if (*--s != '.')
		return (NO);

	return (YES);
}

/*
 *             kill
 * Input : nothing
 * Output : nothing but updates lptr and line[lptr]
 *
 * lptr and line[lptr] are set to zero what appears to clear the current
 * input line
 *
 */
void kill (void)
{
	lptr = 0;
	line[lptr] = 0;
}

/*
 * unget_line
 * Input : # of characters to kill preceding what we see in line[lptr]
 * Output : nothing
 *
 * This function looks at line and lptr variables with a line of text
 * and moves the file pointer back as though the line was never read
 * then kills line/lptr
 *
 * Only use this function for operations at the top level (ie. the
 * file pointer 'fp')
 *
 */
void unget_line (void)
{
	long i;

	i = strlen(line);
	if (i > 0) {
		fseek(input, 0 - i - CR_LEN, SEEK_CUR);
		line_number--;
	}

	kill();
}


/*
 *            readline
 * Input : nothing
 * Output : nothing
 *
 * This function seems to fill line and lptr variables with a line of text
 * coming either form an included file or the main one
 *
 */
void readline (void)
{
	long k;
	FILE *unit;

	FOREVER {
		if (!input || feof(input))
			return;

		if ((unit = input2) == NULL)
			unit = input;
		kill();
		while ((k = fgetc(unit)) != EOF) {
			if ((k == '\r') | (k == EOL) | (lptr >= LINEMAX))
				break;
			line[lptr++] = k;
		}
		line_number++;
		line[lptr] = 0;
		if (k <= 0)
			if (input2 != NULL) {
				if (globals_h_in_process) {
					/* Add special treatment to ensure globals.h stuff appears at the beginning */
					dumpglbs();
					ol(".code");
					globals_h_in_process = 0;
				}
				input2 = inclstk[--inclsp];
				line_number = inclstk_line[inclsp];
				fclose(unit);
			}
		if (lptr) {
			if ((ctext) & (cmode)) {
				flush_ins();
				comment();
				outstr(line);
				nl();
			}
			lptr = 0;
			return;
		}
	}
}

/*
 *              inbyte
 * Input : nothing
 * Output : long, (actualy char)
 *
 * Uses the preprocessor as much as possible to get readable data
 * then read the next char and make lptr points to the next one
 *
 */
long inbyte (void)
{
	while (ch() == 0) {
		if (feof(input))
			return (0);

		preprocess();
	}
	return (gch());
}

/*
 *               inchar
 * Input : nothing
 * Output : long, (actualy char)
 *
 * Returns the current char, making lptr points to the next one
 * If the buffer if empty, fill it with next line from input
 *
 */
long inchar (void)
{
	if (ch() == 0)
		readline();
	if (feof(input))
		return (0);

	return (gch());
}

/*
 *              gch
 * Input : nothing
 * Output : long, (actualy char)
 *
 * If the pointed char (by line and lptr) is 0, return this value
 * else return the current pointed char and advance the lptr to point
 * on the following char
 *
 */
long gch (void)
{
	if (ch() == 0)
		return (0);
	else
		return (line[lptr++] & 127);
}

/*
 *                 nch
 * Input : nothing
 * Output : long, (actualy char)
 *
 * If called when the pointed char is at the end of the line, return 0
 * else return the following char
 * Doesn't change line nor lptr variable
 *
 */
long nch (void)
{
	if (ch() == 0)
		return (0);
	else
		return (line[lptr + 1] & 127);
}

/*
 *           ch
 *
 * Input : nothing but use global line and lptr variables
 * Output : long, (actually char), corresponding to the current pointed char
 *    during the parsing
 *
 * Appears to be the major function used during the parsing.
 * The global variables line and lptr aren't changed
 *
 */
long ch (void)
{
	return (line[lptr] & 127);
}

/*
 *	print a carriage return and a string only to console
 *
 */
void pl (char *str)
/*char	*str; */
{
	long k;

	k = 0;
	putchar(EOL);
	while (str[k])
		putchar(str[k++]);
}

/*
 *	glabel - generate label
 */
void glabel (char *lab)
/*char	*lab;*/
{
	flush_ins();	/* David - optimize.c related */
	prefix();
	outstr(lab);
	col();
	nl();
}

/*
 *	gnlabel - generate numeric label
 */
void gnlabel (long nlab)
{
	out_ins(I_LABEL, T_VALUE, nlab);
}

/*
 *	Output internal generated label prefix
 */
void olprfix (void)
{
	outstr("LL");
}

/*
 *	Output a label definition terminator
 */
void col (void)
{
	outstr(":\n");
}

/*
 *	begin a comment line for the assembler
 *
 */
void comment (void)
{
	outbyte(';');
}

/*
 *	Output a prefix in front of user labels
 */
void prefix (void)
{
	outbyte('_');
}

/*
 *               tab
 * Input : nothing
 * Output : nothing
 *
 * Write a tab charater in the assembler file
 *
 */
void tab (void)
{
	outbyte(9);
}

/*
 *               ol
 * Input : char* ptr
 * Output : nothing
 *
 * Writes the string ptr to the assembler file, preceded by a tab, ended by
 * a newline
 *
 */
void ol (char *ptr)
/*char ptr[]; */
{
	ot(ptr);
	nl();
}

/*
 *                ot
 * Input : char* ptr
 * Output : nothing
 *
 * Writes the string ptr to the assembler file, preceded by a tab
 *
 */
void ot (char *ptr)
/*char ptr[]; */
{
	tab();
	outstr(ptr);
}

/*
 *            nl
 * Input : nothing
 * Output : nothing
 *
 * Display a newline in the assembler file
 *
 */
void nl (void)
{
	outbyte(EOL);
}

/*
 *         outsymbol
 * Input : char* ptr
 * Output : nothing
 *
 * Writes the string ptr preceded with the result of the function prefix
 *
 */
void outsymbol (char *ptr)
{
	/* Hmmm... try to improve check for things on zero-page */

	if (strcmp(ptr, "_temp") == 0)
		outstr("<");
	prefix();
	outstr(ptr);
}

/*
 *	print specified number as label
 */
void outlabel (long label)
{
	olprfix();
	outdec(label);
}

/*
 *  Output a decimal number to the assembler file
 */
/*
   void outdec (long number)
   {
        long	k, zs;
        char	c;

        if (number == -32768) {
                outstr ("-32768");
                return;
        }
        zs = 0;
        k = 10000;
        if (number < 0) {
                number = (-number);
                outbyte ('-');
        }
        while (k >= 1) {
                c = number / k + '0';
                if ((c != '0' | (k == 1) | zs)) {
                        zs = 1;
                        outbyte (c);
                }
                number = number % k;
                k = k / 10;
        }
   }
 */

/* Newer version, shorter and certainly faster */
void outdec (long number)
{
	char s[21];
	int i = 0;

	sprintf(s, "%ld", number);

	while (s[i])
		outbyte(s[i++]);
}

/*
 *  Output an hexadecimal unsigned number to the assembler file
 */

/*
   void outhex (long number)
   {
        long	k, zs;
        char	c;

        zs = 0;
        k = 0x10000000;

        outbyte('$');

        while (k >= 1) {
                c = number / k;
                if (c <= 9)
                  c += '0';
                else
                  c += 'A' - 10;

                outbyte (c);

                number = number % k;
                k = k / 16;
        }
   }
 */

/* Newer version, shorter and certainly faster */
void outhex (long number)
{
	int i = 0;
	char s[10];

	outbyte('$');

	sprintf(s, "%0X", (int)number);

	while (s[i])
		outbyte(s[i++]);
}

/*
 * Output an hexadecimal number with a certain number of digits
 */
void outhexfix (long number, long length)
{
	int i = 0;
	char s[10];
	char format[10];

	outbyte('$');

	sprintf(format, "%%0%dX", (int)length);

	sprintf(s, format, number);

	while (s[i])
		outbyte(s[i++]);
}


/*
 *             outbyte
 * Input : char c
 * Output : same as input, c
 *
 * if c is different of zero, write it to the output file
 *
 */
char outbyte (char c)
{
	if (c == 0)
		return (0);

	fputc(c, output);
	return (c);
}

/*
 *               outstr
 * Input : char*, ptr
 * Output : nothing
 *
 * Send the input char* to the assembler file
 *
 */
void outstr (char *ptr)
/*char	ptr[];*/
{
	long k;

	k = 0;
	while (outbyte(ptr[k++])) ;
}
