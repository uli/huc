/*	File stmt.c: 2.1 (83/03/20,16:02:17) */
/*% cc -O -c %
 *
 */


#ifndef _STMT_H
#define _STMT_H

long statement (long func);
long stdecl (void);
long doldcls (long stclass);
void stst (void);
void compound (long func);
void doif (void);
void dowhile (void);
void dodo (void);
void dofor (void);
void doswitch (void);
void docase (void);
void dodefault (void);
void doreturn (void);
void dobreak (void);
void docont (void);
void dolabel (char *name);
void dogoto (void);
void dumpsw (long *ws);
void test (long label, long ft);

#endif
