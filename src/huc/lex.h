#ifndef _INCLUDE_LEX_H
#define _INCLUDE_LEX_H

void ns (void);

void junk (void);

long endst (void);

void needbrack (char *str);

long alpha (char c);

long numeric (char c);

long an (char c);

long sstreq (char* str1);

long streq (char* str1, char* str2);

long astreq (char* str1, char* str2, long len);

long match (char* lit);

long amatch (char* lit, long len);

void blanks (void);

#endif
