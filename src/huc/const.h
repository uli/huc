/*	File const.h: 2.1 (00/07/17,16:02:19) */
/*% cc -O -c %
 *
 */

#ifndef _CONST_H
#define _CONST_H

void new_const(void );
void add_const(int typ);
int array_initializer(int typ, int id, int stor);
int get_string_ptr(int typ);
int get_raw_value(void );
void add_buffer (char *p, char c);
void dump_const (void );

#endif

