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
int fastcall_look(char *fname, int nargs, struct fastcall **p);
int symhash(char *sym);
int symget(char *sname);
int strmatch(char *lit);
void skip_blanks(void );

#endif

