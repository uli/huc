/*	File main.c: 2.7 (84/11/28,10:14:56)
 *
 * PC Engine C Compiler
 * Made by <unknown guy>, hacked to work on Pc Engine by David Michel
 * resumed work by Zeograd
 *
 * 00/02/22 : added oldargv variable to show real exe name in usage function
 */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "const.h"
#include "error.h"
#include "function.h"
#include "gen.h"
#include "io.h"
#include "lex.h"
#include "main.h"
#include "optimize.h"
#include "pragma.h"
#include "preproc.h"
#include "pseudo.h"
#include "sym.h"

int main (int argc,char* argv[])
{
	char	*p,*bp;
	char** oldargv = argv;
	long smacptr;
	macptr = 0;
	ctext = 0;
	argc--; argv++;
	errs = 0;
	sflag = 0;
	cdflag = 0;
	verboseflag = 0;
	startup_incl = 0;
	optimize = 2;	/* -O2 by default */
	overlayflag = 0;
	asmdefs[0] = '\0';

	while ( (p = *argv++) )
		if (*p == '-') while (*++p)
			switch(*p) {
				case 't': case 'T':
					ctext = 1;
					break;

				case 'c':
					if ( (*(p+1) == 'd') ) {
						cdflag = 1;	/* pass '-cd' to assembler */
						p++;
						break;
					} else {
						usage(oldargv[0]);
						break;
					}

				case 's':
					if (strncmp(p, "scd", 3) == 0) {
						cdflag = 2;	/* pass '-scd' to assembler */
						p += 2;
						break;
					}
				case 'S':
					sflag = 1;
					break;

				/* defines to pass to assembler */
				case 'a': case 'A':
					bp = ++p;
					if (!*p) usage(oldargv[0]);
					while (*p && *p != '=') p++;
					strncat(asmdefs, bp, (p-bp));
/*					if (*p == '=') *p = '\t'; */
					bp = ++p;
					strcat(asmdefs, "\t= ");
					if (*bp == '\0') {
						strcat(asmdefs, "1\n");
					} else {
						strcat(asmdefs, bp);
						strcat(asmdefs, "\n");
					}
					break;


				case 'v':
					verboseflag = 1;
					ctext = 1;		/* "C" code in asm output */
					break;
					
				case 'd': case 'D':
					bp = ++p;
					if (!*p) usage(oldargv[0]);
					while (*p && *p != '=') p++;
					if (*p == '=') *p = '\t';
					while (*p) p++;
					p--;
					defmac(bp);
					break;

				case 'o':
					if (strncmp(p, "over", 4) == 0) {
						overlayflag = 1;
						if (strncmp(p, "overlay", 7) == 0)
							p += 6;
						else
							p += 3;
						break;
					}
				case 'O':
					/* David, made -O equal to -O2
					 * I'm too lazy to tape -O2 each time :)
					 */
					if (!p[1]) optimize = 2;
					else       optimize = atoi(++p);
					break;

				default:
					usage(oldargv[0]);
			}
			else break;

	smacptr = macptr;
	if (!p)
		usage(oldargv[0]);
	printf(HUC_VERSION);
	printf("\n");
	init_path();
	while (p) {
		errfile = 0;
		if (extension(p) == 'c') {
			glbptr = STARTGLB;
			locptr = STARTLOC;
			wsptr = ws;
			inclsp =
			iflevel =
			skiplevel =
			swstp =
			litptr =
			stkp =
			errcnt =
			ncmp =
			lastst =
			quote[1] =
			line_number = 0;
			macptr = smacptr;
			input2 = NULL;
			quote[0] = '"';
			cmode = 1;
			glbflag = 1;
			nxtlab = 0;
			litlab = getlabel ();
			defmac("end\tmemory");
			addglb("memory", ARRAY, CCHAR, 0, EXTERN);
			addglb("stack", ARRAY, CCHAR, 0, EXTERN);
			rglbptr = glbptr;
			addglb ("etext", ARRAY, CCHAR, 0, EXTERN);
			addglb ("edata", ARRAY, CCHAR, 0, EXTERN);
			/* PCE specific externs */
			addglb ("font_base", VARIABLE, CINT, 0, EXTERN);
			addglb_far("vdc", CINT);
			addglb_far("vram", CCHAR);
			/* end specific externs */
			defpragma();
			defmac("short\tint");
			defmac("huc6280\t1");
			defmac("huc\t1");

			if (cdflag == 1)
				defmac("_CD\t1");
			else if (cdflag == 2)
				defmac("_SCD\t1");
			else
				defmac("_ROM\t1");

			if (overlayflag == 1)
				defmac("_OVERLAY\t1");

//			initmac();
			/*
			 *	compiler body
			 */
			if (!openin (p))
				exit(1);
			if (!openout ())
				exit(1);
			header();
			asmdefines();
//			gtext ();
			parse ();
			fclose (input);
//			gdata ();
			dumplits ();
			dumpglbs ();
			errorsummary ();
//			trailer ();
			fclose (output);
			pl ("");
			errs = errs || errfile;
#ifndef	NOASLD
		}
		if (!errfile && !sflag) {
				errs = errs || assemble(p);
		}
#else
		} else {
			fputs("Don't understand file ", stderr);
			fputs(p, stderr);
			errs = 1;
		}
#endif
		p = *argv++;
	}
// David, removed because link() doesn't exist
//
//#ifndef	NOASLD
//	if (!errs && !sflag && !cflag)
//		errs = errs || link();
//#endif
	exit(errs != 0);
}

void FEvers(void )
{
	outstr("\tFront End (2.7,84/11/28)");
}

void usage(char* exename)
{
	fprintf(stderr,HUC_VERSION);
	fprintf(stderr,"\n\n");
	fprintf(stderr,"USAGE: %s [-options] infile\n\n", exename);
	fprintf(stderr,"Options to compile step:\n");
	fprintf(stderr,"-s/-S       : create asm output only (do not invoke assembler)\n");
	fprintf(stderr,"-t/-T       : include text of 'C' in asm output/listings\n");
	fprintf(stderr,"-Dsym[=val] : define symbol 'sym' when compiling\n");
	fprintf(stderr,"-O[val]     : invoke optimization (level <value>)\n");
	fprintf(stderr,"-cd/-scd    : create CDROM output\n");
	fprintf(stderr,"-over(lay)  : create CDROM overlay section\n\n");
	fprintf(stderr,"Options to assembler step:\n");
	fprintf(stderr,"-Asym[=val] : define symbol 'sym' to assembler\n");
	fprintf(stderr,"-v/-V       : verbose - maximum information in output files\n\n");
	exit(1);
}

/*
 *	process all input text
 *
 *	at this level, only static declarations, defines, includes,
 *	and function definitions are legal.
 *
 */
void parse (void )
{
	while (!feof (input)) {

// Note:
// At beginning of 'parse' call, the header has been output to '.s'
// file, as well as all the -Asym=val operands from command line.
//
// But initial '#include "startup.asm"' statement was not yet output
// (allowing for additional asm define's to be parsed and output first.
//
// We can parse some trivial tokens first (including the asmdef...),
// but the #include "startup.asm" line must be output before actual code
// (And only once...)
//
// This should clear confusion (the 'parse' code is otherwise trivial)
//
		if (amatch ("extern", 6))
		{
			if (!startup_incl) {
				inc_startup();
				unget_line();
				incl_globals();
			} else {
				dodcls(EXTERN);
			}
		}
		else if (amatch ("static",6))
		{
			if (!startup_incl) {
				inc_startup();
				unget_line();
				incl_globals();
			} else {
				dodcls(STATIC);
			}
		}
		else if (amatch ("const",5))
		{
			if (!startup_incl) {
				inc_startup();
				unget_line();
				incl_globals();
			} else {
				dodcls(CONST);
			}
		}
		else if (dodcls(PUBLIC)) ;
		else if (match ("#asmdef"))
			doasmdef ();
		else if (match ("#asm"))
		{
			if (!startup_incl) {
				inc_startup();
				unget_line();
				incl_globals();
			} else {
				doasm ();
			}
		}
		else if (match ("#include"))
		{
			if (!startup_incl) {
				inc_startup();
				unget_line();
				incl_globals();
			} else {
				doinclude ();
			}
		}
		else if (match ("#define"))
			dodefine();
		else if (match ("#undef"))
			doundef();
		else if (match ("#pragma"))
			dopragma();
		else if (match("#inc"))
		{
			if (!startup_incl) {
				inc_startup();
				unget_line();
				incl_globals();
			} else {
				dopsdinc();
			}
		}
		else if (match("#def"))
		{
			if (!startup_incl) {
				inc_startup();
				unget_line();
				incl_globals();
			} else {
				dopsddef();
			}
		}
		else
		{
			if (!startup_incl) {
				inc_startup();
				unget_line();
				incl_globals();
			} else {
				newfunc (NULL);
			}
		}
		blanks ();
	}
	if (optimize)
		flush_ins();
}

/*
 *		parse top level declarations
 */

long dodcls(long stclass)
{
	long err;

	blanks();
	if (amatch("char", 4))
		err = declglb(CCHAR, stclass);
	else if (amatch("int", 3))
		err = declglb(CINT, stclass);
	else if (stclass == PUBLIC)
		return(0);
	else
		err = declglb(CINT, stclass);

	if (err == 2) /* function */
		return 1;
	else if (err)
		kill ();
	else
		ns ();
	return(1);
}


/*
 *	dump the literal pool
 */
void dumplits (void )
{
	long	j, k;

	if ((litptr == 0) && (const_nb == 0))
		return;
	outstr("\t.data\n");
	outstr("\t.bank CONST_BANK\n\t.org $4000\n");
	if (litptr) {
		outlabel (litlab);
		col ();
		k = 0;
		while (k < litptr) {
			defbyte ();
			j = 8;
			while (j--) {
				outdec (litq[k++] & 0xFF);
				if ((j == 0) | (k >= litptr)) {
					nl ();
					break;
				}
				outbyte (',');
			}
		}
	}
	if (const_nb)
		dump_const ();
}

/*
 *	dump all static variables
 */
void dumpglbs (void )
{
	long i = 1;
	long	j;

	if (glbflag) {
		cptr = rglbptr;
		while (cptr < glbptr) {
			if (cptr[IDENT] != FUNCTION) {
//				ppubext(cptr);
				if ((cptr[STORAGE] & WRITTEN) == 0) { /* Not yet written to file */
					if (cptr[STORAGE] != EXTERN) {
						if (i) {
							i = 0;
							nl();
							gdata();
						}
						prefix ();
						outstr (cptr);
						outstr (":\t");
						defstorage ();
						j = glint(cptr);
						if ((cptr[TYPE] == CINT) ||
								(cptr[IDENT] == POINTER))
							j = j * INTSIZE;
						outdec (j);
						nl ();
						cptr[STORAGE] |= WRITTEN;
					}
				}
			} else {
//				fpubext(cptr);
			}
			cptr = cptr + SYMSIZ;
		}
	}
	if (i) {
		nl();
		gdata();
	}
	if (globals_h_in_process != 1) {
		outstr("__arg:\n");
	}
}

/*
 *	report errors
 */
void errorsummary (void )
{
	if (ncmp)
		error ("missing closing bracket");
	nl ();
	comment ();
	outdec (errcnt);
	if (errcnt) errfile = YES;
	outstr (" error(s) in compilation");
	nl ();
	comment();
	ot("literal pool:");
	outdec(litptr);
	nl();
	comment();
	ot("constant pool:");
	outdec(const_size);
	nl();
	comment();
	ot("global pool:");
	outdec(glbptr-rglbptr);
	nl();
	comment();
	ot("Macro pool:");
	outdec(macptr);
	nl();
	pl (errcnt ? "Error(s)" : "No errors");
}

char extension(char *s)
{
	s += strlen(s) - 2;
	if (*s == '.')
		return(*(s+1));
	return(' ');
}

long assemble(char *s)
{
	char buf[100];
	char *exe;
	char *opts[10];
	long i;
//	long j;

	i = 0;


#ifdef DJGPP
	exe = "pceas.exe";
	opts[i++] = "pceas.exe";
#elif defined(linux) || defined(unix)
	exe = "pceas";
	opts[i++] = "pceas";
#elif defined(WIN32)
	exe = "pceas.exe";
	opts[i++] = "pceas.exe";
#else
  #error Add calling sequence depending on your OS
#endif
	switch (cdflag) {
		case 1:
			opts[i++] = "-cd";
			break;

		case 2:
			opts[i++] = "-scd";
			break;

		default:
			break;
	}

	if (overlayflag) {
		opts[i++] = "-over";	/* compile as overlay */
	}

	if (verboseflag) {
		opts[i++] = "-S";	/* asm: display full segment map */
		opts[i++] = "-l 3";	/* top listing output */
		opts[i++] = "-m";	/* force macros also */
	}

	strcpy(buf, s);
	buf[strlen(buf)-1] = 's';
	opts[i++] = buf;

	opts[i++] = 0;

// Comment this out later...

//	printf("invoking pceas:\n");
//	for (j = 0; j < i; j++) {
//		printf("arg[%d] = %s\n", j, opts[j]);
//	}
// .....
#if defined(WIN32)
	return (execvp(exe, (const char* const*)opts));
#else
	return(execvp(exe, (char* const*)opts));
#endif
}
