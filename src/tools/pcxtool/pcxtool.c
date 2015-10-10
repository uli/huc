/*
 * PCXTOOL - to manipulate some attributes of PCX files according to
 *           the special requirements of PC Engine developers
 */


#define	MAX_X	512
#define	MAX_Y	512
#define	MAX_PAL	256


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined _WIN32 || defined _WIN64
#define strcasecmp stricmp
#endif


/* PCX header info */

typedef struct
{
  char   format_id;
  char   version;
  char   compress;
  char   bpp;
  short  x_min;
  short  y_min;
  short  x_max;
  short  y_max;
  short  hres;
  short  vres;
  char   palette16[48];
  char   reserved1;
  char   planes;
  short  bytes_line;
  short  palette_type;
  char   reserved2[58];
} pcx_header_t;

typedef struct
{
   pcx_header_t header;
  
   /* for now, maximum size of image = 256 x 256 pixels */
   unsigned char pixel[MAX_X*MAX_Y];

   unsigned int palette_reference[MAX_PAL];

   unsigned char pal_r[MAX_PAL];
   unsigned char pal_g[MAX_PAL];
   unsigned char pal_b[MAX_PAL];
} pcx_t;

pcx_t pcx;

/* command-line options */

int showref, dump_palette;
int swap, swapfrom, swapto;
int pcepal;
int fixmask, maskcolor;
int reverse;
int append, append_paletteslot;
const char* append_file;


/* and now for the program */

int
pcx_width( pcx_t *in ) {
   return in->header.x_max - in->header.x_min + 1;
}


int
pcx_height( pcx_t *in ) {
   return in->header.y_max - in->header.y_min + 1;
}


void
error(char *string, long pos)
{
   printf("\n");
   printf(string);
   if (pos >= 0)
   {
      printf("\n");
      printf("At position %ld in file", pos);
   }
   printf("\n\n");
   exit(1);
}


void
init(void)
{
int i;
int j;

   memset(&pcx.header, 0, 128);

   for (i = 0; i < MAX_PAL; i++)
   {
      pcx.palette_reference[i] = 0;

      pcx.pal_r[i] = 0;
      pcx.pal_g[i] = 0;
      pcx.pal_b[i] = 0;
   }

   for (i = 0; i < MAX_X; i++)
   {
      for (j = 0; j < MAX_X; j++)
      {
         pcx.pixel[i*MAX_X+j] = 0;
      }
   }
}


void
usage(void)
{
   printf("pcxtool: Manipulate pcx images for PC Engine developers\n\n");
   printf("Usage:\n");
   printf("pcxtool [options] <input> [<output>]\n\n");
   printf("Input file will not be changed; output file creation is optional\n\n");
   printf("Options:\n");
   printf("-dump          : display all palette values\n");
   printf("-help          : show this help\n");
   printf("-pcepal        : truncate palette to 3-bits (ie. $00,$20,...,$E0)\n");
   printf("-pcepal2       : snap palette to nearest of 8 linear values (ie. $00,$24,$49,...,$FF)\n");
/*   printf("-rngexp        : expand palette to full range before applying pce color depth cutoff\n"); */
   printf("-ref           : show referenced palette values\n");
   printf("-swap a b      : swap palette order of 2 palette entries without changing appearance\n");
   printf("                 of picture. 'a' and 'b' may be decimal values (no prefix) or\n");
   printf("                 hexadecimal (prefixed by '$' or '0x')\n");
   printf("-fixmask color : ensures that 'color', if used, is the first color in all 16 sub-palettes.\n");
   printf("                 'color' may be a decimal value (no prefix) or hexadecimal (prefixed by '$' or '0x'\n");
   printf("-reverse       : reverse palette entries without changing the appearance (photoshop fixup)\n");
   printf("-append f p    : append another pcx of the same width. 'f' is a pcx file. 'p' is a value from 0-15\n");
   printf("                 where to store the source palette (first 16 palette values)\n");
}


int
get_val(char *val)
{
int i;
int base, startidx, cumul_val, temp;

   cumul_val = 0;

   if (val[0] == '$')
   {
      base = 16;
      startidx = 1;
   }
   else if  ((val[0] == '0') && (val[1] == 'x'))
   {
      base = 16;
      startidx = 2;
   }
   else
   {
      base = 10;
      startidx = 0;
   }

   cumul_val = 0;

   for (i = startidx; i < strlen(val); i++)
   {
      if ((val[i] >= '0') && (val[i] <= '9'))
      {
         temp = val[i] - '0';
         cumul_val = cumul_val * base + temp;
      }
      else if ((val[i] >= 'A') && (val[i] <= 'F') && (base == 16))
      {
         temp = val[i] - 'A' + 10;
         cumul_val = cumul_val * base + temp;
      }
      else if ((val[i] >= 'a') && (val[i] <= 'f') && (base == 16))
      {
         temp = val[i] - 'a' + 10;
         cumul_val = cumul_val * base + temp;
      }
      else break;
   }

   return(cumul_val);
}


void
swap_palette(void)
{
int x, y, temp;

   for (y=0; y < (pcx.header.y_max-pcx.header.y_min+1); y++)
   {
      for(x=0; x < pcx.header.bytes_line; x++)
      {
         if (pcx.pixel[y*MAX_X+x] == swapfrom)
         {
            pcx.pixel[y*MAX_X+x] = swapto;
         }
         else if (pcx.pixel[y*MAX_X+x] == swapto)
         {
            pcx.pixel[y*MAX_X+x] = swapfrom;
         }
      }
   }

   temp                = pcx.pal_r[swapfrom];
   pcx.pal_r[swapfrom] = pcx.pal_r[swapto];
   pcx.pal_r[swapto]   = temp;

   temp                = pcx.pal_g[swapfrom];
   pcx.pal_g[swapfrom] = pcx.pal_g[swapto];
   pcx.pal_g[swapto]   = temp;

   temp                = pcx.pal_b[swapfrom];
   pcx.pal_b[swapfrom] = pcx.pal_b[swapto];
   pcx.pal_b[swapto]   = temp;

   temp                            = pcx.palette_reference[swapfrom];
   pcx.palette_reference[swapfrom] = pcx.palette_reference[swapto];
   pcx.palette_reference[swapto]   = temp;
}


void
fixmaskcolor(void)
{
int i, j;
int maskr = (maskcolor >> 16) & 0xFF;
int maskg = (maskcolor >> 8) & 0xFF;
int maskb = maskcolor & 0xFF;
int num_swapped = 0;

   for (i=0; i < MAX_PAL; i += 16)
   {
      for (j=i + 1; j < i + 16; j++)
      {
         if (pcx.pal_r[j] == maskr && pcx.pal_g[j] == maskg && pcx.pal_b[j] == maskb)
         {
            swapto = i;
            swapfrom = j;
            swap_palette();
            num_swapped++;
            break;
         }         
      }
   }

   if (num_swapped == 0)
   {
      printf("\nNo color found that matches '0x%X'\n", maskcolor);
   }
}

void
reverse_palette(void)
{
int i;
int j;

   for (i = 0; i < MAX_PAL / 2; i++)
   {
      j = MAX_PAL - i - 1;
      swapto = i;
      swapfrom = j;
      swap_palette();
   }
}

void
pcepal_adjust(void)
{
int i;

   for (i=0; i < MAX_PAL; i++)
   {
      if (pcepal == 1)
      {
         pcx.pal_r[i] = pcx.pal_r[i] & 0xE0;
         pcx.pal_g[i] = pcx.pal_g[i] & 0xE0;
         pcx.pal_b[i] = pcx.pal_b[i] & 0xE0;
      }
      else if (pcepal == 2)
      {
         pcx.pal_r[i] = (pcx.pal_r[i] & 0xE0) * 255 / 224;
         pcx.pal_g[i] = (pcx.pal_g[i] & 0xE0) * 255 / 224;
         pcx.pal_b[i] = (pcx.pal_b[i] & 0xE0) * 255 / 224;
      }
   }
}


void
read_pcx(FILE *in, pcx_t *dest)
{
int x,y;
int repeat, temp;

   temp = fread(&dest->header, 1, 128, in);
   if (temp < 128)
      error("read_pcx: read header short", ftell(in));

   for (y=0; y < (dest->header.y_max-dest->header.y_min+1); y++)
   {
      x = 0;

      while (x < dest->header.bytes_line)
      {
         temp = fgetc(in);

         if (temp == EOF)
            error("read_pcx: get byte", ftell(in));

         if ((dest->header.compress == 1) && ((temp & 0xC0) == 0xC0))
         {
            repeat = temp & 0x3F;

            temp = fgetc(in);
            if (temp == EOF)
               error("read_pcx: get repeated byte", ftell(in));

            dest->palette_reference[temp]++;
            while (repeat > 0)
            {
               dest->pixel[y*MAX_X+x] = temp;
               repeat--;
               x++;
            }
         }
         else
         {
            dest->palette_reference[temp]++;
            dest->pixel[y*MAX_X+x] = temp;
            x++;
         }
      }
   }

   temp = fgetc(in);
   if (temp == EOF)
      error("read_pcx: no palette", ftell(in));
   if (temp != 0x0c)
      error("read_pcx: unexpected byte instead of palette ID 0x0c", ftell(in));

   for (x = 0; x < MAX_PAL; x++)
   {
      temp = fgetc(in);
      if (temp == EOF)
         error("read_pcx: end in palette loop - r", ftell(in));
      dest->pal_r[x] = temp;

      temp = fgetc(in);
      if (temp == EOF)
         error("read_pcx: end in palette loop - g", ftell(in));
      dest->pal_g[x] = temp;
      
      temp = fgetc(in);
      if (temp == EOF)
         error("read_pcx: end in palette loop - b", ftell(in));
      dest->pal_b[x] = temp;
   }
}

void
append_pcx(void)
{
FILE *infile;
pcx_t pcx2;
int i;
int width;
int height;
int height2;

   infile = fopen(append_file, "rb");
   if (infile == NULL)
      error("append_pcx: couldn't open infile", -1);

   read_pcx(infile, &pcx2);
   fclose(infile);

   if (pcx_width(&pcx) != pcx_width(&pcx2))
      error("append_pcx: width must be the same in both pcx files", -1);

   width = pcx_width(&pcx);
   height = pcx_height(&pcx);
   height2 = pcx_height(&pcx2);

   pcx.header.y_max += height2;
   if (pcx_height(&pcx) > MAX_Y)
      error("append_pcx: new height exeeded internal maximum height", -1);

   // Adjust pixels to the new palette
   for (i=MAX_X * MAX_Y; i--;)
      pcx2.pixel[i] += append_paletteslot * 16;

   // Copy rows
   unsigned char *dest = pcx.pixel + MAX_X * height;
   unsigned char *src = pcx2.pixel;
   for (i=height2; i--;)
   {
      memcpy(dest, src, width);
      dest += MAX_X;
      src += MAX_X;
   }

   memcpy(pcx.pal_r + append_paletteslot * 16, pcx2.pal_r, 16);
   memcpy(pcx.pal_g + append_paletteslot * 16, pcx2.pal_g, 16);
   memcpy(pcx.pal_b + append_paletteslot * 16, pcx2.pal_b, 16);
}


void
write_pcx(FILE *out)
{
int x,y;
int repeat, temp;

   pcx.header.compress = 1;

   temp = fwrite(&pcx.header, 1, 128, out);
   if (temp < 128)
      error("write_pcx: write header short", ftell(out));

   for (y=0; y < (pcx.header.y_max-pcx.header.y_min+1); y++)
   {
      x = 0;
      repeat = 1;

      while (x < pcx.header.bytes_line)
      {
         temp = pcx.pixel[y*MAX_X+x];

         if ( (repeat == 0x3F) ||
              (x >= pcx.header.bytes_line-1) ||
              (temp != pcx.pixel[y*MAX_X+x+1]) )
         {
            if ((repeat > 1) || ((temp & 0xc0) == 0xc0))
            {
               if (fputc((repeat | 0xc0), out) == EOF)
                  error("write_pcx: error writing repeater", ftell(out));
            }

            if (fputc(temp, out) == EOF)
               error("write_pcx: error writing byte", ftell(out));
            repeat = 1;
            x++;
         }
         else
         {
            repeat++;
            x++;
         }
      }
   }

   fputc(0x0c, out);

   for (x = 0; x < MAX_PAL; x++)
   {
      fputc(pcx.pal_r[x], out);
      fputc(pcx.pal_g[x], out);
      fputc(pcx.pal_b[x], out);
   }
}


int
main(int argc, char ** argv)
{
char *filename1, *filename2;
char *ref_string;
FILE *infile, *outfile;
int argcnt;
int i, palcount;
int cmd_err;

/* initialize global command-line variables */

   showref = 0;
   dump_palette = 0;

   swap = 0;
   swapfrom = 0;
   swapto = 0;

   cmd_err = 0;

   init();


   printf("argc = %d\n", argc);
   printf("argv[1] = %s\n", argv[1]);

   argcnt = 1;

   while ((argcnt < argc) && (argv[argcnt][0] == '-'))
   {
      if (strcasecmp(argv[argcnt], "-help") == 0)
      {
         usage();
         exit(0);
      }
      else if (strcasecmp(argv[argcnt], "-pcepal2") == 0)
      {
         pcepal = 2;
         argcnt++;
      }
      else if (strcasecmp(argv[argcnt], "-pcepal") == 0)
      {
         pcepal = 1;
         argcnt++;
      }
      else if (strcasecmp(argv[argcnt], "-dump") == 0)
      {
         dump_palette = 1;
         argcnt++;
      }
      else if (strcasecmp(argv[argcnt], "-ref") == 0)
      {
         showref = 1;
         argcnt++;
      }
      else if (strcasecmp(argv[argcnt], "-swap") == 0)
      {
         if ((argcnt + 3) >= argc)
         {
            cmd_err = 1;
            break;
         }
         swap = 1;
         argcnt++;
         swapfrom = get_val(argv[argcnt++]);
         swapto   = get_val(argv[argcnt++]);
         printf("swapfrom = %d\nswapto = %d\n", swapfrom, swapto);
      }
      else if (strcasecmp(argv[argcnt], "-fixmask") == 0)
      {
         if ((argcnt + 2) >= argc)
         {
            cmd_err = 1;
            break;
         }
         fixmask = 1;
         argcnt++;
         maskcolor = get_val(argv[argcnt++]);
         printf("maskcolor = %d\n", maskcolor);
      }
      else if (strcasecmp(argv[argcnt], "-reverse") == 0)
      {
         reverse = 1;
         argcnt++;
      }
      else if (strcasecmp(argv[argcnt], "-append") == 0)
      {
         append = 1;
         argcnt++;
         append_file = argv[argcnt++];
         append_paletteslot = get_val(argv[argcnt++]);

         if (append_paletteslot < 0 || append_paletteslot > 15)
            error("Second argument to -append must be a value from 0 - 15", -1);
      }
      else
      {
         cmd_err = 1;
         break;
      }
   }

   if ((argcnt >= argc) || (cmd_err != 0))
   {
      printf("\nWrong command-line parameters\n");
      usage();
      exit(1);
   }

   filename1 = argv[argcnt++];
   if (argcnt >= argc)
      filename2 = NULL;
   else
      filename2 = argv[argcnt];

   infile  = fopen(filename1, "rb");
   if (infile == NULL)
      error("main: couldn't open infile", 0);

   read_pcx(infile, &pcx);
   fclose(infile);


   if (swap)
   {
      swap_palette();
   }

   if (reverse)
   {
      reverse_palette();
   }

   if (fixmask)
   {
      fixmaskcolor();
   }

   if (pcepal)
   {
      pcepal_adjust();
   }

   if (append)
   {
      append_pcx();
   }

   palcount = 0;
   for (i = 0; i < MAX_PAL; i++)
   {
      if (pcx.palette_reference[i] > 0)
         palcount++;
   }

   if (dump_palette)
   {
      printf("\nPalette Entries:\n\n");

      for (i = 0; i < MAX_PAL; i++)
      {
         if (pcx.palette_reference[i] > 0)
            ref_string = "(referenced)";
         else
            ref_string = "";

         printf("Palette $%02X: RGB = %02X%02X%02X   %s\n",
                      i, pcx.pal_r[i], pcx.pal_g[i], pcx.pal_b[i], ref_string);
      }
   }
   else if (showref)
   {
      printf("\nList of Referenced Palette Entries:\n\n");

      for (i = 0; i < MAX_PAL; i++)
      {
         if (pcx.palette_reference[i] > 0)
            printf("Palette $%02X: RGB = %02X%02X%02X\n", i, pcx.pal_r[i], pcx.pal_g[i], pcx.pal_b[i]);
      }
   }

   printf("\nNumber of colours actually referenced = %d\n\n", palcount);

   if (filename2 != NULL)
   {
      outfile = fopen(filename2, "wb");
      if (outfile == NULL)
         error("main: couldn't open outfile", 0);

      write_pcx(outfile);
      fclose(outfile);
   }

   exit(0);
}

