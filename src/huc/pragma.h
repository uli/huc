/*	File pragma.c: 2.1 (00/08/09,04:59:24) */
/*% cc -O -c %
 *
 */

#ifndef _PRAGMA_H
#define _PRAGMA_H

void dopragma(void );
void defpragma(void );
void parse_pragma(void );
void new_fastcall(void );
long fastcall_look(char *fname, long nargs, struct fastcall **p);
long symhash(char *sym);
long symget(char *sname);
long strmatch(char *lit);
void skip_blanks(void );

extern struct fastcall *fastcall_tbl[256];

#endif

