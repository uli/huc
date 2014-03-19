/*	File function.c: 2.1 (83/03/20,16:02:04) */
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
#include "optimize.h"
#include "pragma.h"
#include "pseudo.h"
#include "stmt.h"
#include "sym.h"
#include "struct.h"

/* locals */
static INS  ins_stack[1024];
static long  ins_stack_idx;
/* static char ins_stack_fname[NAMESIZE]; */
static long  arg_list[32][2];
/* static long  arg_list_idx; */
static long  arg_idx;
static long  func_call_stack;

/* globals */
long arg_stack_flag;
long argtop;

/* protos */
void arg_flush(long arg, long adj);
void arg_to_fptr(struct fastcall *fast, long i, long arg, long adj);
void arg_to_dword(struct fastcall *fast, long i, long arg, long adj);

/* function declaration styles */
#define KR 0
#define ANSI 1

/* argument address pointers for ANSI arguments */
short *fixup[32];
char current_fn[NAMESIZE];

/*
 *	begin a function
 *
 *	called from "parse", this routine tries to make a function out
 *	of what follows
 *	modified version.  p.l. woods
 *
 */
void newfunc (const char *sname)
{
	char n[NAMESIZE];
	SYMBOL *ptr;
	long  nbarg;

	if (sname) {
		strcpy(current_fn, sname);
		strcpy(n, sname);
	}
	else {
		/* allow (and ignore) return type */
		amatch("const", 5);
		amatch("unsigned", 8);
		amatch("short", 5);
		if (amatch("char", 4) || amatch("int", 3) || amatch("void", 4)) {
			while (match("*")) {
			}
		}

		if (!symname (n) ) {
			error ("illegal function or declaration");
			kill ();
			return;
		}
		strcpy(current_fn, n);
		if (!match ("("))
			error ("missing open paren");
	}
	locptr = STARTLOC;
	argstk = 0;
	argtop = 0;
	nbarg = 0;
	memset(fixup, 0, sizeof(fixup));
	while (!match (")")) {
		/* check if we have an ANSI argument */
		int sign = CSIGNED;
		if (amatch("register", 8)) {
			/* ignore */
		}
		if (amatch("const", 5)) {
			/* ignore */
		}
		if (amatch("struct", 6) || amatch("union", 5)) {
			if (symname(n)) {
				int otag = find_tag(n);
				if (otag < 0) {
					error("unknown struct name");
					junk();
				}
				else {
					getarg(CSTRUCT, ANSI, otag);
					nbarg++;
				}
			}
			else {
				error("illegal struct name");
				junk();
			}
		}
		else {
			if (amatch("unsigned", 8))
				sign = CUNSIGNED;
			if (amatch("signed", 6) && sign == CUNSIGNED)
				error("conflicting signedness");
			if (amatch("char", 4)) {
				getarg(CCHAR | sign, ANSI, 0);
				nbarg++;
			} else if (amatch("short", 5)) {
				amatch("int", 3);
				getarg(CINT | sign, ANSI, 0);
				nbarg++;
			} else if (amatch("int", 3)) {
				getarg(CINT | sign, ANSI, 0);
				nbarg++;
			} else if (amatch("void", 4)) {
				if (match(")"))
					break;
				getarg(CVOID, ANSI, 0);
				nbarg++;
			} else if (sign == CUNSIGNED) {
				getarg(CINT | sign, ANSI, 0);
				nbarg++;
			} else {
				/* no valid type, assuming K&R argument */
				if (symname (n)) {
					if (findloc (n))
						multidef (n);
					else {
						addloc (n, 0, 0, argstk, AUTO, INTSIZE);
						argstk = argstk + INTSIZE;
						nbarg++;
					}
				} else {
					error ("illegal argument name");
					junk ();
				}
			}
		}
		blanks ();
		if (!streq (line + lptr, ")")) {
			if (!match (","))
				error ("expected comma");
		}
		if (endst ())
			break;
	}

	/* ignore function prototypes */
	if (match(";"))
		return;

	stkp = 0;
	argtop = argstk;
	while (argstk) {
		/* We only know the final argument offset once we have parsed
		   all of them. That means that for ANSI arguments we have
		   to fix up the addresses for the locations generated in
		   getarg() here. */
		if (fixup[argstk/INTSIZE - 1]) {
			argstk -= INTSIZE;
			*fixup[argstk/INTSIZE] += argtop;
		} else {
			int sign = CSIGNED;
			if (amatch("register", 8)) {
				/* ignore */
			}
			if (amatch("struct", 6)) {
				if (symname(n)) {
					int otag = find_tag(n);
					if (otag < 0) {
						error("unknown struct name");
						junk();
					}
					else {
						getarg(CSTRUCT, KR, otag);
						ns();
					}
				}
				else {
					error("illegal struct name");
					junk();
				}
			}
			else {
				if (amatch("unsigned", 8))
					sign = CUNSIGNED;
				if (amatch("signed", 6) && sign == CUNSIGNED)
					error("conflicting signedness");
				if (amatch ("char", 4)) {
					getarg (CCHAR | sign, KR, 0);
					ns ();
				} else if (amatch ("short", 5)) {
					amatch("int", 3);
					getarg (CINT | sign, KR, 0);
					ns();
				} else if (amatch ("int", 3) || sign == CUNSIGNED) {
					getarg (CINT | sign, KR, 0);
					ns ();
				} else {
					error ("wrong number args");
					break;
				}
			}
		}
	}

	fexitlab = getlabel();

	if ( (ptr = findglb (current_fn)) ) {
		if (ptr->ident != FUNCTION)
			multidef (current_fn);
		else if (ptr->offset == FUNCTION)
			multidef (current_fn);
		else
			ptr->offset = FUNCTION;
	} else
		addglb (current_fn, FUNCTION, CINT, FUNCTION, PUBLIC);

	flush_ins(); /* David, .proc directive support */
	ot (".proc ");
	prefix ();
	outstr (current_fn);
	nl ();

	if (nbarg)      /* David, arg optimization */
		gpusharg (0);
	statement(YES);
	gnlabel(fexitlab);
	modstk (nbarg * INTSIZE);
	gret (); /* generate the return statement */
	flush_ins();    /* David, optimize.c related */
	ol (".endp");   /* David, .endp directive support */
	nl ();
	stkp = 0;
	locptr = STARTLOC;
}

/*
 *	declare argument types
 *
 *	called from "newfunc", this routine add an entry in the local
 *	symbol table for each named argument
 *	completely rewritten version.  p.l. woods
 *
 */
void getarg (long t, int syntax, int otag)
{
	long	j, legalname, address;
	char	n[NAMESIZE];
	SYMBOL	*argptr;
	int ptr_order = 0;
/*	char	c; */

	FOREVER {
		if (syntax == KR && argstk == 0)
			return;
		j = VARIABLE;
		while (match ("*")) {
			j = POINTER;
			ptr_order++;
		}

		if (t == CSTRUCT && j != POINTER)
			error("passing structures as arguments by value not implemented yet");

		if (t == CVOID) {
			if (j != POINTER)
				error("illegal argument type \"void\"");
			else
				t = CUINT;
		}

		if (!(legalname = symname (n))) {
			if (syntax == ANSI && (ch() == ',' || ch() == ')')) {
				sprintf(n, "__anon_%ld\n", -argstk);
			}
			else {
				illname ();
				junk();
			}
		}
		if (match ("[")) {
			while (inbyte () != ']')
				if (endst ())
					break;
			j = POINTER;
			ptr_order++;
		}
		if (legalname) {
			if (syntax == ANSI) {
				if (findloc (n))
					multidef (n);
				else {
					addloc (n, 0, 0, argstk, AUTO, INTSIZE);
					argstk = argstk + INTSIZE;
				}
			}
			if ( (argptr = findloc (n)) ) {
				argptr->ident = j;
				argptr->type = t;
				address = argtop - glint(argptr) - 2;
				if ((t == CCHAR || t == CUCHAR) && j == VARIABLE)
					address = address + BYTEOFF;
				argptr->offset = address;
				argptr->tagidx = otag;
				argptr->ptr_order = ptr_order;
				if (syntax == ANSI)
					fixup[argstk/INTSIZE - 1] = &argptr->offset;
			} else
				error ("expecting argument name");
		}
		if (syntax == KR) {
			argstk = argstk - INTSIZE;
			if (endst ())
				return;
			if (!match (","))
				error ("expected comma");
		}
		else {
			blanks();
			if (streq(line + lptr, ")") || streq(line + lptr, ","))
				return;
			else
				error ("expected comma or closing bracket");
		}
	}
}

/*
 *	perform a function call
 *
 *	called from "heir11", this routine will either call the named
 *	function, or if the supplied ptr is zero, will call the contents
 *	of HL
 *
 */
void callfunction (char *ptr)
{
	extern char *new_string(long, char *);
	struct fastcall *fast;
	long call_stack_ref;
	long i, j;
	long nb;
	long adj;
	long idx;
	long	nargs;
	long cnt;

	cnt = 0;
	nargs = 0;
	fast = NULL;
	call_stack_ref = ++func_call_stack;

	/* skip blanks */
	blanks ();

	/* check if it's a special function,
	 * if yes handle it externaly
	 */
	if (ptr) {
		     if (!strcmp(ptr,"bank")) { do_asm_func(T_BANK); return; }
		else if (!strcmp(ptr,"vram")) { do_asm_func(T_VRAM); return; }
		else if (!strcmp(ptr,"pal"))  { do_asm_func(T_PAL);  return; }
		else if (!strcmp(ptr,"set_bgpal")) { doset_bgpalstatement(); return; }
		else if (!strcmp(ptr,"set_sprpal")) { doset_sprpalstatement(); return; }
		else if (!strcmp(ptr,"load_sprites")) { doload_spritesstatement(); return; }
		else if (!strcmp(ptr,"load_background")) { doload_backgroundstatement(); return; }
	}

	/* indirect call (push func address) */
	if (ptr == NULL)
		gpush ();

	/* fastcall check */
	if (ptr == NULL)
		nb = 0;
	else
		nb = fastcall_look(ptr, -1, NULL);

	/* calling regular functions in fastcall arguments is OK */
	if (!nb)
		--func_call_stack;

	/* flush instruction stack */
	flush_ins();

	/* get args */
	while (!streq (line + lptr, ")")) {
		if (endst ())
			break;
		/* fastcall func */
		if (nb) {
			if (nargs)
				stkp = stkp - INTSIZE;
			arg_stack(arg_idx++);
			expression (NO);
			flush_ins();
		}
		/* standard func */
		else {
			if (nargs)
				gpusharg (INTSIZE);
			expression (NO);
		}
		nargs = nargs + INTSIZE;
		cnt++;
		if (!match (","))
			break;
	}

	/* adjust arg stack */
	if (nb) {
		if (cnt) {
			arg_list[arg_idx - 1][1] = ins_stack_idx;
			arg_idx -= cnt;
		}
		if (arg_idx)
			ins_stack_idx  = arg_list[arg_idx - 1][1];
		else {
			ins_stack_idx  = 0;
			arg_stack_flag = 0;
		}
	}

	/* fastcall func */
	if (nb) {
		nb = fastcall_look(ptr, cnt, &fast);

		/* flush arg instruction stacks */
		if (nb) {
			/* error checking */
			if (fast->flags & 0x02) {
				if (func_call_stack != call_stack_ref) {
					char tmp[NAMESIZE+80];
					sprintf(tmp, "funcs can't be called from "
								 "inside '%s' func call", ptr);
					error(tmp);
				}
			}

			/* fastcall */
			for (i = 0, j = 0, adj = 0, idx = fast->argsize; i < cnt; i++) {
				/* flush arg stack (except for farptr and dword args) */
				if ((fast->argtype[j] != 0x03) &&
					(fast->argtype[j] != 0x04))
				{
					arg_flush(arg_idx + i, adj);
				}
	
				/* store arg */
				switch (fast->argtype[j]) {
				case 0x01: /* byte */
					out_ins(I_STB, T_SYMBOL, (long)fast->argname[j]);
					break;
				case 0x02: /* word */
					out_ins(I_STW, T_SYMBOL, (long)fast->argname[j]);
					break;
				case 0x03: /* farptr */
					arg_to_fptr(fast, j,  arg_idx + i, adj);
					j += 1;
					break;
				case 0x04: /* dword */
					arg_to_dword(fast, j, arg_idx + i, adj);
					j += 2;
					break;
				case 0x11: /* auto byte */
					out_ins_ex(I_PHB, T_VALUE, idx, (idx==fast->argsize) ? idx : 0);
					idx -= 1;
					break;
				case 0x12: /* auto word */
					out_ins_ex(I_PHW, T_VALUE, idx, (idx==fast->argsize) ? idx : 0);
					idx -= 2;
					break;
				case 0x00: /* acc */
					break;
			 	default:
					error("fastcall internal error");
					break;
				}
	
				/* next */
				adj += INTSIZE;
				j++;
			}
		}
		else {
			/* standard call */
			for (i = 0; i < cnt;) {
				arg_flush(arg_idx + i, 0);
				i++;
				if (i < cnt)
					gpusharg (0);
			}
		}
	}
	
	/* reset func call stack */
	if (call_stack_ref == 1)
		func_call_stack = 0;

	/* close */
	needbrack (")");

	/* push number of args */
	if (nb == 0)
		gnargs(ptr, cnt);

	/* call function */
	if (ptr == NULL)
		callstk (nargs);
	else {
		if (fast) {
			if (fast->flags & 0x01)
				goto l1;
		}
		if (nb)
			gcall (ptr, cnt);
		else
			gcall (ptr, 0);
	}
l1:
	/* adjust stack */
	if (nargs > INTSIZE) {
		nargs = nargs - INTSIZE;
		stkp  = stkp + nargs;
		out_ins(I_ADDMI, T_NOP, nargs);
	}
}

/*
 * start arg instruction stacking
 *
 */
void arg_stack(long arg)
{
	if (arg > 31)
		error("too many args");
	else {
		/* close previous stack */
		if (arg)
	   		arg_list[arg - 1][1] = ins_stack_idx;

		/* init new stack */
		ins_stack_idx += 4;
		arg_list[arg][0] = ins_stack_idx;
		arg_list[arg][1] = -1;
		arg_stack_flag = 1;
	}
}

/*
 * put instructions in a temporary stack (one for each func arg)
 *
 */
void arg_push_ins(INS *ptr)
{
	if (ins_stack_idx < 1024)
		ins_stack[ins_stack_idx++] = *ptr;
	else {
		if (ins_stack_idx < 1025) {
			ins_stack_idx++;
			error("arg stack full");
		}
	}
}

/*
 * flush arg instruction stacks
 *
 */
void
arg_flush(long arg, long adj)
{
	INS *ins;
	long  idx;
	long  nb;
	long  i;

	if (arg > 31)
		return;
	idx = arg_list[arg][0];
	nb  = arg_list[arg][1] - arg_list[arg][0];

	for(i = 0; i < nb;) {
		/* adjust stack refs */
		i++;
		ins = &ins_stack[idx];

		if((ins->type == T_STACK) && (ins->code == I_LDW)) {
			if (i < nb) {
				ins = &ins_stack[idx + 1];
				if((ins->code == I_ADDWI) && (ins->type == T_VALUE))
					ins->data -= adj;
			}
		}
		else {
			switch (ins->code) {
			case X_LEA_S:
			case X_PEA_S:
			case X_LDB_S:
			case X_LDUB_S:
			case X_LDW_S:
			case X_LDD_S_B:
			case X_LDD_S_W:
			case X_STB_S:
			case X_STW_S:
			case X_INCW_S:
			case X_ADDW_S:
			case X_STBI_S:
			case X_STWI_S:
				ins->data -= adj;
				break;
			}
		}

		/* flush */
		gen_ins(&ins_stack[idx++]);
	}		
}

void
arg_to_fptr(struct fastcall *fast, long i, long arg, long adj)
{
	INS  *ins, tmp;
	SYMBOL *sym;
	long   idx;
	long   err;
	long   nb;

	if (arg > 31)
		return;

	idx =  arg_list[arg][0];
	nb  =  arg_list[arg][1] - arg_list[arg][0];
	ins = &ins_stack[idx];
	err =  0;

	/* check arg */
	if (nb == 0)
		err = 1;
	else if (ins->type != T_SYMBOL)
		err = 1;
	else {
		if((ins->code != I_LDWI) && (ins->code != I_LDW))
			err = 1;
		else {
			sym = (SYMBOL *)ins->data;

			/* check symbol type */
			if (ins->code == I_LDW) {
				if((nb < 2) || (sym->ident == VARIABLE) || (sym->ident == ARRAY))
					err = 1;
			}
			else {
				if((sym->ident == POINTER) || (sym->ident == VARIABLE))
					err = 1;
			}
		}
	}
	if (err) {
		error("can't get farptr");
		return;
	}

	/* ok */	
	if (nb == 1) {
		ins->code   = I_FARPTR;
		ins->arg[0] = (long)fast->argname[i];
		ins->arg[1] = (long)fast->argname[i+1];
		gen_ins(ins);
	}
	else {
		sym = (SYMBOL *)ins->data;

		/* check symbol type */
		if (sym->far) {
			tmp.code = I_FARPTR_I;
			tmp.type = T_SYMBOL;
			tmp.data = ins->data;
			tmp.arg[0] = (long)fast->argname[i];
			tmp.arg[1] = (long)fast->argname[i+1];
			ins->type = T_VALUE;
			ins->data = 0;
		}
		else {
			if(((sym->ident == ARRAY) ||
				(sym->ident == POINTER)) &&
				(sym->type  == CINT || sym->type == CUINT))
			{
				tmp.code = I_FARPTR_GET;
				tmp.type = (long)NULL;
				tmp.data = (long)NULL;
				tmp.arg[0] = (long)fast->argname[i];
				tmp.arg[1] = (long)fast->argname[i+1];
			}
			else {
				error("can't get farptr");
				return;
			}
		}
		arg_flush(arg, adj);
		gen_ins(&tmp);
	}
}

void
arg_to_dword(struct fastcall *fast, long i, long arg, long adj)
{
	INS  *ins, *ptr, tmp;
	SYMBOL *sym;
	long   idx;
	long   gen;
	long   err;
	long   nb;

	if (arg > 31)
		return;

	idx =  arg_list[arg][0];
	nb  =  arg_list[arg][1] - arg_list[arg][0];
	ins = &ins_stack[idx];
	gen =  0;
	err =  1;

	/* check arg */
	if (nb == 1) {
		/* immediate value */
		if((ins->code == I_LDWI) && (ins->type == T_VALUE)) {
			ins->code = X_LDD_I;
			ins->arg[0] = (long)fast->argname[i+1];
			ins->arg[1] = (long)fast->argname[i+2];
			gen = 1;
		}

		/* var/ptr */
		else if(  (((ins->code == I_LDW) || (ins->code == I_LDB) || (ins->code == I_LDUB))
			 && (ins->type == T_SYMBOL)) || (ins->type == T_LABEL) )
		{
			/* check special cases */
			if (ins->type == T_LABEL) {
				error("dword arg can't be a static var");
				return;
			}

			/* get symbol */
			sym = (SYMBOL *)ins->data;

			/* check type */
			if (sym->ident == POINTER)
				gen = 1;
			else if (sym->ident == VARIABLE) {
				if (ins->code == I_LDW)
					ins->code  = X_LDD_W;
				else
					ins->code  = X_LDD_B;

				ins->type   = T_SYMBOL;
				ins->data   = (long)sym;
				ins->arg[0] = (long)fast->argname[i+1];
				ins->arg[1] = (long)fast->argname[i+2];
				gen = 1;
			}
		}

		/* var/ptr */
		else if ((ins->code == X_LDW_S) || (ins->code == X_LDB_S) || ins->code == X_LDUB_S) {
			/* get symbol */
			sym = ins->sym;

			/* check type */
			if (sym) {
				if (sym->ident == POINTER)
					gen = 1;
				else if (sym->ident == VARIABLE) {
					if (ins->code == X_LDW_S)
						ins->code  = X_LDD_S_W;
					else
						ins->code  = X_LDD_S_B;
	
					ins->data  -= adj;
					ins->arg[0] = (long)fast->argname[i+1];
					ins->arg[1] = (long)fast->argname[i+2];
					gen = 1;
				}
			}
		}

		/* array */
		else if (ins->code == X_LEA_S) {
			sym = ins->sym;

			if (sym && (sym->ident == ARRAY)) {
				ins->data -= adj;
				gen = 1;
			}
		}

		/* array */
		else if((ins->code == I_LDWI) && (ins->type == T_SYMBOL)) {
			/* get symbol */
			sym = (SYMBOL *)ins->data;

			/* check type */
			if (sym->ident == ARRAY)
				gen = 1;
		}
	}
	else if (nb == 2) {
		/* array */
		if((ins->code == I_LDWI) && (ins->type == T_SYMBOL)) {
			/* get symbol */
			sym = (SYMBOL *)ins->data;

			/* check type */
			if (sym->ident == ARRAY) {
				ptr =  ins;
				ins = &ins_stack[idx+1];

				if((ins->code == I_ADDWI) && (ins->type == T_VALUE)) {
					gen_ins(ptr);
					gen = 1;
				}
			}
		}
	}

	/* gen code */
	if (gen) {
		gen_ins(ins);
		err = 0;

		if (strcmp(fast->argname[i], "#acc") != 0) {
			tmp.code = I_STW;
			tmp.type = T_SYMBOL;
			tmp.data = (long)fast->argname[i];
			gen_ins(&tmp);
		}
	}

	/* errors */
	if (err) {
		if (optimize < 1)
			error("dword arg support works only with optimization enabled");
		else
			error("invalid or too complex dword arg syntax");
	}
}

