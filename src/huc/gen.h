
#ifndef _GEN_H
#define _GEN_H

void gnargs (char *name, long nb);
long getlabel (void );
void getmem (char *sym);
void getio (char *sym);
void getvram (char *sym);
void getloc (char *sym);
void putmem (char *sym);
void putstk (char typeobj);
void putio (char *sym);
void putvram (char *sym);
void indirect (char typeobj);
void farpeek(char *ptr);
void immed (long type, long data);
void gpush (void );
void gpusharg (long size);
void gpop (void );
void swapstk (void );
void gcall (char *sname, long nargs);
void gbank(unsigned char bank, unsigned short offset);
void gret (void );
void callstk (long nargs);
void jump (long label);
void testjump (long label, long ft);
long modstk (long newstkp);
void gaslint (void );
void gasrint(void );
void gjcase(void );
void gadd (long *lval, long *lval2);
void gsub (void );
void gmult (int is_unsigned);
void gdiv (int is_unsigned);
void gmod (int is_unsigned);
void gor (void );
void gxor (void );
void gand (void );
void gasr (int is_unsigned);
void gasl (void );
void gneg (void );
void gcom (void );
void gbool (void );
void glneg (void );
void ginc (long * lval);
void gdec (long *lval);
void geq (int is_byte );
void gne (int is_byte);
void glt (int is_byte);
void gle (int is_byte);
void ggt (int is_byte);
void gge (int is_byte);
void gult (int is_byte);
void gule (int is_byte);
void gugt (int is_byte);
void guge (int is_byte);

void scale_const(int type, int otag, long *size);

#endif

