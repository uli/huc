/*	File sym.c: 2.1 (83/03/20,16:02:19) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "const.h"
#include "error.h"
#include "gen.h"
#include "io.h"
#include "lex.h"
#include "primary.h"
#include "pragma.h"
#include "sym.h"
#include "function.h"
#include "struct.h"

/*
 *	declare a static variable
 *
 *  David, added support for const arrays and improved error detection
 *
 */
long
declglb (long typ, long stor, TAG_SYMBOL *mtag, int otag, int is_struct)
{
	long	 k, id;
	char sname[NAMESIZE];
	int ptr_order;

	for (;;) {
		for (;;) {
			ptr_order = 0;
			if (endst())
				return 0;
			k = 1;
			id = VARIABLE;
			while(match ("*")) {
				id = POINTER;
				ptr_order++;
			}
			if(!symname (sname))
				illname ();
			if (match("(")) {
				newfunc(sname, ptr_order, typ, otag);
				return 2;
			}
			if (findglb (sname))
				multidef (sname);
			if (match ("[")) {
				k = array_initializer(typ, id, stor);
				if (k == -1)
					return (1);
				if (id == POINTER)
					typ = CINT;
				if (k || (stor == EXTERN))
					id = ARRAY;
				else {
					if (stor == CONST) {
						error ("empty const array");
						id = ARRAY;
					}
					else if (id == POINTER)
						id = ARRAY;
					else {
						id = POINTER;
						ptr_order++;
					}
				}
			} else {
				if (stor == CONST) {
					/* stor  = PUBLIC; XXX: What is this for? */
					scalar_initializer(typ, id, stor);
				}
			}
			if (mtag == 0) {
				if (typ == CSTRUCT) {
					if (id == VARIABLE)
						k = tag_table[otag].size;
					else if (id == POINTER)
						k = INTSIZE;
					else if (id == ARRAY)
						k *= tag_table[otag].size;
				}
				if (stor != CONST) {
					SYMBOL *c = addglb (sname, id, typ, k, stor);
					if (typ == CSTRUCT)
						c->tagidx = otag;
					c->ptr_order = ptr_order;
				}
				else {
					SYMBOL *c = addglb (sname, id, typ, k, CONST);
					if (c) {
						add_const(typ);
						if (typ == CSTRUCT)
							c->tagidx = otag;
					}
					c->ptr_order = ptr_order;
				}
			}
			else if (is_struct) {
				add_member(sname, id, typ, mtag->size, stor);
				if (id == POINTER)
					typ = CUINT;
				scale_const(typ, otag, &k);
				mtag->size += k;
			}
			else {
				add_member(sname, id, typ, 0, stor);
				if (id == POINTER)
					typ = CUINT;
				scale_const(typ, otag, &k);
				if (mtag->size < k)
					mtag->size = k;
			}
			break;
		}
		if (endst ())
			return (0);
		if (!match (",")) {
			error ("syntax error");
			return (1);
		}
	}
	return (0);
}

/*
 *  declare local variables
 *
 *  works just like "declglb", but modifies machine stack and adds
 *  symbol table entry with appropriate stack offset to find it again
 *
 *  zeo : added "totalk" stuff and global stack modification (00/04/12)
 */
void declloc (long typ, long stclass, int otag)
{
	long  k = 0, j;
	char sname[NAMESIZE];
	long  totalk = 0;

	for (;;) {
		for (;;) {
			int ptr_order = 0;
			if (endst ())
			{
				if (!norecurse)
					stkp = modstk (stkp - totalk);
				return;
			}
			j = VARIABLE;
			while (match ("*")) {
				j = POINTER;
				ptr_order++;
			}
			if (!symname (sname))
				illname ();
			if (findloc (sname))
				multidef (sname);
			if (match ("[")) {
				k = needsub ();
				if (k) {
					if (typ == CINT || typ == CUINT || j == POINTER)
						k = k * INTSIZE;
					else if (typ == CSTRUCT)
						k *= tag_table[otag].size;
					j = ARRAY;
				} else {
					j = POINTER;
					ptr_order++;
					k = INTSIZE;
				}
			} else {
				if ((typ == CCHAR || typ == CUCHAR) & (j != POINTER))
					k = 1;
				else if (typ == CSTRUCT) {
					if (j == VARIABLE)
						k = tag_table[otag].size;
					else if (j == POINTER)
						k = INTSIZE;
				}
				else
					k = INTSIZE;
			}
			if (stclass == LSTATIC) {
				SYMBOL *c = addloc( sname, j, typ, k, LSTATIC, k);
				if (typ == CSTRUCT)
					c->tagidx = otag;
				c->ptr_order = ptr_order;
			}
			else {
				SYMBOL *c;
//				k = galign(k);
				totalk += k;
				// stkp = modstk (stkp - k);
				// addloc (sname, j, typ, stkp, AUTO);
				if (!norecurse)
					c = addloc (sname, j, typ, stkp - totalk, AUTO, k);
				else
					c = addloc (sname, j, typ, locals_ptr - totalk, AUTO, k);
				if (typ == CSTRUCT)
					c->tagidx = otag;
				c->ptr_order = ptr_order;
			}
			break;
		}
		if (match("=")) {
			long num[1];
			if (stclass == LSTATIC)
				error("initialization of static local variables unimplemented");
			if (!norecurse)
				stkp = modstk (stkp - totalk);
			else
				locals_ptr -= totalk;
			totalk -= k;
			if (const_expr(num, ",", ";")) {
				/* XXX: minor memory leak */
				char *locsym = malloc(80);
				gtext();
				if (k == 1) {
					if (norecurse) {
						sprintf(locsym, "_%s_lend-%ld", current_fn, -locals_ptr);
						out_ins_ex(I_STBI, T_SYMBOL, (long)locsym, T_VALUE, *num);
					}
					else
						out_ins_ex(X_STBI_S, T_VALUE, 0, T_VALUE, *num);
				}
				else if (k == 2) {
					if (norecurse) {
						sprintf(locsym, "_%s_lend-%ld", current_fn, -locals_ptr);
						out_ins_ex(I_STWI, T_SYMBOL, (long)locsym, T_VALUE, *num);
					}
					else
					out_ins_ex(X_STWI_S, T_VALUE, 0, T_VALUE, *num);
				}
				else
					error("complex type initialization not implemented");
			}
			else {
				error("cannot parse initializer");
			}
		}
		if (!match (","))
		{
			if (!norecurse)
				stkp = modstk (stkp - totalk);
			else
				locals_ptr -= totalk;
			return;
		}
	}
}

/*
 *	get required array size
 */
long needsub (void)
{
	long	num[1];

	if (match ("]"))
		return (0);
	if (!const_expr (num, "]", NULL)) {
		error ("must be constant");
		num[0] = 1;
	}
	if (!match("]"))
		error("internal error");
	if (num[0] < 0) {
		error ("negative size illegal");
		num[0] = (-num[0]);
	}
	return (num[0]);
}

SYMBOL* findglb (char *sname)
{
	SYMBOL	*ptr;

	ptr = STARTGLB;
	while (ptr != glbptr) {
		if (astreq (sname, ptr->name, NAMEMAX))
			return (ptr);
		ptr++;
	}
	return NULL;
}

SYMBOL* findloc (char *sname)
{
	SYMBOL	*ptr;

	ptr = locptr;
	while (ptr != STARTLOC) {
		ptr--;
		if (astreq (sname, ptr->name, NAMEMAX))
			return (ptr);
	}
	return NULL;
}

SYMBOL *addglb (char* sname,char id,char typ,long value,long stor)
{
	char	*ptr;

	cptr = findglb (sname);
	if (cptr)
		return (cptr);
	if (glbptr >= ENDGLB) {
		error ("global symbol table overflow");
		return NULL;
	}
	cptr = glbptr;
	ptr = glbptr->name;
	while (an (*ptr++ = *sname++));
	cptr->ident = id;
	cptr->type = typ;
	cptr->storage = stor;
	cptr->offset = value;
	cptr->size = value;
	if (id == FUNCTION)
		cptr->size = 0;
	else if (id == POINTER)
		cptr->size = INTSIZE;
	else if (typ == CINT || typ == CUINT)
		cptr->size *= 2;
	glbptr++;
	return (cptr);
}

SYMBOL *addglb_far (char* sname, char typ)
{
	SYMBOL *ptr;

	ptr = addglb(sname, ARRAY, typ, 0, EXTERN);
	if (ptr)
		ptr->far = 1;
	return (ptr);
}


SYMBOL *addloc (char* sname,char id,char typ,long value,long stclass, long size)
{
	char	*ptr;
	long	k;

	cptr = findloc (sname);
	if (cptr)
		return (cptr);
	if (locptr >= ENDLOC) {
		error ("local symbol table overflow");
		return NULL;
	}
	cptr = locptr;
	ptr = locptr->name;
	while (an (*ptr++ = *sname++));
	cptr->ident = id;
	cptr->type = typ;
	cptr->storage = stclass;
	if (stclass == LSTATIC) {
		gdata();
		ol(".bss");
		outlabel(k = getlabel());
		outstr(":\t");
		defstorage();
		outdec(size);
		nl();
		value = k;
	}
//	else
//		value = galign(value);
	cptr->offset = value;
	cptr->size = size;
	locptr++;
	return (cptr);
}

/*
 *	test if next input string is legal symbol name
 *
 */
long symname (char* sname)
{
	long	k;
/*	char	c; */

	blanks ();
	if (!alpha (ch ()))
		return (0);
	k = 0;
	while (an (ch ()))
		sname[k++] = gch ();
	sname[k] = 0;
	return (1);
}

void illname (void)
{
	error ("illegal symbol name");
}

void multidef (char* sname)
{
	error ("already defined");
	comment ();
	outstr (sname);
	nl ();
}

long glint(SYMBOL* sym)
{
	return sym->offset;
}
