/*	File expr.c: 2.2 (83/06/21,11:24:26) */
/*% cc -O -c %
 *
 */

#ifndef _EXPR_H
#define _EXPR_H

void expression (int comma);
long expression_ex (LVALUE *lval, int comma, int norval);
long heir1 (LVALUE *lval, int comma);
long heir1a (LVALUE *lval, int comma);
long heir1b (LVALUE *lval, int comma);
long heir1c (LVALUE *lval, int comma);
long heir2 (LVALUE *lval, int comma);
long heir3 (LVALUE *lval, int comma);
long heir4 (LVALUE *lval, int comma);
long heir5 (LVALUE *lval, int comma);
long heir6 (LVALUE *lval, int comma);
long heir7 (LVALUE *lval, int comma);
long heir8 (LVALUE *lval, int comma);
long heir9 (LVALUE *lval, int comma);
long heir10 (LVALUE *lval, int comma);
long heir11 (LVALUE *lval, int comma);
void store (LVALUE *lval);
void rvalue (LVALUE *lval);
void needlval (void);

int is_byte (LVALUE *lval);

#endif
