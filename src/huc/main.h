/*	File main.c: 2.7 (84/11/28,10:14:56)
 *
 * PC Engine C Compiler
 * Made by <unknown guy>, hacked to work on Pc Engine by David Michel
 * resumed work by Zeograd
 *
 * 00/02/22 : added oldargv variable to show real exe name in usage function
 */
/*% cc -O -c %
 *
 */

#ifndef _MAIN_H
#define _MAIN_H

#define HUC_VERSION	"HuC (v3.21, 2005/04/09, Denki release)"

void FEvers(void );
void usage(char* exename);
void parse (void );
long dodcls(long stclass, TAG_SYMBOL *mtag, int is_struct);
void dumplits (void );
void dumpglbs (void );
void errorsummary (void );
char extension(char *s);
long assemble(char *s);

#endif

