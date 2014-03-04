/*      File code.h: 2.2 (84/11/27,16:26:11) */
/*% cc -O -c %
 *
 */

#ifndef _CODE_H
#define _CODE_H

extern int segment;

void gdata (void );
void gtext (void );
void header(void );
void inc_startup(void );
void asmdefines(void );
void defbyte (void );
void defstorage (void );
void defword (void );
void out_ins(int code, int type, int data);
void out_ins_ex(int code, int type, int data, int imm);
void out_ins_sym(int code, int type, int data, char *sym);
void gen_ins(INS *tmp);
void gen_code(INS *tmp);

#endif
