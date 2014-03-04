#ifndef _INCLUDE_LEX_H
#define _INCLUDE_LEX_H

void ns (void);

void junk (void);

int endst (void);

void needbrack (char *str);

int alpha (char c);

int numeric (char c);

int an (char c);

int sstreq (char* str1);

int streq (char* str1, char* str2);

int astreq (char* str1, char* str2, int len);

int match (char* lit);

int amatch (char* lit, int len);

void blanks (void);

#endif
