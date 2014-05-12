/* PCE Sound Designer
   Experiment with PSG waveforms and envelopes.
   Copyright (c) 2014, Ulrich Hecht
   All rights reserved.
   See LICENSE for details on use and redistribution. */

#include <huc.h>
#include <st.h>
#include "pointer.h"
#include "ui.h"

#incspr(slider, "slider_vertical_classic.pcx", 0, 0, 1, 1)
#incpal(slider_pal, "slider_vertical_classic.pcx")

#incchr_ex(piano, "three-octave-420.pcx", 0, 0, 52, 4, 8)
#incpal(piano_pal, "three-octave-420.pcx")

#define SLIDER_VADDR 0x5200
#define SLIDER_PAL 17

#define RGB(r, g, b) ( ((r) << 3) | ((g) << 6) | (b) )

unsigned char env[16];
void init_env(void)
{
	unsigned char i;
	unsigned int x;
	x = ENV_X;
	for (i = 0; i < 16; i++) {
		spr_set(i + ENV_START);
		spr_ctrl(FLIP_MAS|SIZE_MAS, NO_FLIP_X|SZ_16x16);
		spr_pattern(SLIDER_VADDR);
		spr_x(x);
		set_env(i, 31 - i*2);
		spr_pal(1);
		spr_pri(2);
		put_string("---", x/8, ENV_Y / 8);
		put_string("---", x/8, (ENV_Y - ENV_STEP_Y * 32)/8);
		x += ENV_STEP_X;
	}
	set_font_pal(1);
	put_string("Envelope", (ENV_X_RIGHT - ENV_X) / 2 / 8 - 4,
		(ENV_Y - ENV_STEP_Y * 8)/8);
	set_font_pal(0);
}

void set_env(unsigned char idx, unsigned char val)
{
	put_number(val, 2, (ENV_X + idx * ENV_STEP_X)/8, ENV_Y/8 + 1);
	env[idx] = val;
	spr_set(idx + ENV_START);
	spr_y((ENV_Y - ENV_CENTER) - val * ENV_STEP_Y);
}

unsigned char wave[32];
extern const unsigned char standard_waves[45*32];

void set_wave(unsigned char idx, unsigned char val)
{
	put_number(val, 2, (WAVE_X + idx * WAVE_STEP_X)/8,
		WAVE_Y/8 + 1 + (idx & 1));
	wave[idx] = val;
	spr_set(idx + WAVE_START);
	spr_y((WAVE_Y - WAVE_CENTER) - val * WAVE_STEP_Y);
}

void init_wave(void)
{
	unsigned char i;
	unsigned int x;
	x = WAVE_X;
	for (i = 0; i < 32; i++) {
		spr_set(i + WAVE_START);
		spr_ctrl(FLIP_MAS|SIZE_MAS, NO_FLIP_X|SZ_16x16);
		spr_pattern(SLIDER_VADDR);
		spr_x(x);
		set_wave(i, i >= 16 ? 31 : 0);
		spr_pal(1);
		spr_pri(2);
		put_string("--", x/8, WAVE_Y / 8);
		put_string("--", x/8, (WAVE_Y - WAVE_STEP_Y * 32)/8);
		x += WAVE_STEP_X;
	}
	set_font_pal(1);
	put_string("Waveform", (WAVE_X_RIGHT - WAVE_X) / 2 / 8 - 4,
		(WAVE_Y - WAVE_STEP_Y * 16)/8);
	set_font_pal(0);
}

#define USE_MAP	
#ifdef USE_MAP
unsigned char piano_map[52*4];
#endif
void init_piano(void)
{
	unsigned int i;
	unsigned int t,x;
	load_palette(8, piano_pal, 1);
	set_tile_data(piano);
	load_tile(0x1000);

#ifdef USE_MAP
	for (i = 0; i < 52*4; i++)
		piano_map[i] = i;
	set_map_data(piano_map, 52, 4);
	load_map(PIANO_X, PIANO_Y, 0, 0, 52, 4);
#else
	for (i = 0, t = 0, x = PIANO_X; i < 52; i++, t++, x++)
		put_tile(t, x, PIANO_Y);
	for (i = 0, t = 52, x = PIANO_X; i < 52; i++, t++, x++)
		put_tile(t, x, PIANO_Y+1);
	for (i = 0, t = 104, x = PIANO_X; i < 52; i++, t++, x++)
		put_tile(t, x, PIANO_Y+2);
	for (i = 0, t = 156, x = PIANO_X; i < 52; i++, t++, x++)
		put_tile(t, x, PIANO_Y+3);
#endif
}

#define CHAN 4

/* converts frequency to divider (.1 Hz units) */
#define P(f) (35800000 / 32 / f)

/* black and white piano keys */
const unsigned int piano_freqs_bw[] = {
	/* 1st octave (low C) */
	P(1308), P(1386), P(1468), P(1556), P(1648), P(1746), P(1850),
	P(1960), P(2077), P(2200), P(2330), P(2469),
	/* 2nd octave (middle C) */
	P(2616), P(2772), P(2937), P(3111), P(3296), P(3492), P(3700),
	P(3920), P(4153), P(4400), P(4662), P(4939),
	/* 3rd octave (tenor C) */
	P(5232), P(5544), P(5873), P(6223), P(6593), P(6985), P(7400),
	P(7840), P(8306), P(8800), P(9323), P(9878)
};

/* white keys only */
const unsigned int piano_freqs_white[] = {
	/* 1st octave (low C) */
	P(1308), P(1468), P(1648), P(1746),
	P(1960), P(2200), P(2469),
	/* 2nd octave (middle C) */
	P(2616), P(2937), P(3296), P(3492),
	P(3920), P(4400), P(4939),
	/* 3rd octave (tenor C) */
	P(5232), P(5873), P(6593), P(6985),
	P(7840), P(8800), P(9878)
};

unsigned int text_pal[16] = {0};
int main()
{
	unsigned char i;
	unsigned char manip_env = 0;
	unsigned char manip_wave = 0;
	unsigned char noise = 0;
	signed char octave = 0;
	unsigned char stdwave = 0;

	init_satb();
	load_default_font();
	set_xres(RES_X);

	load_palette(SLIDER_PAL, slider_pal, 1);
	load_vram(SLIDER_VADDR, slider, 0x40);

	init_pointer();
	init_env();
	init_wave();
	init_piano();

	/* screen background */
	set_color(0, RGB(1, 1, 7));

	text_pal[1] = RGB(6, 6, 0);
	load_palette(1, text_pal, 1);
	text_pal[1] = RGB(0, 6, 0);
	load_palette(2, text_pal, 1);

	set_font_pal(2);
	put_string("PCE Sound Designer", RES_X/2/8 - 9, 0);
	set_font_pal(1);
	put_string("[ ] Noise", NOISE_X - 1, NOISE_Y);
	put_string("Octave", OCTAVE_MINUS_X, OCTAVE_MINUS_Y-1);
	put_string("[+]", OCTAVE_PLUS_X-1, OCTAVE_PLUS_Y);
	put_string("[-]", OCTAVE_MINUS_X-1, OCTAVE_MINUS_Y);
	put_string("Std Wave", STDWAVE_MINUS_X, STDWAVE_MINUS_Y-1);
	put_string("[+]", STDWAVE_PLUS_X-1, STDWAVE_PLUS_Y);
	put_string("[-]", STDWAVE_MINUS_X-1, STDWAVE_MINUS_Y);
	set_font_pal(0);
	put_number(octave, 2, OCTAVE_NUM_X, OCTAVE_NUM_Y);
	put_number(stdwave, 2, STDWAVE_NUM_X, STDWAVE_NUM_Y);

	st_init();
	st_reset();
	st_set_env(CHAN, env);
	st_load_wave(CHAN, wave);
	st_set_vol(CHAN, 15, 15);
	
	for (;;) {
		char val, idx;
		unsigned char ev;
		update_pointer();
		ev = poll_event();
		if (ev & EV_LEFT_BUTTON_HELD) {
			if (manip_env || (mp_y > ENV_Y_TOP && mp_y < ENV_Y &&
				mp_x >= ENV_X && mp_x < ENV_X_RIGHT)) {
				manip_env = 1;
				val = -((mp_y - ENV_Y) / ENV_STEP_Y);
				if (val < 0) val = 0;
				if (val > 31) val = 31;
				idx = (mp_x - ENV_X) / ENV_STEP_X;
				if (idx < 0) idx = 0;
				if (idx > 15) idx = 15;
				set_env(idx, val);
			}
			if (manip_wave || (mp_y > WAVE_Y_TOP && mp_y < WAVE_Y &&
				mp_x >= WAVE_X && mp_x < WAVE_X_RIGHT)) {
				manip_wave = 1;
				val = -((mp_y - WAVE_Y) / WAVE_STEP_Y);
				if (val < 0) val = 0;
				if (val > 31) val = 31;
				idx = (mp_x - WAVE_X) / WAVE_STEP_X;
				if (idx < 0) idx = 0;
				if (idx > 31) idx = 31;
				set_wave(idx, val);
				st_load_wave(CHAN, wave);
			}
		}
		else {
			manip_env = manip_wave = 0;
		}
		if (ev & EV_LEFT_BUTTON_DOWN) {
			if (mp_y > PIANO_Y*8 && mp_x >= PIANO_X*8 &&
			    mp_x < PIANO_X*8 + 52*8) {
			    	unsigned int freq, off;
			    	off = mp_x - PIANO_X*8;
			    	if (noise)
			    		st_effect_noise(CHAN, 31 - (off / 12), 30);
				else {
					if (mp_y > PIANO_Y*8 + 20) /* white keys */
						freq = piano_freqs_white[off / 20];
					else	/* black and white keys */
						freq = piano_freqs_bw[off / 12];
					if (octave < 0)
						freq <<= -octave;
					else if (octave > 0)
						freq >>= octave;
					st_effect_wave(CHAN, freq, 30);
					put_number(freq, 4, 1, 24);
				}
			}
			else if (pointer_at(NOISE_X, NOISE_Y)) {
				noise = !noise;
				if (noise)
					put_char('X', NOISE_X, NOISE_Y);
				else
					put_char(' ', NOISE_X, NOISE_Y);
			}
			else if (pointer_at(OCTAVE_PLUS_X, OCTAVE_PLUS_Y)) {
				if (octave < 3)
					octave++;
				put_number(octave, 2, OCTAVE_NUM_X, OCTAVE_NUM_Y);
			}
			else if (pointer_at(OCTAVE_MINUS_X, OCTAVE_MINUS_Y)) {
				if (octave > -2)
					octave--;
				put_number(octave, 2, OCTAVE_NUM_X, OCTAVE_NUM_Y);
			}
			else if (pointer_at(STDWAVE_PLUS_X, STDWAVE_PLUS_Y)) {
				unsigned char *w;
				if (stdwave < 44)
					stdwave++;
				w = &standard_waves[stdwave*32];
				for (i = 0; i < 32; i++, w++)
					set_wave(i, *w);
				st_load_wave(CHAN, wave);
				put_number(stdwave, 2, STDWAVE_NUM_X, STDWAVE_NUM_Y);
			}
			else if (pointer_at(STDWAVE_MINUS_X, STDWAVE_MINUS_Y)) {
				unsigned char *w;
				if (stdwave > 0)
					stdwave--;
				w = &standard_waves[stdwave*32];
				for (i = 0; i < 32; i++, w++)
					set_wave(i, *w);
				st_load_wave(CHAN, wave);
				put_number(stdwave, 2, STDWAVE_NUM_X, STDWAVE_NUM_Y);
			}
		}
		vsync();
		satb_update();
	}
}
