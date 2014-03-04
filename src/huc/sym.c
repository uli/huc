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


/*
 *	declare a static variable
 *
 *  David, added support for const arrays and improved error detection
 *
 */
long
declglb (long typ, long stor)
{
	long	 k, id;
	char sname[NAMESIZE];

	for (;;) {
		for (;;) {
			k = 1;
			if (match ("*"))
				id = POINTER;
			else
				id = VARIABLE;
			if(!symname (sname))
				illname ();
			if (findglb (sname))
				multidef (sname);
			if (match ("[")) {
				if((id == POINTER) && (stor != CONST))
					error ("array of pointers not supported");
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
					stor  = PUBLIC;
					error ("const variable not supported");
				}
			}
			if (stor != CONST)
				addglb (sname, id, typ, k, stor);
			else {
				if (addglb (sname, id, typ, k, STATIC))
					add_const(typ);
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
void declloc (long typ, long stclass)
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
					if (typ == CINT)
						k = k * INTSIZE;
				} else {
					j = POINTER;
					k = INTSIZE;
				}
			} else {
				if ((typ == CCHAR) & (j != POINTER))
					k = 1;
				else
					k = INTSIZE;
			}
			if (stclass == LSTATIC)
				addloc( sname, j, typ, k, LSTATIC);
			else {
//				k = galign(k);
				totalk += k;
				// stkp = modstk (stkp - k);
				// addloc (sname, j, typ, stkp, AUTO);
				addloc (sname, j, typ, stkp - totalk, AUTO);
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

char* findglb (char *sname)
{
	char	*ptr;

	ptr = STARTGLB;
	while (ptr != glbptr) {
		if (astreq (sname, ptr, NAMEMAX))
			return (ptr);
		ptr = ptr + SYMSIZ;
	}
	return NULL;
}

char* findloc (char *sname)
{
	char	*ptr;

	ptr = locptr;
	while (ptr != STARTLOC) {
		ptr = ptr - SYMSIZ;
		if (astreq (sname, ptr, NAMEMAX))
			return (ptr);
	}
	return NULL;
}

char *addglb (char* sname,char id,char typ,long value,long stor)
{
	char	*ptr;

	cptr = findglb (sname);
	if (cptr)
		return (cptr);
	if (glbptr >= ENDGLB) {
		error ("global symbol table overflow");
		return NULL;
	}
	cptr = ptr = glbptr;
	while (an (*ptr++ = *sname++));
	cptr[IDENT] = id;
	cptr[TYPE] = typ;
	cptr[STORAGE] = stor;
	cptr[OFFSET] = value & 0xff;	
	cptr[OFFSET+1] = (value >> 8) & 0xff;
	glbptr = glbptr + SYMSIZ;
	return (cptr);
}

char *addglb_far (char* sname, char typ)
{
	char *ptr;

	ptr = addglb(sname, ARRAY, typ, 0, EXTERN);
	if (ptr)
		ptr[FAR] = 1;
	return (ptr);
}


char *addloc (char* sname,char id,char typ,long value,long stclass)
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
	cptr = ptr = locptr;
	while (an (*ptr++ = *sname++));
	cptr[IDENT] = id;
	cptr[TYPE] = typ;
	cptr[STORAGE] = stclass;
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
	cptr[OFFSET] = value & 0xff;
	cptr[OFFSET+1] = (value >> 8) & 0xff;
	locptr = locptr + SYMSIZ;
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

long glint(char* sym)
{
	long l,u,r;
	l = sym[OFFSET];
	u = sym[OFFSET+1];
	r = (l & 0xff) + ((u << 8) & ~0x00ff);
	return (r);
}
