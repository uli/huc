/*
 * pseudo.h
 */

#ifndef _PSEUDO_H
#define _PSEUDO_H

void dopsdinc(void);
void dopsddef(void);
long outcomma(void);
long outnameunderline(void);
long outconst(void);
void doset_bgpalstatement(void);
void doset_sprpalstatement(void);
void doload_spritesstatement(void);
void doload_backgroundstatement(void);
void do_asm_func(long type);
char * new_string(long und, char *a);

#endif

