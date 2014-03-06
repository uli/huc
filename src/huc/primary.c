/*	File primary.c: 2.4 (84/11/27,16:26:07) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "data.h"
#include "error.h"
#include "expr.h"
#include "gen.h"
#include "io.h"
#include "lex.h"
#include "primary.h"
#include "sym.h"

long primary (long* lval)
{
	char	*ptr, sname[NAMESIZE];
	long	num[1];
	long	k;

	lval[2] = 0;  /* clear pointer/array type */
	lval[3] = 0;
	if (match ("(")) {
		indflg = 0;
		k = heir1 (lval);
		needbrack (")");
		return (k);
	}
	if (amatch("sizeof", 6)) {
		indflg = 0;
		needbrack("(");
		if (amatch("int", 3))
			immed (T_VALUE, INTSIZE);
		else if (amatch("char", 4))
			immed (T_VALUE, 1);
		else if (symname(sname)) {
			if ((ptr = findloc(sname)) ||
				(ptr = findglb(sname))) {
				if ((ptr[STORAGE] & ~WRITTEN) == LSTATIC)
					error("sizeof local static");
				k = glint(ptr);
				if ((ptr[TYPE] == CINT) || ptr[TYPE] == CUINT ||
					(ptr[IDENT] == POINTER))
					k *= INTSIZE;
				immed (T_VALUE, k);
			} else {
				error("sizeof undeclared variable");
				immed (T_VALUE, 0);
			}
		} else {
			error("sizeof only on type or variable");
		}
		needbrack(")");
		return(lval[0] = lval[1] = 0);
	}
	if (symname (sname)) {
		ptr = findloc (sname);
		if (ptr) {
			/* David, patched to support
			 *        local 'static' variables
			 */
			lval[0] = (long)ptr;
			lval[1] = ptr[TYPE];
			if (ptr[IDENT] == POINTER) {
				if ((ptr[STORAGE] & ~WRITTEN) == LSTATIC)
					lval[1] = 0;
				else {
					lval[1] = CINT;
					getloc (ptr);
				}
				lval[2] = ptr[TYPE];
				return (1);
			}
			if (ptr[IDENT] == ARRAY) {
				getloc (ptr);
				lval[2] = ptr[TYPE];
//				lval[2] = 0;
				return (0);
			}
			if ((ptr[STORAGE] & ~WRITTEN) == LSTATIC)
				lval[1] = 0;
			else
				getloc (ptr);
			return (1);
		}
		ptr = findglb (sname);
		if (ptr) {
			if (ptr[IDENT] != FUNCTION) {
				lval[0] = (long)ptr;
				lval[1] = 0;
				if (ptr[IDENT] != ARRAY) {
					if (ptr[IDENT] == POINTER)
						lval[2] = ptr[TYPE];
					return (1);
				}
				if (!ptr[FAR])
					immed (T_SYMBOL, (long)ptr);
				else {
					/* special variables */
					blanks ();
					if ((ch() != '[') && (ch() != '(')) {
						/* vram */
						if (strcmp(ptr, "vram") == 0) {
							if (indflg)
								return (1);
							else
								error ("can't access vram this way");
						}
						/* others */
						immed (T_SYMBOL, (long)ptr);
//						error ("can't access far array");
					}
				}
				lval[1] = lval[2] = ptr[TYPE];
//				lval[2] = 0;
				return (0);
			}
		}
		blanks ();
		if (ch() != '(') {
			if (ptr && (ptr[IDENT] == FUNCTION)) {
				lval[0] = (long)ptr;
				lval[1] = 0;
				return (0);
			}
			error("undeclared variable");
		}
		ptr = addglb (sname, FUNCTION, CINT, 0, PUBLIC);
		indflg = 0;
		lval[0] = (long)ptr;
		lval[1] = 0;
		return (0);
	}
	if (constant (num)) {
		indflg = 0;
		return (lval[0] = lval[1] = 0);
	}
	else {
		indflg = 0;
		error ("invalid expression");
		immed (T_VALUE, 0);
		junk ();
		return (0);
	}
}

/*
 *	true if val1 -> long pointer or long array and val2 not pointer or array
 */
long dbltest (long val1[],long val2[])
{
	if (val1 == NULL)
		return (FALSE);
	if (val1[2] != CINT && val1[2] != CUINT)
		return (FALSE);
	if (val2[2])
		return (FALSE);
	return (TRUE);
}

/*
 *	determine type of binary operation
 */
void result (long lval[],long lval2[])
{
	if (lval[2] && lval2[2])
		lval[2] = 0;
	else if (lval2[2]) {
		lval[0] = lval2[0];
		lval[1] = lval2[1];
		lval[2] = lval2[2];
	}
}

long constant (long val[])
{
	if (number (val))
		immed (T_VALUE,  val[0]);
	else if (pstr (val))
		immed (T_VALUE,  val[0]);
	else if (qstr (val)) {
		immed (T_STRING, val[0]);
	} else
		return (0);
	return (1);
}

long number (long val[])
{
	long	k, minus, base;
	char	c;

	k = minus = 1;
	while (k) {
		k = 0;
		if (match ("+"))
			k = 1;
		if (match ("-")) {
			minus = (-minus);
			k = 1;
		}
	}
	if (!numeric (c = ch ()))
		return (0);
	if (match ("0x") || match ("0X"))
		while (numeric (c = ch ()) ||
		       (c >= 'a' && c <= 'f') ||
		       (c >= 'A' && c <= 'F')) {
			inbyte ();
			k = k * 16 +
			    (numeric (c) ? (c - '0') : ((c & 07) + 9));
		}
	else {
		base = (c == '0') ? 8 : 10;
		while (numeric (ch ())) {
			c = inbyte ();
			k = k * base + (c - '0');
		}
	}
	if (minus < 0)
		k = (-k);
	val[0] = k;
	return (1);
}

/*
 *         pstr
 * pstr parses a character than can eventually be 'double' i.e. like 'a9'
 * returns 0 in case of failure else 1
 */
long pstr (long val[])
{
	long	k;
	char	c;

	k = 0;
	if (!match ("'"))
		return (0);
	while ((c = gch ()) != '\'') {
		c = (c == '\\') ? spechar(): c;
		k = (k & 255) * 256 + (c & 255);
	}
	val[0] = k;
	return (1);
}

/*
 *         qstr
 * qstr parses a double quoted string into litq
 * return 0 in case of failure and 1 else
 */
long qstr (long val[])
{
	char	c;

	if (!match (quote))
		return (0);
	val[0] = litptr;
	while (ch () != '"') {
		if (ch () == 0)
			break;
		if (litptr >= LITMAX) {
			error ("string space exhausted");
			while (!match (quote))
				if (gch () == 0)
					break;
			return (1);
		}
		c = gch();
		litq[litptr++] = (c == '\\') ? spechar(): c;
	}
	gch ();
	litq[litptr++] = 0;
	return (1);
}

/*
 *         readqstr
 * readqstr parses a double quoted string into litq2
 * return 0 in case of failure and 1 else
 * Zeograd: this function don't dump the result of the reading in the literal
 * pool, it is rather intended for use in pseudo code
 */
long readqstr (void )
{
	char	c;
        long	posptr = 0;

	if (!match (quote))
		return (0);
	while (ch () != '"') {
		if (ch () == 0)
			break;
		if (posptr >= LITMAX2) {
			error ("string space exhausted");
			while (!match (quote))
				if (gch () == 0)
					break;
			return (1);
		}
		c = gch();
		litq2[posptr++] = (c == '\\') ? spechar(): c;
	}
	gch ();
	litq2[posptr] = 0;
	return (1);
}

/*
 *         readstr
 * reaqstr parses a string into litq2
 * it only read alpha numeric characters
 * return 0 in case of failure and 1 else
 * Zeograd: this function don't dump the result of the reading in the literal
 * pool, it is rather intended for use in pseudo code
 */
long readstr (void )
{
	char	c;
        long	posptr = 0;

	while (an(ch ()) || (ch()=='_') ) {
		if (ch () == 0)
			break;
		if (posptr >= LITMAX2) {
			error ("string space exhausted");
			return (1);
		}
		c = gch();
		litq2[posptr++] = (c == '\\') ? spechar(): c;
	}
	litq2[posptr] = 0;
	return (1);
}


/*
 *	decode special characters (preceeded by back slashes)
 */
long spechar(void )
{
	char c;
	c = ch();

	if	(c == 'n') c = EOL;
	else if	(c == 't') c = TAB;
	else if (c == 'r') c = CR;
	else if (c == 'f') c = FFEED;
	else if (c == 'b') c = BKSP;
	else if (c == '0') c = EOS;
	else if (c == EOS) return(c);

	gch();
	return (c);
}
