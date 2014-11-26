#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "develo.h"

/* locals */
static unsigned char buffer[128];
static unsigned char block_buffer[256];
static char line[256];
static int bank_base;
static int bank, old_bank;
static int block, old_block, last_block;
static int buffer_start, buffer_index;
static int display;

/* protos */
static int htoi(char *str, int nb);
static int upload(unsigned char *data, int addr, int cnt);
static int flush(int end);


/* ----
 * dv_load_mx()
 * ----
 */

int
dv_load_mx(char *fname, int *a, int *b, int disp)
{
	FILE *fp;
	char *ptr;
	char type;
	int addr, data, cnt, chksum;
	int start;
	int ln;
	int i;

	/* set global display flag */
	display = disp;

	/* upload message */
	printf("uploading program");
	fflush(stdout);

	/* open the mx file */
	if ((fp = fopen(fname, "r")) == NULL) {
		printf("... can not open file '%s'!\n", fname);
		return (DV_ERR);
	}

	/* new line */
	printf("\n");

	/* init variables */
	buffer_start = -1;
	buffer_index = 0;
	old_block = 0;
	old_bank = 0;
	start = 0;

	/* line counter */
	ln = 1;

	/* main read loop */
	while (fgets(line, 254, fp) != NULL) {
		if (line[0] == 'S') {
			/* get s-record type */
			type = line[1];

			/* skip unsupported records */
			if ((type != '2') && (type != '8'))
				continue;

			/* check the line length */
			if (strlen(line) < 12)
				goto err;

			/* count, address */
			cnt = htoi(&line[2], 2);
			addr = htoi(&line[4], 6);

			if ((cnt < 4) || (addr == -1))
				goto err;

			/* adjust count */
			cnt -= 4;

			/* checksum */
			chksum = cnt + ((addr >> 16) & 0xFF) +
				 ((addr >> 8) & 0xFF) +
				 ((addr) & 0xFF) + 4;

			/* data */
			ptr = &line[10];

			for (i = 0; i < cnt; i++) {
				data = htoi(ptr, 2);
				buffer[i] = data;
				chksum += data;
				ptr += 2;

				if (data == -1)
					goto err;
			}

			/* checksum test */
			chksum = (~chksum) & 0xFF;
			data = htoi(ptr, 2);

			if (data != chksum)
				goto err;

			/* record switch */
			if (type == '8') {
				/* S8: start address - end */
				if (addr)
					start = addr;
				break;
			}
			else {
				/* S2: data */
				if (start == 0)
					start = addr;
				if (upload(buffer, addr, cnt) != DV_OK)
					return (DV_ERR);
			}
		}

		/* incremente line counter */
		ln++;
	}

	/* flush block buffer */
	if (flush(1) != DV_OK)
		return (DV_ERR);

	/* ok - return start address and bank base index */
	*a = start;
	*b = bank_base;

	return (DV_OK);

	/* error */
err:
	printf("... error in the mx file, line %i!\n", ln);
	return (DV_ERR);
}


/* ----
 * htoi()
 * ----
 */

static int
htoi(char *str, int nb)
{
	char c;
	int val;
	int i;

	val = 0;

	for (i = 0; i < nb; i++) {
		c = toupper(str[i]);

		if ((c >= '0') && (c <= '9'))
			val = (val << 4) + (c - '0');
		else if ((c >= 'A') && (c <= 'F'))
			val = (val << 4) + (c - 'A' + 10);
		else
			return (-1);
	}

	/* ok */
	return (val);
}


/* ----
 * upload()
 * ----
 */

static int
upload(unsigned char *data, int addr, int cnt)
{
	int offset;
	int i;

	/* calculate bank and block indexes */
loop:
	bank = (addr >> 13);
	block = (addr >> 8);
	offset = (addr & 0xFF);

	/* develo compatibility */
	if (bank < 8) {
		if (bank < 3 || bank == 7) {
			printf("incorrect destination address, %04X!\n", addr);
			return (DV_ERR);
		}
		bank += 0x81;
	}

	/* block change */
	if (block != old_block)
		if (flush(0) != DV_OK)
			return (DV_ERR);

	/* bank change */
	if (bank != old_bank) {
		/* save starting bank index */
		if (old_bank == 0)
			bank_base = bank;

		/* update bank index */
		old_bank = bank;

		/* status */
		if (display) {
			if (last_block) {
				/* mask block index */
				last_block = (last_block - 1) & 0x1F;

				/* progress bar */
				if (last_block < 31) {
					for (i = last_block; i < 31; i++)
						printf("þ");

					/* ok */
					printf(" OK\n");
				}
			}

			/* bank message */
			printf("bank %02X: ", bank);
			fflush(stdout);
		}

		/* reset block index */
		last_block = 0;

		/* select the bank */
		if (dv_set_bank(4, bank) != DV_OK) {
			printf("\n%s, can not set bank!\n", dv_get_errmsg());
			return (DV_ERR);
		}
	}

	/* set block starting offset */
	if (buffer_start == -1)
		buffer_start = offset;

	/* set current offset */
	buffer_index = offset;

	/* copy data */
	while (cnt) {
		/* copy a byte */
		block_buffer[buffer_index++] = *data++;
		addr += 1;
		cnt -= 1;

		/* buffer full */
		if (buffer_index > 255) {
			/* flush current buffer */
			if (flush(0) != DV_OK)
				return (DV_ERR);

			/* loop */
			if (cnt)
				goto loop;
		}
	}

	/* ok */
	return (DV_OK);
}


/* ----
 * flush()
 * ----
 */

static int
flush(int end)
{
	unsigned char *src;
	int dst;
	int len;
	int i;

	/* transfer previous block - if any */
	if (buffer_index) {
		/* status */
		if (display) {
			/* set last block if new bank */
			if (last_block == 0)
				last_block = (old_block & 0xFFFFE0);

			/* progress bar */
			for (i = last_block; i < old_block; i++)
				printf("þ");

			/* output */
			fflush(stdout);
		}

		/* destination & source addresses, and length */
		dst = (0x8000) + ((old_block & 0x1F) << 8) + buffer_start;
		src = (block_buffer + buffer_start);
		len = (buffer_index - buffer_start);

		/* transfer */
		if (len) {
			if (dv_set_ram(dst, src, len) != DV_OK) {
				printf("\n%s!\n", dv_get_errmsg());
				return (DV_ERR);
			}
		}

		/* save block index */
		last_block = (old_block + 1);

		/* status */
		if (display) {
			/* current block */
			printf("þ");
			fflush(stdout);

			/* mask block index */
			old_block = (old_block) & 0x1F;

			/* ok */
			if (old_block == 31)
				printf(" OK\n");
			else {
				/* end transfer */
				if (end) {
					for (i = old_block; i < 31; i++)
						printf("þ");

					printf(" OK\n");
				}
			}
		}
	}

	/* reset block buffer */
	memset(block_buffer, 0xFF, 256);
	buffer_start = -1;
	buffer_index = 0;
	old_block = block;

	/* ok */
	return (DV_OK);
}

