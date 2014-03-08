/*	File sym.c: 2.1 (83/03/20,16:02:19) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
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

	for (;;) {
		for (;;) {
			if (endst())
				return 0;
			k = 1;
			if (match ("*"))
				id = POINTER;
			else
				id = VARIABLE;
			if(!symname (sname))
				illname ();
			if (match("(")) {
				newfunc(sname);
				return 2;
			}
			if (findglb (sname))
				multidef (sname);
			if (match ("[")) {
				if((id == POINTER) && (stor != CONST))
					error ("array of variable pointers not supported");
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
					else
						id = POINTER;
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
				}
				else {
					SYMBOL *c = addglb (sname, id, typ, k, STATIC);
					if (c) {
						add_const(typ);
						if (typ == CSTRUCT)
							c->tagidx = otag;
					}
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
	long  k, j;
	char sname[NAMESIZE];
	long  totalk = 0;

	for (;;) {
		for (;;) {
			if (endst ())
			{
				stkp = modstk (stkp - totalk);
				return;
			}
			if (match ("*"))
				j = POINTER;
			else
				j = VARIABLE;
			if (!symname (sname))
				illname ();
			if (findloc (sname))
				multidef (sname);
			if (match ("[")) {
				k = needsub ();
				if (k) {
					j = ARRAY;
					if (typ == CINT || typ == CUINT)
						k = k * INTSIZE;
				} else if (typ == CSTRUCT) {
					k *= tag_table[otag].size;
				} else {
					j = POINTER;
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
				SYMBOL *c = addloc( sname, j, typ, k, LSTATIC);
				if (typ == CSTRUCT)
					c->tagidx = otag;
			}
			else {
				SYMBOL *c;
//				k = galign(k);
				totalk += k;
				// stkp = modstk (stkp - k);
				// addloc (sname, j, typ, stkp, AUTO);
				c = addloc (sname, j, typ, stkp - totalk, AUTO);
				if (typ == CSTRUCT)
					c->tagidx = otag;
			}
			break;
		}
		if (!match (","))
		{
			stkp = modstk (stkp - totalk);
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
	if (!number (num)) {
		error ("must be constant");
		num[0] = 1;
	}
	if (num[0] < 0) {
		error ("negative size illegal");
		num[0] = (-num[0]);
	}
	needbrack ("]");
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


SYMBOL *addloc (char* sname,char id,char typ,long value,long stclass)
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
		outdec(value);
		nl();
		value = k;
	}
//	else
//		value = galign(value);
	cptr->offset = value;
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
