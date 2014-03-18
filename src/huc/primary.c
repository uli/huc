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
#include "struct.h"

extern char current_fn[];

static void ignore_ast(void)
{
        while (match("*")) {
        }
}

long primary (LVALUE* lval, int comma)
{
	SYMBOL	*ptr;
	char	sname[NAMESIZE];
	long	num[1];
	long	k;

	lval->ptr_type = 0;  /* clear pointer/array type */
	lval->symbol2 = 0;
	if (match ("(")) {
		indflg = 0;
		/* need to use expression_ex() (not heir1()), otherwise
		   the comma operator is not handled */
		k = expression_ex (lval, comma, YES);
		needbrack (")");
		return (k);
	}
	if (amatch("sizeof", 6)) {
	        int have_paren;
		indflg = 0;
		have_paren = match("(");
		if (amatch("int", 3)) {
		        /* int* same size as int */
		        ignore_ast();
			immed (T_VALUE, INTSIZE);
                }
		else if (amatch("char", 4)) {
		        if (match("*")) {
		                ignore_ast();
		                immed(T_VALUE, INTSIZE);
                        }
                        else
				immed (T_VALUE, 1);
                }
                else if (amatch("struct", 6)) {
                        if (symname(sname)) {
                                int tag = find_tag(sname);
                                if (tag == -1)
                                        error("unknown struct");
                                else {
                                        if (match("*")) {
                                                ignore_ast();
                                                immed(T_VALUE, INTSIZE);
                                        }
                                        else
                                                immed(T_VALUE, tag_table[tag].size);
                                }
                        }
                        else
                                error("missing struct name");
                }
		else if (symname(sname)) {
			if ((ptr = findloc(sname)) ||
				(ptr = findglb(sname))) {
				if ((ptr->storage & ~WRITTEN) == LSTATIC)
					error("sizeof local static");
				k = ptr->size;
                                if (ptr->type == CSTRUCT && ptr->ident != POINTER)
                                        k = tag_table[ptr->tagidx].size;
				immed (T_VALUE, k);
			} else {
				error("sizeof undeclared variable");
				immed (T_VALUE, 0);
			}
                }
                else if (readqstr()) {
                        immed(T_VALUE, strlen(litq2));
		} else {
			error("sizeof only on type or variable");
		}
		if (have_paren)
			needbrack(")");
		lval->symbol = 0;
		lval->indirect = 0;
		return 0;
	}
	if (amatch("__FUNCTION__", 12)) {
	        const_str(num, current_fn);
                immed(T_STRING, num[0]);
		indflg = 0;
		lval->value = num[0];
		lval->symbol = 0;
		lval->indirect = 0;
		return 0;
	}
	if (symname (sname)) {
		ptr = findloc (sname);
		if (ptr) {
			/* David, patched to support
			 *        local 'static' variables
			 */
			lval->symbol = (SYMBOL *)ptr;
			lval->indirect = ptr->type;
			lval->tagsym = 0;
			if (ptr->type == CSTRUCT)
			        lval->tagsym = &tag_table[ptr->tagidx];
			if (ptr->ident == POINTER) {
				if ((ptr->storage & ~WRITTEN) == LSTATIC)
					lval->indirect = 0;
				else {
					lval->indirect = CUINT;
					getloc (ptr);
				}
				lval->ptr_type = ptr->type;
				return (1);
			}
			if (ptr->ident == ARRAY ||
			    (ptr->ident == VARIABLE && ptr->type == CSTRUCT)) {
				getloc (ptr);
				lval->ptr_type = ptr->type;
//				lval->ptr_type = 0;
                                if (ptr->type == CSTRUCT && ptr->ident == VARIABLE)
                                        return 1;
                                else
				        return 0;
			}
			if ((ptr->storage & ~WRITTEN) == LSTATIC)
				lval->indirect = 0;
			else
				getloc (ptr);
			return (1);
		}
		ptr = findglb (sname);
		if (ptr) {
			if (ptr->ident != FUNCTION) {
				lval->symbol = (SYMBOL *)ptr;
				lval->indirect = 0;
				lval->tagsym = 0;
				if (ptr->type == CSTRUCT)
				        lval->tagsym = &tag_table[ptr->tagidx];
				if (ptr->ident != ARRAY &&
				    (ptr->ident != VARIABLE || ptr->type != CSTRUCT)) {
					if (ptr->ident == POINTER)
						lval->ptr_type = ptr->type;
					return (1);
				}
				if (!ptr->far)
					immed (T_SYMBOL, (long)ptr);
				else {
					/* special variables */
					blanks ();
					if ((ch() != '[') && (ch() != '(')) {
						/* vram */
						if (strcmp(ptr->name, "vram") == 0) {
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
				lval->indirect = lval->ptr_type = ptr->type;
//				lval->ptr_type = 0;
                                if (ptr->ident == VARIABLE && ptr->type == CSTRUCT)
                                        return 1;
                                else
        				return (0);
			}
		}
		blanks ();
		if (ch() != '(') {
			if (ptr && (ptr->ident == FUNCTION)) {
				lval->symbol = (SYMBOL *)ptr;
				lval->indirect = 0;
				return (0);
			}
			error("undeclared variable");
		}
		ptr = addglb (sname, FUNCTION, CINT, 0, PUBLIC);
		indflg = 0;
		lval->symbol = (SYMBOL *)ptr;
		lval->indirect = 0;
		return (0);
	}
	if ((k = constant (num))) {
		indflg = 0;
		lval->value = num[0];
		lval->symbol = 0;
		lval->indirect = 0;
		if (k == 2)
		        lval->ptr_type = CCHAR;
		return 0;
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
 *	true if val1 -> int pointer or int array and val2 not pointer or array
 */
long dbltest (LVALUE val1[],LVALUE val2[])
{
	if (val1 == NULL || !val1->ptr_type)
		return (FALSE);
	if (val1->ptr_type == CCHAR || val1->ptr_type == CUCHAR)
		return (FALSE);
	if (val2->ptr_type)
		return (FALSE);
	return (TRUE);
}

/*
 *	determine type of binary operation
 */
void result (LVALUE lval[],LVALUE lval2[])
{
	if (lval->ptr_type && lval2->ptr_type)
		lval->ptr_type = 0;
	else if (lval2->ptr_type) {
		lval->symbol = lval2->symbol;
		lval->indirect = lval2->indirect;
		lval->ptr_type = lval2->ptr_type;
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
		return 2;
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

int const_expr(long *num, char *end)
{
        long num2;
        if (!number(num))
                return 0;
        while (!match(end)) {
                if (match("-") && number(&num2))
                        *num -= num2;
                else if (match("+") && number(&num2))
                        *num += num2;
                else if (match("*") && number(&num2))
                        *num *= num2;
                else if (match("/") && number(&num2))
                        *num /= num2;
                else
                        return 0;
        }
        return 1;
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

long const_str(long *val, const char *str)
{
        if (litptr + strlen(str) + 1 >= LITMAX) {
                error("string space exhausted");
                return 1;
        }
        strcpy(&litq[litptr], str);
        *val = litptr;
        litptr += strlen(str) + 1;
        return 1;
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
	else if (numeric(c) && c < '8') {
	        /* octal character specification */
	        int n = 0;
	        while (numeric(c) && c < '8') {
	                n = (n << 3) | (c - '0');
	                gch();
	                c = ch();
	        }
	        return n;
	}
	else if (c == EOS) return(c);

	gch();
	return (c);
}
