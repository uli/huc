/* Sample game for HuC, using new language features and SimpleTracker.
 * Based on the sample game at http://obeybrew.com/tutorials.html
 */

#include <huc.h>
#include <st.h>

#incspr(bonk,"charwalk.pcx",0,0,3,1)
#incpal(bonkpal,"charwalk.pcx")
#incspr(bullet,"bullet.pcx",0,0,1,1)
#incpal(bulletpal,"bullet.pcx")
#incspr(ship,"ship.pcx",0,0,2,8)
#incpal(shippal,"ship.pcx")
#incspr(explosion,"explosion.pcx",0,0,2,16)
#incpal(explosionpal,"explosion.pcx")

#incchr(scene_chr,"scene.pcx")
#incpal(scene_pal,"scene.pcx")
#incbat(scene_bat,"scene.pcx",0x1000,32,28)

#define SPEED_X 2
#define SPEED_Y 2

#define SPEED_BULLET 4
#define MAX_BULLETS 10
#define BULLET_SPRITE 1

struct bullet {
    int x, y;
    char active;
};

struct bullet bullets[MAX_BULLETS];

#define MAX_SHIPS 5
#define SPEED_SHIP 1
#define SHIP_SPRITE (BULLET_SPRITE + MAX_BULLETS)
#define SCORE_SHIP 100

struct ship {
    int x, y;
    char active;
    char vx, vy;
};

struct ship ships[MAX_SHIPS];

unsigned int frames;
unsigned int score, hiscore;

#incasm("bgm.asm")
extern struct st_header bgm[];

const unsigned char snd_bullet_env[] = {
	31, 31, 31, 31, 31, 31, 31, 30, 30, 29, 29, 28, 27, 26, 25, 23
};
const unsigned char snd_bullet_wave[] = {
	0x1d, 0x0f, 0x09, 0x05, 0x03, 0x01, 0x03, 0x05,
	0x09, 0x0f, 0x15, 0x19, 0x1b, 0x1d, 0x1b, 0x19,
	0x15, 0x0f, 0x07, 0x03, 0x01, 0x03, 0x07, 0x0f,
	0x17, 0x1b, 0x1d, 0x1b, 0x17, 0x0f, 0x01, 0x0f
};

void do_ships(void)
{
	unsigned int i,j;
	unsigned char r;
	struct ship *sp;
	struct bullet *bp;

        r = rand();
        if ((r & 0x7e) == 2) {
                for (i = 0, sp = ships; i < MAX_SHIPS; ++i, ++sp) {
                        if (!sp->active) {
                                sp->active = 1;
                                spr_set(SHIP_SPRITE + i);
                                if (r & 1) {
                                        spr_ctrl(FLIP_MAS|SIZE_MAS,FLIP_X|SZ_32x32);
                                        sp->vx = SPEED_SHIP;
                                        sp->vy = 0;
                                        sp->x = -32;
                                }
                                else {
                                        spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP_X|SZ_32x32);
                                        sp->vx = -SPEED_SHIP;
                                        sp->vy = 0;
                                        sp->x = 256;
                                }
                                sp->y = rand() % 210;
                                break;
                        }
                }
        }

        for (i = 0, sp = ships; i < MAX_SHIPS; ++i, ++sp) {
                if (sp->active) {
                        spr_set(SHIP_SPRITE + i);
                        spr_x(sp->x);
                        spr_y(sp->y);
                        if (sp->active > 1) {
                                spr_pattern(0x5900 + (sp->active >> 3) * 0x100);
                                spr_pal(3);
                                sp->active++;
                                if (sp->active == 64) {
					sp->active = 0;
					spr_x(-32);
					spr_y(16);
					spr_pal(2);
				}
			}
			else {
				spr_pattern(0x5500 + (((frames >> 4) & 3) * 0x100));
				for (j = 0, bp = bullets; j < MAX_BULLETS; ++j, ++bp) {
					if (bp->active) {
						if (bp->x > sp->x - 4 &&
						    bp->x < sp->x + 20 &&
						    bp->y > sp->y - 3 &&
						    bp->y < sp->y + 16) {
							/* explosion */
							sp->active = 2;
							sp->vx = 0;
							sp->vy = 0;
							bp->active = 0;
							spr_set(BULLET_SPRITE + j);
							spr_x(-16);
							spr_y(0);
							score += SCORE_SHIP;
							st_effect_noise(5, 15, 20);
							st_set_vol(5, 15, 15);
							st_set_env(5, snd_bullet_env);
						}
					}
				}
			}
			sp->x += sp->vx;
			sp->y += sp->vy;
			if (sp->x > 256 || sp->x < -32) {
				sp->active = 0;
				spr_x(-32);
				spr_y(16);
			}
		}
	}
#ifdef DEBUG
	for (i = 0, sp = ships; i < MAX_SHIPS; ++i, ++sp) {
		if (sp->active) {
		    put_number(sp->x, 4, 1, 4+i);
		    put_number(sp->y, 4, 5, 4+i);
		}
	}
#endif
}

void main(void)
{
	unsigned int j1;
	int bonkx, bonky;
	unsigned int tic;
	unsigned char i, j;
	unsigned char bullet_wait;
	char bonk_dir;
	char r;
	unsigned char dead;
	struct ship *sp;
	struct bullet *bp;

	hiscore = 0;

	st_init();
	st_set_song(bank(bgm), bgm);

	/* no goto yet, so we have to use this instead */
	for (;;) {

	tic = 0;
	frames = 0;
	score = 0;
	dead = 0;
	bonkx = 104;
	bonky = 153;
	bullet_wait = 0;
	bonk_dir = 1;

	st_reset();
	st_set_env(3, snd_bullet_env);
	st_load_wave(3, snd_bullet_wave);
	st_set_vol(3, 15, 15);
	st_play_song();

	init_satb();
	spr_set(0);
	spr_x(bonkx);
	spr_y(bonky);
	spr_pattern(0x5000);
	spr_ctrl(FLIP_MAS|SIZE_MAS,FLIP_X|SZ_32x32);
	spr_pal(0);
	spr_pri(1);

	for (i = 0; i < MAX_BULLETS; i++) {
		spr_set(BULLET_SPRITE + i);
		spr_x(-16);
		spr_y(0);
		spr_pattern(0x5400);
		spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP|SZ_16x16);
		spr_pal(1);
		spr_pri(1);
		bullets[i].active = 0;
	}

	for (i = 0; i < MAX_SHIPS; i++) {
		spr_set(SHIP_SPRITE + i);
		spr_x(-32);
		spr_y(16);
		spr_pattern(0x5500);
		spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP|SZ_32x32);
		spr_pal(2);
		spr_pri(1);
		ships[i].active = 0;
	}
	load_palette(16,bonkpal,1);
	load_palette(17,bulletpal,1);
	load_palette(18,shippal,1);
	load_palette(19,explosionpal,1);

	load_vram(0x5000,bonk,0x400);
	load_vram(0x5400,bullet,0x40);
	load_vram(0x5500,ship,0x400);
	load_vram(0x5900,explosion,0x800);

	satb_update();

	load_background(scene_chr,scene_pal,scene_bat,32,28);

	set_font_color(15, 6);
	load_default_font();

	while(!dead)
	{
		vsync();
		j1 = joy(0);
		if (joytrg(0) & JOY_RUN) {
			vsync();
			while (!(joytrg(0) & JOY_RUN))
				vsync();
		}
		if (j1 & JOY_LEFT)
		{
			spr_ctrl(FLIP_X_MASK,NO_FLIP_X);
			if (bonkx > -8) bonkx -= SPEED_X;
			tic++;
			bonk_dir = -1;
		}
		if (j1 & JOY_RIGHT)
		{
			spr_ctrl(FLIP_X_MASK,FLIP_X);
			if (bonkx < 232) bonkx += SPEED_X;
			tic++;
			bonk_dir = 1;
		}
		if (j1 & JOY_UP)
		{
			if (bonky > -8) bonky -= SPEED_Y;
		}
		if (j1 & JOY_DOWN)
		{
			if (bonky < 212) bonky += SPEED_Y;
		}
		if ((j1 & JOY_II) && !bullet_wait) {
			for (bp = bullets, i = 0; i < MAX_BULLETS; ++i, ++bp) {
				if (!bp->active) {
					bp->active = bonk_dir;
					bp->x = bonkx + 8 + bonk_dir * 16;
					bp->y = bonky + 10;
					bullet_wait = 10;
					spr_set(BULLET_SPRITE + i);
					if (bonk_dir > 0)
						spr_ctrl(FLIP_X_MASK, FLIP_X);
					else
						spr_ctrl(FLIP_X_MASK, NO_FLIP_X);
					st_effect_wave(3, 800, 16);
					break;
				}
			}
		}

		if (bullet_wait)
			bullet_wait--;

		bp = bullets;
		for (i = 0; i < MAX_BULLETS; i++) {
			if (bp->active) {
				spr_set(BULLET_SPRITE + i);
				spr_x(bp->x);
				spr_y(bp->y);
				bp->x += bp->active * SPEED_BULLET;
				//put_number(bp->x, 4, 0, 0);
				if (bp->x > 256 || bp->x < -16) {
					bp->active = 0;
					spr_x(-16);
					spr_y(0);
				}
			}
			++bp;
		}

		do_ships();

		sp = ships;
		for (i = 0; i < MAX_SHIPS; i++) {
			if (sp->active == 1 &&
			    ((sp->x > bonkx - 24 &&
			    sp->x < bonkx + 24 &&
			    sp->y > bonky - 20 &&
			    sp->y < bonky + 9) ||
			    (sp->x > bonkx - 18 &&
			    sp->x < bonkx + 18 &&
			    sp->y > bonky + 8 &&
			    sp->y < bonky + 25))) {
				put_string("GAME OVER", 11, 12);
				for (j = 0; j < 100; j++)
					vsync();
				dead = 1;
				if (score > hiscore)
				    hiscore = score;
			}
			sp++;
		}
		spr_set(0);
		spr_x(bonkx);
		//put_number(bonkx, 4, 0, 1);
		spr_y(bonky);
		spr_pattern(0x5000 + (((tic >> 2) & 3) * 0x100));
		satb_update();
		put_number(score, 5, 26, 1);
		put_string("HI", 1, 1);
		put_number(hiscore, 5, 4, 1);
		frames++;
	}
	}
}
