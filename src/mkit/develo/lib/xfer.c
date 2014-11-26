#include <stdio.h>
#ifdef __linux__
#include <sys/io.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#define outportb(port, value) outb((value), (port))
#define inportb inb
#define getkey	getchar
#else
#include <pc.h>
#endif
#include "develo.h"
#include "crc16.h"

/*#define PC98	1*/	/* uncomment this for PC98 machines - UNTESTED!! */

#ifdef __linux__
static void
changemode(int dir)
{
	static struct termios oldt, newt;

	if (dir == 1) {
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	}
	else
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int
kbhit(void)
{
	struct timeval tv;
	fd_set rdfs;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_ZERO(&rdfs);
	FD_SET(STDIN_FILENO, &rdfs);

	select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
	return (FD_ISSET(STDIN_FILENO, &rdfs));
}
#endif

/* externs */
extern int develo;		/* develo box presence flag */
extern int develo_lpt;		/* parallel I/O port */
extern int develo_com;		/* serial I/O port */
extern int develo_wait1;	/* timeout delays */
extern int develo_wait2;
extern int develo_error;	/* latest error */
extern int develo_status;	/* transfer status */
extern int slave_mode;
extern int slave_wait;

/* protos */
static int  dv_crc(unsigned char *buf, int len);
static int  dv_request(int cmd);
static void dv_output(int data);
static int  dv_output_and_wait(int c, int val);
static int  dv_input(void);
static void dv_delay(void);


/* ----
 * dv_send_cmd()
 * ----
 */

int
dv_send_cmd(unsigned char *buf)
{
	if (dv_request(DV_CMD) != DV_OK)
		return (DV_ERR);
	if (dv_send_block(buf, 8) != DV_OK)
		return (DV_ERR);
	if (dv_wait_ack() != DV_OK)
		return (DV_ERR);

	/* ok */
	return (DV_OK);
}


/* ----
 * dv_send_block()
 * ----
 */

int
dv_send_block(unsigned char *buf, int len)
{
	int i, crc;
	unsigned char *p = buf;

	/* send block */
	for (i = 0; i < len; i++) {
		if (dv_send_byte(*p) != DV_OK)
			return (DV_ERR);
		p++;
	}

	/* calculate and send CRC */
	crc = dv_crc(buf, len);

	if (dv_send_byte(crc >> 8) != DV_OK)
		return (DV_ERR);
	if (dv_send_byte(crc & 0xFF) != DV_OK)
		return (DV_ERR);

	/* reset the output lines */
	dv_output(0x0F);

	/* ok */
	return (DV_OK);
}


/* ----
 * dv_recv_block()
 * ----
 */

int
dv_recv_block(unsigned char *buf, int len)
{
	int i;
	int c;
	int l, h;
	int crc, crc_l, crc_h;
	unsigned char *p = buf;

	/* get block */
	for (i = 0; i < len; i++) {
		if ((c = dv_recv_byte()) == DV_ERR)
			return (DV_ERR);
		*p++ = c;
	}

	/* calculate CRC */
	crc = dv_crc(buf, len);
	crc_h = (crc & 0xFF00) >> 8;
	crc_l = (crc & 0x00FF);

	/* check CRC */
	if ((h = dv_recv_byte()) == DV_ERR)
		return (DV_ERR);
	if ((l = dv_recv_byte()) == DV_ERR)
		return (DV_ERR);

	if ((crc_l != l) || (crc_h != h)) {
		develo_error = DV_CRC_ERR;
		return (DV_ERR);
	}

	/* reset the output lines */
	dv_output(0x0F);

	/* ok */
	return (DV_OK);
}


/* ----
 * dv_wait_ack()
 * ----
 */

int
dv_wait_ack(void)
{
	switch (dv_recv_byte()) {
	case DV_ACK:
		/* ok */
		break;

	case DV_NAK:
		/* bad CRC */
		develo_error = DV_CRC_ERR;
		return (DV_ERR);

	default:
		/* timeout */
		return (DV_ERR);
	}

	/* reset the output lines */
	dv_output(0x0F);

	/* ok */
	return (DV_OK);
}


/* ----
 * dv_crc()
 * ----
 */

static int
dv_crc(unsigned char *buf, int len)
{
	int i;
	unsigned int crc = 0;
	unsigned char *p = buf;

	for (i = 0; i < len; i++) {
		crc = crc16tbl[(crc >> 8) ^ (*p)] ^ (crc << 8);
		crc &= 0xFFFF;
		p++;
	}

	return (crc);
}


/* ----
 * dv_request()
 * ----
 */

static int
dv_request(int cmd)
{
	int i;

	/* develo status */
	develo_status = cmd;
	develo_error = DV_OK;

	/* request */
	switch (cmd) {
	case DV_CMD:
		/* command */
		for (i = 0; i < 10; i++) {
			if (dv_send_byte('@') != DV_OK)
				continue;
			if (dv_recv_byte() == '@')
				return (DV_OK);
		}

		/* error */
		return (DV_ERR);

	case DV_SEND:
		/* sending */
		return (dv_output_and_wait(0x09, 2));

	case DV_RECV:
		/* receiving */
#ifdef __linux__
		changemode(1);
#endif
		for (i = 0; i < 20; i++) {
			/* if in slave mode, check if the user
			 * want to exit
			 */
			if (slave_wait) {
				if (dv_check_user_break()) {
					slave_mode = 0;
					break;
				}
			}

			/* request */
			if (dv_output_and_wait(0x0B, 2) != DV_OK)
				continue;
			if (dv_output_and_wait(0x0F, 3) == DV_OK) {
				/* reset slave wait flag */
				slave_wait = 0;

				/* ok */
#ifdef __linux__
				changemode(0);
#endif
				return (DV_OK);
			}
		}
#ifdef __linux__
		changemode(0);
#endif

		/* reset slave wait flag */
		slave_wait = 0;

		/* error */
		return (DV_ERR);

	default:
		/* unsupported request */
		develo_error = DV_INTERNAL_ERR;
		return (DV_ERR);
	}
}


/* ----
 * dv_send_byte()
 * ----
 */

int
dv_send_byte(unsigned char c)
{
	int bit02;
	int bit35;
	int bit67;

	/* request transfer */
	if (dv_request(DV_SEND) != DV_OK)
		return (DV_ERR);

	/* split byte */
	bit02 = ((c)) & 0x07;
	bit35 = ((c >> 3) & 0x07) | 0x08;
	bit67 = ((c >> 6) & 0x03);

	/* send byte */
	if (dv_output_and_wait(bit02, 3) != DV_OK)
		return (DV_ERR);
	if (dv_output_and_wait(bit35, 2) != DV_OK)
		return (DV_ERR);
	if (dv_output_and_wait(bit67, 3) != DV_OK)
		return (DV_ERR);

	/* ok */
	return (DV_OK);
}


/* ----
 * dv_recv_byte()
 * ----
 */

int
dv_recv_byte(void)
{
	int i, j, s;
	int c;

	/* request transfer */
	if (dv_request(DV_RECV) != DV_OK)
		return (DV_ERR);

	/* get a byte */
	c = 0;
	s = 0;

	for (i = 0; i < 8; i++) {
		dv_output(0x0C);

		/* wait develo */
		for (j = 0; j < develo_wait1; j++) {
			if ((s = dv_input()) != 3)
				break;
		}

		/* get a bit */
		switch (s) {
		case 3:
			/* error */
			develo_error = DV_TIMEOUT_ERR;
			return (DV_ERR);

		case 2:
			/* '0' */
			if (dv_output_and_wait(0x0D, 3) != DV_OK)
				return (DV_ERR);

			c >>= 1;
			break;

		case 1:
		case 0:
			/* '1' */
			if (dv_output_and_wait(0x0E, 2) != DV_OK)
				return (DV_ERR);
			if (dv_output_and_wait(0x0A, 0) != DV_OK)
				return (DV_ERR);
			if (dv_output_and_wait(0x08, 3) != DV_OK)
				return (DV_ERR);

			c >>= 1;
			c |= 0x80;
			break;
		}
	}

	dv_output(0x0C);

	/* ok */
	return (c);
}


/* ----
 * dv_output()
 * ----
 */

static void
dv_output(int data)
{
	static int old = 0;

	outportb(develo_lpt, (data & 0x07) | (old & 0x08));
	dv_delay();
	outportb(develo_lpt, (data & 0x0F));
	old = data;
}


/* ----
 * dv_output_and_wait()
 * ----
 */

static int
dv_output_and_wait(int c, int val)
{
	int i;

	/* output */
	dv_output(c);

	/* wait loop */
	for (i = 0; i < develo_wait1; i++) {
		if (dv_input() == val)
			return (DV_OK);
	}

	/* timeout */
	develo_error = DV_TIMEOUT_ERR;
	return (DV_ERR);
}


/* ----
 * dv_input()
 * ----
 */

static int
dv_input(void)
{
	int c;

#ifdef PC98
	/* PC98 code - THIS HAS NOT BEEN TESTED!! */
	switch (inportb(0x33) & 0xA0) {
	case 0x00: c = 0x00; break;
	case 0x20: c = 0x01; break;
	case 0x80: c = 0x02; break;
	default:   c = 0x03; break;
	}
#else
	/* IBMPC code */
	if (develo_com) {
		/* develo box */
		switch (inportb(develo_com + 6) & 0xC0) {
		case 0x00: c = 0x03; break;
		case 0x40: c = 0x01; break;
		case 0x80: c = 0x02; break;
		default:   c = 0x00; break;
		}
	}
	else {
		/* develo link */
		c = (inportb(develo_lpt + 1) & 0x30) >> 4;
	}
#endif
	/* return result */
	return (c);
}


/* ----
 * dv_delay()
 * ----
 */

static void
dv_delay(void)
{
	int i;

	for (i = 0; i < 100; i++) ;
}


/* ----
 * dv_check_user_break()
 * ----
 */

int
dv_check_user_break(void)
{
	while (kbhit()) {
		if (getkey() == 0x1B)
			return (1);
	}

	return (0);
}

