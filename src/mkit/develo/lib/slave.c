#include <stdio.h>
#include <string.h>
#include <dir.h>
#include <dos.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "develo.h"

/* externs */
extern int slave_mode;
extern int slave_wait;

/* locals */
static int slave_display;
static unsigned char file_buffer[65536];
static int dir_nb_records;
static int dir_index;
static struct {
	char fname[13];
	char packed_fname[11];
	unsigned short fdate;
	unsigned short ftime;
	unsigned int fsize;
} dir_info[256];

/* protos */
static void cmd_get_dir(unsigned char *pkt);
static void cmd_file_read(unsigned char *pkt);
static void cmd_file_write(unsigned char *pkt);
static void cmd_file_create(unsigned char *pkt);
static void cmd_file_delete(unsigned char *pkt);
static void cmd_reg_report(unsigned char *pkt);
static void cmd_file_dskf(unsigned char *pkt);
static void pack_file_name(char *buffer, char *fname);
static void unpack_file_name(char *fname, char *str);
static void pack_dir_record(unsigned char *buffer, int n);
static int  send_packet(unsigned char *pkt);


/* ----
 * cmd_get_dir()
 * ----
 */

static void
cmd_get_dir(unsigned char *pkt)
{
	char *msg;
	int i;
	int n;

	/* reset directory index */	
	if (pkt[1] != 0xFF)
		dir_index = 0;

	/* get number of requested records */
	n = pkt[2];

	/* echo command */
	if (slave_display)
		printf("get %i directory record(s) from index %i\n", n, dir_index);

	/* check directory limit */		
	if ((dir_index + n) > dir_nb_records) {
		n = dir_nb_records - dir_index;

		if (slave_display)
			printf("    send %i record(s)\n", n);
	}

	/* send data */
	memset(pkt, 0, 16);
	pkt[0] = n;
	pkt[1] =  (n * 32) & 0xFF;
	pkt[2] = ((n * 32) >> 8) & 0xFF;

	if (send_packet(pkt) != DV_OK)
		msg = dv_get_errmsg();
	else {
		msg = "OK";

		if (n) {
			/* get packed dir records */
			for (i = 0; i < n; i++)
				pack_dir_record(&file_buffer[i * 32], dir_index + i);

			/* and send them */
			if (dv_send_block(file_buffer, n * 32) != DV_OK)
				msg = dv_get_errmsg();
			else {
				/* ok */
				dir_index += n;
			}
		}
	}

	/* echo result */
	if (slave_display)
		printf("    %s\n", msg);
}


/* ----
 * cmd_file_read()
 * ----
 */

static void
cmd_file_read(unsigned char *pkt)
{
	FILE *fp;
	char *msg;
	int len, loc;
	int n;
	
	len = 0;
	loc = 0;
	
	/* execute command */
	if (pkt[3] == 0xFF) {
		/* get file record index */
		n = pkt[1] + (pkt[2] << 8);

		if (n >= dir_nb_records) {
			/* incorrect file record index */
			if (slave_display)
				printf("read file - incorrect directory record index: %i (max. %i)!\n", n, dir_nb_records);
		}
		else {
			/* echo command */
			if (slave_display)
				printf("read file \"%s\"\n", dir_info[n].fname);

			/* open file */
			if ((fp = fopen(dir_info[n].fname, "rb")) == NULL) {
				if (slave_display)
					printf("    can not open file!\n");
			}
			else {
				/* get read args */
				len = pkt[8] + (pkt[9] << 8);
				loc = pkt[4] + (pkt[5] << 8) + (pkt[6] << 16) + (pkt[7] << 24);

				if (slave_display) {
					printf("    read loc  : %i\n", loc);
					printf("    read size : %i\n", len);
				}

				/* read data */
				if (fseek(fp, loc, SEEK_SET))
					len = 0;
				else {
					len = fread(file_buffer, 1, len, fp);

					if (len < 0)
						len = 0;
				}

				if (slave_display)
					printf("    read %i byte(s)\n", len);

				/* close file */
				fclose(fp);
			}
		}
	}
	
	/* return ack packet */
	memset(pkt, 0, 16);
	pkt[0] = (len) & 0x0FF;
	pkt[1] = (len >> 8) & 0x0FF;

	if (send_packet(pkt) != DV_OK)
		msg = dv_get_errmsg();
	else {
		msg = "OK";

		/* transfer data */	
		if (len) {
			if (dv_send_byte(DV_ACK) != DV_OK)
				msg = dv_get_errmsg();
			else {
				if (dv_send_block(file_buffer, len) != DV_OK)
					msg = dv_get_errmsg();
			}
		}
	}

	/* echo result */
	if (slave_display)
		printf("    %s\n", msg);
}


/* ----
 * cmd_file_write()
 * ----
 */

static void
cmd_file_write(unsigned char *pkt)
{
	FILE *fp;
	char *msg;
	int len, loc;
	int n;

	/* get record index */
	n   = pkt[1] + (pkt[2] << 8);
	fp  = NULL;	
	len = 0;
	loc = 0;
	
	if (pkt[3] != 0xFF) {
		/* packet error */
		if (slave_display)
			printf("write file - incorrect request (pkt[3] != 0xFF)!\n");
	}	
	else {
		/* check record index */
		if (n >= dir_nb_records) {
			/* incorrect file record index */
			if (slave_display)
				printf("write file - incorrect directory record index: %i (max. %i)!\n", n, dir_nb_records);
		}
		else {
			/* echo command */
			if (slave_display)
				printf("write file \"%s\"\n", dir_info[n].fname);

			/* open file */
			if ((fp = fopen(dir_info[n].fname, "ab")) == NULL) {
				if (slave_display)
					printf("    can not open file!\n");
			}
			else {
				/* get write args */
				len = pkt[8] + (pkt[9] << 8);
				loc = pkt[4] + (pkt[5] << 8) + (pkt[6] << 16) + (pkt[7] << 24);

				if (slave_display) {
					printf("    write loc  : %i\n", loc);
					printf("    write size : %i\n", len);
				}

				/* ??? */
				dv_recv_byte();
			}
		}
	}

	/* get data */	
	if (dv_recv_block(file_buffer, len) != DV_OK) {
		/* error */
		msg = dv_get_errmsg();

		if (dv_get_err() == DV_CRC_ERR) {
			if (dv_send_byte(DV_NAK) != DV_OK)
				msg = dv_get_errmsg();
		}
	}
	else {	
		/* ok */
		if (dv_send_byte(DV_ACK) != DV_OK)
			msg = dv_get_errmsg();
		else {
			/* write file */
			if (fp) {
				fseek(fp, loc, SEEK_SET);
				fwrite(file_buffer, len, 1, fp);
	
				/* update file record */
				if (dir_info[n].fsize < (loc + len))
					dir_info[n].fsize = (loc + len);
			}
	
			/* second acknowledge (???) */
			if (dv_send_byte(DV_ACK) != DV_OK)
				msg = dv_get_errmsg();
			else
				msg = "OK";
		}
	}

	/* close file */
	if (fp)
		fclose(fp);

	/* echo result */
	if (slave_display)
		printf("    %s\n", msg);
}


/* ----
 * cmd_file_create()
 * ----
 */

static void
cmd_file_create(unsigned char *pkt)
{
	int fh;
	char fname[13];
	struct ftime ft;
	char *msg;
	int i;
	
	/* get file name */
	unpack_file_name(fname, &pkt[1]);

	/* echo command */
	if (slave_display)
		printf("create file \"%s\"\n", fname);

	/* create file */	
	if ((fh = creat(fname, S_IRUSR | S_IWUSR)) < 0)
		msg = "can not create file!";
	else {
		/* get file infos */
		getftime(fh, &ft);
		close(fh);

		/* search if the file already had a record */
		for (i = 0; i < dir_nb_records; i++) {
			if (memcmp(dir_info[i].packed_fname, &pkt[1], 11) == 0)
				break;
		}

		/* fill the record */
		strcpy(dir_info[i].fname, fname);
		memcpy(dir_info[i].packed_fname, &pkt[1], 11);
		dir_info[i].fdate = ((unsigned short *)&ft)[1];
		dir_info[i].ftime = ((unsigned short *)&ft)[0];
		dir_info[i].fsize = 0;

		/* adjust dir record number */
		if (dir_nb_records == i)
			dir_nb_records++;

		/* acknowledge command */
		if (dv_send_byte(DV_ACK) != DV_OK)
			msg = dv_get_errmsg();
		else
			msg = "OK";
	}

	/* echo result */
 	if (slave_display)
		printf("    %s\n", msg);
}


/* ----
 * cmd_file_delete()
 * ----
 */

static void
cmd_file_delete(unsigned char *pkt)
{
	char fname[16];
	char *msg;
	unsigned char c;
	int i;
	
	/* get file name */
	unpack_file_name(fname, &pkt[1]);

	/* echo command */
	if (slave_display)
		printf("delete file \"%s\"\n", fname);

	/* search the file */	
	for (i = 0; i < dir_nb_records; i++) {
		if (memcmp(dir_info[i].packed_fname, &pkt[1], 11) == 0) {
			memset(dir_info[i].packed_fname, 0, 11);
			break;
		}
	}

	/* execute command */
	if (i < dir_nb_records) {
		/* delete file */
		if (remove(fname)) {
			if (slave_display)
				printf("    can not delete file!\n");

			c = DV_NAK;
		}
		else
			c = DV_ACK;
	}
	else {
		/* file doesn't exist */
		if (slave_display)
			printf("    file does not exist!\n");

		c = DV_NAK;
	}

	/* send result */
	if (dv_send_byte(c) != DV_OK)
		msg = dv_get_errmsg();
	else
		msg = "OK";

	/* echo result */
	if (slave_display)
		printf("    %s\n", msg);
}


/* ----
 * cmd_reg_report()
 * ----
 */

static void
cmd_reg_report(unsigned char *pkt)
{
	printf("dump registers : PC = %04X A=%02X X=%02X Y=%02X S=%02X F=",
		   pkt[6] + (pkt[7] << 8) - 2,
		   pkt[1], pkt[3],
		   pkt[4], pkt[5]);
	if (pkt[2] & 0x80) putchar('N'); else putchar('-');
	if (pkt[2] & 0x40) putchar('V'); else putchar('-');
	if (pkt[2] & 0x20) putchar('T'); else putchar('-');
	if (pkt[2] & 0x10) putchar('B'); else putchar('-');
	if (pkt[2] & 0x08) putchar('D'); else putchar('-');
	if (pkt[2] & 0x04) putchar('I'); else putchar('-');
	if (pkt[2] & 0x02) putchar('Z'); else putchar('-');
	if (pkt[2] & 0x01) putchar('C'); else putchar('-');
	printf("\n");
}


/* ----
 * cmd_file_dskf()
 * ----
 */

static void
cmd_file_dskf(unsigned char *pkt)
{
	struct dfree df;
	char *msg;
	int size;

	/* get disk free space */
	getdfree(0, &df);
	size = (df.df_avail * df.df_bsec * df.df_sclus);

	/* echo command */	
	if (slave_display) {
		printf("get disk free size\n");
		printf("    size : %iKB\n", size / 1024);
	}

	/* send data */	
	memset(pkt, 0, 16);
	pkt[0] = (size) & 0xFF;
	pkt[1] = (size >>  8) & 0xFF;
	pkt[2] = (size >> 16) & 0xFF;
	pkt[3] = (size >> 24) & 0xFF;

	if (send_packet(pkt) != DV_OK)
		msg = dv_get_errmsg();
	else
		msg = "OK";

	/* echo result */
	if (slave_display)
		printf("    %s\n", msg);
}


/* ----
 * pack_file_name()
 * ----
 */

static void
pack_file_name(char *buffer, char *fname)
{
	int i, j;

	/* pack name */
	for (i = 0, j = 0; i < 8; i++,j++) {
		if (fname[j] == '.')
			break;
		if (fname[j] == 0)
			break;
		buffer[i] = fname[j];
	}
	for (; i < 8; i++)
		buffer[i] = ' ';
	if (fname[j] == '.')
		j++;
	for (; i < 11; i++, j++) {
		if (fname[j] == 0)
			break;
		buffer[i] = fname[j];
	}
	for (; i < 11; i++)
		buffer[i] = ' ';
}


/* ----
 * unpack_file_name()
 * ----
 */

static void
unpack_file_name(char *fname, char *str)
{
	int i, j, k;

	/* unpack file name */
	for (i = 0, j = 0; i < 8; i++) {
		if (str[i] == ' ')
			continue;

		/* copy char */
		fname[j++] = str[i];
	}

	k = j; 

	/* unpack file extension */
	for (i = 8; i < 11; i++) {
		if (str[i] == ' ')
			continue;
		if (k == j)
			fname[j++] = '.';

		/* copy char */
		fname[j++] = str[i];
	}

	/* add null char */
	fname[j] = '\0';
}


/* ----
 * pack_dir_record()
 * ----
 */

static void
pack_dir_record(unsigned char *buffer, int n)
{
	/* clear buffer */
	memset(buffer, 0, 32);

	/* file name */
	memcpy(buffer, dir_info[n].packed_fname, 11);

	/* file date and time */
	buffer[12] = (dir_info[n].fdate) & 0xFF;
	buffer[13] = (dir_info[n].fdate >> 8) & 0xFF;
	buffer[14] = (dir_info[n].ftime) & 0xFF;
	buffer[15] = (dir_info[n].ftime >> 8) & 0xFF;

	/* directory record index */
	buffer[16] = (n) & 0xFF;
	buffer[17] = (n >> 8) & 0xFF;

	/* file size */
	buffer[19] = (dir_info[n].fsize) & 0xFF;
	buffer[20] = (dir_info[n].fsize >>  8) & 0xFF;
	buffer[21] = (dir_info[n].fsize >> 16) & 0xFF;
	buffer[22] = (dir_info[n].fsize >> 24) & 0xFF;

	/* ?? */
	buffer[11] = 1;
	buffer[18] = 0xFF;
}


/* ----
 * send_packet()
 * ----
 */

static int
send_packet(unsigned char *pkt)
{
	/* send */
	return (dv_send_block(pkt, 14));
}


/* ----
 * dv_slave()
 * ----
 */

void
dv_slave(int disp)
{
	unsigned char pkt[16];

	/* globals */
	slave_display = disp;
	slave_mode = 1;

	/* main loop */	
	while (slave_mode) {
		/* wait packet */
		slave_wait = 1;

		if (dv_recv_byte() != '$')
			continue;
		if (dv_send_byte('$') != DV_OK)
			continue;

		/* get command packet */		
		if (dv_recv_block(pkt, 14) != DV_OK)
			continue;

		/* acknowledge command */
		if (dv_send_byte(DV_ACK) != DV_OK)
			continue;

		/* execute command */		
		switch(pkt[0]) {
		case 255:
			/* program quit */
			return;
		case 1:
			/* get directory info */
			cmd_get_dir(pkt);
			break;
		case 2:
			/* read file */
			cmd_file_read(pkt);
			break;
		case 3:
			/* write file */
			cmd_file_write(pkt);
			break;
		case 4:
			/* create file */
			cmd_file_create(pkt);
			break;
		case 5:
			/* delete file */
			cmd_file_delete(pkt);
			break;
		case 6:
			/* register report */
			cmd_reg_report(pkt);
			break;
		case 8:
			/* disk free */
			cmd_file_dskf(pkt);
			break;
		}
	}
}


/* ----
 * dv_slave_init()
 * ----
 */

int
dv_slave_init(void)
{
	struct ffblk ff;
	int done;
	int n;

	/* scan current directory */	
	done = findfirst("*.*", &ff, 0);
	n = 0;

	while(!done) {
		/* get file infos */
		pack_file_name(dir_info[n].packed_fname, ff.ff_name); /* packed name */
		strncpy(dir_info[n].fname, ff.ff_name, 12);           /* name */
		dir_info[n].fname[12] = '\0';
		dir_info[n].fdate = ff.ff_fdate;    /* date */
		dir_info[n].ftime = ff.ff_ftime;    /* time */
		dir_info[n].fsize = ff.ff_fsize;    /* size */

		/* next entry */
		done = findnext(&ff);
		n++;
	}

	/* number of directory records */
	dir_nb_records = n;

	/* ok */	
	return (DV_OK);
}

