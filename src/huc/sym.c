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
#include "initials.h"
#include "io.h"
#include "lex.h"
#include "primary.h"
#include "pragma.h"
#include "sym.h"
#include "function.h"
#include "struct.h"

/**
 * evaluate one initializer, add data to table
 * @param symbol_name
 * @param type
 * @param identity
 * @param dim
 * @param tag
 * @return
 */
static int init (char *symbol_name, int type, int identity, int *dim, TAG_SYMBOL *tag)
{
	long value;
	int number_of_chars;

	if (identity == POINTER) {
		error("initializing non-const pointers unimplemented");
		kill();
		return (0);
	}

	if (qstr(&value)) {
		if ((identity == VARIABLE) || (type != CCHAR && type != CUCHAR))
			error("found string: must assign to char pointer or array");	/* XXX: make this a warning? */
		if (identity == POINTER) {
			/* unimplemented */
			printf("initptr %s value %ld\n", symbol_name, value);
			abort();
			// add_data_initials(symbol_name, CUINT, value, tag);
		}
		else {
			number_of_chars = litptr - value;
			*dim = *dim - number_of_chars;
			while (number_of_chars > 0) {
				add_data_initials(symbol_name, CCHAR, litq[value++], tag);
				number_of_chars = number_of_chars - 1;
			}
		}
	}
	else if (number(&value)) {
		add_data_initials(symbol_name, CINT, value, tag);
		*dim = *dim - 1;
	}
	else if (qstr(&value)) {
		add_data_initials(symbol_name, CCHAR, value, tag);
		*dim = *dim - 1;
	}
	else
		return (0);

	return (1);
}

/**
 * initialise structure
 * @param tag
 */
void struct_init (TAG_SYMBOL *tag, char *symbol_name)
{
	int dim;
	int member_idx;

	member_idx = tag->member_idx;
	while (member_idx < tag->member_idx + tag->number_of_members) {
		init(symbol_name, member_table[tag->member_idx + member_idx].type,
		     member_table[tag->member_idx + member_idx].ident, &dim, tag);
		++member_idx;
		if ((match(",") == 0) && (member_idx != (tag->member_idx + tag->number_of_members))) {
			error("struct initialisaton out of data");
			break;
		}
	}
}

/**
 * initialize global objects
 * @param symbol_name
 * @param type char or integer or struct
 * @param identity
 * @param dim
 * @return 1 if variable is initialized
 */
int initials (char *symbol_name, int type, int identity, int dim, int otag)
{
	int dim_unknown = 0;

	if (dim == 0)	// allow for xx[] = {..}; declaration
		dim_unknown = 1;
	if (match("=")) {
		if (type != CCHAR && type != CUCHAR && type != CINT && type != CUINT && type != CSTRUCT)
			error("unsupported storage size");
		// an array or struct
		if (match("{")) {
			// aggregate initialiser
			if ((identity == POINTER || identity == VARIABLE) && type == CSTRUCT) {
				// aggregate is structure or pointer to structure
				dim = 0;
				struct_init(&tag_table[otag], symbol_name);
			}
			else {
				while ((dim > 0) || dim_unknown) {
					if (identity == ARRAY && type == CSTRUCT) {
						// array of struct
						needbrack("{");
						struct_init(&tag_table[otag], symbol_name);
						--dim;
						needbrack("}");
					}
					else {
						if (init(symbol_name, type, identity, &dim, 0))
							dim_unknown++;
					}
					if (match(",") == 0)
						break;
				}
			}
			needbrack("}");
			// single constant
		}
		else
			init(symbol_name, type, identity, &dim, 0);
	}
	return (identity);
}


/*
 *	declare a static variable
 *
 *  David, added support for const arrays and improved error detection
 *
 */
long declglb (long typ, long stor, TAG_SYMBOL *mtag, int otag, int is_struct)
{
	long k, id;
	char sname[NAMESIZE];
	int ptr_order;
	SYMBOL *s;

	for (;;) {
		for (;;) {
			ptr_order = 0;
			if (endst())
				return (0);

			k = 1;
			id = VARIABLE;
			while (match("*")) {
				id = POINTER;
				ptr_order++;
			}
			if (amatch("__fastcall", 10)) {
				newfunc(NULL, ptr_order, typ, otag, 1);
				return (2);
			}
			if (!symname(sname))
				illname();
			if (match("(")) {
				newfunc(sname, ptr_order, typ, otag, 0);
				return (2);
			}
			if ((s = findglb(sname))) {
				if (s->storage != EXTERN && !mtag)
					multidef(sname);
			}
			if (mtag && find_member(mtag, sname))
				multidef(sname);
			if (match("[")) {
				if (stor == CONST)
					k = array_initializer(typ, id, stor);
				else
					k = needsub();
				if (k == -1)
					return (1);

				/* XXX: This doesn't really belong here, but I
				   can't think of a better place right now. */
				if (id == POINTER && (typ == CCHAR || typ == CUCHAR || typ == CVOID))
					k *= INTSIZE;
				if (k || (stor == EXTERN))
					id = ARRAY;
				else {
					if (stor == CONST) {
						error("empty const array");
						id = ARRAY;
					}
					else if (id == POINTER)
						id = ARRAY;
					else {
						id = POINTER;
						ptr_order++;
					}
				}
			}
			else {
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
					id = initials(sname, typ, id, k, otag);
					SYMBOL *c = addglb(sname, id, typ, k, stor, s);
					if (typ == CSTRUCT)
						c->tagidx = otag;
					c->ptr_order = ptr_order;
				}
				else {
					SYMBOL *c = addglb(sname, id, typ, k, CONST, s);
					if (c) {
						add_const(typ);
						if (typ == CSTRUCT)
							c->tagidx = otag;
					}
					c->ptr_order = ptr_order;
				}
			}
			else if (is_struct) {
				add_member(sname, id, typ, mtag->size, stor, otag, ptr_order);
				if (id == POINTER)
					typ = CUINT;
				scale_const(typ, otag, &k);
				mtag->size += k;
			}
			else {
				add_member(sname, id, typ, 0, stor, otag, ptr_order);
				if (id == POINTER)
					typ = CUINT;
				scale_const(typ, otag, &k);
				if (mtag->size < k)
					mtag->size = k;
			}
			break;
		}
		if (endst())
			return (0);

		if (!match(",")) {
			error("syntax error");
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
	long k = 0, j;
	long elements = 0;
	char sname[NAMESIZE];
	long totalk = 0;

	for (;;) {
		for (;;) {
			int ptr_order = 0;
			if (endst()) {
				if (!norecurse)
					stkp = modstk(stkp - totalk);
				return;
			}
			j = VARIABLE;
			while (match("*")) {
				j = POINTER;
				ptr_order++;
			}
			if (!symname(sname))
				illname();
			if (findloc(sname))
				multidef(sname);
			if (match("[")) {
				elements = k = needsub();
				if (k) {
					if (typ == CINT || typ == CUINT || j == POINTER)
						k = k * INTSIZE;
					else if (typ == CSTRUCT)
						k *= tag_table[otag].size;
					j = ARRAY;
				}
				else {
					j = POINTER;
					ptr_order++;
					k = INTSIZE;
					elements = 1;
				}
			}
			else {
				elements = 1;
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
				/* Local statics are identified in two
				   different ways: The human-readable
				   identifier as given in the source text,
				   and the internal label that is used in
				   the assembly output.

				   The initializer code wants the label, and
				   it is also used to add a global to make
				   sure the right amount of space is
				   reserved in .bss and the initialized data
				   is dumped eventually.

				   addloc(), OTOH, wants the identifier so
				   it can be found when referenced further
				   down in the source text.  */
				char lsname[NAMESIZE];
				int label = getlabel();
				sprintf(lsname, "LL%d", label);

				j = initials(lsname, typ, j, k, otag);
				/* NB: addglb() expects the number of
				   elements, not a byte size.  Unless, of
				   course, we have a CSTRUCT. *sigh* */
				if (typ == CSTRUCT)
					addglb(lsname, j, typ, k, LSTATIC, 0);
				else
					addglb(lsname, j, typ, elements, LSTATIC, 0);

				SYMBOL *c = addloc(sname, j, typ, label, LSTATIC, k);
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
					c = addloc(sname, j, typ, stkp - totalk, AUTO, k);
				else
					c = addloc(sname, j, typ, locals_ptr - totalk, AUTO, k);
				if (typ == CSTRUCT)
					c->tagidx = otag;
				c->ptr_order = ptr_order;
			}
			break;
		}
		if (match("=")) {
			long num[1];
			if (!norecurse)
				stkp = modstk(stkp - totalk);
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
			else
				error("cannot parse initializer");
		}
		if (!match(",")) {
			if (!norecurse)
				stkp = modstk(stkp - totalk);
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
	long num[1];

	if (match("]"))
		return (0);

	if (!const_expr(num, "]", NULL)) {
		error("must be constant");
		num[0] = 1;
	}
	if (!match("]"))
		error("internal error");
	if (num[0] < 0) {
		error("negative size illegal");
		num[0] = (-num[0]);
	}
	return (num[0]);
}

SYMBOL *findglb (char *sname)
{
	SYMBOL *ptr;

	ptr = STARTGLB;
	while (ptr != glbptr) {
		if (astreq(sname, ptr->name, NAMEMAX))
			return (ptr);

		ptr++;
	}
	return (NULL);
}

SYMBOL *findloc (char *sname)
{
	SYMBOL *ptr;

	ptr = locptr;
	while (ptr != STARTLOC) {
		ptr--;
		if (astreq(sname, ptr->name, NAMEMAX))
			return (ptr);
	}
	return (NULL);
}

SYMBOL *addglb (char *sname, char id, char typ, long value, long stor, SYMBOL *replace)
{
	char *ptr;

	if (!replace) {
		cptr = findglb(sname);
		if (cptr)
			return (cptr);

		if (glbptr >= ENDGLB) {
			error("global symbol table overflow");
			return (NULL);
		}
		cptr = glbptr;
		glbptr++;
	}
	else
		cptr = replace;

	ptr = cptr->name;
	while (an(*ptr++ = *sname++)) ;
	cptr->ident = id;
	cptr->type = typ;
	cptr->storage = stor;
	cptr->offset = value;
	cptr->size = value;
	cptr->far = 0;
	if (id == FUNCTION)
		cptr->size = 0;
	else if (id == POINTER)
		cptr->size = INTSIZE;
	else if (typ == CINT || typ == CUINT)
		cptr->size *= 2;
	return (cptr);
}

SYMBOL *addglb_far (char *sname, char typ)
{
	SYMBOL *ptr;

	ptr = addglb(sname, ARRAY, typ, 0, EXTERN, 0);
	if (ptr)
		ptr->far = 1;
	return (ptr);
}


SYMBOL *addloc (char *sname, char id, char typ, long value, long stclass, long size)
{
	char *ptr;

	cptr = findloc(sname);
	if (cptr)
		return (cptr);

	if (locptr >= ENDLOC) {
		error("local symbol table overflow");
		return (NULL);
	}
	cptr = locptr;
	ptr = locptr->name;
	while (an(*ptr++ = *sname++)) ;
	cptr->ident = id;
	cptr->type = typ;
	cptr->storage = stclass;
	cptr->offset = value;
	cptr->size = size;
	locptr++;
	return (cptr);
}

/*
 *	test if next input string is legal symbol name
 *
 */
long symname (char *sname)
{
	long k;

/*	char	c; */

	blanks();
	if (!alpha(ch()))
		return (0);

	k = 0;
	while (an(ch()))
		sname[k++] = gch();
	sname[k] = 0;
	return (1);
}

void illname (void)
{
	error("illegal symbol name");
}

void multidef (char *sname)
{
	error("already defined");
	comment();
	outstr(sname);
	nl();
}

long glint (SYMBOL *sym)
{
	return (sym->offset);
}
