/* ISOLINK
 *
 * This program appends the IPL.BIN file together with
 * a list of binary segments which may be overlays or
 * data, and produces an ISO output file
 *
 * An array of segment information is also produced and
 * stored (I'm not sure where yet)
 *
 */


/*************/
/* INCLUDES  */
/*************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "main.h"
#include "../../include/overlay.h"


/*************/
/* DEFINES   */
/*************/

#define	OVERLAY_SUFFIX	"ovl"


/*************/
/* GLOBALS   */
/*************/

int sector_array[200][2];
int array_count;
int cderr_flag = 0;
int cderr_ovl = 0;
static char incpath[10][256];
static int debug;


/*************/
/* CODE      */
/*************/

void
init_path(void)
{
   const char *p, *pl;
   int  i, l;

   strcpy(incpath[0], ".");
   strcat(incpath[0], PATH_SEPARATOR_STRING);

   p = getenv("PCE_INCLUDE");

   if (p == NULL) {
      p =
#ifdef WIN32
	 "c:\\huc\\include\\pce",
#else
         "/usr/local/lib/huc/include/pce;" \
         "/usr/local/huc/include/pce;" \
         "/usr/local/share/huc/include/pce;" \
         "/usr/local/include/pce;" \
         "/usr/lib/huc/include/pce;" \
         "/usr/share/huc/include/pce;" \
         "/usr/include/pce"
#endif
      ;
   }

   for (i = 1; i < 10; i++)
   {
      pl = strchr(p, ';');

      if (pl == NULL)
         l = strlen(p);
      else
         l = pl - p;

      if (l) {
         strncpy(incpath[i], p, l);
         p += l;
         while (*p == ';')
            p++;
      }
      incpath[i][l] = '\0';

      if (l) {
         if (incpath[i][l - 1] != PATH_SEPARATOR)
            strcat(incpath[i], PATH_SEPARATOR_STRING);
      }
   }
}


FILE *
file_open(char * name, char * mode)
{
   FILE * fp = NULL;
   char testname[256];
   int i;

/* search current directory, then PCE_INCLUDE path */

   for (i = 0; i < 10; i++) {
      if (strlen(incpath[i])) {
         strcpy(testname, incpath[i]);
         strcat(testname, name);
         fp = fopen(testname, mode);
         if (fp != NULL) {
            if (debug) {
               printf("filename      = %s\n", testname);
            }
            break;
         }
      }
   }

   return(fp);
}


void
file_write(FILE *outfile, FILE *infile, char * filename, int curr_filenum)
{
   char buffer[2048];
   long size;
   int  sectors;
   int bytes_read, bytes_written;
   int i, j;
   int code;


   code = 0;
   if (strcmp(OVERLAY_SUFFIX, &filename[strlen(filename)-3]) == 0) {
      code = 1;
   }

   /* check size of file */

   fseek(infile, 0L, SEEK_END);
   size = ftell(infile);
   rewind(infile);


   /* get proper number of sectors if not an exact size multiple */

   sectors = size/2048;
   if ((sectors * 2048) != size) {
      sectors++;
   }

   for (i = 0; i < sectors; i++)
   {
      bytes_read = fread((void *)buffer, 1, 2048, infile);

      if (bytes_read != 2048)
      {
         /* wrong place to get an incomplete read */

         if ( ((sectors*2048) == size) || ((i+1) != sectors) )
         {
             printf("Error while reading file %s\n", filename);
             exit(1);
         }
         else
         {
            /* fill buffer with zeroes */

            for (j = bytes_read; j < 2048; j++)
            {
               buffer[j] = 0;
            }
         }
      }

      if (code == 1)
      {
         if (i == 0)	/* ie. boot segment */
         {
            /* This byte is the place where the overlay entry point */
            /* declares "I am overlay number <n>", and now running  */

            buffer[1] = curr_filenum;

            if ((cderr_flag == 1) && (curr_filenum == 1))
            {
               buffer[(CDERR_OVERRIDE & 0x07FF)] = 1;
               buffer[(CDERR_OVERLAY_NUM & 0x07FF)] = cderr_ovl << 2;
            }
         }
         else if (i == DATA_SECTOR)
         {
            for (j = 0; j < array_count; j++)
            {

               /* sector_array[0] is ipl.bin which is a segment    */
               /* but not an addressable one - still, it is stored */
               /* Encode this array into DATA_SEGMENT of all code  */
               /* overlays on disk, in Hu6280 addressable order    */

               buffer[j*4]   = (sector_array[j][0]) & 255;
               buffer[j*4+1] = (sector_array[j][0]) >> 8;
               buffer[j*4+2] = (sector_array[j][1]) & 255;
               buffer[j*4+3] = (sector_array[j][1]) >> 8;
            }
         }
      }

      bytes_written = fwrite((void *)buffer, 1, 2048, outfile);

      if (bytes_written != 2048)
      {
          printf("Error writing output file while processing %s\n", filename);
          exit(1);
      }
   }
}


void
ipl_write(FILE *outfile, FILE *infile, char * name)
{
   char ipl_buffer[4096];
   long size;

   /* check size of file */

   fseek(infile, 0L, SEEK_END);
   size = ftell(infile);
   rewind(infile);

   if (size > 4096) {
      printf("ipl.bin file is wrong size - %ld, should be 4096\n", size);
      exit(1);
   }

   fread(ipl_buffer, 1, 4096, infile);

   memset(&ipl_buffer[0x800], 0, 32);

   /* prg sector base */
   ipl_buffer[0x802] = 2;

   /* nb_sectors */
   ipl_buffer[0x803] = 16;	/* Get boot segments first; up to and including */
				/* overlay array.  The boot segments will load */
				/* the remaining segments and relocate code if */
				/* necessary           */

   /* loading address */
   ipl_buffer[0x804] = 0x00;
   ipl_buffer[0x805] = 0x40;

   /* starting address */
   ipl_buffer[0x806] = (BOOT_ENTRY_POINT & 0xff);	/* boot entry point */
   ipl_buffer[0x807] = (BOOT_ENTRY_POINT >> 8) & 0xff;

   /* mpr registers */
   ipl_buffer[0x808] = 0x00;
   ipl_buffer[0x809] = 0x01;
   ipl_buffer[0x80A] = 0x02;
   ipl_buffer[0x80B] = 0x03;
   ipl_buffer[0x80C] = 0x00;	/* boot loader also @ $C000 */

   /*load mode */
   ipl_buffer[0x80D] = 0x60;

   fwrite(ipl_buffer, 1, 4096, outfile);
}


void
zero_write(FILE *outfile, int sectors)
{
   char zero_buf[2048];
   int i;

   memset(zero_buf, 0, 2048);
   for (i = 0; i < sectors; i++)
   {
      fwrite(zero_buf, 1, 2048, outfile);
   }
}


void
usage(void)
{
   printf("Usage: ISOLINK <outfile> <infile_1> <infile_2> -cderr <infile_n>. . .\n");
   printf("\n\n");
   printf("-cderr :  Indicates that the following overlay is to be used \n");
   printf("          instead of the default text message when SCD programs\n");
   printf("          are executed on plain CD systems\n\n");
   printf("          Note: this overlay must be compiled as '-cd', not '-scd'\n\n");
}


int
main(int argc, char *argv[])
{
   int i, j;
   int curr_sector, sectors, zero_fill;
   long file_len;
   FILE * infile;
   FILE * outfile;

   debug = 0;
   array_count = 0;
   curr_sector = 0;
   j = 0;

   init_path();

   /********************************************************/
   /* parse command-line                                   */
   /* if any sort of command-line options are found, abort */
   /*                                                      */
   /* otherwise, step through filenames testing them       */
   /* for existence, and creating an array which holds     */
   /* size and number of sectors                           */
   /********************************************************/

   for (i = 1; i < argc; i++)
   {
      if (argv[i][0] == '-')
      {
         if ((strcmp(argv[i], "-cderr") == 0) &&
             (i != 1) &&		/* not valid for first arg */
             (i < (argc-1)) &&		/* must have filename after */
             (cderr_flag == 0))		/* only valid once on line */
         {
            cderr_flag = 1;
            cderr_ovl = j;
            continue;
         }
         else
         {
            usage();
            exit(1);
         }
      }

      if (i == 1) {
         infile = file_open("ipl.bin", "rb");
      } else {
         infile = file_open(argv[i], "rb");
      }

      if (infile == NULL)
      {
         printf("Could not open file: %s\n", (i == 1 ? "ipl.bin" : argv[i]));
         printf("Operation aborted\n\n");
         exit(1);
      }

      fseek(infile, 0L, SEEK_END);
      file_len = ftell(infile);
      rewind(infile);

      sectors = (file_len/2048);
      if ((sectors * 2048) != file_len) {
         sectors++;
      }

      if (debug) {
         printf("length = %ld\n", file_len);
         printf("start sector  = %d\n", curr_sector);
         printf("len (sectors) = %d\n", sectors);
      }

      sector_array[j][0] = curr_sector;
      sector_array[j][1] = sectors;
      curr_sector += sectors;
      j++;

      fclose(infile);
   }

   array_count = j;

   /* OK, let's open them for real now   */
   /* and copy them from input to output */

   outfile = fopen(argv[1], "wb");

   infile = file_open("ipl.bin", "rb");
   ipl_write(outfile, infile, "ipl.bin");
   fclose(infile);

   j = 0;
   for (i = 2; i < argc; i++)
   {
      if (argv[i][0] != '-')
      {
         j++;
         infile = file_open(argv[i], "rb");
         file_write(outfile, infile, argv[i], j);
         fclose(infile);
      }
   }

   /* pad it out to 6 seconds to comply */
   /* with CDROM specification */

   zero_fill = (6*75)-curr_sector;

   /* pad at least 2 seconds of trailing zeroes */

   if (zero_fill < (2*75))
	zero_fill = 2*75;

   zero_write(outfile, zero_fill);

   fclose(outfile);
   
   return(0);
}
