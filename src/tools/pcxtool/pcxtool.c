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
} pcx_header;

pcx_header pcxhdr;


/* for now, maximum size of image = 256 x 256 pixels */

char pixel[MAX_X*MAX_Y];

char palette_reference[MAX_PAL];

unsigned char pal_r[MAX_PAL];
unsigned char pal_g[MAX_PAL];
unsigned char pal_b[MAX_PAL];

/* command-line options */

int showref, dump_palette;
int swap, swapfrom, swapto;
int pcepal;



/* and now for the program */

void
error(char *string, long pos)
{
   printf("\n");
   printf(string);
   printf("\n");
   printf("At position %ld in file", pos);
   printf("\n\n");
   exit(1);
}


void
init(void)
{
int i;
int j;

   memset(&pcxhdr, 0, 128);

   for (i = 0; i < MAX_PAL; i++)
   {
      palette_reference[i] = 0;

      pal_r[i] = 0;
      pal_g[i] = 0;
      pal_b[i] = 0;
   }

   for (i = 0; i < MAX_X; i++)
   {
      for (j = 0; j < MAX_X; j++)
      {
         pixel[i*MAX_X+j] = 0;
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
   printf("-dump     : display all palette values\n");
   printf("-help     : show this help\n");
   printf("-pcepal   : truncate palette to 3-bits (ie. $00,$20,...,$E0)\n");
   printf("-pcepal2  : snap palette to nearest of 8 linear values (ie. $00,$24,$49,...,$FF)\n");
/*   printf("-rngexp   : expand palette to full range before applying pce color depth cutoff\n"); */
   printf("-ref      : show referenced palette values\n");
   printf("-swap a b : swap palette order of 2 palette entries without changing appearance\n");
   printf("            of picture. 'a' and 'b' may be decimal values (no prefix), or\n");
   printf("            hexadecimal (prefixed by '$' or '0x')\n");
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

   for (y=0; y < (pcxhdr.y_max-pcxhdr.y_min+1); y++)
   {
      for(x=0; x < pcxhdr.bytes_line; x++)
      {
         if (pixel[y*MAX_X+x] == swapfrom)
         {
            pixel[y*MAX_X+x] = swapto;
         }
         else if (pixel[y*MAX_X+x] == swapto)
         {
            pixel[y*MAX_X+x] = swapfrom;
         }
      }
   }

   temp            = pal_r[swapfrom];
   pal_r[swapfrom] = pal_r[swapto];
   pal_r[swapto]   = temp;

   temp            = pal_g[swapfrom];
   pal_g[swapfrom] = pal_g[swapto];
   pal_g[swapto]   = temp;

   temp            = pal_b[swapfrom];
   pal_b[swapfrom] = pal_b[swapto];
   pal_b[swapto]   = temp;

   temp                        = palette_reference[swapfrom];
   palette_reference[swapfrom] = palette_reference[swapto];
   palette_reference[swapto]   = temp;
}


void
pcepal_adjust(void)
{
int i;

   for (i=0; i < MAX_PAL; i++)
   {
      if (pcepal == 1)
      {
         pal_r[i] = pal_r[i] & 0xE0;
         pal_g[i] = pal_g[i] & 0xE0;
         pal_b[i] = pal_b[i] & 0xE0;
      }
      else if (pcepal == 2)
      {
         pal_r[i] = (pal_r[i] & 0xE0) * 255 / 224;
         pal_g[i] = (pal_g[i] & 0xE0) * 255 / 224;
         pal_b[i] = (pal_b[i] & 0xE0) * 255 / 224;
      }
   }
}


void
read_pcx(FILE *in)
{
int x,y;
int repeat, temp;

   temp = fread(&pcxhdr, 1, 128, in);
   if (temp < 128)
      error("read_pcx: read header short", ftell(in));

   for (y=0; y < (pcxhdr.y_max-pcxhdr.y_min+1); y++)
   {
      x = 0;

      while (x < pcxhdr.bytes_line)
      {
         temp = fgetc(in);

         if (temp == EOF)
            error("read_pcx: get byte", ftell(in));

         if ((pcxhdr.compress == 1) && ((temp & 0xC0) == 0xC0))
         {
            repeat = temp & 0x3F;

            temp = fgetc(in);
            if (temp == EOF)
               error("read_pcx: get repeated byte", ftell(in));

            palette_reference[temp]++;
            while (repeat > 0)
            {
               pixel[y*MAX_X+x] = temp;
               repeat--;
               x++;
            }
         }
         else
         {
            palette_reference[temp]++;
            pixel[y*MAX_X+x] = temp;
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
      pal_r[x] = temp;

      temp = fgetc(in);
      if (temp == EOF)
         error("read_pcx: end in palette loop - g", ftell(in));
      pal_g[x] = temp;
      
      temp = fgetc(in);
      if (temp == EOF)
         error("read_pcx: end in palette loop - b", ftell(in));
      pal_b[x] = temp;
   }
}


void
write_pcx(FILE *out)
{
int x,y;
int repeat, temp;

   pcxhdr.compress = 1;

   temp = fwrite(&pcxhdr, 1, 128, out);
   if (temp < 128)
      error("write_pcx: write header short", ftell(out));

   for (y=0; y < (pcxhdr.y_max-pcxhdr.y_min+1); y++)
   {
      x = 0;
      repeat = 1;

      while (x < pcxhdr.bytes_line)
      {
         temp = pixel[y*MAX_X+x];

         if ( (repeat == 0x3F) ||
              (x >= pcxhdr.bytes_line) ||
              (temp != pixel[y*MAX_X+x+1]) )
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
      fputc(pal_r[x], out);
      fputc(pal_g[x], out);
      fputc(pal_b[x], out);
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

   read_pcx(infile);
   fclose(infile);


   if (swap)
   {
      swap_palette();
   }

   if (pcepal)
   {
      pcepal_adjust();
   }

   palcount = 0;
   for (i = 0; i < MAX_PAL; i++)
   {
      if (palette_reference[i] > 0)
         palcount++;
   }

   if (dump_palette)
   {
      printf("\nPalette Entries:\n\n");

      for (i = 0; i < MAX_PAL; i++)
      {
         if (palette_reference[i] > 0)
            ref_string = "(referenced)";
         else
            ref_string = "";

         printf("Palette $%02X: RGB = %02X%02X%02X   %s\n",
                      i, pal_r[i], pal_g[i], pal_b[i], ref_string);
      }
   }
   else if (showref)
   {
      printf("\nList of Referenced Palette Entries:\n\n");

      for (i = 0; i < MAX_PAL; i++)
      {
         if (palette_reference[i] > 0)
            printf("Palette $%02X: RGB = %02X%02X%02X\n", i, pal_r[i], pal_g[i], pal_b[i]);
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

