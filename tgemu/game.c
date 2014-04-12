#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "shared.h"

int main(int argc, char **argv)
{
    int res;
    if (argc != 2)
	abort();
	fprintf(stderr, "loading ROM\n");
	res = load_rom(argv[1], 0, 0);
	if (res != 1) {
		fprintf(stderr, "failed to load ROM: %d\n", res);
		return -1;
	}
    unsigned char *pixels = malloc(480*272*2);
    bitmap.width = 480;
    bitmap.height = 272;
    bitmap.depth = 16;
    bitmap.granularity = (bitmap.depth >> 3);
    bitmap.data = pixels;
    bitmap.pitch = (bitmap.width * bitmap.granularity);
    bitmap.viewport.w = 256;
    bitmap.viewport.h = 240;
    bitmap.viewport.x = 0x20;
    bitmap.viewport.y = 0x00;
	
    fprintf(stderr, "system_init\n");
    system_init(44100);
    fprintf(stderr, "system_reset\n");
    system_reset();
	
	while (1) {
			bitmap.data = pixels;
			system_frame(0);
	}
	return 0;
}
