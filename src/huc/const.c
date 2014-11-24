/*	File const.c: 2.1 (00/07/17,16:02:19) */
/*% cc -O -c %
 *
 */

/* XXX: This passes the initializer more or less verbatim to the assembler,
   which unsurprisingly does not care much for C semantics, breaking stuff
   like pointer arithmetic, and probably many non-trivial expressions.
   Needs a rewrite. */

#include <stdio.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "const.h"
#include "error.h"
#include "io.h"
#include "lex.h"
#include "primary.h"
#include "sym.h"

/*
 *	setup a new const array
 *
 */
void new_const (void)
{
	const_ptr = &const_var[const_nb];
	const_val_idx = const_val_start;
	const_data_idx = const_data_start;
}


/*
 *	add a const array
 *
 */
void add_const (long typ)
{
	if ((const_data_idx >= MAX_CONST_DATA) || (const_val_idx >= MAX_CONST_VALUE))
		error("too much constant data (> 8KB)");
	if (const_nb >= MAX_CONST)
		error("too much constant arrays");
	else {
		const_ptr->sym = cptr;
		const_ptr->typ = typ;
		const_ptr->size = const_val_idx - const_val_start;
		const_ptr->data = const_val_start;
		const_val_start = const_val_idx;
		const_data_start = const_data_idx;
		const_nb++;
	}
}


/*
 *	array initializer
 *
 */
long array_initializer (long typ, long id, long stor)
{
	long nb;
	long k;
	long i;

	nb = 0;
	k = needsub();

	if (stor == CONST)
		new_const();
	if (match("=")) {
		if (stor != CONST)
			error("can't initialize non-const arrays");
		if (!match("{")) {
			error("syntax error");
			return (-1);
		}
		if (!match("}")) {
			for (;;) {
				if (match("}")) {
					error("value missing");
					break;
				}
				if (match(",")) {
					error("value missing");
					continue;
				}
				if ((ch() == '\"') && (id == POINTER))
					i = get_string_ptr(typ);
				else
					i = get_raw_value(',');
				nb++;
				blanks();
				if (const_val_idx < MAX_CONST_VALUE)
					const_val[const_val_idx++] = i;
				if ((ch() != ',') && (ch() != '}')) {
					error("syntax error");
					return (-1);
				}
				if (match("}"))
					break;
				gch();
			}
		}
		if (k == 0)
			k = nb;
		if (nb > k) {
			nb = k;
			error("excess elements in array initializer");
		}
	}
	if (stor == CONST) {
		while (nb < k) {
			nb++;
			if (const_val_idx < MAX_CONST_VALUE)
				const_val[const_val_idx++] = -1;
		}
	}
	return (k);
}

/*
 *	scalar initializer
 *
 */
long scalar_initializer (long typ, long id, long stor)
{
	long i;

	if (stor == CONST)
		new_const();
	if (match("=")) {
		if (stor != CONST)
			error("can't initialize non-const scalars");
		blanks();
		if (ch() == ';') {
			error("value missing");
			return (-1);
		}
		if (ch() == '\"' && id == POINTER)
			i = get_string_ptr(typ);
		else
			i = get_raw_value(';');
		if (const_val_idx < MAX_CONST_VALUE)
			const_val[const_val_idx++] = i;
		blanks();
		if (ch() != ';') {
			error("syntax error");
			return (-1);
		}
	}
	return (1);
}


/*
 *  add a string to the literal pool and return a pointer (index) to it
 *
 */
long get_string_ptr (long typ)
{
	long num[1];

	if (typ == CINT || typ == CUINT)
		error("incompatible pointer type");
	if (qstr(num))
		return (-(num[0] + 1024));
	else
		return (-1);
}


/*
 *  get value raw text
 *
 */
long get_raw_value (char sep)
{
	char c;
	char tmp[LINESIZE + 1];
	char *ptr;
	long level;
	long flag;
	long start;
	int is_address = 0;
	int had_address = 0;

	flag = 0;
	level = 0;
	start = const_data_idx;
	ptr = tmp;

	for (;;) {
		c = ch();

		/* discard blanks */
		if ((c == ' ') || (c == '\t')) {
			gch();
			continue;
		}
		/* string */
		if (c == '\"') {
			for (;;) {
				gch();

				/* add char */
				if (const_data_idx < MAX_CONST_DATA)
					const_data[const_data_idx++] = c;

				/* next */
				c = ch();

				if ((c == 0) || (c == '\"'))
					break;
			}
		}
		/* parenthesis */
		if (c == '(')
			level++;
		else if (c == ')')
			level--;
		/* comma separator */
		else if (c == sep) {
			if (level == 0)
				break;
		}
		/* end */
		else if ((c == 0) || (c == '}')) {
			if (level)
				error("syntax error");
			break;
		}

		/* parse */
		if (an(c)) {
			flag = 1;
			*ptr++ = c;
		}
		else {
			/* add buffer */
			if (flag) {
				flag = 0;
				*ptr = '\0';
				ptr = tmp;
				had_address += add_buffer(tmp, c, is_address);
				is_address = 0;
				if ((c == '+' || c == '-') && had_address) {
					/* Initializers are passed almost
					   verbatim to the assembler, which
					   doesn't know anything about types
					   and thus doesn't know how to do
					   pointer arithmetic correctly, so
					   we don't allow it. */
					error("pointer arithmetic in initializers not supported");
					return (0);
				}
			}

			/* add char */
			if (c == '&') {
				/* we want the succeeding identifier's address */
				is_address = 1;
				/* we need to remember that we had an address
				   somewhere so we can barf if the identifier
				   contains arithmetic */
				had_address = 1;
			}
			else if (const_data_idx < MAX_CONST_DATA)
				const_data[const_data_idx++] = c;
		}
		gch();
	}
	/* add buffer */
	if (flag) {
		*ptr = '\0';
		add_buffer(tmp, c, is_address);
	}
	/* close string */
	if (const_data_idx < MAX_CONST_DATA)
		const_data[const_data_idx++] = '\0';

	return (start);
}


/*
 *	add a string to the constant pool
 *  handle underscore
 *
 */
int add_buffer (char *p, char c, int is_address)
{
	SYMBOL *s = 0;

	/* underscore */
	if (alpha(*p)) {
		if (!is_address) {
			s = findglb(p);
			if (!s) {
				error("undefined global");
				return (0);
			}
			/* Unless preceded by an address operator, we
			   need to get the value, and it better be
			   constant... */
			p = get_const(s);
			if (!p) {
				error("non-constant initializer");
				return (0);
			}
		}
		else if (c != '(') {
			/* If we want the address, we need an underscore
			   prefix. */
			if (const_data_idx < MAX_CONST_DATA)
				const_data[const_data_idx++] = '_';
		}
	}

	/* string */
	while (*p) {
		if (const_data_idx < MAX_CONST_DATA)
			const_data[const_data_idx++] = *p;
		p++;
	}

	/* tell the caller if there were any addresses involved */
	return ((s && s->ident == POINTER) || is_address);
}

char *get_const (SYMBOL *s)
{
	int i;
	struct const_array *const_ptr;

	if (const_nb) {
		const_ptr = const_var;

		for (i = 0; i < const_nb; i++) {
			if (const_ptr->sym == s) {
				int j = const_val[const_ptr->data];
				if (j >= 0)
					return (&const_data[j]);
				else
					return (0);
			}
			const_ptr++;
		}
	}
	return (0);
}

/*
 *	dump the constant pool
 *
 */
void dump_const (void)
{
	long i, j, k;
	long size;

/*	long c; */

	if (const_nb) {
		const_ptr = const_var;

		for (i = 0; i < const_nb; i++) {
			size = const_ptr->size;
			cptr = const_ptr->sym;
			cptr->storage = EXTERN;
			prefix();
			outstr(cptr->name);
			outstr(":");
			nl();
			j = const_ptr->data;

			while (size) {
				k = const_val[j++];

				if ((cptr->type == CCHAR || cptr->type == CUCHAR) &&
				    cptr->ident != POINTER &&
				    !(cptr->ident == ARRAY && cptr->ptr_order > 0)) {
					defbyte();
					const_size += 1;
				}
				else {
					defword();
					const_size += 2;
				}
				if ((k == -1) || (k >= MAX_CONST_DATA))
					outstr("0");
				else if (k <= -1024) {
					k = (-k) - 1024;
					outlabel(litlab);
					outbyte('+');
					outdec(k);
				}
				else
					outstr(&const_data[k]);
				nl();
				size--;
			}
			const_ptr++;
		}
	}
}
