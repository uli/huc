/*	File function.c: 2.1 (83/03/20,16:02:04) */
/*% cc -O -c %
 *
 */

#ifndef _FUNCTION_H
#define _FUNCTION_H

void newfunc (void);
void getarg (int t);
void callfunction (char *ptr);
void arg_stack(int arg);
void arg_push_ins(INS *ptr);
void arg_flush(int arg, int adj);
void arg_to_fptr(struct fastcall *fast, int i, int arg, int adj);
void arg_to_dword(struct fastcall *fast, int i, int arg, int adj);

#endif

