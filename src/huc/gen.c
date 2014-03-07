/*	File gen.c: 2.1 (83/03/20,16:02:06) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "primary.h"
#include "sym.h"

static char *needargs[] = {
	"vreg",
	"vsync",
	"spr_hide", "spr_show",
	"satb_update",
	"map_load_tile",
	NULL
};

/*
 *	gen arg count
 *
 */
void gnargs (char *name, long nb)
{
	char *ptr;
	long   i;

	if (name == NULL)
		return;
	for (i = 0; ; i++) {
		ptr = needargs[i];

		if (ptr == NULL)
			break;
		if (strcmp(name, ptr) == 0) {
			out_ins(I_NARGS, T_VALUE, nb);
			break;
		}
	}
}

/*
 *	return next available internal label number
 *
 */
long getlabel (void )
{
	return (nxtlab++);
}

/*
 *	fetch a static memory cell into the primary register
 */
void getmem (char *sym)
{
	if ((sym[IDENT] != POINTER) && (sym[TYPE] == CCHAR || sym[TYPE] == CUCHAR)) {
		int op = I_LDB;
		if (sym[TYPE] & CUNSIGNED)
			op = I_LDUB;
		if ((sym[STORAGE] & ~WRITTEN) == LSTATIC)
			out_ins(op, T_LABEL, glint(sym));
		else
			out_ins(op, T_SYMBOL, (long)(sym + NAME));
	} else {
		if ((sym[STORAGE] & ~WRITTEN) == LSTATIC)
			out_ins(I_LDW, T_LABEL, glint(sym));
		else
			out_ins(I_LDW, T_SYMBOL, (long)(sym + NAME));
	}
}

/*
 *	fetch a hardware register into the primary register
 */
void getio (char *sym)
{
	out_ins(I_CALL, T_LIB, (long)"getvdc");
}
void getvram (char *sym)
{
	out_ins(I_CALL, T_LIB, (long)"readvram");
}

/*
 *	fetch the address of the specified symbol into the primary register
 *
 */
void getloc (char *sym)
{
	long value;

	if ((sym[STORAGE] & ~WRITTEN) == LSTATIC)
		out_ins(I_LDWI, T_LABEL, glint(sym));
	else {
		value = glint(sym) - stkp;
		out_ins_sym(X_LEA_S, T_STACK, value, sym);

//		out_ins(I_LDW, T_STACK, NULL);
//
//		if (value || optimize) {
//			out_ins(I_ADDWI, T_VALUE, value);
//		}
	}
}

/*
 *	store the primary register into the specified static memory cell
 *
 */
void putmem (char *sym)
{
	long code;

	if ((sym[IDENT] != POINTER) & (sym[TYPE] == CCHAR || sym[TYPE] == CUCHAR))
		code = I_STB;
	else
		code = I_STW;

	if ((sym[STORAGE] & ~WRITTEN) == LSTATIC)
		out_ins(code, T_LABEL, glint(sym));
	else
		out_ins(code, T_SYMBOL, (long)(sym + NAME));
}

/*
 *	store the specified object type in the primary register
 *	at the address on the top of the stack
 *
 */
void putstk (char typeobj)
{
	if (typeobj == CCHAR || typeobj == CUCHAR)
		out_ins(I_STBPS, (long)NULL, (long)NULL);
	else
		out_ins(I_STWPS, (long)NULL, (long)NULL);
	stkp = stkp + INTSIZE;
}

/*
 *	store the primary register
 *	at the address on the top of the stack
 *
 */
void putio (char *sym)
{
	out_ins(I_JSR, T_LIB, (long)"setvdc");
	stkp = stkp + INTSIZE;
}
void putvram (char *sym)
{
	out_ins(I_JSR, T_LIB, (long)"writevram");
	stkp = stkp + INTSIZE;
}

/*
 *	fetch the specified object type indirect through the primary
 *	register into the primary register
 *
 */
void indirect (char typeobj)
{
	out_ins(I_STW, T_PTR, (long)NULL);
	if (typeobj == CCHAR)
		out_ins(I_LDBP, T_PTR, (long)NULL);
	else if (typeobj == CUCHAR)
		out_ins(I_LDUBP, T_PTR, (long)NULL);
	else {
		out_ins(I_LDWP, T_PTR, (long)NULL);
	}
}

void farpeek(char *ptr)
{
	if (ptr[TYPE] == CCHAR)
		out_ins(I_FGETB, T_SYMBOL, (long)ptr);
	else if (ptr[TYPE] == CUCHAR)
		out_ins(I_FGETUB, T_SYMBOL, (long)ptr);
	else
		out_ins(I_FGETW, T_SYMBOL, (long)ptr);
}

/*
 *	print partial instruction to get an immediate value into
 *	the primary register
 *
 */
void immed (long type, long data)
{
	out_ins(I_LDWI, type, data);
}

/*
 *	push the primary register onto the stack
 *
 */
void gpush (void )
{
//	out_ins(I_PUSHWZ, T_VALUE, zpstkp);
//	zpstkp = zpstkp - INTSIZE;

	out_ins(I_PUSHW, T_VALUE, INTSIZE);
	stkp = stkp - INTSIZE;
}

/*
 *	push the primary register onto the stack
 *
 */
void gpusharg (long size)
{
	out_ins(I_PUSHW, T_SIZE, size);
	stkp = stkp - size;
}

/*
 *	pop the top of the stack into the secondary register
 *
 */
void gpop (void )
{
	out_ins(I_POPW, (long)NULL, (long)NULL);
	stkp = stkp + INTSIZE;
}

/*
 *	swap the primary register and the top of the stack
 *
 */
void swapstk (void )
{
	out_ins(I_SWAPW, (long)NULL, (long)NULL);
}

/*
 *	call the specified subroutine name
 *
 */
void gcall (char *sname, long nargs)
{
	out_ins_ex(I_CALL, T_SYMBOL, (long)sname, nargs);
}

/*
 *         generate a bank pseudo instruction
 *
 */
void gbank(unsigned char bank, unsigned short offset)
{
	out_ins(I_BANK, T_VALUE, bank);
	out_ins(I_OFFSET, T_VALUE, offset);
}

/*
 *	return from subroutine
 *
 */
void gret (void )
{
	out_ins(I_RTS, (long)NULL, (long)NULL);
}

/*
 *	perform subroutine call to value on top of stack
 *
 */
void callstk (long nargs)
{
	if (nargs <= INTSIZE)
		out_ins(I_CALLS, T_STACK, 0);
	else
		out_ins(I_CALLS, T_STACK, nargs - INTSIZE);

	stkp = stkp + INTSIZE;
}

/*
 *	jump to specified internal label number
 *
 */
void jump (long label)
{
	out_ins(I_LBRA, T_LABEL, label);
}

/*
 *	test the primary register and jump if false to label
 *
 */
void testjump (long label, long ft)
{
	out_ins(I_TSTW, (long)NULL, (long)NULL);
	if (ft)
		out_ins(I_LBNE, T_LABEL, label);
	else
		out_ins(I_LBEQ, T_LABEL, label);
}

/*
 *	modify the stack pointer to the new value indicated
 *      Is it there that we decrease the value of the stack to add local vars ?
 */
long modstk (long newstkp)
{
	long	k;

//	k = galign(newstkp - stkp);
	k = newstkp - stkp;
	if (k) {
		gtext();
		out_ins(I_ADDMI, T_STACK, k);
	}
	return (newstkp);
}

/*
 *	multiply the primary register by INTSIZE
 */
void gaslint (void )
{
	out_ins(I_ASLW, (long)NULL, (long)NULL);
}

/*
 *	divide the primary register by INTSIZE
 */
void gasrint(void )
{
	out_ins(I_ASRW, (long)NULL, (long)NULL);
}

/*
 *	Case jump instruction
 */
void gjcase(void )
{
	out_ins(I_JMP, T_SYMBOL, (long)"__case");
}

/*
 *	add the primary and secondary registers
 *	if lval2 is int pointer and lval is int, scale lval
 */
void gadd (long *lval, long *lval2)
{
	if (dbltest (lval2, lval)) {
		out_ins(I_ASLWS, (long)NULL, (long)NULL);
	}
	out_ins(I_ADDWS, (long)NULL, (long)NULL);
	stkp = stkp + INTSIZE;
}

/*
 *	subtract the primary register from the secondary
 *
 */
void gsub (void )
{
	out_ins(I_SUBWS, (long)NULL, (long)NULL);
	stkp = stkp + INTSIZE;
}

/*
 *	multiply the primary and secondary registers
 *	(result in primary)
 *
 */
void gmult (int is_unsigned)
{
	if (is_unsigned)
		out_ins(I_JSR, T_LIB, (long)"umul");
	else
		out_ins(I_JSR, T_LIB, (long)"smul");
	stkp = stkp + INTSIZE;
}

void gmult_imm (int value)
{
    gpush();
    immed(T_VALUE, value);
    gmult(1);
}

/*
 *	divide the secondary register by the primary
 *	(quotient in primary, remainder in secondary)
 *
 */
void gdiv (int is_unsigned)
{
	if (is_unsigned)
		out_ins(I_JSR, T_LIB, (long)"udiv");
	else
		out_ins(I_JSR, T_LIB, (long)"sdiv");
	stkp = stkp + INTSIZE;
}

/*
 *	compute the remainder (mod) of the secondary register
 *	divided by the primary register
 *	(remainder in primary, quotient in secondary)
 *
 */
void gmod (int is_unsigned)
{
	if (is_unsigned)
		out_ins(I_JSR, T_LIB, (long)"umod");
	else
		out_ins(I_JSR, T_LIB, (long)"smod");
	stkp = stkp + INTSIZE;
}

/*
 *	inclusive 'or' the primary and secondary registers
 *
 */
void gor (void )
{
	out_ins(I_ORWS, (long)NULL, (long)NULL);
	stkp = stkp + INTSIZE;
}

/*
 *	exclusive 'or' the primary and secondary registers
 *
 */
void gxor (void )
{
	out_ins(I_EORWS, (long)NULL, (long)NULL);
	stkp = stkp + INTSIZE;
}

/*
 *	'and' the primary and secondary registers
 *
 */
void gand (void )
{
	out_ins(I_ANDWS, (long)NULL, (long)NULL);
	stkp = stkp + INTSIZE;
}

/*
 *	arithmetic shift right the secondary register the number of
 *	times in the primary register
 *	(results in primary register)
 *
 */
void gasr (int is_unsigned)
{
	if (is_unsigned)
		out_ins(I_JSR, T_LIB, (long)"lsr");
	else
		out_ins(I_JSR, T_LIB, (long)"asr");
	stkp = stkp + INTSIZE;
}

/*
 *	arithmetic shift left the secondary register the number of
 *	times in the primary register
 *	(results in primary register)
 *
 */
void gasl (void )
{
	out_ins(I_JSR, T_LIB, (long)"asl");
	stkp = stkp + INTSIZE;
}

/*
 *	two's complement of primary register
 *
 */
void gneg (void )
{
	out_ins(I_NEGW, (long)NULL, (long)NULL);
}

/*
 *	one's complement of primary register
 *
 */
void gcom (void )
{
	out_ins(I_COMW, (long)NULL, (long)NULL);
}

/*
 *	convert primary register into logical value
 *
 */
void gbool (void )
{
	out_ins(I_BOOLW, (long)NULL, (long)NULL);
}

/*
 *	logical complement of primary register
 *
 */
void glneg (void )
{
	out_ins(I_NOTW, (long)NULL, (long)NULL);
}

/*
 *	increment the primary register by 1 if char, INTSIZE if
 *      long
 */
void ginc (long * lval)
/* long lval[]; */
{
	if (lval[2] == CINT || lval[2] == CUINT)
		out_ins(I_ADDWI, T_VALUE, 2);
	else if (lval[2] == CSTRUCT) {
		TAG_SYMBOL *tag = (TAG_SYMBOL *)(lval[5]);
		out_ins(I_ADDWI, T_VALUE, tag->size);
        }
	else
		out_ins(I_ADDWI, T_VALUE, 1);
}

/*
 *	decrement the primary register by one if char, INTSIZE if
 *	long
 */
void gdec (long *lval)
/* long lval[]; */
{
	if (lval[2] == CINT || lval[2] == CUINT)
		out_ins(I_SUBWI, T_VALUE, 2);
	else if (lval[2] == CSTRUCT) {
		TAG_SYMBOL *tag = (TAG_SYMBOL *)(lval[5]);
		out_ins(I_SUBWI, T_VALUE, tag->size);
        }
	else
		out_ins(I_SUBWI, T_VALUE, 1);
}

/*
 *	following are the conditional operators.
 *	they compare the secondary register against the primary register
 *	and put a literl 1 in the primary if the condition is true,
 *	otherwise they clear the primary register
 *
 */

/*
 *	equal
 *
 */
void geq (int is_byte)
{
	if (is_byte)
		out_ins(I_JSR, T_LIB, (long)"eqb");
	else
		out_ins(I_JSR, T_LIB, (long)"eq");
	stkp = stkp + INTSIZE;
}

/*
 *	not equal
 *
 */
void gne (int is_byte)
{
	if (is_byte)
		out_ins(I_JSR, T_LIB, (long)"neb");
	else
		out_ins(I_JSR, T_LIB, (long)"ne");
	stkp = stkp + INTSIZE;
}

/*
 *	less than (signed)
 *
 */
/*void glt (long lvl) */
void glt (int is_byte)
{
	if (is_byte)
		out_ins(I_JSR,  T_LIB, (long)"ltb");
	else
		out_ins(I_JSR,  T_LIB, (long)"lt");
	stkp = stkp + INTSIZE;
}

/*
 *	less than or equal (signed)
 *
 */
void gle (int is_byte)
{
	if (is_byte)
		out_ins(I_JSR, T_LIB, (long)"leb");
	else
		out_ins(I_JSR, T_LIB, (long)"le");
	stkp = stkp + INTSIZE;
}

/*
 *	greater than (signed)
 *
 */
void ggt (int is_byte)
{
	if (is_byte)
		out_ins(I_JSR, T_LIB, (long)"gtb");
	else
		out_ins(I_JSR, T_LIB, (long)"gt");
	stkp = stkp + INTSIZE;
}

/*
 *	greater than or equal (signed)
 *
 */
void gge (int is_byte)
{
	if (is_byte) 
		out_ins(I_JSR, T_LIB, (long)"geb");
	else
		out_ins(I_JSR, T_LIB, (long)"ge");
	stkp = stkp + INTSIZE;
}

/*
 *	less than (unsigned)
 *
 */
/*void gult (long lvl)*/
void gult (int is_byte)
{
	if (is_byte)
		out_ins(I_JSR, T_LIB, (long)"ublt");
	else
		out_ins(I_JSR, T_LIB, (long)"ult");
	stkp = stkp + INTSIZE;
}

/*
 *	less than or equal (unsigned)
 *
 */
void gule (int is_byte)
{
	if (is_byte)
		out_ins(I_JSR, T_LIB, (long)"uble");
	else
		out_ins(I_JSR, T_LIB, (long)"ule");
	stkp = stkp + INTSIZE;
}

/*
 *	greater than (unsigned)
 *
 */
void gugt (int is_byte)
{
	if (is_byte)
		out_ins(I_JSR, T_LIB, (long)"ubgt");
	else
		out_ins(I_JSR, T_LIB, (long)"ugt");
	stkp = stkp + INTSIZE;
}

/*
 *	greater than or equal (unsigned)
 *
 */
void guge (int is_byte)
{
	if (is_byte)
		out_ins(I_JSR, T_LIB, (long)"ubge");
	else
		out_ins(I_JSR, T_LIB, (long)"uge");
	stkp = stkp + INTSIZE;
}

void scale_const(int type, int otag, long *size) {
    switch (type) {
        case CINT:
        case CUINT:
            *size += *size;
            break;
        case CSTRUCT:
            *size *= tag_table[otag].size;
            break;
        default:
            break;
    }
}
