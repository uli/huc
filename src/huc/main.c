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
#include "initials.h"
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
static void dumpfinal(void);

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
					else if (strncmp(p, "sgx", 3) == 0) {
						strcat(asmdefs, "_SGX = 1\n");
						defmac("_SGX");
						p += 2;
						break;
					}
					/* fallthrough */
				case 'S':
					sflag = 1;
					break;

				/* defines to pass to assembler */
				case 'a':
					if (strncmp(p, "acd", 3) == 0) {
						cdflag = 2;	/* pass '-scd' to assembler */
						strcat(asmdefs, "_AC = 1\n");
						defmac("_AC");
						p += 2;
						break;
					}
					/* fallthrough */
				case 'A':
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
		if (extension(p) == 'c' || extension(p) == 'C') {
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
			memset(member_table, 0, sizeof(member_table));
			tag_table_index = 0;
			norecurse = user_norecurse;
			typedef_ptr = 0;
			enum_ptr = 0;
			enum_type_ptr = 0;
			memset(fastcall_tbl, 0, sizeof(fastcall_tbl));
			defpragma();

			/* Macros and globals have to be reset for each
			   file, so we have to define the defaults all over
			   each time. */
			defmac("__end\t__memory");
			addglb("__memory", ARRAY, CCHAR, 0, EXTERN, 0);
			addglb("stack", ARRAY, CCHAR, 0, EXTERN, 0);
			rglbptr = glbptr;
			addglb ("etext", ARRAY, CCHAR, 0, EXTERN, 0);
			addglb ("edata", ARRAY, CCHAR, 0, EXTERN, 0);
			/* PCE specific externs */
			addglb ("font_base", VARIABLE, CINT, 0, EXTERN, 0);
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
			if (first)
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
	dumpfinal();
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
	if (!startup_incl) {
		inc_startup();
		incl_globals();
	}

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
	outstr("\t.bank CONST_BANK\n");
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

/**
 * dump struct data
 * @param symbol struct variable
 * @param position position of the struct in the array, or zero
 */
int dump_struct(SYMBOL *symbol, int position) {
	int dumped_bytes = 0;
	int i, number_of_members, value;
	number_of_members = tag_table[symbol->tagidx].number_of_members;
	for (i=0; i<number_of_members; i++) {
		// i is the index of current member, get type
		int member_type = member_table[tag_table[symbol->tagidx].member_idx + i].type;
		if (member_type == CCHAR || member_type == CUCHAR) {
			defbyte();
			dumped_bytes += 1;
		} else {
			/* XXX: compound types? */
			defword();
			dumped_bytes += 2;
		}
		if (position < get_size(symbol->name)) {
			// dump data
			value = get_item_at(symbol->name, position*number_of_members+i, &tag_table[symbol->tagidx]);
			outdec(value);
		} else {
			// dump zero, no more data available
			outdec(0);
		}
		nl();
	}
	return dumped_bytes;
}

static int have_init_data = 0;
/* Initialized data must be kept in one contiguous block; pceas does not
   provide segments for that, so we keep the definitions and data in
   separate buffers and dump them all together after the last input file. 
 */
#define DATABUFSIZE 65536
static FILE *data = 0;
char data_buf[DATABUFSIZE];
static FILE *rodata = 0;
char rodata_buf[DATABUFSIZE];

/*
 *	dump all static variables
 */
void dumpglbs (void )
{
	long i = 1;
	int dim, list_size, line_count;
	int j;
	FILE *save = output;
	if (!data)
		data = fmemopen(data_buf, DATABUFSIZE, "w");
	if (!rodata)
		rodata = fmemopen(rodata_buf, DATABUFSIZE, "w");

	/* This is done in several passes:
	   Pass 0: Dump initialization data into const bank.
	   Pass 1: Define space for uninitialized data.
	   Pass 2: Define space for initialized data.
	 */
	if (glbflag) {
		int pass = 0;
next:
		i = 1;
		for (cptr = rglbptr; cptr < glbptr; cptr++) {
			if (cptr->ident != FUNCTION) {
//				ppubext(cptr);
				if ((cptr->storage & WRITTEN) == 0 && /* Not yet written to file */
				    cptr->storage != EXTERN) {
					dim = cptr->offset;
					if (find_symbol_initials(cptr->name)) { // has initials
						/* dump initialization data */
						if (pass == 1)	/* initialized data not handled in pass 1 */
							continue;
						else if (pass == 2) {
							/* define space for initialized data */
							output = data;
							if (cptr->storage != LSTATIC)
								prefix ();
							outstr (cptr->name);
							outstr (":\t");
							defstorage ();
							outdec (cptr->size);
							nl ();
							cptr->storage |= WRITTEN;
							output = save;
							continue;
						}
						/* output initialization data into const bank */
						output = rodata;
						have_init_data = 1;
						list_size = 0;
						line_count = 0;
						list_size = get_size(cptr->name);
						if (cptr->type == CSTRUCT) {
							list_size /= tag_table[cptr->tagidx].number_of_members;
						}
						if (dim == -1) {
							dim = list_size;
						}
						int item;
						/* dim is an item count for non-compound types and a byte size
						   for compound types; dump_struct() wants an item number, so
						   we have to count both to get the right members out. */
						for (j = item = 0; j < dim; j++, item++) {
						    if (cptr->type == CSTRUCT) {
							j += dump_struct(cptr, item) - 1;
						    } else {
							if (line_count % 10 == 0) {
							    nl();
							    if (cptr->type == CCHAR || cptr->type == CUCHAR) {
								defbyte();
							    } else {
								defword();
							    }
							}
							if (j < list_size) {
							    // dump data
							    int value = get_item_at(cptr->name, j, &tag_table[cptr->tagidx]);
							    outdec(value);
							} else {
							    // dump zero, no more data available
							    outdec(0);
							}
							line_count++;
							if (line_count % 10 == 0) {
							    line_count = 0;
							} else {
							    if (j < dim-1) {
								outbyte( ',' );
							    }
							}
						    }
						}
						nl();
						output = save;
					}
					else {
						if (pass == 0)
							continue;
						/* define space in bss */
						if (i) {
							i = 0;
							nl();
							gdata();
						}
						if (cptr->storage != LSTATIC)
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
		}
		if (++pass < 3)
			goto next;
	}
	if (i) {
		nl();
		gdata();
	}
	output = save;
}

static void dumpfinal(void)
{
	int i;
	if (leaf_cnt) {
		outstr("leaf_loc: .ds ");
		outdec(leaf_size);
		nl();
		for (i = 0; i < leaf_cnt; i++) {
			outstr("__");
			outstr(leaf_functions[i]);
			outstr("_lend:\n");
		}
	}
	if (data) {
		fclose(data);
		outstr("huc_data:\n");
		outstr(data_buf);
		outstr("huc_data_end:\n");
	}
	if (globals_h_in_process != 1) {
		outstr("__heap_start:\n");
	}
	if (rodata) {
		fclose(rodata);
		ol(".data");
		ol(".bank CONST_BANK");
		outstr("huc_rodata:\n");
		outstr(rodata_buf);
		outstr("huc_rodata_end:\n");
	}
	fseek(output, output_globdef, SEEK_SET);
	if (have_irq_handler || have_sirq_handler)
		outstr("HAVE_IRQ = 1\n");
	if (have_sirq_handler)
		outstr("HAVE_SIRQ = 1\n");
	if (have_init_data)
		outstr("HAVE_INIT = 1\n");
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


	exe = getenv("PCE_PCEAS");
	if (!exe) {
#if defined(DJGPP) || defined(WIN32)
		exe = "pceas.exe";
#elif defined(linux) || defined(unix) || defined(osx)
		exe = "pceas";
#else
  #error Add calling sequence depending on your OS
#endif
	}
	opts[i++] = exe;
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
