#ifndef _SYM_H
#define _SYM_H

int declglb (int typ, int stor);
void declloc (int typ, int stclass);
int needsub (void);
char* findglb (char *sname);
char* findloc (char *sname);
char *addglb (char* sname,char id,char typ,int value,int stor);
char *addglb_far (char* sname, char typ);
char *addloc (char* sname,char id,char typ,int value,int stclass);
int symname (char* sname);
void illname (void);
void multidef (char* sname);
int glint(char* sym);

#endif

