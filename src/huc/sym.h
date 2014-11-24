#ifndef _SYM_H
#define _SYM_H

long declglb (long typ, long stor, TAG_SYMBOL *mtag, int otag, int is_struct);
void declloc (long typ, long stclass, int otag);
long needsub (void);
SYMBOL *findglb (char *sname);
SYMBOL *findloc (char *sname);
SYMBOL *addglb (char *sname, char id, char typ, long value, long stor, SYMBOL *replace);
SYMBOL *addglb_far (char *sname, char typ);
SYMBOL *addloc (char *sname, char id, char typ, long value, long stclass, long size);
long symname (char *sname);
void illname (void);
void multidef (char *sname);
long glint (SYMBOL *sym);

#endif
