#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shared.h"
#define SCR_W 320
#define SCR_H 240

unsigned char *pixels;
char *rom_name;

void fint(FILE *fp, int v)
{
	fwrite(&v, 4, 1, fp);
}
void fshort(FILE *fp, short v)
{
	fwrite(&v, 2, 1, fp);
}

void dump_screen(void)
{
	FILE *fp;
	int i;
	/* Random BMP header bits, extracted from a file created by GIMP. */
	const char hdr[] = {
		0x13, 0x0b , 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8 , 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x1f, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x47 , 0x52, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00, 0x00,
	};

	char *scrname = strdup(rom_name);
	*strrchr(scrname, '.') = 0;
	strcat(scrname, ".bmp");

	fp = fopen(scrname, "r");
	if (!fp) {
		fprintf(stderr, "no reference screen found, creating one\n");
		/* Write screen data as BMP file. */
		fp = fopen(scrname, "w");
		fprintf(fp, "BM");
		fint(fp, SCR_W * SCR_H * 2 + 0x8a);	/* file size */
		fint(fp, 0);
		fint(fp, 0x8a);	/* pixel data offset */
		fint(fp, 0x7c);	/* DIB header size */
		fint(fp, SCR_W);
		fint(fp, SCR_H);
		fshort(fp, 1);	/* planes */
		fshort(fp, 16);	/* bpp */
		fint(fp, 3);	/* compression (none) */
		fint(fp, SCR_W * SCR_H * 2);	/* pixel data size */
		fwrite(hdr, sizeof(hdr), 1, fp);	/* random crap */
		/* lines have to be written backwards... */
		for (i = SCR_H - 1; i >= 0; i--)
			fwrite(pixels + i * SCR_W * 2, SCR_W * 2, 1, fp);
		fclose(fp);
		exit(2);
	}
	unsigned char *refpixels = malloc(SCR_W * SCR_H * 2);
	fseek(fp, 0x8a, SEEK_SET);	/* skip to pixel data */
	fread(refpixels, SCR_W * SCR_H * 2, 1, fp);
	fclose(fp);
	/* Lines in BMP file are reversed. */
	for (i = 0; i < SCR_H; i++) {
		if (memcmp(pixels + i * SCR_W * 2, refpixels + (SCR_H - i - 1) * SCR_W * 2, SCR_W * 2)) {
			fprintf(stderr, "screen differs from reference\n");
			exit(1);
		}
	}
	exit(0);
}

int main(int argc, char **argv)
{
    int res;
    if (argc != 2)
	abort();
	fprintf(stderr, "loading ROM\n");
	rom_name = argv[1];
	res = load_rom(rom_name, 0, 0);
	if (res != 1) {
		fprintf(stderr, "failed to load ROM: %d\n", res);
		return -1;
	}
    pixels = calloc(SCR_W * SCR_H * 2, 1);
    bitmap.width = SCR_W;
    bitmap.height = SCR_H;
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
