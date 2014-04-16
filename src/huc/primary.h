/*	File primary.c: 2.4 (84/11/27,16:26:07) */
/*% cc -O -c %
 *
 */

#ifndef _PRIMARY_H
#define _PRIMARY_H

long primary (LVALUE* lval, int comma);
long dbltest (LVALUE val1[],LVALUE val2[]);
void result (LVALUE lval[],LVALUE lval2[]);
long constant (long val[]);
long number (long val[]);
int const_expr(long *, char *, char *);
long pstr (long val[]);
long qstr (long val[]);
long const_str(long val[], const char *);
long readqstr (void );
long readstr (void );
long spechar(void );

int match_type(struct type *, int do_ptr);

#endif

