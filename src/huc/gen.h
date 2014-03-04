
#ifndef _GEN_H
#define _GEN_H

void gnargs (char *name, int nb);
int getlabel (void );
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
void immed (int type, int data);
void gpush (void );
void gpusharg (int size);
void gpop (void );
void swapstk (void );
void gcall (char *sname, int nargs);
void gbank(unsigned char bank, unsigned short offset);
void gret (void );
void callstk (int nargs);
void jump (int label);
void testjump (int label, int ft);
int modstk (int newstkp);
void gaslint (void );
void gasrint(void );
void gjcase(void );
void gadd (int *lval, int *lval2);
void gsub (void );
void gmult (void );
void gdiv (void );
void gmod (void );
void gor (void );
void gxor (void );
void gand (void );
void gasr (void );
void gasl (void );
void gneg (void );
void gcom (void );
void gbool (void );
void glneg (void );
void ginc (int * lval);
void gdec (int *lval);
void geq (void );
void gne (void );
void glt (void);
void gle (void );
void ggt (void );
void gge (void );
void gult (void );
void gule (void );
void gugt (void );
void guge (void );

#endif

