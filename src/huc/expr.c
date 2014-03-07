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
 *	lval[0] - symbol table address, else 0 for constant
 *	lval[1] - type indirect object to fetch, else 0 for static object
 *	lval[2] - type pointer or array, else 0
 */

void expression (long comma)
{
	long	lval[6];

	do {
		if (heir1 (lval))
			rvalue (lval);
		if (!comma)
			return;
	} while (match (","));
}

static int is_unsigned(long *lval)
{
	if (!lval[0])
		return 0;
	if (((char *)lval[0])[TYPE] & CUNSIGNED)
		return 1;
	return 0;
}

long heir1 (long * lval)
/*long	lval[]; */
{
	long	k, lval2[6];
	char	fc;

	k = heir1a (lval);
	if (match ("=")) {
		if (k == 0) {
			needlval ();
			return (0);
		}
		if (lval[1])
			gpush ();
		if (heir1 (lval2))
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
			if (lval[1])
				gpush ();
			rvalue (lval);
			gpush ();
			if (heir1 (lval2))
				rvalue (lval2);
			switch (fc) {
				case '-':	{
					if (dbltest(lval,lval2))
						gaslint();
					gsub();
					result (lval, lval2);
					break;
				}
				case '+':	{
					if (dbltest(lval,lval2))
						gaslint();
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

long heir1a (long *lval)
/* long	lval[]; */
{
	long	k, lval2[6], lab1, lab2;

	k = heir1b (lval);
	blanks ();
	if (ch () != '?')
		return (k);
	if (k)
		rvalue (lval);
	FOREVER
		if (match ("?")) {
			testjump (lab1 = getlabel (), FALSE);
			if (heir1b (lval2))
				rvalue (lval2);
			jump (lab2 = getlabel ());
			gnlabel (lab1);
			blanks ();
			if (!match (":")) {
				error ("missing colon");
				return (0);
			}
			if (heir1b (lval2))
				rvalue (lval2);
			gnlabel (lab2);
		} else
			return (0);
}

long heir1b (long *lval)
/*long	lval[]; */
{
	long	k, lval2[6], lab;

	k = heir1c (lval);
	blanks ();
	if (!sstreq ("||"))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER
		if (match ("||")) {
			testjump (lab = getlabel (), TRUE);
			if (heir1c (lval2))
				rvalue (lval2);
			gnlabel (lab);
			gbool();
		} else
			return (0);
}

long heir1c (long *lval)
/*long	lval[]; */
{
	long	k, lval2[6], lab;

	k = heir2 (lval);
	blanks ();
	if (!sstreq ("&&"))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER
		if (match ("&&")) {
			testjump (lab = getlabel (), FALSE);
			if (heir2 (lval2))
				rvalue (lval2);
			gnlabel (lab);
			gbool();
		} else
			return (0);
}

long heir2 (long *lval)
/*long	lval[]; */
{
	long	k, lval2[6];

	k = heir3 (lval);
	blanks ();
	if ((ch() != '|') | (nch() == '|') | (nch() == '='))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if ((ch() == '|') & (nch() != '|') & (nch() != '=')) {
			inbyte ();
			gpush ();
			if (heir3 (lval2))
				rvalue (lval2);
			gor ();
			blanks();
		} else
			return (0);
	}
}

long heir3 (long *lval)
/* long	lval[]; */
{
	long	k, lval2[6];

	k = heir4 (lval);
	blanks ();
	if ((ch () != '^') | (nch() == '='))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if ((ch() == '^') & (nch() != '=')){
			inbyte ();
			gpush ();
			if (heir4 (lval2))
				rvalue (lval2);
			gxor ();
			blanks();
		} else
			return (0);
	}
}

long heir4 (long *lval)
/* long	lval[]; */
{
	long	k, lval2[6];

	k = heir5 (lval);
	blanks ();
	if ((ch() != '&') | (nch() == '|') | (nch() == '='))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if ((ch() == '&') & (nch() != '&') & (nch() != '=')) {
			inbyte ();
			gpush ();
			if (heir5 (lval2))
				rvalue (lval2);
			gand ();
			blanks();
		} else
			return (0);
	}
}

static int is_byte(long *lval)
{
	if (!lval[0]) {
		if (lval[4] < 0x80)
			return 1;
		else
			return 0;
	}
	if (((char *)lval[0])[TYPE] == CCHAR || ((char *)lval[0])[TYPE] == CUCHAR)
		return 1;
	return 0;
}

long heir5 (long *lval)
/*long	lval[]; */
{
	long	k, lval2[6];

	k = heir6 (lval);
	blanks ();
	if (!sstreq ("==") &
	    !sstreq ("!="))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if (match ("==")) {
			gpush ();
			if (heir6 (lval2))
				rvalue (lval2);
			geq (is_byte(lval) && is_byte(lval2));
		} else if (match ("!=")) {
			gpush ();
			if (heir6 (lval2))
				rvalue (lval2);
			gne (is_byte(lval) && is_byte(lval2));
		} else
			return (0);
	}
}

long heir6 (long *lval)
/* long	lval[]; */
{
	long	k, lval2[6];

	k = heir7 (lval);
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
			if (heir7 (lval2))
				rvalue (lval2);
			if (lval[2] || lval2[2] ||
			    is_unsigned(lval) ||
			    is_unsigned(lval2)
			   ) {
				gule (is_byte(lval) && is_byte(lval2));
				continue;
			}
			gle (is_byte(lval) && is_byte(lval2));
		} else if (match (">=")) {
			gpush ();
			if (heir7 (lval2))
				rvalue (lval2);
			if (lval[2] || lval2[2] ||
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
			if (heir7 (lval2))
				rvalue (lval2);
			if (lval[2] || lval2[2] ||
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
			if (heir7 (lval2))
				rvalue (lval2);
			if (lval[2] || lval2[2] ||
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

long heir7 (long *lval)
/*long	lval[]; */
{
	long	k, lval2[6];

	k = heir8 (lval);
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
			if (heir8 (lval2))
				rvalue (lval2);
			gasr (is_unsigned(lval));
		} else if (sstreq("<<") && ! sstreq("<<=")) {
			inbyte(); inbyte();
			gpush ();
			if (heir8 (lval2))
				rvalue (lval2);
			gasl ();
		} else
			return (0);
		blanks();
	}
}

long heir8 (long *lval)
/*long	lval[]; */
{
	long	k, lval2[6];

	k = heir9 (lval);
	blanks ();
	if ( ( (ch () != '+') && (ch () != '-') ) || (nch() == '=') )
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if (match ("+")) {
			gpush ();
			if (heir9 (lval2))
				rvalue (lval2);
			/* if left is pointer and right is int, scale right */
			if (dbltest (lval, lval2)) {
				if (lval[5]) {
					TAG_SYMBOL *tag = (TAG_SYMBOL *)(lval[5]);
					if (tag->size == 2)
						gaslint();
					else if (tag->size > 1)
						gmult_imm(tag->size);
				}
				else
					gaslint ();
			}
			/* will scale left if right int pointer and left int */
			gadd (lval,lval2);
			result (lval, lval2);
		} else if (match ("-")) {
			gpush ();
			if (heir9 (lval2))
				rvalue (lval2);
			/* if dbl, can only be: pointer - int, or
						pointer - pointer, thus,
				in first case, int is scaled up,
				in second, result is scaled down. */
			if (dbltest (lval, lval2)) {
				if (lval[5]) {
					TAG_SYMBOL *tag = (TAG_SYMBOL *)(lval[5]);
					if (tag->size == 2)
						gaslint();
					else if (tag->size > 1)
						gmult_imm(tag->size);
				}
				else
					gaslint ();
			}
			gsub ();
			/* if both pointers, scale result */
			/* XXX: structs? */
			if ((lval[2] == CINT) && (lval2[2] == CINT)) {
				gasrint(); /* divide by intsize */
			}
			result (lval, lval2);
		} else
			return (0);
	}
}

long heir9 (long *lval)
/* long	lval[]; */
{
	long	k, lval2[6];

	k = heir10 (lval);
	blanks ();
	if (((ch () != '*') && (ch () != '/') &&
		(ch () != '%')) || (nch() == '='))
		return (k);
	if (k)
		rvalue (lval);
	FOREVER {
		if (match ("*")) {
			gpush ();
			if (heir10 (lval2))
				rvalue (lval2);
			gmult (is_unsigned(lval) || is_unsigned(lval2));
		} else if (match ("/")) {
			gpush ();
			if (heir10 (lval2))
				rvalue (lval2);
			gdiv (is_unsigned(lval) || is_unsigned(lval2));
		} else if (match ("%")) {
			gpush ();
			if (heir10 (lval2))
				rvalue (lval2);
			gmod (is_unsigned(lval) || is_unsigned(lval2));
		} else
			return (0);
	}
}

long heir10 (long *lval)
/* long	lval[]; */
{
	long	k;
	char	*ptr;

	if (match ("++")) {
		indflg = 0;
		if ((k = heir10 (lval)) == 0) {
			needlval ();
			return (0);
		}
		if (lval[1])
			gpush ();
		rvalue (lval);
		ginc (lval);
		store (lval);
		return (0);
	} else if (match ("--")) {
		indflg = 0;
		if ((k = heir10 (lval)) == 0) {
			needlval ();
			return (0);
		}
		if (lval[1])
			gpush ();
		rvalue (lval);
		gdec (lval);
		store (lval);
		return (0);
	} else if (match ("-")) {
		indflg = 0;
		k = heir10 (lval);
		if (k)
			rvalue (lval);
		gneg ();
		return (0);
	} else if (match ("~")) {
		indflg = 0;
		k = heir10 (lval);
		if (k)
			rvalue (lval);
		gcom ();
		return (0);
	} else if (match ("!")) {
		indflg = 0;
		k = heir10 (lval);
		if (k)
			rvalue (lval);
		glneg ();
		return (0);
	} else if (ch()=='*' && nch() != '=') {
		inbyte();
		indflg = 1;
		k = heir10 (lval);
		indflg = 0;
		ptr = (char*)lval[0];
		/* vram */
		if (ptr && !strcmp(ptr, "vram")) {
			lval[2] = 0;
			return (1);
		}
		if (k)
			rvalue (lval);
		if ( (ptr = (char*)lval[0]) )
			lval[1] = ptr[TYPE];
		else
			lval[1] = CINT;
		lval[2] = 0;  /* flag as not pointer or array */
		return (1);
	} else if (ch()=='&' && nch()!='&' && nch()!='=') {
		indflg = 0;
		inbyte();
		k = heir10 (lval);
		if (k == 0) {
			error ("illegal address");
			return (0);
		}
		if (lval[0]) {
			ptr = (char*)lval[0];
			lval[2] = ptr[TYPE];
		}
		if (lval[1])
			return (0);
		/* global and non-array */
		ptr = (char*)lval[0];
		immed (T_SYMBOL, (long)ptr);
		lval[1] = ptr[TYPE];
		return (0);
	} else {
		k = heir11 (lval);
		ptr = (char*)lval[0];
		if (match ("++")) {
			if (k == 0) {
				needlval ();
				return (0);
			}
			/* vram */
			if (ptr && !strcmp(ptr, "vram"))
				return (0);
			if (lval[1])
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
			if (ptr && !strcmp(ptr, "vram")) {
				error("can't decrement vram pointer");
				return (0);
			}
			if (lval[1])
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

long heir11 (long *lval)
/*long	*lval; */
{
	long	direct,k;
	char	*ptr;
	char    sname[NAMESIZE];

	k = primary (lval);
	ptr = (char*)lval[0];
	blanks ();
	for (;;) {
		if (match ("[")) {
			if (ptr == 0) {
				error ("can't subscript");
				junk ();
				needbrack ("]");
				return (0);
			}
			else if (ptr[IDENT] == POINTER)
				rvalue (lval);
			else if (ptr[IDENT] != ARRAY) {
				error ("can't subscript");
				k = 0;
			}
			if (!ptr[FAR])
				gpush ();
			expression (YES);
			needbrack ("]");
			if (ptr[TYPE] == CINT || ptr[TYPE] == CUINT)
				gaslint ();
			else if (ptr[TYPE] == CSTRUCT) {
				int tagidx = ptr[TAGIDX] | (ptr[TAGIDX+1] << 8);
				int size = tag_table[tagidx].size;
				if (size == 2)
					gaslint();
				else if (size > 1)
					gmult_imm(size);
			}
			if (!ptr[FAR])
				gadd (NULL,NULL);
			lval[0] = 0;
			lval[1] = ptr[TYPE];
			lval[2] = 0;//VARIABLE; /* David, bug patch ?? */
			lval[3] = ptr[FAR] ? (long)ptr : (long)NULL;
			k = 1;
		}
		else if (match ("(")) {
			if (ptr == 0) {
				error("invalid or unsupported function call");
				callfunction (0);
			}
			else if (ptr[IDENT] != FUNCTION) {
				if (strcmp(ptr, "vram") == 0)
					callfunction (ptr);
				else {
					if (ptr[FAR]) {
						lval[3] = (long)ptr;
						immed (T_VALUE, 0);
					}
					rvalue (lval);
					callfunction (0);
				}
			} else
				callfunction (ptr);
			k = lval[0] = 0;
		} else if ((direct=match(".")) || match("->")) {
			if (lval[5] == 0) {
			    error("can't take member") ;
			    junk() ;
			    return 0 ;
			}
			if (symname(sname) == 0 ||
			   ((ptr=(char *)find_member((TAG_SYMBOL *)lval[5], sname)) == 0)) {
			    error("unknown member") ;
			    junk() ;
			    return 0 ;
			}
			if (k && direct == 0)
			    rvalue(lval);
			out_ins(I_ADDWI, T_VALUE, ptr[OFFSET] | (ptr[OFFSET+1] << 8));
			//add_offset(ptr[OFFSET] | (ptr[OFFSET+1] << 8)); // move pointer from struct begin to struct member
			lval[0] = (long)ptr;
			lval[1] = ptr[TYPE]; // lval->indirect = lval->val_type = ptr->type
			lval[2] = 0;
			lval[5] = (long)NULL_TAG;
			if (ptr[TYPE] == CSTRUCT)
			    lval[5] = (long)&tag_table[ptr[TAGIDX] | (ptr[TAGIDX+1] << 8)];
			if (ptr[IDENT] == POINTER) {
			    lval[1] = CINT;
			    lval[2] = ptr[TYPE];
			    //lval->val_type = CINT;
			}
			if (ptr[IDENT]==ARRAY ||
			    (ptr[TYPE]==CSTRUCT && ptr[IDENT]==VARIABLE)) {
			    // array or struct
			    lval[2] = ptr[TYPE];
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
	if (ptr[IDENT] == FUNCTION) {
		immed (T_SYMBOL, (long)ptr);
		return (0);
	}
	return (k);
}

void store (long *lval)
/* long	*lval; */
{
	if (lval[3]) {
		/* far arrays (or special arrays) */
		if (!strcmp ((char *)lval[3], "vdc"))
			putio ((char *)lval[3]);
		else if (!strcmp ((char *)lval[3], "vram"))
			putvram ((char *)lval[3]);
		else {
			error ("const arrays can't be written");
			gpop ();
		}
	}
	else {
		/* other */
		if (lval[1] != 0)
			putstk (lval[1]);
		else {
			if (strcmp((char *)lval[0], "vram") == 0)
				out_ins(I_VPUTW, (long)NULL, (long)NULL);
			else
				putmem ((char *)lval[0]);
		}
	}
}

void rvalue (long *lval)
/* long	*lval; */
{
	if ((lval[0] != 0) && (lval[1] == 0)) {
		if (strcmp((char *)lval[0], "vram") == 0)
			out_ins(I_VGETW, (long)NULL, (long)NULL);
		else
			getmem ((char *)lval[0]);
	}
	else {
		if (lval[3] == 0)
			indirect (lval[1]);
		else {
			/* far arrays (or special arrays) */
			if (!strcmp ((char *)lval[3], "vdc"))
				getio ((char *)lval[3]);
			else if (!strcmp ((char *)lval[3], "vram"))
				getvram ((char *)lval[3]);
			else
				farpeek ((char *)lval[3]);
		}
	}
}

void needlval (void )
{
	error ("must be lvalue");
}

