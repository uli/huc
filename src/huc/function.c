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
#include "primary.h"
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
void newfunc (const char *sname, int ret_ptr_order, int ret_type, int ret_otag)
{
	char n[NAMESIZE];
	SYMBOL *ptr;
	long  nbarg;
	int is_irq_handler = 0;
	int is_firq_handler = 0;

	if (sname) {
		strcpy(current_fn, sname);
		strcpy(n, sname);
	}
	else {
		/* No explicit return type. */
		ret_type = CINT;
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
		struct type t;
		if (match_type(&t, NO, NO)) {
			if (t.type == CVOID) {
				if (match(")"))
					break;
			}
			getarg(t.type, ANSI, t.otag);
			nbarg++;
		}
		else {
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
		blanks ();
		if (!streq (line + lptr, ")")) {
			if (!match (","))
				error ("expected comma");
		}
		if (endst ())
			break;
	}

	if (amatch("__firq", 6))
		is_firq_handler = 1;
	else if (amatch("__irq", 5))
		is_irq_handler = 1;

	/* ignore function prototypes */
	if (match(";"))
		return;

	stkp = 0;
	clabel_ptr = 0;
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
			struct type t;
			if (match_type(&t, NO, NO)) {
				getarg(t.type, KR, t.otag);
				ns();
			}
			else {
				error ("wrong number args");
				break;
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
		ptr = addglb (current_fn, FUNCTION, ret_type, FUNCTION, PUBLIC, 0);
	ptr->ptr_order = ret_ptr_order;
	ptr->tagidx = ret_otag;

	flush_ins(); /* David, .proc directive support */
	gtext();
	if (is_firq_handler) {
		ol(".bank LIB1_BANK");
		prefix(); outstr(current_fn); outstr(":"); nl();
	}
	else if (is_irq_handler) {
		ol(".bank LIB1_BANK");
		prefix(); outstr(current_fn); outstr(":"); nl();
		ot("maplibfunc\t"); outstr(current_fn); nl();
		ol("rts");
		ot(".proc "); outstr(current_fn); nl();
	}
	else {
		ot(".proc ");
		prefix ();
		outstr (current_fn);
		nl();
	}

	if (nbarg)      /* David, arg optimization */
		gpusharg (0);

	/* When using fixed-address locals, locals_ptr is used to
	   keep track of their memory offset instead of stkp, so
	   we have to reset it before producing code. */
	if (norecurse)
		locals_ptr = 0;

	statement(YES);
	gnlabel(fexitlab);
	modstk (nbarg * INTSIZE);
	gtext();
	gret (); /* generate the return statement */
	flush_ins();    /* David, optimize.c related */
	if (!is_firq_handler)
		ol (".endp");   /* David, .endp directive support */

	/* Add space for fixed-address locals to .bss section. */
	if (norecurse && locals_ptr < 0) {
		ot(".data"); nl();
		ot(".bss"); nl();
		ot("__"); outstr(current_fn); outstr("_loc: .ds ");
		outdec(-locals_ptr); nl();
		ot("__"); outstr(current_fn); outstr("_lend:"); nl();
		ot(".code"); nl();
	}

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
#define SPILLB(a) { \
	spilled_args[sparg_idx] = (a); \
	spilled_arg_sizes[sparg_idx++] = 1; \
	out_ins(I_SAVEB, 0, 0); \
}

#define SPILLW(a) { \
	spilled_args[sparg_idx] = (a); \
	spilled_arg_sizes[sparg_idx++] = 2; \
	out_ins(I_SAVEW, 0, 0); \
}

void callfunction (char *ptr)
{
	extern char *new_string(long, char *);
	struct fastcall *fast;
	long call_stack_ref;
	long i, j;
	long nb;
	long adj;
	long	nargs;
	long cnt;
	int max_fc_arg = 0;	/* highest arg with a fastcall inside */
	/* args spilled to the native stack */
	const char *spilled_args[MAX_FASTCALL_ARGS];
	/* byte sizes of spilled args */
	int spilled_arg_sizes[MAX_FASTCALL_ARGS];
	int sparg_idx = 0;	/* index into spilled_args[] */
	int uses_acc = 0;	/* does callee use acc? */

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

	if (nb)
		flush_ins();

	/* get args */
	while (!streq (line + lptr, ")")) {
		if (endst ())
			break;
		/* fastcall func */
		if (nb) {
			int nfc = func_call_stack;

			if (nargs)
				stkp = stkp - INTSIZE;
			arg_stack(arg_idx++);
			expression (NO);
			flush_ins();

			/* Check if we had a fastcall in our argument. */
			if (nfc < func_call_stack) {
				/* Remember the last argument with an FC. */
				if (max_fc_arg < arg_idx - 1)
					max_fc_arg = arg_idx - 1;
			}
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
			/* fastcall */
			for (i = 0, j = 0, adj = 0; i < cnt; i++) {
				/* flush arg stack (except for farptr and dword args) */
				if ((fast->argtype[j] != 0x03) &&
					(fast->argtype[j] != 0x04))
				{
					arg_flush(arg_idx + i, adj);
				}
	
				/* Either store the argument in its designated
				   location, or save it on the (native) stack
				   if there is another fastcall ahead. */
				switch (fast->argtype[j]) {
				case 0x01: /* byte */
					if (i < max_fc_arg)
						SPILLB(fast->argname[j])
					else
						out_ins(I_STB, T_SYMBOL, (long)fast->argname[j]);
					break;
				case 0x02: /* word */
					if (i < max_fc_arg)
						SPILLW(fast->argname[j])
					else
						out_ins(I_STW, T_SYMBOL, (long)fast->argname[j]);
					break;
				case 0x03: /* farptr */
					arg_to_fptr(fast, j,  arg_idx + i, adj);
					if (i < max_fc_arg) {
						out_ins(I_LDUB, T_SYMBOL, (long)fast->argname[j]);
						SPILLB(fast->argname[j])
						out_ins(I_LDW, T_SYMBOL, (long)fast->argname[j+1]);
						SPILLW(fast->argname[j+1])
					}
					j += 1;
					break;
				case 0x04: /* dword */
					arg_to_dword(fast, j, arg_idx + i, adj);
					if (i < max_fc_arg) {
						out_ins(I_LDW, T_SYMBOL, (long)fast->argname[j]);
						SPILLW(fast->argname[j])
						out_ins(I_LDW, T_SYMBOL, (long)fast->argname[j+1]);
						SPILLW(fast->argname[j+1])
						out_ins(I_LDW, T_SYMBOL, (long)fast->argname[j+2]);
						SPILLW(fast->argname[j+2])
					}
					j += 2;
					break;
				case 0x00: /* acc */
					if (i < max_fc_arg)
						SPILLW(0)
					uses_acc = 1;
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
	/* Reload fastcall arguments spilled to the native stack. */
	if (sparg_idx) {
		/* Reloading corrupts acc, so we need to save it if it
		   is used by the callee. */
		if (uses_acc)
			out_ins(I_STW, T_LITERAL, (long)"<__temp");

		for (i = sparg_idx-1; i>-1; i--) {
			if (spilled_arg_sizes[i] == 1) {
				out_ins(I_RESB, 0, 0);
				out_ins(I_STB, T_SYMBOL, (long)spilled_args[i]);
			}
			else {
				out_ins(I_RESW, 0, 0);
				if (spilled_args[i])
					out_ins(I_STW, T_SYMBOL, (long)spilled_args[i]);
			}
		}

		if (uses_acc)
			out_ins(I_LDW, T_LITERAL, (long)"<__temp");
	}

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
			case X_ADDB_S:
			case X_ADDUB_S:
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

