#ifndef _INCLUDE_PREPROC_H
#define _INCLUDE_PREPROC_H

void doinclude (void);

void incl_globals (void);

FILE* fixiname (void);

void init_path (void);

void doasmdef (void);

void doasm (void);

void dodefine (void);

void doundef (void);

void preprocess (void);

void doifdef (int ifdef);

int ifline(void);

void noiferr(void);

int cpp (void);

int keepch (char c);

void defmac(char* s);

void addmac (void);

void delmac(int mp);

int putmac (char c);

int findmac (char* sname);

void toggle (char name, int onoff);

#endif
