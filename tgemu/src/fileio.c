
#include "shared.h"
#include "pcecrc.h"
#include <fcntl.h>
#include <sys/stat.h>

/* Name of the loaded file */
char game_name[0x100];

/* split : 1= Split image (only needed for 512k versions of 384k images)
   flip  : 1= Bit-flip image (only for some TurboGrafx-16 images) */
int load_rom(char *filename, int split, int flip)
{
    #include "bitflip.h"
    uint8 header[0x200];
    uint8 *ptr = NULL, *buf = NULL;
    uint32 crc;
    int size, n;

    /* Default */
    strcpy(game_name, filename);

#if 0
    if(check_zip(filename))
    {
        unzFile *fd = NULL;
        unz_file_info info;
        int ret = 0;

        /* Attempt to open the archive */
        fd = unzOpen(filename);
        if(!fd) return (0);

        /* Go to first file in archive */
        ret = unzGoToFirstFile(fd);
        if(ret != UNZ_OK) {
            unzClose(fd);
            return (0);
        }

        /* Get information on the file */
        ret = unzGetCurrentFileInfo(fd, &info, game_name, 0x100, NULL, 0, NULL, 0);
        if(ret != UNZ_OK) {
            unzClose(fd);
            return (0);
        }

        /* Open the file for reading */
        ret = unzOpenCurrentFile(fd);
        if(ret != UNZ_OK) {
            unzClose(fd);
            return (0);
        }

        /* Allocate file data buffer */
        size = info.uncompressed_size;
        buf = malloc(size);
        if(!buf) {
            unzClose(fd);
            return (0);
        }

        /* Read (decompress) the file */
        ret = unzReadCurrentFile(fd, buf, info.uncompressed_size);
        if(ret != info.uncompressed_size)
        {
            free(buf);
            unzCloseCurrentFile(fd);
            unzClose(fd);
            return (0);
        }

        /* Close the current file */
        ret = unzCloseCurrentFile(fd);
        if(ret != UNZ_OK) {
            free(buf);
            unzClose(fd);
            return (0);
        }

        /* Close the archive */
        ret = unzClose(fd);
        if(ret != UNZ_OK) {
            free(buf);
            return (0);
        }
    }
    else
#endif
    {
        int gd = 0;

        /* Open file */
        if ((gd = open(filename, O_RDONLY, &gd)) < 0)
          return (2);

        struct stat st;
        fstat(gd, &st);
        /* Get file size */
        size = st.st_size; //gzsize(gd);

        /* Allocate file data buffer */
        buf = malloc(size);
        if(!buf) {
            close(gd);
            return (0);
        }

        /* Read file data */
        int res = 0;
        
        res = read(gd, buf, size);
        if (res != size)
            return 42;

        /* Close file */
        close(gd);
    }

    /* Check for 512-byte header */
    ptr = buf;
    if((size / 512) & 1)
    {
        memcpy(header, buf, 0x200);
        size -= 0x200;
        buf += 0x200;
    }

#if 0
    /* Generate CRC and print information */
    crc = crc32(0, buf, size);

    /* Look up game CRC in the CRC database, and set up flip and
       split options accordingly */
    int found = 0;
    for(n = 0; n < (sizeof(pcecrc_list) / sizeof(t_pcecrc)); n += 1)
    {
        if(crc == pcecrc_list[n].crc)
        {
            found = 1;
            return (int)crc;
            if(pcecrc_list[n].flag & FLAG_BITFLIP) flip = 1;
            if(pcecrc_list[n].flag & FLAG_SPLIT) split = 1;
        }
    }
    if (!found)
        return 23;
#endif
#if 0
    int fd, res;
    fd = open("wback.bin", O_CREAT|O_WRONLY|O_TRUNC);
    write(fd, buf, size);
    close(fd);
#endif

    /* Bit-flip image */
    if(flip)
    {
        uint8 temp;
        int count;

        for(count = 0; count < size; count += 1)
        {
            temp = buf[count];
            buf[count] = bitflip[temp];
        }
    }

    /* Always split 384K images */
    if(size == 0x60000)
    {
        memcpy(rom + 0x00000, buf + 0x00000, 0x40000);
        memcpy(rom + 0x80000, buf + 0x40000, 0x20000);
    }
    else /* Split 512K images if requested */
    if(split && (size == 0x80000))
    {
        memcpy(rom + 0x00000, buf + 0x00000, 0x40000);
        memcpy(rom + 0x80000, buf + 0x40000, 0x40000);
    }
    else
    {
        memcpy(rom, buf, (size > 0x100000) ? 0x100000 : size);
    }

    /* Free allocated memory and exit */
    free(ptr);

#ifdef DOS
    /* I need Allegro to handle this... */
    strcpy(game_name, get_filename(game_name));
#endif

    return (1);
}


int load_file(char *filename, char *buf, int size)
{
    int fd = 0;
    fd = open(filename, O_RDONLY);
    if(!fd) return (0);
    int res;
    res = read(fd, buf, size);
    close(fd);
    return (1);
}


int save_file(char *filename, char *buf, int size)
{
    int fd = 0;
    fd = open(filename, O_WRONLY);
    if(!fd) return (0);
    int res;
    res = write(fd, buf, size);
    close(fd);
    return (1);
}


#if 0
int check_zip(char *filename)
{
#ifdef DOS
    if(stricmp(strrchr(filename, '.'), ".zip") == 0) return (1);
#else
    uint8 buf[2];
    FILE *fd = NULL;
    fd = fopen(filename, "rb");
    if(!fd) return (0);
    fread(buf, 2, 1, fd);
    fclose(fd);
    /* Look for hidden ZIP magic */
    if(memcmp(buf, "PK", 2) == 0) return (1);
#endif
    return (0);
}
/* Because gzio.c doesn't work */
int gzsize(gzFile *gd)
{
    #define CHUNKSIZE   0x10000
    int size = 0, length = 0;
    unsigned char buffer[CHUNKSIZE];
    gzrewind(gd);
    do {
        size = gzread(gd, buffer, CHUNKSIZE);
        if(size <= 0) break;
        length += size;
    } while (!gzeof(gd));
    gzrewind(gd);
    return (length);
    #undef CHUNKSIZE
}

#endif
