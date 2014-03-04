#ifndef _SYM_H
#define _SYM_H

long declglb (long typ, long stor);
void declloc (long typ, long stclass);
long needsub (void);
char* findglb (char *sname);
char* findloc (char *sname);
char *addglb (char* sname,char id,char typ,long value,long stor);
char *addglb_far (char* sname, char typ);
char *addloc (char* sname,char id,char typ,long value,long stclass);
long symname (char* sname);
void illname (void);
void multidef (char* sname);
long glint(char* sym);

#endif

