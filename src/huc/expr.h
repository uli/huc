/*	File expr.c: 2.2 (83/06/21,11:24:26) */
/*% cc -O -c %
 *
 */

#ifndef _EXPR_H
#define _EXPR_H

void expression (long comma);
long heir1 (LVALUE *lval);
long heir1a (LVALUE *lval);
long heir1b (LVALUE *lval);
long heir1c (LVALUE *lval);
long heir2 (LVALUE *lval);
long heir3 (LVALUE *lval);
long heir4 (LVALUE *lval);
long heir5 (LVALUE *lval);
long heir6 (LVALUE *lval);
long heir7 (LVALUE *lval);
long heir8 (LVALUE *lval);
long heir9 (LVALUE *lval);
long heir10 (LVALUE *lval);
long heir11 (LVALUE *lval);
void store (LVALUE *lval);
void rvalue (LVALUE *lval);
void needlval (void );

int is_byte(LVALUE *lval);

#endif

