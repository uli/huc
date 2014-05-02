/*	File const.h: 2.1 (00/07/17,16:02:19) */
/*% cc -O -c %
 *
 */

#ifndef _CONST_H
#define _CONST_H

void new_const(void );
void add_const(long typ);
long array_initializer(long typ, long id, long stor);
long scalar_initializer(long typ, long id, long stor);
long get_string_ptr(long typ);
long get_raw_value(char sep);
int add_buffer (char *p, char c, int is_address);
void dump_const (void );
char *get_const(SYMBOL *);

#endif

