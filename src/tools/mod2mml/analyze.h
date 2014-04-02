#ifndef _ANALYZE_H
#define _ANALYZE_H

typedef struct {
	int start, end;
	int vol;
	int asymmetry_time;
	int asymmetry_vol;
	int end_dist;
	int interpol;
	int score;
} wave_info;

typedef struct {
	char name[22];			/* sample name from MOD */
	unsigned int length;		/* length in samples */
	int tune;
	unsigned char volume;		/* volume from MOD */
	unsigned int repeat_at;		/* repeat position from MOD */
	unsigned int repeat_length;	/* repeat length from MOD */
	signed char *data;		/* sample data */
	int envelope;			/* detected envelope */
	wave_info *wave;		/* detected waveform */
	int waveno;			/* number of detected waveform */
} sample_info;

void analyze_sample(sample_info *s);

#endif
