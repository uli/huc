/*	File const.h: 2.1 (00/07/17,16:02:19) */
/*% cc -O -c %
 *
 */

#ifndef _CONST_H
#define _CONST_H

void new_const(void );
void add_const(long typ);
long array_initializer(long typ, long id, long stor);
long get_string_ptr(long typ);
long get_raw_value(void );
void add_buffer (char *p, char c);
void dump_const (void );

#endif

