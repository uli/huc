/* 
 *  file:        scroll.c 
 *  description: a three-screen horizontal scrolling demo
 *  author:      David Michel (dmichel@easynet.fr)
 *  date:        2000.08.16
 *  files:       scroll.pcx
 *  version:     1.0
 */

#include "huc.h"

#define TRUE  1
#define FALSE 0

/* vram allocation */
#define TILE_VRAM    0x1000
#define PANEL_VRAM   0x2000
#define SPRITE_VRAM  0x4000

/*  palette allocation */
#define TILE_PAL     0
#define PANEL_PAL    2
#define SPRITE_PAL  16

/*  tiles */
#inctile(tile_gfx, "scroll.pcx", 4, 3)
#incpal(tile_pal,  "scroll.pcx", 0)

/*  sprites */
#incspr(sprite,     "scroll.pcx", 64, 0, 2, 1)
#incpal(sprite_pal, "scroll.pcx", 1)

/*  control panel */
#incchr(panel_gfx, "scroll.pcx", 0, 48, 32, 4)
#incpal(panel_pal, "scroll.pcx", 2)
#incbat(panel_bat, "scroll.pcx", PANEL_VRAM, 0, 48, 32, 4)

/*  define the demo map
 *
 *  (should be #incbin'ed if the map is big
 *   or uses more than a dozen of tiles)
 */
#define MAP_WIDTH  48
#define MAP_HEIGHT 12
#define NB_TILE    12

const char demo_map[] = {
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,6,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,4,5,4,5,4,5,4,5,4, 4,4,5,5,4,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,4,6,4,5,4,5,4,5,4, 5,5,5,5,4,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,4,4,4,5,4,5,4,5,4, 5,5,5,5,4,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,4,5,4,5,4,5,4,5,4, 6,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,4,5,4,5,4,4,4,5,4, 4,4,5,5,4,5,6,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,0,8,2,2,2,2,2,2,2,2,2, 2,2,2,0,0,0,2,2,0,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6, 5,7,7,7,7,0,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,0,5,5,5,5,5,5,6,
	2,5,5,5,5,5,6,5,5,5,5,5,5,5,5,8, 2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,0,9,9,9,9,9,8,2,
	1,5,7,7,2,2,2,0,0,8,9,9,9,9,9,0, 1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,1,
	1,2,2,2,1,1,1,1,1,0,3,3,3,3,3,0, 1,1,1,1,1,1,1,1,1,0,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,1,
	1,1,1,1,1,1,1,1,1,0,3,3,3,3,3,0, 1,1,1,1,1,1,1,1,1,0,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,1
};

/*  palette index for tiles (one index per tile) */
const char tile_pal_ref[NB_TILE] = {
	TILE_PAL<<4, TILE_PAL<<4, TILE_PAL<<4, TILE_PAL<<4,
	TILE_PAL<<4, TILE_PAL<<4, TILE_PAL<<4, TILE_PAL<<4,
	TILE_PAL<<4, TILE_PAL<<4, TILE_PAL<<4, TILE_PAL<<4
};

/*  scroll vars */
int tx;
int sx, sy;
int map_x;
int dir;

pause()
{
	/*  show 'paused' sprite */
	spr_set(0);
	spr_pal(SPRITE_PAL);
	spr_pattern(SPRITE_VRAM);
	spr_ctrl(SIZE_MAS|FLIP_MAS, SZ_32x16|NO_FLIP);
	spr_pri(1);
	spr_x(112);
	spr_y(88);
	satb_update();

	/*  wait that the user press START again */
	do {
		vsync();
	} while (!(joytrg(0) & JOY_STRT));

	/*  hide sprite */
	spr_hide();
	satb_update();

	/* reset joypad */
	clear_joy_events(0x01);
}

main()
{
	int flag;
	int score;

	/*  disable display */
	disp_off();

	/*  clear display */
	cls();

	/*  init sprite */
	load_sprites(SPRITE_VRAM, sprite, 1);
	load_palette(SPRITE_PAL, sprite_pal, 1);

	/*  init map */
	set_map_data(demo_map, MAP_WIDTH, MAP_HEIGHT);
	set_tile_data(tile_gfx, NB_TILE, tile_pal_ref);
	load_tile(TILE_VRAM);
	load_palette(TILE_PAL, tile_pal, 1);
	load_map(0, 0, 0, 0, 17, 12);

	/*  init font */
	set_font_color(1, 10);
	set_font_pal(PANEL_PAL);
	load_default_font();

	/*  init control panel */
	load_vram(PANEL_VRAM, panel_gfx, 2048);
	load_bat(0x600, panel_bat, 32, 4);
	load_palette(PANEL_PAL, panel_pal, 1);
	put_string("score: ", 1, 25);

	/*  split screen */
	scroll(0, 0,   0,   0, 191, 0xC0);
	scroll(1, 0, 192, 192, 223, 0x80);

	/*  enable display */
	disp_on();

	/*  set joypad callback routine (pause mode) */
	set_joy_callback(0, 0x01, JOY_STRT, pause);

	/*  init scroll */
	sx = 0;
	tx = 0;
	dir = 1;
	map_x = 0;
	score = 0;

	/*  demo main loop */
	for (;;)
	{
		/*  get move direction */
		if (joy(0) & JOY_LEFT) {
			dir  = -1;
			flag =  TRUE;
		}
		else
		if (joy(0) & JOY_RGHT) {
			dir  =  1;
			flag =  TRUE;
		}
		else {
			flag = FALSE;
		}

		/*  calculate the new scroll position */
		if (flag == TRUE)
			tx = sx + (dir << 1);
		else
			tx = sx + (dir);

		/*  compare the old and new screen position
		 *  (on a tile basis), and update the screen map
		 *  if they are different
		 */
		if((tx & 0xFFF0) != (sx & 0xFFF0)) {
			/*  right dir */
			if (dir == 1) {
				map_x = map_x + 1;

				if (map_x  > MAP_WIDTH)
					map_x -= MAP_WIDTH;

				load_map(
				  (tx + 256) >> 4,  /*  screen x coordinate (in tile unit) */
				   0,               /*  screen y */    
				   map_x + 16,  /*  map x */
				   0,   /*  map y */
				   1,   /*  nb of map column to load */
				   12); /*  nb of row */
			}

			/*  left dir */
			else {
				map_x = map_x - 1;

				if (map_x  < 0)
					map_x += MAP_WIDTH;

				load_map(
				  (tx) >> 4,    /*  screen x coordinate */
				   0,           /*  screen y */    
				   map_x,	/*  map x */
				   0,   /*  map y */
				   1,   /*  nb of map column to load */
				   12); /*  nb of row */
			}
			score += 1;
		}

		/*  earthquake :) */
		if (!(joy(0) & (JOY_A | JOY_B)))
			sy = 0;
		else {
			sy = rand() & 0x07;
			score += sy;
		}

		/*  display score */
		put_number(score, 5, 8, 25);

		/*  set new scroll position */
		sx = tx;
		scroll(0, sx, 0, sy, sy + 191, 0xC0);
		vsync();
	}
}

