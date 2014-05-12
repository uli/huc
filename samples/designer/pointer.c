/* pointer.c
   Part of PCE Sound Designer
   Copyright (c) 2014, Ulrich Hecht
   All rights reserved.
   See LICENSE for details on use and redistribution. */

#include <huc.h>
#include "pointer.h"
#include "ui.h"

#incspr(pointer,"pointer.pcx",0,0,1,1)
#incpal(pointer_pal,"pointer.pcx")
int mp_x, mp_y;
void init_pointer(void)
{
	mp_x = mp_y = 0;
	load_palette(16, pointer_pal, 1);
	load_vram(0x5000, pointer, 0x40);
	spr_set(0);
	spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP_X|SZ_16x16);
	spr_pattern(0x5000);
	spr_x(mp_x);
	spr_y(mp_y);
	spr_pal(0);
	spr_pri(1);

	if (mouse_exists()) {
		mouse_enable();
	}
	else {
		put_string("NO MOUSE", 0, 0);
	}
}

void update_pointer(void)
{
	mp_x -= mouse_x();
	mp_y -= mouse_y();
	if (mp_x < 0) mp_x = 0;
	if (mp_x > RES_X - 2) mp_x = RES_X - 2;
	if (mp_y < 0) mp_y = 0;
	if (mp_y > 222) mp_y = 222;
	spr_set(0);
	spr_x(mp_x);
	spr_y(mp_y);
	put_number(mp_x, 3, 5, 0);
	put_number(mp_y, 3, 9, 0);
}

unsigned char pointer_at(unsigned char x, unsigned char y)
{
	unsigned int xp, yp;
	xp = (x-1) * 8; yp = y * 8;
	return (mp_x >= xp && mp_x < xp+16 && mp_y >= yp && mp_y < yp+8);
}

unsigned char poll_event(void)
{
	static unsigned char lb, rb;
	unsigned int j;
	unsigned char ev = EV_NONE;
	j = joy(0);
	if (j & JOY_II) {
		ev |= EV_LEFT_BUTTON_HELD;
		if (!lb) {
			lb = 1;
			ev |= EV_LEFT_BUTTON_DOWN;
		}
	}
	else {
		if (lb) {
			lb = 0;
			ev |= EV_LEFT_BUTTON_UP;
		}
	}
	if (j & JOY_I) {
		ev |= EV_RIGHT_BUTTON_HELD;
		if (!rb) {
			rb = 1;
			ev |= EV_RIGHT_BUTTON_DOWN;
		}
	}
	else {
		if (rb) {
			rb = 0;
			ev |= EV_RIGHT_BUTTON_UP;
		}
	}
	put_hex(ev, 2, 1, 0);
	return ev;
}

