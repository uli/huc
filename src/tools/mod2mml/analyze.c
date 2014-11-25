#include <stdio.h>
#include "mod2mml.h"
#include "analyze.h"

#define DEBUG 1

#if DEBUG > 1
#define PRINTENV(x...) printf(x)
#define PRINTWAV(x...) printf(x)
#else
#define PRINTENV(x...)
#define PRINTWAV(x...)
#endif

#if DEBUG > 1
/* visualizes a single value */
void dump_value(int value, int divisor, int sign)
{
	int j;
	value /= divisor;
	if (!sign) {
		for (j = 0; j < value; j++) {
			printf("#");
		}
	}
	else {
		if (value < 0) {
			if (value < -32) {
				printf("<");
			}
			else {
				for (j = 0; j < 33 + value; j++)
					printf(" ");
			}
		}
		else {
			for (j = 0; j < 33; j++)
				printf(" ");
		}
		for (j = 0; j < (abs(value) > 32 ? 32 : abs(value)); j++)
			printf("#");
		if (value > 32)
			printf(">");
	}
	printf("\n");
}

/* visualizes a set of unsigned values */
void dump_curve(int *data, int len, int divisor)
{
	int i;
	for (i = 0; i < len; i++) {
		dump_value(data[i], divisor, 0);
	}
}
#endif

void analyze_sample(sample_info *s)
{
	int i, j;

	/* ===== Find a Volume Envelope ===== */

	/* Number of samples considered per volume value. */
	int volavgsize = s->length / 16;
	/* Number of volume values. */
	int avgsize = s->length / volavgsize;
	int vol[s->length];	/* volume per sample */
	int avgvol[avgsize];	/* averaged volume per volavgsize samples */

	/* Find volume for each sample by finding local maxima. */
	int last_vol = -1;
	for (i = 2; i < s->length-2; i++) {
		if (abs(s->data[i-1]) < abs(s->data[i]) &&
		    abs(s->data[i-2]) < abs(s->data[i]) &&
		    abs(s->data[i+2]) < abs(s->data[i]) &&
		    abs(s->data[i+1]) < abs(s->data[i])) {
			//printf("peak at %d of %d\n", i, s->data[i]);
			for (j = last_vol+1; j <= i; j++)
				vol[j] = abs(s->data[i]);
			last_vol = i;
		}
	}
	for (i = last_vol + 1; i < s->length; i++)
		vol[i] = vol[last_vol];

	/* Calculate 8 averaged volumes per instrument. */
	for (i = 0; i < s->length-volavgsize+1; i+=volavgsize) {
		avgvol[i/volavgsize] = 0;
		for (j = i; j < i+volavgsize; j++) {
			//printf("%4d ", s->data[j]);
			avgvol[i/volavgsize] += vol[j];
		}
		avgvol[i/volavgsize] /= volavgsize;
		//printf("avgvol %d: %d\n", i, avgvol[i/volavgsize]);
	}
#if DEBUG > 1
	dump_curve(avgvol, avgsize, 4);

	/* Smooth the volumes a bit. */
	printf("smoothed\n");
#endif
	avgvol[0] = (avgvol[0] + avgvol[1]) / 2;
	for (i = 1; i < avgsize - 1; i++) {
		avgvol[i] = (avgvol[i-1] + avgvol[i] + avgvol[i+1]) / 3;
	}
	avgvol[avgsize-1] = (avgvol[avgsize-1] +
			     avgvol[avgsize-2]) / 2;
#if DEBUG > 1
	dump_curve(avgvol, avgsize, 4);
#endif
	memcpy(s->env_data, avgvol, sizeof(s->env_data));
	s->max_env = 0;
	s->avg_env = 0;
	for (i = 0; i < avgsize; i++) {
		if (avgvol[i] > s->max_env)
			s->max_env = avgvol[i];
		s->avg_env += avgvol[i];
	}
	s->avg_env /= avgsize;

	/* Detect volume envelope. */
	int has_strong_up = 0;
	int has_up = 0;
	int has_down = 0;
	int has_strong_down = 0;
	int has_sustain = 0;
	int sustain_before_down = 0;
	int down_before_up = 0;
	int up_after_down = 0;
	for (i = 1; i < avgsize; i++) {
		double diff = (avgvol[i] - avgvol[i-1]) / (double)avgvol[i-1];
		if (diff > .02) {
			if (diff > .3) {
				PRINTENV("U");
				has_strong_up++;
			}
			else {
				PRINTENV("u");
				has_up++;
			}
			if (has_down || has_strong_down)
				up_after_down = 1;
		}
		else if (diff < -.02) {
			if (diff < -.3) {
				PRINTENV("D");
				has_strong_down++;
			}
			else {
				PRINTENV("d");
				has_down++;
			}
			if (!has_up && !has_strong_up)
				down_before_up = 1;
		}
		else {
			PRINTENV("-");
			has_sustain++;
			if (!has_down && !has_strong_down)
				sustain_before_down = 1;
		}
	}
	PRINTENV("\n");

	/* Try to match the volume envelope to a predefined MML envelope. */
	if ((has_strong_down || has_down) &&
	    !(has_up || has_strong_up) &&
	    !(has_sustain && sustain_before_down)) {
		/* continuous decay */
		if (has_strong_down > has_down)
			s->envelope = 6;
		else
			s->envelope = 4;
	}
	else if ((has_up || has_strong_up) &&
		 (has_down||has_strong_down) &&
		 !down_before_up && !up_after_down) {
		/* attack/decay */
		s->envelope = 7;
	}

	PRINTENV("env %d\n", s->envelope);

	/* ===== Extract Wave Data ===== */

	/* Try to find cycles suitable for use as a custom wave.
	   This works better than one might think because the
	   cycle length in Protracker MODs is often around 32
	   samples. */
	wave_info waves[s->length / 32];
	memset(waves, 0, sizeof(waves));

	int wc = 0;

	int max = 0;	/* Highest sample in current cycle. */
	int min = 0;	/* Lowest sample in current cycle. */
	int pos = 0;	/* Number of positive samples. */
	int neg = 0;	/* Number of negative samples. */
	int len;	/* Cycle length. */
	/* Position the wave last crossed 0 in an upwards direction;
	   in other words: the start of the last cycle. */
	int lastcross = 0;
	/* Iterate over all samples and try to find cycles. */
	for (i = 1; i < s->length - 1; i++) {
#if DEBUG > 1
		printf("%3d ", s->data[i]);
		dump_value(s->data[i], 2, 1);
#endif

		/* Record minima and maxima and number of positive and
		   negative samples; this is later used to assess cycle
		   quality. */
		if (s->data[i] > max)
			max = s->data[i];
		else if (s->data[i] < min)
			min = s->data[i];
		if (s->data[i] < 0)
			neg++;
		else
			pos++;

		/* Crossing 0 can happen in two ways: a 0 sample than is
		   preceded by a lower sample and succeeded by a higher one,
		   or a positive sample preceded by a negative one. */
		if (s->data[i] == 0) {
			/* Find the preceding non-zero sample. */
			/* XXX: Isn't it safe to assume that that's always
			   the immediately preceding sample? */
			j = i-1;
			while (s->data[j] == 0 && j > 0)
				j--;
			/* If that is a negative sample, we may be crossing
			   the time axis in an upward direction. */
			if (s->data[j] < 0) {
				/* Find the succeeding non-zero sample. */
				j = i+1;
				while (s->data[j] == 0 && j < s->length - 1)
					j++;
				/* If this is a positive sample, we're in. */
				if (s->data[j] > 0) {
					PRINTWAV("slownullcross up at %d\n", i);
					goto cross;
				}
			}
		}
		else if (s->data[i] > 0 && s->data[i-1] < 0) {
			PRINTWAV("fastnullcross up at %d\n", i);
			goto cross;
		}
		/* These are not the samples you're looking for. */
		continue;
cross:
		/* Check if the samples from the last crossing to this one
		   are suitable as a custom wave. */
		len = i - lastcross;
		if (max > 30 && min < -30 &&	/* minimum volume */
			/* at most 1 byte deviation from the desired length */
		    len >= 31 && len <= 33 &&
			/* some degree of symmetry */
		    neg > 10 && pos > 10) {
			PRINTWAV("found wave in \"%s\" from %d size %d [%d-%d]\n", s->name, lastcross, len, min, max);
			waves[wc].start = lastcross;
			waves[wc].end = i;
			waves[wc].interpol = -1;
			/* If the cycle is not exactly 32 samples, we need
			   to make some adjustments. */
			if (len > 32) {
				/* Drop the sample the absence of which results
				   in the smallest gap between the start and
				   end of the cycle. */
				if (abs(s->data[lastcross] - s->data[i-2]) <
				    abs(s->data[lastcross+1] - s->data[i-1])) {
					PRINTWAV("cutting wave at the end\n");
					waves[wc].end--;
				}
				else {
					PRINTWAV("cutting wave at the start\n");
					waves[wc].start++;
				}
			}
			else if (len < 32) {
				/* Add the preceding or succeeding sample if
				   doing so does not result in a downward
				   spike. If that happens in both cases,
				   interpolate the missing sample. */
				if (s->data[i] < s->data[lastcross]) {
					PRINTWAV("extending wave at the end\n");
					waves[wc].end++;
				}
				else if (s->data[lastcross - 1] > s->data[i-1]) {
					PRINTWAV("extending wave at the start\n");
					waves[wc].start--;
				}
				else {
					waves[wc].interpol = (s->data[lastcross] + s->data[i-1]) / 2;
					PRINTWAV("interpolate missing sample %d\n", waves[wc].interpol);
				}
			}
			/* Record a number of properties that will be used
			   to determine cycle quality. */
			waves[wc].end_dist = abs(s->data[waves[wc].start] -
						 s->data[waves[wc].end-1]);
			waves[wc].asymmetry_time = abs(pos - neg);
			waves[wc].asymmetry_vol = abs(abs(min) - max);
			waves[wc].vol = max > -min ? max : -min;
			/* Calculate a score; gives heavy emphasis on
			   the difference between the first and last
			   samples to avoid clicks. */
			waves[wc].score = -waves[wc].end_dist * 12
					  - waves[wc].asymmetry_time
					  - waves[wc].asymmetry_vol
					  + waves[wc].vol * 2;

			wc++;
		}
		lastcross = i;
		min = max = pos = neg = 0;
	}

	if (wc) {
		/* Find the wave with the best quality. */
		int hiscore = -10000;
		int hiwave = -1;
#if DEBUG > 0
		printf("Wave summary for %s:\n", s->name);
#endif
		for (i = 0; i < wc; i++) {
#if DEBUG > 0
			printf("wave %d at %d: score %d (vol %d, dist %d, asymt %d, asymv %d)\n",
			       i, waves[i].start, waves[i].score, waves[i].vol,
			       waves[i].end_dist, waves[i].asymmetry_time, waves[i].asymmetry_vol);
#endif
			if (waves[i].score > hiscore) {
				hiscore = waves[i].score;
				hiwave = i;
			}
		}
#if DEBUG > 0
		printf("Best wave: %d\n", hiwave);
#endif
		/* If a suitable wave has been found, add it to the sample. */
		if (hiwave != -1) {
			s->wave = (wave_info *)malloc(sizeof(wave_info));
			*s->wave = waves[hiwave];
		}
	}
}

