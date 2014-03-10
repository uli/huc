#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "shared.h"

int main(int argc, char **argv)
{
//    atexit(SDL_Quit);
    int res, fd;
	fprintf(stderr, "loading ROM\n");
	res = load_rom(argv[1], 0, 0);
	if (res != 1) {
		fprintf(stderr, "failed to load ROM: %d\n", res);
		close(fd);
		return 0;
	}
    //SDL_Surface *sf = SDL_SetVideoMode(480, 272, 16, SDL_HWSURFACE);
    char *pixels = malloc(480*272*2);
    bitmap.width = 480;
    bitmap.height = 272;
    bitmap.depth = 16;
    bitmap.granularity = (bitmap.depth >> 3);
    //uint8_t bitmap_buf[bitmap.width * bitmap.height * bitmap.granularity];
    bitmap.data = pixels;//getLCDShadowBuffer(); //bitmap_buf;
    bitmap.pitch = (bitmap.width * bitmap.granularity);
    bitmap.viewport.w = 256;
    bitmap.viewport.h = 240;
    bitmap.viewport.x = 0x20;
    bitmap.viewport.y = 0x00;
	
	fprintf(stderr, "system_init\n");
	system_init(44100);
	fprintf(stderr, "system_reset\n");
	system_reset();
	
	int skip = 0;
	while (1) {
			bitmap.data = pixels;
			system_frame(0);
			//memcpy(getLCDShadowBuffer(), bitmap_buf, getLCDWidth() * getLCDHeight() * 2);//sizeof(bitmap_buf));
			//for (i = 0; i < 256; i++) {
			//	memcpy(getLCDShadowBuffer() + i * getLCDWidth(), bitmap_buf + i * (32 + 512 + 32) * 2 + 64, 480 * 2);
			//}
#if 0
			SDL_Flip(sf);
			SDL_Event ev;
			while (SDL_PollEvent(&ev)) {
			    if (ev.type == SDL_QUIT)
			        return 0;
			}
#endif
	}
	return 0;
}
