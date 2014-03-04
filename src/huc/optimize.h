/*	File opt.c: 2.1 (83/03/20,16:02:09) */
/*% cc -O -c %
 *
 */

#ifndef _OPTIMIZE_H
#define _OPTIMIZE_H

void push_ins(INS *ins);
void flush_ins(void);
void gen_asm(INS *inst);

#endif

