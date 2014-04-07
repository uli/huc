#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void
print_error (char *format, ...);

void
log_warning (char *format, ...);

typedef struct {
    int period; /* -1 for rest, else index in the period table */
    int instrument; /* only predefined waveform supported,
		     * 0 -> sine
		     * 1 -> saw tooth
		     * 2 -> square
		     * or similar :)
		     */
    int volume; /* maximum is 0x40 */
    int last_note; /* number of elapsed row since last note releasing */
    int percussion;
    unsigned char panning; /* 0 is left, 0xff for right */
    unsigned char col_written; /* number of operandes written on the curent line */
    unsigned char mode;
} Channel_info;

const unsigned char standard_waves[45][32];
