/*	File io.h: 2.1 (83/03/20,16:02:07) */
/*% cc -O -c %
 *
 */

#ifndef _IO_H
#define _IO_H

#if defined(DJGPP) || defined(MSDOS) || defined(WIN32)
#define CR_LEN  2
#else
#define CR_LEN  1
#endif


long openin (char *p);
long openout (void);
void outfname (char *s);
void fixname (char *s);
long checkname (char *s);
#if defined(osx) || defined(__CYGWIN__)
void _kill (void);
#define kill _kill
#else
void kill (void);
#endif
void unget_line (void);
void readline (void);

/* could otherwise be char */
long inbyte (void);
long inchar (void);
long gch (void);
long nch (void);
long ch (void);

void pl (char *str);
void glabel (char *lab);
void gnlabel (long nlab);
void olprfix (void);
void col (void);
void comment (void);
void prefix (void);
void tab (void);
void ol (char *ptr);
void ot (char *ptr);
void nl (void);
void outsymbol (char *ptr);
void outlabel (long label);
void outdec (long number);
void outhex (long number);
void outhexfix (long number, long length);
char outbyte (char c);
void outstr (char *ptr);

#endif
