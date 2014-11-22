/*	File pragma.c: 2.1 (00/08/09,04:59:24) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "data.h"
#include "error.h"
#include "io.h"
#include "lex.h"
#include "pragma.h"
#include "sym.h"
#include "fastcall.h"

/* locals */
struct fastcall  ftemp;
struct fastcall *fastcall_tbl[256];
static char   cmd[LINESIZE];
static char  *cmdptr;

/* default pragma's */
static char *pragma_init[] = {
	/* far pointer support funcs */
	"fastcall farpeekb(farptr fbank:fptr)",
	"fastcall farpeekw(farptr fbank:fptr)",
	"fastcall farmemget(word bx, farptr fbank:fptr, word acc)",
	/* asm-lib wrappers */
	"fastcall load_palette(byte al, farptr bl:si, byte cl)",
	"fastcall load_bat(word di, farptr bl:si, byte cl, byte ch)",
	"fastcall load_vram(word di, farptr bl:si, word cx)",
	"fastcall load_vram2(word di, word si, byte bl, word cx)",
	"fastcall snd_trkreg(byte al, farptr bl:si)",
	/* text funcs */
	"fastcall cls(word dx)",
	"fastcall set_xres(word ax)",
	"fastcall set_xres(word ax, byte cl)",
	"fastcall set_font_color(byte al, byte acc)",
	"fastcall load_font(farptr bl:si, byte cl)",
	"fastcall load_font(farptr bl:si, byte cl, word di)",
	"fastcall load_default_font(byte dl)",
	"fastcall load_default_font(byte dl, word di)",
	"fastcall put_digit(byte dl, word acc)",
	"fastcall put_digit(byte dl, byte cl, byte acc)",
	"fastcall put_char(byte dl, word acc)",
	"fastcall put_char(byte dl, byte cl, byte acc)",
	"fastcall put_raw(word dx, word acc)",
	"fastcall put_raw(word dx, byte cl, byte acc)",
	"fastcall put_number(word dx, byte cl, word acc)",
	"fastcall put_number(word dx, byte cl, byte bl, byte acc)",
	"fastcall put_hex(word dx, byte cl, word acc)",
	"fastcall put_hex(word dx, byte cl, byte bl, byte acc)",
	"fastcall put_string(word si, word acc)",
	"fastcall put_string(word si, byte bl, byte acc)",
	/* gfx lib funcs */
	"fastcall gfx_plot(word bx, word cx, word acc)",
	"fastcall gfx_point(word bx, word cx)",
	"fastcall gfx_line(word bx, word cx, word si, word bp, word acc)",

	"fastcall vram_addr(byte al, byte acc)",
	"fastcall spr_ctrl(byte al, byte acc)",
	"fastcall get_color(word color_reg)",
	"fastcall set_color(word color_reg, word color_data) nop",
	"fastcall set_color_rgb(word color_reg, byte al, byte ah, byte acc)",
	"fastcall fade_color(word ax, byte acc)",
	"fastcall fade_color(word color_reg, word ax, byte acc)",
	/* map lib funcs */
	"fastcall scan_map_table(word si, word ax, word cx)",
	"fastcall load_map(byte al, byte ah, word di, word bx, byte dl, byte dh)",
	"fastcall set_map_data(word acc)",
	"fastcall set_map_data(farptr bl:si, word ax, word acc)",
	"fastcall set_map_data(farptr bl:si, word ax, word dx, byte acc)",
	"fastcall set_tile_data(word di)",
	"fastcall set_tile_data(farptr bl:si, word cx, farptr al:dx)",
	"fastcall put_tile(word dx, word acc)",
	"fastcall put_tile(word dx, byte al, byte acc)",
	"fastcall map_get_tile(byte dl, byte acc)",
	"fastcall map_put_tile(byte dl, byte dh, byte acc)",
	/* misc funcs */
	"fastcall get_joy_events(byte acc)",
	"fastcall get_joy_events(byte al, byte acc)",
	"fastcall set_joy_callback(byte dl, byte al, byte ah, farptr bl:si)",
	"fastcall poke(word bx, word acc)",
	"fastcall pokew(word bx, word acc)",
	"fastcall srand32(word dx, word cx)",
	/* 32-bit math funcs */
	"fastcall mov32(word di, dword acc:ax|bx)",
	"fastcall add32(word di, dword acc:ax|bx)",
	"fastcall sub32(word di, dword acc:ax|bx)",
	"fastcall mul32(word bp, dword acc:ax|bx)",
	"fastcall div32(word bp, dword acc:ax|bx)",
	"fastcall cmp32(word di, dword acc:ax|bx)",
	"fastcall com32(word di)",
	/* bcd math funcs */
	"fastcall bcd_init(word bx, word acc)",
	"fastcall bcd_set(word bx, word acc)",
	"fastcall bcd_mov(word bx, word acc)",
	"fastcall bcd_add(word di, word acc)",
	/* bram funcs */
	"fastcall bm_rawwrite(word bx, word acc)",
	"fastcall bm_read(word di, word bx, word bp, word acc)",
	"fastcall bm_write(word di, word bx, word bp, word acc)",
	"fastcall bm_create(word bx, word acc)",
	"fastcall bm_getptr(word bp, word acc)",
	/* string funcs */
	"fastcall strcpy(word di, word si)",
	"fastcall strncpy(word di, word si, word acc)",
	"fastcall strcat(word di, word si)",
	"fastcall strncat(word di, word si, word acc)",
	"fastcall strcmp(word di, word si)",
	"fastcall strncmp(word di, word si, word acc)",
	"fastcall strlen(word si)",
	"fastcall memcpy(word di, word si, word acc)",
	"fastcall mempcpy(word di, word si, word acc)",
	"fastcall memcmp(word di, word si, word acc)",
	"fastcall memset(word di, word bx, word acc)",
	/* CDROM funcs */
	"fastcall cd_trkinfo(word ax, word cx, word dx, word bp)",
	"fastcall cd_playtrk(word bx, word cx, word acc)",
	"fastcall cd_playmsf(byte al, byte ah, byte bl, byte cl, byte ch, byte dl, word acc)",
	"fastcall cd_loadvram(word di, word si, word bx, word acc)",
	"fastcall cd_loaddata(word di, word si, farptr bl:bp, word acc)",
	/* ADPCM funcs */
	"fastcall ad_trans(word di, word si, byte al, word bx)",
	"fastcall ad_read(word cx, byte dh, word bx, word ax)",
	"fastcall ad_write(word cx, byte dh, word bx, word ax)",
	"fastcall ad_play(word bx, word ax, byte dh, byte dl)",

	"fastcall __builtin_ffs(word acc)",
	NULL
};

/* protos */
long fastcall_look(const char *fname, long nargs, struct fastcall **p);


/* ----
 * dopragma()
 * ----
 * handle pragma directive
 *
 */
void dopragma(void )
{
	long i;

	/* make a local copy of the pragma command line */
	for (i = 0;; i++) {
		if (ch() == 0)
			break;
		cmd[i] = gch();
	}
	cmd[i] = 0;

	/* parse */
	parse_pragma();
}


/* ----
 * defpragma()
 * ----
 * default pragmas
 *
 */
void defpragma(void )
{
	long i;

	for (i = 0;; i++) {
		if (pragma_init[i] == NULL)
			break;
		strcpy(cmd, pragma_init[i]);
		parse_pragma();
	}
}


/* ----
 * parse_pragma()
 * ----
 * parse pragma command line
 *
 */
void parse_pragma(void )
{
	char sname[NAMESIZE];

	/* get command */
	cmdptr = cmd;

	if(!symget(sname)) {
		error ("illegal symbol name");
		return;
	}

	/* fastcall */
	if (strcmp(sname, "fastcall") == 0)
		new_fastcall();
/*		new_fastcall(sname); */
	else if (!strcmp(sname, "no_recursive"))
		norecurse = 1;
	else if (!strcmp(sname, "recursive"))
		norecurse = 0;
	/* others */
	else
		error ("unknown pragma");
}


/* ----
 * new_fastcall()
 * ----
 * setup a new fastcall
 *
 * ie. #pragma fastcall func(word dx, byte al, byte ah)
 *
 */
void new_fastcall(void )
{
	struct fastcall *ptr;
	char   fname[NAMESIZE];
	char   sname[NAMESIZE];
	long    hash;
	long    cnt;
	long    i;

	ptr = &ftemp;
	cnt = 0;
	ptr->nargs = 0;
	ptr->flags = 0;

	/* get func name */
	if(!symget(fname)) {
		error("illegal symbol name");
		return;
	}

	/* open */
	if(!strmatch("(")) {
		error ("missing bracket");
		return;
	}

	/* extract args (max. 8) */
	for (i = 0; i < 8; i++) {
		/* get type */
		if(!symget(sname)) {
			if (*cmdptr == ')')
				break;
			error ("syntax error");
			return;
		}
		if (strcmp(sname, "byte") == 0)
			ptr->argtype[i] = TYPE_BYTE;
		else if (strcmp(sname, "word") == 0)
			ptr->argtype[i] = TYPE_WORD;
		else if (strcmp(sname, "farptr") == 0)
			ptr->argtype[i] = TYPE_FARPTR;
		else if (strcmp(sname, "dword") == 0)
			ptr->argtype[i] = TYPE_DWORD;
		else {
			error ("fastcall unknown type");
			return;
		}

		/* get name */
		if(!symget(sname)) {
			/* auto */
			if (*cmdptr != ',')
				ptr->argtype[i]  = TYPE_ACC;
			else {
				error("fastcall register missing");
				return;
			}
		}
		else {
			/* dword */
			if (ptr->argtype[i] == TYPE_DWORD) {
				/* ptr */
				if (strcmp(sname, "acc") == 0)
					strcpy(ptr->argname[i++], "#acc");
				else
					strcpy(ptr->argname[i++], sname);

				/* low word */
				if(*cmdptr++ != ':') {
					error ("syntax error");
					return;
				}
				if(!symget(sname)) {
					error ("illegal symbol name");
					return;
				}

				/* copy */
				strcpy(ptr->argname[i++], sname);

				/* high word */
				if(*cmdptr++ != '|') {
					error ("syntax error");
					return;
				}
				if(!symget(sname)) {
					error ("illegal symbol name");
					return;
				}

				/* copy */
				strcpy(ptr->argname[i], sname);
				cnt++;
			}

			/* far ptr */
			else if (ptr->argtype[i] == TYPE_FARPTR) {
				/* bank */
				strcpy(ptr->argname[i++], sname);
				ptr->argtype[i] = TYPE_WORD;

				/* addr */
				if(*cmdptr++ != ':') {
					error ("syntax error");
					return;
				}
				if(!symget(sname)) {
					error ("illegal symbol name");
					return;
				}

				/* copy */
				strcpy(ptr->argname[i], sname);
				cnt++;
			}

			/* other */
			else {
				if (strcmp(sname, "acc") == 0) {
					/* accumulator */
					ptr->argtype[i] = TYPE_ACC;
				}
				else {
					/* variable */
					strcpy(ptr->argname[i], sname);
					cnt++;
				}
			}
		}

		/* increment arg counter */
		ptr->nargs++;

		/* next */
		if(!strmatch(","))
			break;
	}

	/* close */
	if(!strmatch(")")) {
		error ("missing bracket");
		return;
	}

	/* extra infos */
	if (cnt) {
		if (ptr->nargs > 1)
			ptr->flags |= 0x02;
	}
	if (symget(sname)) {
		if (strcmp(sname, "nop") == 0)
			ptr->flags |= 0x01;
	}

	/* check arg number */
	if (ptr->nargs == 0)
		return;

	/* copy func name */
	strcpy(ptr->fname, fname);

	/* search for multi-decl */
	if (fastcall_look(fname, ptr->nargs, NULL)) {
		error("already defined");
		return;
	}

	/* ok */
	ptr = (void *)malloc(sizeof(struct fastcall));

	if (ptr == NULL)
		error("out of memory");
	else {
		/* dup struct */
		*ptr = ftemp;

		/* add to hash table */
		hash = symhash(fname);
		ptr->next = fastcall_tbl[hash];
		fastcall_tbl[hash] = ptr;
	}
}


/* ----
 * fastcall_look()
 * ----
 * search a fastcall function
 *
 */
long
fastcall_look(const char *fname, long nargs, struct fastcall **p)
{
	struct fastcall *ptr;
	struct fastcall *ref;
	long hash;
	long nb;

	/* search */
	hash = symhash(fname);
	ptr  = fastcall_tbl[hash];
	ref  = NULL;
	nb   = 0;
	while (ptr) {
		if (strcmp(ptr->fname, fname) == 0) {
			nb++;
			if (nargs != -1) {
				if (ptr->nargs == nargs)
					ref = ptr;
			}
		}
		ptr = ptr->next;
	}
	if (nargs != -1) {
		if(!ref)
			nb = 0;
	}

	/* return result */
	if (p)
	   *p = ref;
	return (nb);
}


/* ----
 * symhash()
 * ----
 * calculate the hash value of a symbol
 *
 */

long
symhash(const char *sym)
{
	long  i;
	char c;
	long  hash = 0;

	/* calc hash value */
	for (i = 0;; i++) {
		c = sym[i];
		if (c == 0)
			break;
		hash += c;
		hash  = (hash << 3) + (hash >> 5) + c;
	}

	/* ok */
	return (hash & 0xFF);
}


/* ----
 * symget()
 * ----
 * extract a symbol name
 *
 */
long
symget(char *sname)
{
	long	i;

	skip_blanks();

	/* first char must be alpha */
	if (!alpha(*cmdptr))
		return (0);

	/* extract symbol name (stops at first non-alphanum char) */
	for (i = 0;; i++) {
		if(!an(*cmdptr))
			break;
		sname[i] = *cmdptr++;
	}
	sname[i] = 0;

	/* ok */
	return (1);
}


/* ----
 * strmatch()
 * ----
 * test if next input string is legal symbol name
 *
 */
long
strmatch(char *lit)
{
	long	i;

	skip_blanks();

	/* compare */
	i = streq (cmdptr, lit);
	
	if (i) {
		/* match */
		cmdptr += i;
		return (1);
	}

	/* different */
	return (0);
}


/* ----
 * skip_blanks()
 * ----
 * skips blank chars (stops at end of input line)
 *
 */
void skip_blanks(void )
{
	while ((*cmdptr == ' ') || (*cmdptr == '\t'))
		cmdptr++;
}

