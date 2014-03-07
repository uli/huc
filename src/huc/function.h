/*	File function.c: 2.1 (83/03/20,16:02:04) */
/*% cc -O -c %
 *
 */

#ifndef _FUNCTION_H
#define _FUNCTION_H

void newfunc (const char *sname);
void getarg (long t, int syntax, int otag);
void callfunction (char *ptr);
void arg_stack(long arg);
void arg_push_ins(INS *ptr);
void arg_flush(long arg, long adj);
void arg_to_fptr(struct fastcall *fast, long i, long arg, long adj);
void arg_to_dword(struct fastcall *fast, long i, long arg, long adj);

#endif

