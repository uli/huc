/* Copyright (c) 2014, Ulrich Hecht
   All rights reserved.
   See LICENSE for details on use and redistribution. */

#include <stdlib.h>
#include <string.h>
#include "data.h"
#include "error.h"
#include "io.h"
#include "lex.h"
#include "primary.h"
#include "sym.h"

int define_enum (char *sname, int storage)
{
	char n[NAMESIZE];
	int count, min, max;
	int start = enum_ptr;

	printf("defenum %s\n", sname);
	needbrack("{");
	count = min = max = 0;
	for (;;) {
		if (!symname(n)) {
			illname();
			return (-1);
		}
		/* Add to global table of enum values. */
		enums = realloc(enums, (enum_ptr + 1) * sizeof(*enums));
		strcpy(enums[enum_ptr].name, n);
		/* optional initializer */
		if (match("=")) {
			long num;
			if (const_expr(&num, ",", "}"))
				count = num;
		}
		/* Remember minima and maxima to find the shortest type. */
		if (count > max) max = count;
		if (count < min) min = count;
		enums[enum_ptr].value = count++;
		enum_ptr++;
		if (match("}"))
			break;
		if (!match(",")) {
			error("expected comma");
			return (-1);
		}
		if (match("}"))
			break;
	}
	/* Add to table of enum types. */
	enum_types = realloc(enum_types, (enum_type_ptr + 1) * sizeof(*enum_types));
	struct enum_type *et = &enum_types[enum_type_ptr];
	if (sname)
		strcpy(et->name, sname);
	else
		et->name[0] = 0;
	et->start = start;
	if (!user_short_enums) {
		et->base = CINT;
		if (min < -32768 || max > 32767)
			warning(W_GENERAL, "enum range too large");
	}
	else if (min < 0) {
		if (min < -128 || max > 127) {
			et->base = CINT;
			if (min < -32768 || max > 32767)
				warning(W_GENERAL, "enum range too large");
		}
		et->base = CCHAR;
	}
	else {
		if (max > 65535)
			warning(W_GENERAL, "enum range too large");
		if (max > 127)
			et->base = CUINT;
		else
			et->base = CUCHAR;
	}
	printf("enum base type %d\n", et->base);
	return (enum_type_ptr++);
}

int find_enum_type (char *name)
{
	int i;

	for (i = 0; i < enum_type_ptr; i++) {
		if (!strcmp(enum_types[i].name, name))
			return (i);
	}
	return (-1);
}

int find_enum (char *sname, long *val)
{
	int i;

	for (i = 0; i < enum_ptr; i++) {
		if (!strcmp(sname, enums[i].name)) {
			*val = enums[i].value;
			return (1);
		}
	}
	return (0);
}
