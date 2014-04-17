/*	File main.c: 2.7 (84/11/28,10:14:56)
 *
 * PC Engine C Compiler
 * based on Small C Compiler by Ron Cain, James E. Hendrix, and others
 * hacked to work on PC Engine by David Michel
 * work resumed by Zeograd
 * work resumed again by Ulrich Hecht
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
#include <ctype.h>
#include <sys/stat.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "const.h"
#include "enum.h"
#include "error.h"
#include "function.h"
#include "gen.h"
#include "io.h"
#include "lex.h"
#include "main.h"
#include "optimize.h"
#include "pragma.h"
#include "preproc.h"
#include "primary.h"
#include "pseudo.h"
#include "sym.h"
#include "struct.h"

static char **link_libs = 0;
static int link_lib_ptr;
static char **infiles = 0;
static int infile_ptr;

static int user_norecurse = 0;

static char *lib_to_file(char *lib)
{
	int i;
	static char libfile[FILENAMESIZE];
	char **incd = include_dirs();
	for (i = 0; incd[i]; i++) {
		struct stat st;
		sprintf(libfile, "%s/%s.c", incd[i], lib);
		if (!stat(libfile, &st))
			return libfile;
	}
	return 0;
}
static void dumparg(void);

int main (int argc,char* argv[])
{
	char	*p,*pp,*bp;
	char** oldargv = argv;
	char **link_lib;
	long smacptr;
	int first = 1;
	char *asmdefs_global_end;

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

	while ( (p = *argv++) ) {
		if (*p == '-') {
			while (*++p) switch(*p) {
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
					verboseflag++;
					if (verboseflag > 1)
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
					}
					else {
						bp = ++p;
						while (*p && *p != ' ' && *p != '\t')
							p++;
						memcpy(user_outfile, bp, p-bp);
						user_outfile[p - bp] = 0;
						p--;
					}
					break;
				case 'O':
					/* David, made -O equal to -O2
					 * I'm too lazy to tape -O2 each time :)
					 */
					if (!p[1]) optimize = 2;
					else       optimize = atoi(++p);
					break;

				case 'f':
					p++;
					if (!strcmp(p, "no-recursive")) {
						user_norecurse = 1;
						p += 11;
					}
					else if (!strcmp(p, "recursive")) {
						user_norecurse = 0;
						p += 8;
					}
					else if (!strcmp(p, "no-short-enums")) {
						user_short_enums = 0;
						p += 13;
					}
					else if (!strcmp(p, "short-enums")) {
						user_short_enums = 1;
						p += 10;
					}
					else
						goto unknown_option;
					break;

				case 'l':
					bp = ++p;
					while (*p && *p != ' ' && *p != '\t')
						p++;
					link_libs = realloc(link_libs, (link_lib_ptr + 2) * sizeof(*link_libs));
					link_libs[link_lib_ptr] = malloc(p - bp + 1);
					memcpy(link_libs[link_lib_ptr], bp, p - bp);
					link_libs[link_lib_ptr][p - bp] = 0;
					strcat(asmdefs, "LINK_");
					strcat(asmdefs, link_libs[link_lib_ptr]);
					strcat(asmdefs, "\t= 1\n");
					link_libs[++link_lib_ptr] = 0;
					p--;
					break;

				case 'm':
					if (!strcmp(p+1, "small")) {
						strcat(asmdefs, "SMALL\t= 1\n");
						p += 5;
					}
					else {
unknown_option:
						fprintf(stderr, "unknown option %s\n", p);
						exit(1);
					}
					break;

				default:
					usage(oldargv[0]);
			}
		}
		else {
			infiles = realloc(infiles, (infile_ptr + 2) * sizeof(*infiles));
			infiles[infile_ptr++] = p;
			infiles[infile_ptr] = 0;
		}
	}

	smacptr = macptr;
	if (!infiles)
		usage(oldargv[0]);
	printf(HUC_VERSION);
	printf("\n");
	init_path();
	/* Remember the first file, it will be used as the base for the
	   output file name unless there is a user-specified outfile. */
	p = pp = infiles[0];
	/* Labels count is not reset for each file because labels are
	   global and conflicts would arise. */
	nxtlab = 0;
	defpragma();
	link_lib = link_libs;
	infile_ptr = 1;
	/* Remember where the global assembler defines end so we can
	   reset to that point for each file. */
	/* XXX: Even if we don't repeat the local asm defines, they
	   are still defined because we compile everything into one
	   assembly file. */
	asmdefs_global_end = asmdefs + strlen(asmdefs);
	while (p) {
		errfile = 0;
		/* Truncate asm defines to the point where global
		   defines end. */
		asmdefs_global_end[0] = 0;
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
			const_nb =
			line_number = 0;
			macptr = smacptr;
			input2 = NULL;
			quote[0] = '"';
			cmode = 1;
			glbflag = 1;
			litlab = getlabel ();
			member_table_index = 0;
			tag_table_index = 0;
			norecurse = user_norecurse;
			typedef_ptr = 0;
			enum_ptr = 0;
			enum_type_ptr = 0;

			/* Macros and globals have to be reset for each
			   file, so we have to define the defaults all over
			   each time. */
			defmac("__end\t__memory");
			addglb("__memory", ARRAY, CCHAR, 0, EXTERN);
			addglb("stack", ARRAY, CCHAR, 0, EXTERN);
			rglbptr = glbptr;
			addglb ("etext", ARRAY, CCHAR, 0, EXTERN);
			addglb ("edata", ARRAY, CCHAR, 0, EXTERN);
			/* PCE specific externs */
			addglb ("font_base", VARIABLE, CINT, 0, EXTERN);
			addglb_far("vdc", CINT);
			addglb_far("vram", CCHAR);
			/* end specific externs */
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
			if (first && !openout ())
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
			pl ("");
			errs = errs || errfile;
		} else {
			fputs("Don't understand file ", stderr);
			fputs(p, stderr);
			fputc('\n', stderr);
			exit(1);
		}
		p = infiles[infile_ptr];
		if (!p && link_lib && *link_lib) {
			/* No more command-line files, continue with
			   libraries. */
			p = lib_to_file(*link_lib);
			if (!p) {
				fprintf(stderr, "cannot find library %s\n", *link_lib);
				exit(1);
			}
			link_lib++;
		}
		else
			infile_ptr++;
		first = 0;
	}
	dumparg();
	fclose(output);
	if (!errs && !sflag) {
		if (user_outfile[0])
			errs = errs || assemble(user_outfile);
		else
			errs = errs || assemble(pp);
	}
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
	while (1) {
		blanks();
		if (feof(input))
			break;
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
		if (amatch("#asmdef", 7)) {
			doasmdef ();
			continue;
		}
		
		if (!startup_incl) {
			inc_startup();
			incl_globals();
		}

		if (amatch ("extern", 6))
			dodcls(EXTERN, NULL_TAG, 0);
		else if (amatch ("static",6)) {
			if (amatch("const", 5)) {
				/* XXX: what about the static part? */
				dodcls(CONST, NULL_TAG, 0);
			}
			else
				dodcls(STATIC, NULL_TAG, 0);
		}
		else if (amatch ("const",5))
			dodcls(CONST, NULL_TAG, 0);
		else if (amatch("typedef", 7))
			dotypedef();
		else if (dodcls(PUBLIC, NULL_TAG, 0)) ;
		else if (match ("#asm"))
			doasm ();
		else if (match ("#include"))
			doinclude ();
		else if (match("#inc"))
			dopsdinc();
		else if (match("#def"))
			dopsddef();
		else
			newfunc (NULL, 0, 0, 0);
	}
	if (optimize)
		flush_ins();
}

/*
 *		parse top level declarations
 */

long dodcls(long stclass, TAG_SYMBOL *mtag, int is_struct)
{
	long err;
	struct type t;

	blanks();

	if (match_type(&t, NO, YES)) {
		if (t.type == CSTRUCT && t.otag == -1)
			t.otag = define_struct(t.sname, stclass, !!(t.flags & F_STRUCT));
		else if (t.type == CENUM) {
			if (t.otag == -1)
				t.otag = define_enum(t.sname, stclass);
			t.type = enum_types[t.otag].base;
		}
		err = declglb(t.type, stclass, mtag, t.otag, is_struct);
	}
	else if (stclass == PUBLIC)
		return(0);
	else
		err = declglb(CINT, stclass, mtag, NULL_TAG, is_struct);

	if (err == 2) /* function */
		return 1;
	else if (err) {
		kill ();
		return 0;
	}
	else
		ns ();
	return(1);
}

void dotypedef(void)
{
	struct type t;
	if (!match_type(&t, YES, NO)) {
		error("unknown type");
		kill();
		return;
	}
	if (t.type == CENUM) {
		if (t.otag == -1) {
			if (user_short_enums)
				warning(W_GENERAL,
					"typedef to undefined enum; "
					"assuming base type int");
			t.type = CINT;
		}
		else
			t.type = enum_types[t.otag].base;
	}
	if (!symname(t.sname)) {
		error("invalid type name");
		kill();
		return;
	}
	typedefs = realloc(typedefs, (typedef_ptr + 1) * sizeof(struct type));
	typedefs[typedef_ptr++] = t;
	ns();
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

	if (glbflag) {
		cptr = rglbptr;
		while (cptr < glbptr) {
			if (cptr->ident != FUNCTION) {
//				ppubext(cptr);
				if ((cptr->storage & WRITTEN) == 0) { /* Not yet written to file */
					if (cptr->storage != EXTERN) {
						if (i) {
							i = 0;
							nl();
							gdata();
						}
						prefix ();
						outstr (cptr->name);
						outstr (":\t");
						defstorage ();
						outdec (cptr->size);
						nl ();
						cptr->storage |= WRITTEN;
					}
				}
			} else {
//				fpubext(cptr);
			}
			cptr++;
		}
	}
	if (i) {
		nl();
		gdata();
	}
}

static void dumparg(void)
{
	if (globals_h_in_process != 1) {
		outstr("__heap_start:\n");
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
		if (verboseflag > 1) {
			opts[i++] = "-l 3";	/* top listing output */
			opts[i++] = "-m";	/* force macros also */
		}
		else
			opts[i++] = "-l 0";
	}
	else
		opts[i++] = "-l 0";

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
