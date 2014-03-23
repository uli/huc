/*	File expr.c: 2.2 (83/06/21,11:24:26) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "error.h"
#include "expr.h"
#include "function.h"
#include "gen.h"
#include "io.h"
#include "lex.h"
#include "primary.h"
#include "sym.h"

/*
 *	lval->symbol - symbol table address, else 0 for constant
 *	lval->indirect - type indirect object to fetch, else 0 for static object
 *	lval->ptr_type - type pointer or array, else 0
 */

void expression (int comma)
{
	LVALUE lval[1];
	expression_ex(lval, comma, NO);
}

long expression_ex (LVALUE *lval, int comma, int norval)
{
	long k;
	do {
		if ((k = heir1 (lval, comma)) && !norval)
			rvalue (lval);
		if (!comma)
			return k;
	} while (match (","));
	return k;
}

static int is_unsigned(LVALUE *lval)
{
	if (!lval->symbol)
		return 0;
	/* C only promotes operations with an unsigned int
	   to unsigned, not unsigned char! */
	if (lval->symbol->type == CUINT)
		return 1;
	return 0;
}

static void gen_scale_right(LVALUE *lval, LVALUE *lval2)
{
	if (dbltest (lval, lval2)) {
		if (lval->tagsym) {
			TAG_SYMBOL *tag = (TAG_SYMBOL *)(lval->tagsym);
			if (tag->size == 2)
				gaslint();
			else if (tag->size > 1)
				gmult_imm(tag->size);
		}
		else
			gaslint ();
	}
}

static int is_ptrptr(LVALUE *lval)
{
	SYMBOL *s = lval->symbol;
	return s && (s->ptr_order > 1 || (s->ident == ARRAY && s->ptr_order > 0));
}

long heir1 (LVALUE *lval, int comma)
{
	long	k;
	LVALUE	lval2[1];
	char	fc;

	k = heir1a (lval, comma);
	if (match ("=")) {
		if (k == 0) {
			needlval ();
			return (0);
		}
		if (lval->indirect)
			gpush ();
		if (heir1 (lval2, comma))
			rvalue (lval2);
		store (lval);
		return (0);
	} else
	{	
		fc = ch();
		if  (match ("-=") ||
		    match ("+=") ||
		    match ("*=") ||
		    match ("/=") ||
		    match ("%=") ||
		    match (">>=") ||
		    match ("<<=") ||
		    match ("&=") ||
		    match ("^=") ||
		    match ("|=")) {
			if (k == 0) {
				needlval ();
				return (0);
			}
			if (lval->indirect)
				gpush ();
			rvalue (lval);
			gpush ();
			if (heir1 (lval2, comma))
				rvalue (lval2);
			switch (fc) {
				case '-':	{
					gen_scale_right(lval, lval2);
					gsub();
					result (lval, lval2);
					break;
				}
				case '+':	{
					gen_scale_right(lval, lval2);
					gadd (lval,lval2);
					result(lval,lval2);
					break;
				}
				case '*':	gmult (is_unsigned(lval) || is_unsigned(lval2)); break;
				case '/':	gdiv (is_unsigned(lval) || is_unsigned(lval2)); break;
				case '%':	gmod (is_unsigned(lval) || is_unsigned(lval2)); break;
				case '>':	gasr (is_unsigned(lval)); break;
				case '<':	gasl (); break;
				case '&':	gand (); break;
				case '^':	gxor (); break;
				case '|':	gor (); break;
			}
			store (lval);
			return (0);
		} else
			return (k);
	}
}

long heir1a (LVALUE *lval, int comma)
/* long	lval[]; */
{
	long	k, lab1, lab2;
	LVALUE	lval2[1];

	k = heir1b (lval, comma);
	blanks ();
	if (ch () != '?')
		return (k);
	if (k)
		rvalue (lval);
	FOREVER
		if (match ("?")) {
			testjump (lab1 = getlabel (), FALSE);
			if (heir1b (lval2, comma))
				rvalue (lval2);
			jump (lab2 = getlabel ());
			gnlabel (lab1);
			blanks ();
			if (!match (":")) {
				error ("missing colon");
				return (0);
			}
			if (heir1b (lval2, comma))
				rvalue (lval2);
			gnlabel (lab2);
		} else
			return (0);
}

long heir1b (LVALUE *lval, int comma)
/*long	lval[]; */
{
	long	k, lab;
	LVALUE	lval2[1];

	k = heir1c (lval, comma);
	blanks ();
	if (!sstreq ("||"))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER
		if (match ("||")) {
			testjump (lab = getlabel (), TRUE);
			if (heir1c (lval2, comma))
				rvalue (lval2);
			gnlabel (lab);
			gbool();
		} else
			return (0);
}

long heir1c (LVALUE *lval, int comma)
/*long	lval[]; */
{
	long	k, lab;
	LVALUE	lval2[1];

	k = heir2 (lval, comma);
	blanks ();
	if (!sstreq ("&&"))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER
		if (match ("&&")) {
			testjump (lab = getlabel (), FALSE);
			if (heir2 (lval2, comma))
				rvalue (lval2);
			gnlabel (lab);
			gbool();
		} else
			return (0);
}

long heir2 (LVALUE *lval, int comma)
/*long	lval[]; */
{
	long	k;
	LVALUE	lval2[1];

	k = heir3 (lval, comma);
	blanks ();
	if ((ch() != '|') | (nch() == '|') | (nch() == '='))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if ((ch() == '|') & (nch() != '|') & (nch() != '=')) {
			inbyte ();
			gpush ();
			if (heir3 (lval2, comma))
				rvalue (lval2);
			gor ();
			blanks();
		} else
			return (0);
	}
}

long heir3 (LVALUE *lval, int comma)
/* long	lval[]; */
{
	long	k;
	LVALUE lval2[1];

	k = heir4 (lval, comma);
	blanks ();
	if ((ch () != '^') | (nch() == '='))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if ((ch() == '^') & (nch() != '=')){
			inbyte ();
			gpush ();
			if (heir4 (lval2, comma))
				rvalue (lval2);
			gxor ();
			blanks();
		} else
			return (0);
	}
}

long heir4 (LVALUE *lval, int comma)
/* long	lval[]; */
{
	long	k;
	LVALUE	lval2[1];

	k = heir5 (lval, comma);
	blanks ();
	if ((ch() != '&') | (nch() == '|') | (nch() == '='))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if ((ch() == '&') & (nch() != '&') & (nch() != '=')) {
			inbyte ();
			gpush ();
			if (heir5 (lval2, comma))
				rvalue (lval2);
			gand ();
			blanks();
		} else
			return (0);
	}
}

int is_byte(LVALUE *lval)
{
	if (lval->symbol && !lval->ptr_type &&
	    (lval->symbol->type == CCHAR || lval->symbol->type == CUCHAR))
		return 1;
	return 0;
}

long heir5 (LVALUE *lval, int comma)
/*long	lval[]; */
{
	long	k;
	LVALUE	lval2[1];

	k = heir6 (lval, comma);
	blanks ();
	if (!sstreq ("==") &
	    !sstreq ("!="))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if (match ("==")) {
			gpush ();
			if (heir6 (lval2, comma))
				rvalue (lval2);
			geq (is_byte(lval) && is_byte(lval2));
		} else if (match ("!=")) {
			gpush ();
			if (heir6 (lval2, comma))
				rvalue (lval2);
			gne (is_byte(lval) && is_byte(lval2));
		} else
			return (0);
	}
}

long heir6 (LVALUE *lval, int comma)
/* long	lval[]; */
{
	long	k;
	LVALUE	lval2[1];

	k = heir7 (lval, comma);
	blanks ();
	if (!sstreq ("<") &&
	    !sstreq ("<=") &&
	    !sstreq (">=") &&
	    !sstreq (">"))
		return (k);
	if (sstreq ("<<") || sstreq (">>"))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if (match ("<=")) {
			gpush ();
			if (heir7 (lval2, comma))
				rvalue (lval2);
			if (lval->ptr_type || lval2->ptr_type ||
			    is_unsigned(lval) ||
			    is_unsigned(lval2)
			   ) {
				gule (is_byte(lval) && is_byte(lval2));
				continue;
			}
			gle (is_byte(lval) && is_byte(lval2));
		} else if (match (">=")) {
			gpush ();
			if (heir7 (lval2, comma))
				rvalue (lval2);
			if (lval->ptr_type || lval2->ptr_type ||
			    is_unsigned(lval) ||
			    is_unsigned(lval2)
			   ) {
				guge (is_byte(lval) && is_byte(lval2));
				continue;
			}
			gge (is_byte(lval) && is_byte(lval2));
		} else if ((sstreq ("<")) &&
			   !sstreq ("<<")) {
			inbyte ();
			gpush ();
			if (heir7 (lval2, comma))
				rvalue (lval2);
			if (lval->ptr_type || lval2->ptr_type ||
			    is_unsigned(lval) ||
			    is_unsigned(lval2)
			   ) {
				gult (is_byte(lval) && is_byte(lval2));
				continue;
			}
			glt (is_byte(lval) && is_byte(lval2));
		} else if ((sstreq (">")) &&
			   !sstreq (">>")) {
			inbyte ();
			gpush ();
			if (heir7 (lval2, comma))
				rvalue (lval2);
			if (lval->ptr_type || lval2->ptr_type ||
			    is_unsigned(lval) ||
			    is_unsigned(lval2)
			   ) {
				gugt (is_byte(lval) && is_byte(lval2));
				continue;
			}
			ggt (is_byte(lval) && is_byte(lval2));
		} else
			return (0);
		blanks ();
	}
}

long heir7 (LVALUE *lval, int comma)
/*long	lval[]; */
{
	long	k;
	LVALUE	lval2[1];

	k = heir8 (lval, comma);
	blanks ();
	if (  (!sstreq (">>") && !sstreq ("<<") ) ||
		sstreq(">>=") || sstreq("<<=") )
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if (sstreq(">>") && ! sstreq(">>=")) {
			inbyte(); inbyte();
			gpush ();
			if (heir8 (lval2, comma))
				rvalue (lval2);
			gasr (is_unsigned(lval));
		} else if (sstreq("<<") && ! sstreq("<<=")) {
			inbyte(); inbyte();
			gpush ();
			if (heir8 (lval2, comma))
				rvalue (lval2);
			gasl ();
		} else
			return (0);
		blanks();
	}
}

long heir8 (LVALUE *lval, int comma)
/*long	lval[]; */
{
	long	k;
	LVALUE	lval2[1];

	k = heir9 (lval, comma);
	blanks ();
	if ( ( (ch () != '+') && (ch () != '-') ) || (nch() == '=') )
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if (match ("+")) {
			gpush ();
			if (heir9 (lval2, comma))
				rvalue (lval2);
			/* if left is pointer and right is int, scale right */
			gen_scale_right(lval, lval2);
			/* will scale left if right int pointer and left int */
			gadd (lval,lval2);
			result (lval, lval2);
		} else if (match ("-")) {
			gpush ();
			if (heir9 (lval2, comma))
				rvalue (lval2);
			/* if dbl, can only be: pointer - int, or
						pointer - pointer, thus,
				in first case, int is scaled up,
				in second, result is scaled down. */
			gen_scale_right(lval, lval2);
			gsub ();
			/* if both pointers, scale result */
			if ((lval->ptr_type == CINT || lval->ptr_type == CUINT || is_ptrptr(lval)) &&
			    (lval2->ptr_type == CINT || lval2->ptr_type == CUINT || is_ptrptr(lval2))) {
				gasrint(); /* divide by intsize */
			}
			else if (lval->ptr_type == CSTRUCT && lval2->ptr_type == CSTRUCT) {
				TAG_SYMBOL* tag = lval->tagsym;
				if (tag->size == 2)
					gasrint();
				else if (tag->size > 1)
					gdiv_imm(tag->size);
			}
			result (lval, lval2);
		} else
			return (0);
	}
}

long heir9 (LVALUE *lval, int comma)
/* long	lval[]; */
{
	long	k;
	LVALUE	lval2[1];

	k = heir10 (lval, comma);
	blanks ();
	if (((ch () != '*') && (ch () != '/') &&
		(ch () != '%')) || (nch() == '='))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if (match ("*")) {
			gpush ();
			if (heir10 (lval2, comma))
				rvalue (lval2);
			gmult (is_unsigned(lval) || is_unsigned(lval2));
		} else if (match ("/")) {
			gpush ();
			if (heir10 (lval2, comma))
				rvalue (lval2);
			gdiv (is_unsigned(lval) || is_unsigned(lval2));
		} else if (match ("%")) {
			gpush ();
			if (heir10 (lval2, comma))
				rvalue (lval2);
			gmod (is_unsigned(lval) || is_unsigned(lval2));
		} else
			return (0);
	}
}

long heir10 (LVALUE *lval, int comma)
/* long	lval[]; */
{
	long	k;
	SYMBOL	*ptr;

	if (match ("++")) {
		indflg = 0;
		if ((k = heir10 (lval, comma)) == 0) {
			needlval ();
			return (0);
		}
		if (lval->indirect)
			gpush ();
		rvalue (lval);
		ginc (lval);
		store (lval);
		return (0);
	} else if (match ("--")) {
		indflg = 0;
		if ((k = heir10 (lval, comma)) == 0) {
			needlval ();
			return (0);
		}
		if (lval->indirect)
			gpush ();
		rvalue (lval);
		gdec (lval);
		store (lval);
		return (0);
	} else if (match ("-")) {
		indflg = 0;
		k = heir10 (lval, comma);
		if (k)
			rvalue (lval);
		gneg ();
		return (0);
	} else if (match ("~")) {
		indflg = 0;
		k = heir10 (lval, comma);
		if (k)
			rvalue (lval);
		gcom ();
		return (0);
	} else if (match ("!")) {
		indflg = 0;
		k = heir10 (lval, comma);
		if (k)
			rvalue (lval);
		glneg ();
		return (0);
	} else if (ch()=='*' && nch() != '=') {
		inbyte();
		indflg = 1;
		k = heir10 (lval, comma);
		indflg = 0;
		ptr = lval->symbol;
		/* vram */
		if (ptr && !strcmp(ptr->name, "vram")) {
			lval->ptr_type = 0;
			return (1);
		}
		if (k)
			rvalue (lval);
		if ( (ptr = lval->symbol) && ptr->ptr_order < 2)
			lval->indirect = ptr->type;
		else
			lval->indirect = CUINT;
		/* XXX: what about multiple indirection? */
		lval->ptr_type = 0;  /* flag as not pointer or array */
		return (1);
	} else if (ch()=='&' && nch()!='&' && nch()!='=') {
		indflg = 0;
		inbyte();
		k = heir10 (lval, comma);
		if (k == 0) {
			error ("illegal address");
			return (0);
		}
		if (lval->symbol) {
			ptr = lval->symbol;
			lval->ptr_type = ptr->type;
		}
		if (lval->indirect)
			return (0);
		/* global and non-array */
		ptr = lval->symbol;
		immed (T_SYMBOL, (long)ptr);
		lval->indirect = ptr->type;
		return (0);
	} else {
		k = heir11 (lval, comma);
		ptr = lval->symbol;
		if (match ("++")) {
			if (k == 0) {
				needlval ();
				return (0);
			}
			/* vram */
			if (ptr && !strcmp(ptr->name, "vram"))
				return (0);
			if (lval->indirect)
				gpush ();
			rvalue (lval);
			ginc (lval);
			store (lval);
			gdec (lval);
			return (0);
		} else if (match ("--")) {
			if (k == 0) {
				needlval ();
				return (0);
			}
			/* vram */
			if (ptr && !strcmp(ptr->name, "vram")) {
				error("can't decrement vram pointer");
				return (0);
			}
			if (lval->indirect)
				gpush ();
			rvalue (lval);
			gdec (lval);
			store (lval);
			ginc (lval);
			return (0);
		} else
			return (k);
	}
}

long heir11 (LVALUE *lval, int comma)
/*long	*lval; */
{
	long	direct,k;
	SYMBOL	*ptr;
	char    sname[NAMESIZE];

	k = primary (lval, comma);
	ptr = lval->symbol;
	blanks ();
	for (;;) {
		if (match ("[")) {
			if (ptr == 0) {
				if (lval->ptr_type) {
					/* subscription of anonymous array
					   ATM this can only happen for a
					   string literal. */
					if (lval->ptr_type != CCHAR)
						error("internal error: cannot subscript non-character literals");
					/* Primary contains literal pointer, add subscript. */
					gpush();
					expression(YES);
					needbrack("]");
					gadd(NULL, NULL);
					/* Dereference final pointer. */
					lval->symbol = lval->symbol2 = 0;
					lval->indirect = lval->ptr_type;
					lval->ptr_type = 0;
					k = 1;
					continue;
				}
				else {
					error ("can't subscript");
					junk ();
					needbrack ("]");
					return (0);
				}
			}
			else if (ptr->ident == POINTER)
				rvalue (lval);
			else if (ptr->ident != ARRAY) {
				error ("can't subscript");
				k = 0;
			}
			if (!ptr->far)
				gpush ();
			expression (YES);
			needbrack ("]");
			if (ptr->type == CINT || ptr->type == CUINT || ptr->ptr_order > 1 ||
			    (ptr->ident == ARRAY && ptr->ptr_order > 0))
				gaslint ();
			else if (ptr->type == CSTRUCT) {
				int size = tag_table[ptr->tagidx].size;
				if (size == 2)
					gaslint();
				else if (size > 1)
					gmult_imm(size);
			}
			if (!ptr->far)
				gadd (NULL,NULL);
			lval->symbol = 0;
			if (ptr->ptr_order > 1 || (ptr->ident == ARRAY && ptr->ptr_order > 0))
				lval->indirect = CUINT;
			else
				lval->indirect = ptr->type;
			lval->ptr_type = 0;//VARIABLE; /* David, bug patch ?? */
			lval->symbol2 = ptr->far ? (SYMBOL *)ptr : (SYMBOL *)NULL;
			k = 1;
		}
		else if (match ("(")) {
			if (ptr == 0) {
				error("invalid or unsupported function call");
				callfunction (0);
			}
			else if (ptr->ident != FUNCTION) {
				if (strcmp(ptr->name, "vram") == 0)
					callfunction (ptr->name);
				else {
					if (ptr->far) {
						lval->symbol2 = (SYMBOL *)ptr;
						immed (T_VALUE, 0);
					}
					rvalue (lval);
					callfunction (0);
				}
			} else
				callfunction (ptr->name);
			k = 0;
			/* Encode return type in lval. */
			SYMBOL *s = lval->symbol;
			if (s) {
				if (s->ptr_order >=1)
					lval->ptr_type = s->type;
				if (s->type == CSTRUCT)
					lval->tagsym = &tag_table[s->tagidx];
				lval->symbol = 0;
			}
		} else if ((direct=match(".")) || match("->")) {
			if (lval->tagsym == 0) {
			    error("can't take member") ;
			    junk() ;
			    return 0 ;
			}
			if (symname(sname) == 0 ||
			   ((ptr=find_member(lval->tagsym, sname)) == 0)) {
			    error("unknown member") ;
			    junk() ;
			    return 0 ;
			}
			if (k && direct == 0)
			    rvalue(lval);
			out_ins(I_ADDWI, T_VALUE, ptr->offset); // move pointer from struct begin to struct member
			lval->symbol = (SYMBOL *)ptr;
			lval->indirect = ptr->type; // lval->indirect = lval->val_type = ptr->type
			lval->ptr_type = 0;
			lval->tagsym = (long)NULL_TAG;
			if (ptr->type == CSTRUCT)
			    lval->tagsym = &tag_table[ptr->tagidx];
			if (ptr->ident == POINTER) {
			    lval->indirect = CINT;
			    lval->ptr_type = ptr->type;
			    //lval->val_type = CINT;
			}
			if (ptr->ident==ARRAY ||
			    (ptr->type==CSTRUCT && ptr->ident==VARIABLE)) {
			    // array or struct
			    lval->ptr_type = ptr->type;
			    //lval->val_type = CINT;
			    k = 0;
			}
			else k = 1;
		}
		else
			return (k);
	}
	if (ptr == 0)
		return (k);
	if (ptr->ident == FUNCTION) {
		immed (T_SYMBOL, (long)ptr);
		return (0);
	}
	return (k);
}

void store (LVALUE *lval)
/* long	*lval; */
{
	if (lval->symbol2) {
		/* far arrays (or special arrays) */
		if (!strcmp (lval->symbol2->name, "vdc"))
			putio (lval->symbol2);
		else if (!strcmp (lval->symbol2->name, "vram"))
			putvram (lval->symbol2);
		else {
			error ("const arrays can't be written");
			gpop ();
		}
	}
	else {
		/* other */
		if (lval->indirect != 0)
			putstk (lval->indirect);
		else {
			if (strcmp(lval->symbol->name, "vram") == 0)
				out_ins(I_VPUTW, (long)NULL, (long)NULL);
			else
				putmem (lval->symbol);
		}
	}
}

void rvalue (LVALUE *lval)
/* long	*lval; */
{
	if ((lval->symbol != 0) && (lval->indirect == 0)) {
		if (strcmp(lval->symbol->name, "vram") == 0)
			out_ins(I_VGETW, (long)NULL, (long)NULL);
		else
			getmem (lval->symbol);
	}
	else {
		if (lval->symbol2 == 0)
			indirect (lval->indirect);
		else {
			/* far arrays (or special arrays) */
			if (!strcmp (lval->symbol2->name, "vdc"))
				getio (lval->symbol2);
			else if (!strcmp (lval->symbol2->name, "vram"))
				getvram (lval->symbol2);
			else
				farpeek (lval->symbol2);
		}
	}
}

void needlval (void )
{
	error ("must be lvalue");
}

