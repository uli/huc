/*	File preproc.c: 2.3 (84/11/27,11:47:40) */
/*% cc -O -c %
 *
 */

//#define DEBUG_PREPROC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "code.h"
#include "defs.h"
#include "data.h"
#include "error.h"
#include "io.h"
#include "lex.h"
#include "optimize.h"
#include "pragma.h"
#include "preproc.h"
#include "primary.h"
#include "sym.h"

/* path separator */
#if defined(DJGPP) || defined(MSDOS) || defined(WIN32)
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STRING "\\"
#define DEFAULT_DIRS "c:\\huc\\include\\pce"
#else
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#define DEFAULT_DIRS "/usr/local/lib/huc/include/pce;" \
	"/usr/local/huc/include/pce;" \
	"/usr/local/share/huc/include/pce;" \
	"/usr/local/include/pce;" \
	"/usr/lib/huc/include/pce;" \
	"/usr/share/huc/include/pce;" \
	"/usr/include/pce"
#endif

/* locals */
static char *incpath[10];

static const char *include_path(void)
{
	const char *p;
	p = getenv("PCE_INCLUDE");
	if (!p)
		p = DEFAULT_DIRS;
	return p;
}

char **include_dirs(void)
{
	return incpath;
}

/*
 *  init the include paths
 */
void
init_path(void)
{
	const char *p,*pl;
	long	  i, l;

	p = include_path();

	for (i = 0; i < 10; i++) {
		pl = strchr(p, ';');

		if (pl == NULL)
			l = strlen(p);
		else
			l = pl - p;
		if (l) {
			incpath[i] = (char *)malloc(l + 2);
			strncpy(incpath[i], p, l);
			p += l;
			while (*p == ';')
				p++;
			incpath[i][l] = '\0';
			if (incpath[i][l - 1] != PATH_SEPARATOR)
				strcat(incpath[i],   PATH_SEPARATOR_STRING);
#ifdef DEBUG_PREPROC
			printf("incpath %s\n", incpath[i]);
#endif
		}
		else {
			if (incpath[i])
				free(incpath[i]);
			incpath[i] = 0;
		}
	}
}

/*
 *  open a file - browse paths
 */
FILE *
file_open(char *name, char *mode)
{
	FILE *fp = NULL;
	char  testname[256];
	long	  i;

	for (i = 0; i < 10; i++) {
		if (incpath[i] && strlen(incpath[i])) {
			strcpy(testname, incpath[i]);
			strcat(testname, name);
			strcpy(inclstk_name[inclsp], testname);
			fp = fopen(testname, mode);
			if (fp != NULL) break;
		}
	}

	return (fp);
}

/*
 *	open an include file
 */
void doinclude (void)
{
/*	char	*p; */
	FILE	*inp2;

	blanks ();
	inp2 = fixiname ();
	if (inp2)
	{
		if (inclsp < INCLSIZ) {
                        inclstk_line[inclsp] = line_number;
                        line_number = 0;
			inclstk[inclsp++] = input2;
			input2 = inp2;
		} else {
			fclose (inp2);
			error ("too many nested includes");
		}
	}
	else {
		error ("Could not open include file");
	}
	kill ();
}

void incl_globals(void)
{
	FILE	*inp2;

	/* open the globals.h file to include those variables */
	/* but if we can't open it, it's no problem */

	inp2 = fopen("globals.h", "r");

	if (inp2) {
		if (inclsp < INCLSIZ) {
			inclstk_line[inclsp] = line_number;
			line_number = 0;
			strcpy(inclstk_name[inclsp], "globals.h");
			inclstk[inclsp++] = input2;
			input2 = inp2;
			globals_h_in_process = 1;
		} else {
			fclose (inp2);
			error ("too many nested includes");
		}
	}
}


/*
 *	fixiname - remove "brackets" around include file name
 */
FILE* fixiname (void)
{
	char  c1, c2, *p, *ibp;
	char  buf[LINESIZE];
	char  buf2[LINESIZE*2];
	FILE *fp;

	ibp = &buf[0];
	c1 = gch ();
	c2 = (c1 == '"') ? '"' : '>';
	if  ((c1 != '"') && (c1 != '<')) {
		error ("incorrect file name delimiter");
		return (NULL);
	}
	for (p = line + lptr; *p;) {
		if (*p == c2)
			 break;
		if((*p == '\\') && (p[1] == '\\'))
			 p++;
		*ibp++ = *p++;
	}
	if (*p != c2) {
		error ("file name delimiter missing");
		return (NULL);
	}
	*ibp = 0;
	fp = NULL;
	strcpy(inclstk_name[inclsp], buf);
	if ((c1 == '<') || ((fp = fopen(buf, "r")) == NULL)) {
		if (strlen(DEFLIB)) {
			strcpy(buf2, DEFLIB);
			strcat(buf2, buf);
			strcpy(inclstk_name[inclsp], buf2);
			fp = fopen(buf2, "r");
		}
		if (fp == NULL)
			fp = file_open(buf, "r");
	}
	return(fp);
}

/*
 *	"asm" pseudo-statement
 *
 *	enters mode where assembly language statements are passed
 *	intact through parser
 *
 */
void doasm (void)
{
	flush_ins(); /* David - optimize.c related */
	cmode = 0;
	FOREVER {
		readline ();
		if (match ("#endasm"))
			break;
		if (feof (input))
			break;
		outstr (line);
		nl ();
	}
	kill ();
	cmode = 1;
}

void doasmdef (void)
{
char sname[100];
char sval[100];
long i = 0;

	symname(sname);
	while ( (ch () == ' ') || (ch () == 9) )
                gch ();

	while (ch()) {
		sval[i++] = ch();
		gch();
	}
	sval[i++] = '\0';

	outstr(sname);
	outstr("\t= ");
	outstr(sval);
	nl();
}

void dodefine (void)
{
	addmac();
}

void doundef (void)
{
	struct macro *mp;
	char	sname[NAMESIZE];

	if (!symname(sname)) {
		illname();
		kill();
		return;
	}

	mp = findmac(sname);
	if (mp)
		delmac(mp);
	kill();
}

void preprocess (void)
{
	if (ifline()) return;
	while (cpp(NO));
}

static int max_if_depth = 0;
static int *had_good_elif = 0;

static void bump_iflevel(void)
{
	++iflevel;
	if (iflevel >= max_if_depth) {
		max_if_depth = (max_if_depth + 1) * 2;
		had_good_elif = realloc(had_good_elif, max_if_depth * sizeof(int));
	}
}

void doifdef (long ifdef)
{
	char sname[NAMESIZE];
	long k;

	blanks();
	bump_iflevel();
	had_good_elif[iflevel] = 0;
	if (skiplevel) return;
	k = symname(sname) && findmac(sname);
	if (k != ifdef) skiplevel = iflevel;
}

static void doif (void)
{
	long num;
	blanks();
	bump_iflevel();
	had_good_elif[iflevel] = 0;
	if (skiplevel) return;
	lex_stop_at_eol = 1;
	const_expr(&num, NULL, NULL);
	lex_stop_at_eol = 0;
	if (!num)
		skiplevel = iflevel;
}

static void doelif (void)
{
	long num;
	blanks();
	if (skiplevel && skiplevel < iflevel)
		return;
	if (!skiplevel || had_good_elif[iflevel]) {
		/* previous section was good, so we are not */
		skiplevel = iflevel;
		return;
	}
	lex_stop_at_eol = 1;
	const_expr(&num, NULL, NULL);
	lex_stop_at_eol = 0;
	if (!num)
		skiplevel = iflevel;
	else {
		had_good_elif[iflevel] = 1;
		skiplevel = 0;
	}
}

long ifline(void)
{
	FOREVER {
		readline();
cont_no_read:
		if (!input || feof(input)) return(1);
		if (match("#ifdef")) {
			doifdef(YES);
			continue;
		} else if (match("#ifndef")) {
			doifdef(NO);
			continue;
		} else if (match("#if")) {
			/* need to preprocess the argument because it may
			   contain macros */
			cpp(YES);
			doif();
			/* const_expr() already read the next line */
			goto cont_no_read;
		} else if (match("#elif")) {
			/* need to preprocess the argument because it may
			   contain macros */
			cpp(YES);
			doelif();
			/* const_expr() already read the next line */
			goto cont_no_read;
		} else if (match("#else")) {
			if (iflevel) {
				if (skiplevel == iflevel && !had_good_elif[iflevel])
					skiplevel = 0;
				else if (skiplevel == 0)
					skiplevel = iflevel;
			} else noiferr();
			continue;
		} else if (match("#endif")) {
			if (iflevel) {
				if (skiplevel == iflevel) skiplevel = 0;
				--iflevel;
			} else noiferr();
			continue;
		} else if (!skiplevel) {
			if (match("#define")) {
				dodefine();
				continue;
			} else if (match("#undef")) {
				doundef();
				continue;
			} else if (match("#pragma")) {
				dopragma();
				continue;
			}
		}
		if (!skiplevel) return(0);
	}
}

/* 
 *           noiferr
 * Input : nothing
 * Output : nothing
 * 
 * Called when a #if statement is lacking
 * 
 */

void noiferr(void)
{
	error("no matching #if...");
}


long cpp (int subline)
{
	long	k;
	char	c, sname[NAMESIZE];
	long	tog;
	long	cpped;		/* non-zero if something expanded */
	long llptr;

	cpped = 0;
	/* don't expand lines with preprocessor commands in them */
	if (!subline && (!cmode || line[0] == '#')) {
		if (sstreq("#include"))
			return 0;
		/* except #inc/#def commands */
		if (!match("#inc") && !match("#def"))
			return(0);
	}

	mptr = 0;
	if (subline)
		llptr = lptr;	/* start wherever we are right now */
	else
		llptr = lptr = 0;	/* do the whole line */

	while (ch ()) {
		if ((ch () == ' ') | (ch () == 9)) {
			keepch (' ');
			while ((ch () == ' ') | (ch () == 9))
				gch ();
		} else if (ch () == '"') {
			keepch (ch ());
			gch ();
			while (ch () != '"') {
				if (ch () == 0) {
					error ("missing quote");
					break;
				}
				if (ch() == '\\') keepch(gch());
				keepch (gch ());
			}
			gch ();
			keepch ('"');
		} else if (ch () == '\'') {
			keepch ('\'');
			gch ();
			while (ch () != '\'') {
				if (ch () == 0) {
					error ("missing apostrophe");
					break;
				}
				if (ch() == '\\') keepch(gch());
				keepch (gch ());
			}
			gch ();
			keepch ('\'');
		} else if ((ch () == '/') & (nch () == '*')) {
			inchar ();
			inchar ();
			while ((((c = ch ()) == '*') & (nch () == '/')) == 0)
				if (c == '$') {
					inchar ();
					tog = TRUE;
					if (ch () == '-') {
						tog = FALSE;
						inchar ();
					}
					if (alpha (c = ch ())) {
						inchar ();
						toggle (c, tog);
					}
				} else {
					if (ch () == 0)
						readline ();
					else
						inchar ();
					if (feof (input))
						break;
				}
			inchar ();
			inchar ();
		} else if (ch() == '/' && nch() == '/') {
			while (ch())
				inchar();
		} else if (an (ch ())) {
			k = 0;
			while (an (ch ())) {
				if (k < NAMEMAX)
					sname[k++] = ch ();
				gch ();
			}
			sname[k] = 0;
			struct macro *mp;
			mp = findmac (sname);
			if (mp) {
				char args[40][256];
				int argc = 0;
				int haveargs = 0;
				int nest = 0;
				/* If the macro wants arguments, substitute them.
				   Unlike at the time of definition, here whitespace
				   is permissible between the macro identifier and
				   the opening parenthesis. */
				if (mp->argc && match("(")) {
					if (mp->argc == -1) {
						if (!match(")")) {
							error("missing closing paren");
							return 0;
						}
					}
					else {
						haveargs = 1;
						for (;;) {
							args[argc][0] = 0;
							while (ch() != ',' || nest > 0) {
								char c = gch();
								if (c == '(') {
									nest++;
								}
								if (!c) {
									error("missing closing paren");
									return 0;
								}
								strncat(args[argc], &c, 1);
								if (ch() == ')') {
									if (nest)
										nest--;
									else
										break;
								}
							}
#ifdef DEBUG_PREPROC
							printf("macro arg %s\n", args[argc]);
#endif
							argc++;
							if (ch() == ')') {
								gch();
								break;
							}
							gch();
						}
					}
				}
				if (mp->argc != -1 && argc != mp->argc) {
					error("wrong number of macro arguments");
					return 0;
				}

				cpped = 1;
				k = 0;
				if (haveargs) {
					int i;
					char *buf = malloc(1);
					buf[0] = 0;
					char *dp = mp->def;
					buf[0] = 0;
					for (i = 0; mp->argpos[i].arg != -1; i++) {
						buf = realloc(buf, strlen(buf) +
								   mp->argpos[i].pos - (dp - mp->def) +
								   strlen(args[mp->argpos[i].arg]) + 1);
						strncat(buf, dp, mp->argpos[i].pos - (dp - mp->def));
						strcat(buf, args[mp->argpos[i].arg]);
						dp = mp->def + mp->argpos[i].pos + strlen(mp->args[mp->argpos[i].arg]);
					}
					buf = realloc(buf, strlen(buf) + strlen(dp) + 1);
					strcat(buf, dp);
#ifdef DEBUG_PREPROC
					printf("postproc %s\n", buf);
#endif
					for (i = 0; buf[i]; i++)
						keepch(buf[i]);
					free(buf);
				}
				else {
					while ( (c = mp->def[k++]) )
						keepch (c);
					keepch(' ');
				}
			} else {
				k = 0;
				while ( (c = sname[k++]) )
					keepch (c);
			}
		} else
			keepch (gch ());
	}
	keepch (0);
	if (mptr >= MPMAX)
		error ("line too long");
	/* copy cooked input back to where we got the raw input from */
	strcpy(&line[llptr], mline);
	/* ...and continue processing at that point */
	lptr = llptr;
	return(cpped);
}

long keepch (char c)
{
	mline[mptr] = c;
	if (mptr < MPMAX)
		mptr++;
	return (c);
}

void defmac(char* s)
{
	kill();
	strcpy(line, s);
	addmac();
}

void addmac (void)
{
	char	sname[NAMESIZE];
	struct macro *mp;

	if (!symname (sname)) {
		illname ();
		kill ();
		return;
	}
	mp = findmac(sname);
	if (mp) {
		error("Duplicate define");
		delmac(mp);
	}
	else
		mp = &macq[macptr++];

	mp->name = strdup(sname);

	int argc = 0;
	mp->args = malloc(sizeof(char *) * 1);
	mp->args[0] = 0;
	mp->argpos = malloc(sizeof(*mp->argpos) * 1);
	mp->argpos[0].arg = -1;
	/* Stuff within parentheses is only considered a list of arguments
	   if there is no whitespace between the identifier and the opening
	   paren. */
	if (ch() == '(') {
		gch();
		if (match(")"))
			argc = -1;
		else {
			for (;;) {
				if (!symname(sname)) {
					error("invalid macro argument");
					delmac(mp);
					return;
				}
#ifdef DEBUG_PREPROC
				printf("arg %d %s\n", argc, sname);
#endif
				mp->args = realloc(mp->args, sizeof(char *) * (argc + 2));
				mp->args[argc++] = strdup(sname);
				if (argc >= 40) {
					error("too many arguments");
					delmac(mp);
					return;
				}
				mp->args[argc] = 0;
				if (match(")"))
					break;
				if (!match(",")) {
					error("expected comma");
					delmac(mp);
					return;
				}
			}
		}
	}
	mp->argc = argc;

	while ( (ch () == ' ') || (ch () == 9) )
		gch ();
	char c;
	mp->def = malloc(1);
	mp->def[0] = 0;
	int pos = 0;
	int count = 0;
	for (;;) {
		int found = 0;
		int i;
		if (ch() == '/' && nch() == '/') {
			kill();
			break;
		}
		for (i = 0; i < argc; i++) {
			if (an(ch()) && amatch(mp->args[i], strlen(mp->args[i]))) {
#ifdef DEBUG_PREPROC
				printf("arg %d at offset %d\n", i, pos);
#endif
				mp->def = realloc(mp->def, strlen(mp->def) + strlen(mp->args[i]) + 1);
				strcat(mp->def, mp->args[i]);
				mp->argpos = realloc(mp->argpos, sizeof(*mp->argpos) * (count + 2));
				mp->argpos[count].arg = i;
				mp->argpos[count++].pos = pos;
				mp->argpos[count].arg = -1;
				mp->argpos[count].pos = -1;
				pos += strlen(mp->args[i]);
				found = 1;
				break;
			}
		}
		if (!found) {
			c = gch();
			if (c == '\\') {
				c = gch();
				if (!c) {
					readline();
					c = gch();
				}
			}
			if (!c)
				break;
			mp->def = realloc(mp->def, strlen(mp->def) + 2);
			strncat(mp->def, &c, 1);
			pos++;
		}
	}
#ifdef DEBUG_PREPROC
	printf("macdef %s\n", mp->def);
#endif
	if (macptr >= MACMAX)
		error ("macro table full");
}

void delmac(struct macro *mp)
{
	if (mp->name)
		free(mp->name);
	mp->name = 0;
	if (mp->def)
		free(mp->def);
	mp->def = 0;
	if (mp->args)
		free(mp->args);
	mp->args = 0;
	if (mp->argpos)
		free(mp->argpos);
	mp->argpos = 0;
}
	
struct macro *findmac (char* sname)
{
	long	k;

	k = 0;
	while (k < macptr) {
		if (macq[k].name && astreq (sname, macq[k].name, NAMEMAX)) {
			return (&macq[k]);
		}
		k++;
	}
	return (0);
}

void toggle (char name, long onoff)
{
	switch (name) {
	case 'C':
		ctext = onoff;
		break;
	}
}
