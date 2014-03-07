/*	File primary.c: 2.4 (84/11/27,16:26:07) */
/*% cc -O -c %
 *
 */

#ifndef _PRIMARY_H
#define _PRIMARY_H

long primary (LVALUE* lval);
long dbltest (LVALUE val1[],LVALUE val2[]);
void result (LVALUE lval[],LVALUE lval2[]);
long constant (long val[]);
long number (long val[]);
long pstr (long val[]);
long qstr (long val[]);
long readqstr (void );
long readstr (void );
long spechar(void );

#endif

