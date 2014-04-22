/* SimpleTracker
   Plays MOD files converted to ST format by mod2mml.
   Copyright (c) 2014, Ulrich Hecht
   All rights reserved.
   See LICENSE for details on use and redistribution. */

#include "huc.h"
#include "st.h"

const unsigned char *psg_ch = 0x800;
const unsigned char *psg_bal = 0x801;
const unsigned char *psg_freqlo = 0x802;
const unsigned char *psg_freqhi = 0x803;
const unsigned char *psg_ctrl = 0x804;
const unsigned char *psg_chbal = 0x805;
const unsigned char *psg_data = 0x806;
const unsigned char *psg_noise = 0x807;
const unsigned char *psg_lfofreq = 0x808;
const unsigned char *psg_lfoctrl = 0x809;

unsigned char current_wave[6];
unsigned char *st_chan_env[6];
unsigned char st_chan_env_pos[6];
unsigned char st_chan_len[6];

unsigned char st_row_idx;
unsigned char st_pattern_idx;
unsigned char st_song_bank;
unsigned char **st_pattern_table;
unsigned char *st_chan_map;
unsigned char **st_wave_table;
unsigned char **st_vol_table;

static unsigned char st_tick;

void st_reset(void)
{
	unsigned char j, i;
	irq_disable_user_irq(IRQ_VSYNC);
	*psg_bal = 0xff;
	*psg_lfoctrl = 0;
	for (j = 0; j < 6; j++) {
		*psg_ch = j;
		*psg_ctrl = 0;
		*psg_chbal = 0;
		*psg_freqlo = 0;
		*psg_freqhi = 0;
		*psg_noise = 0;
		for (i = 0; i < 15; i++) {
			*psg_data = (i > 7 ? 0x1f : 0);
		}
	}
	memset(current_wave, 0xff, sizeof(current_wave));
	memset(st_chan_env_pos, 0, sizeof(st_chan_env_pos));
	st_pattern_idx = 0;
	st_row_idx = 0;
	st_tick = 0;
	irq_enable_user_irq(IRQ_VSYNC);
}

static void load_ins(unsigned char ins)
{
	unsigned char i;
	unsigned char *data;
	data = st_wave_table[ins];
	for (i = 0; i < 32; i++) {
		*psg_data = *data;
		data++;
	}
}

static void vsync_handler(void)
#ifdef DEBUG_ST
			 __sirq
#else
			 __irq
#endif
{
	static unsigned char *pat;
	unsigned int freq;
	unsigned char chan;
	unsigned char *chv;
	unsigned char ins;
	unsigned char j;
	unsigned char dummy;
	unsigned char l, m;
	unsigned char is_drum;
	unsigned char save_bank;

	save_bank = mem_mapdatabank(st_song_bank);
	if ((st_tick & 7) == 0) {
		if (st_row_idx == 0) {
			pat = st_pattern_table[st_pattern_idx];
			if (!pat) {
				st_reset();
				pat = st_pattern_table[0];
			}
			st_pattern_idx++;
		}
#ifdef DEBUG_ST
		put_hex(pat, 4, 0, 1);
#endif
		for (j = 0; j < 4; j++) {
			*psg_ch = chan = st_chan_map[j];
			chv = st_chan_env[chan];
			ins = *pat;
			if (ins == 0xff) {
				pat++;
			} else {
				is_drum = (ins & 0xe0) == 0xe0;
				if (!is_drum
				    && ins != current_wave[chan]) {
					load_ins(ins);
					current_wave[chan] = ins;
				}
#ifdef DEBUG_ST
				put_hex(ins, 2, 10, 4 + j);
#endif
				pat++;
				st_chan_len[chan] = *pat++;
				freq = *pat++;
				freq |= (*pat++) << 8;
				st_chan_env_pos[chan] = 0;
				chv = st_chan_env[chan] =
				    st_vol_table[ins & 0x1f];
				if (is_drum && chan >= 4) {
#ifndef DISABLE_DRUMS
					*psg_noise =
					    0x80 | ((freq & 0xf) ^ 31);
#ifdef DEBUG_ST
					put_string("drum", 16, 4 + j);
#endif
					current_wave[chan] = -1;
					st_chan_env_pos[chan] = 0;
					chv = st_chan_env[chan] =
					    st_vol_table[ins &
								 0x1f];
					*psg_chbal = 0xaa;
#else
					*psg_chbal = 0;
#endif
				} else {
					*psg_noise = 0;
					*psg_freqlo = freq & 0xff;
					*psg_freqhi = freq >> 8;
#ifdef DEBUG_ST
					put_string("note", 16, 4 + j);
#endif
					*psg_chbal = 0xff;
				}
#ifdef DEBUG_ST
				put_number(freq, 5, 4, 4 + j);
#endif
			}
		}
		st_row_idx = (st_row_idx + 1) & 0x3f;
	}

	for (m = 0; m < 4; m++) {
		chan = st_chan_map[m];
		*psg_ch = chan;
		chv = st_chan_env[chan];
		if ((st_tick & 7) == 0) {
			if (!st_chan_len[chan]) {
				st_chan_env_pos[chan] = 0xff;
				*psg_ctrl = 0x80;
#ifdef DEBUG_ST
				put_string("--", 23,
					   4 + m);
				put_string("--", 26,
					   4 + m);
#endif
			} else {
				st_chan_len[chan]--;
			}
		}
		l = st_chan_env_pos[chan];
		if (l != 0xff) {
			*psg_ctrl = 0x80 | chv[l];
#ifdef DEBUG_ST
			if (chv[l] > 31) {
				put_string("s", 0, 12);
				put_hex(chv, 4, 0, 13);
				put_hex(st_tick, 2, 5, 13);
			}
			put_number(chv[l], 3, 22,
				   4 + m);
			put_number(l, 2, 26, 4 + m);
#endif
			if (l < 15)
				st_chan_env_pos[chan] = l + 1;
		}
	}
	mem_mapdatabank(save_bank);
#ifdef DEBUG_ST
	put_hex(st_tick, 2, 10, 0);
#endif
	++st_tick;
}

void st_init(void)
{
	irq_disable_user_irq(IRQ_VSYNC);
	irq_add_vsync_handler(vsync_handler);
}

void st_set_song(unsigned char song_bank, struct st_header *song_addr)
{
	unsigned char save;
	save = mem_mapdatabank(song_bank);
	st_song_bank = song_bank;
	st_pattern_table = song_addr->patterns;
	st_chan_map = song_addr->chan_map;
	st_wave_table = song_addr->wave_table;
	st_vol_table = song_addr->vol_table;
	st_reset();
	mem_mapdatabank(save);
}
