/*
 * pseudo.h
 */

#ifndef _PSEUDO_H
#define _PSEUDO_H

void dopsdinc(void);
void dopsddef(void);
int outcomma(void);
int outnameunderline(void);
int outconst(void);
void doset_bgpalstatement(void);
void doset_sprpalstatement(void);
void doload_spritesstatement(void);
void doload_backgroundstatement(void);
void do_asm_func(int type);
char * new_string(int und, char *a);

#endif

