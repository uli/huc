/*	File lex.c: 2.1 (83/03/20,16:02:09) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include "defs.h"
#include "data.h"
#include "error.h"
#include "io.h"
#include "lex.h"
#include "preproc.h"

/*
 *	semicolon enforcer
 *
 *	called whenever syntax requires a semicolon
 *
 */
void ns (void)
{
	if (!match(";"))
		error("missing semicolon");
}

void junk (void)
{
	if (an(inbyte()))
		while (an(ch()))
			gch();
	else
		while (an(ch())) {
			if (ch() == 0)
				break;
			gch();
		}
	blanks();
}

long endst (void)
{
	blanks();
	return ((streq(line + lptr, ";") | (ch() == 0)));
}

void needbrack (char *str)
{
	if (!match(str)) {
		error("missing bracket");
		comment();
		outstr(str);
		nl();
	}
}

/*
 *	test if given character is alpha
 *
 */
long alpha (char c)
{
	c = c & 127;
	return (((c >= 'a') && (c <= 'z')) |
		((c >= 'A') && (c <= 'Z')) |
		(c == '_'));
}

/*
 *	test if given character is numeric
 *
 */
long numeric (char c)
{
	c = c & 127;
	return ((c >= '0') && (c <= '9'));
}

/*
 *	test if given character is alphanumeric
 *
 */
long an (char c)
{
	return ((alpha(c)) | (numeric(c)));
}

long sstreq (char *str1)
{
	return (streq(line + lptr, str1));
}

long streq (char *str1, char *str2)
{
	long k;

	k = 0;
	while (str2[k]) {
		if ((str1[k] != str2[k]))
			return (0);

		k++;
	}
	return (k);
}

long astreq (char *str1, char *str2, long len)
{
	long k;

	k = 0;
	while (k < len) {
		if ((str1[k] != str2[k]))
			break;
		if (str1[k] == 0)
			break;
		if (str2[k] == 0)
			break;
		k++;
	}
	if (an(str1[k]))
		return (0);

	if (an(str2[k]))
		return (0);

	return (k);
}

long match (char *lit)
{
	long k;

	blanks();
	k = streq(line + lptr, lit);
	if (k) {
		lptr = lptr + k;
		return (1);
	}
	return (0);
}

long amatch (char *lit, long len)
{
	long k;

	blanks();
	k = astreq(line + lptr, lit, len);
	if (k) {
		lptr = lptr + k;
		while (an(ch()))
			inbyte();
		return (1);
	}
	return (0);
}

int lex_stop_at_eol = 0;

void blanks (void)
{
	FOREVER {
		while (ch() == 0 && !lex_stop_at_eol) {
			preprocess();
			if (!input || feof(input))
				break;
		}
		if (ch() == ' ')
			gch();
		else if (ch() == 9)
			gch();
		else
			return;
	}
}
