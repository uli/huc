#include <string.h>
#include "develo.h"


/* ----
 * dv_set_bank()
 * ----
 */

int
dv_set_bank(int page, int bank)
{
	unsigned char buf[8];

	buf[0] = 2;
	buf[1] = page;
	buf[2] = bank;
	buf[3] = 0;
	buf[4] = 0;
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;

	if (dv_send_cmd(buf) != DV_OK)
		return (DV_ERR);

	return (DV_OK);
}


/* ----
 * dv_get_bank()
 * ----
 */

int
dv_get_bank(unsigned char *bank)
{
	unsigned char buf[8];

	buf[0] = 3;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	buf[4] = 0;
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;

	if (dv_send_cmd(buf) != DV_OK)
		return (DV_ERR);
	if (dv_recv_block(buf, 8) != DV_OK)
		return (DV_ERR);

	memcpy(bank, buf, 8);
	return (DV_OK);
}


/* ----
 * dv_get_ram()
 * ----
 * transfer a block of memory from the PC Engine to the PC
 */

int
dv_get_ram(char *data, int addr, int len)
{
	unsigned char buf[8];

	buf[0] = 4;
	buf[1] = (addr & 0xFF00) >> 8;
	buf[2] = (addr & 0x00FF);
	buf[3] = (len & 0xFF00) >> 8;
	buf[4] = (len & 0x00FF);
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;

	if (dv_send_cmd(buf) != DV_OK)
		return (DV_ERR);
	if (dv_recv_block(data, len) != DV_OK)
		return (DV_ERR);

	return (DV_OK);
}


/* ----
 * dv_set_ram()
 * ----
 * transfer a block of memory from the PC to the PC Engine
 */

int
dv_set_ram(int addr, char *data, int len)
{
	unsigned char buf[8];
	int size, pos, temp;

	size = len;
	pos = 0;

	/* transfer the block by chunk of 512 bytes */
	while (size) {
		temp = (size > 512) ? 512 : size;

		buf[0] = 5;
		buf[1] = ((addr + pos) & 0xFF00) >> 8;
		buf[2] = ((addr + pos) & 0x00FF);
		buf[3] = (temp & 0xFF00) >> 8;
		buf[4] = (temp & 0x00FF);
		buf[5] = 0;
		buf[6] = 0;
		buf[7] = 0;

		if (dv_send_cmd(buf) != DV_OK)
			return (DV_ERR);
		if (dv_send_block(&data[pos], temp) != DV_OK)
			return (DV_ERR);
		if (dv_wait_ack() != DV_OK)
			return (DV_ERR);

		size -= temp;
		pos += temp;
	}

	return (DV_OK);
}


/* ----
 * dv_get_vram()
 * ----
 */

int
dv_get_vram(char *data, int addr, int len)
{
	unsigned char buf[8];
	int size, pos, temp;

	size = len;
	pos = 0;

	/* get the block by chunk of 512 bytes */
	while (size) {
		temp = (size > 512) ? 512 : size;

		buf[0] = 6;
		buf[1] = ((addr + (pos / 2)) & 0xFF00) >> 8;
		buf[2] = ((addr + (pos / 2)) & 0x00FF);
		buf[3] = (temp & 0xFF00) >> 8;
		buf[4] = (temp & 0x00FF);
		buf[5] = 0;
		buf[6] = 0;
		buf[7] = 0;

		if (dv_send_cmd(buf) != DV_OK)
			return (DV_ERR);
		if (dv_recv_block(&data[pos], temp) != DV_OK)
			return (DV_ERR);

		size -= temp;
		pos += temp;
	}

	return (DV_OK);
}


/* ----
 * dv_set_vram()
 * ----
 */

int
dv_set_vram(int addr, char *data, int len)
{
	unsigned char buf[8];
	int size, pos, temp;

	size = len;
	pos = 0;

	/* transfer the block by chunk of 512 bytes */
	while (size) {
		temp = (size > 512) ? 512 : size;

		buf[0] = 7;
		buf[1] = ((addr + (pos / 2)) & 0xFF00) >> 8;
		buf[2] = ((addr + (pos / 2)) & 0x00FF);
		buf[3] = (temp & 0xFF00) >> 8;
		buf[4] = (temp & 0x00FF);
		buf[5] = 0;
		buf[6] = 0;
		buf[7] = 0;

		if (dv_send_cmd(buf) != DV_OK)
			return (DV_ERR);
		if (dv_send_block(&data[pos], temp) != DV_OK)
			return (DV_ERR);
		if (dv_wait_ack() != DV_OK)
			return (DV_ERR);

		size -= temp;
		pos += temp;
	}

	return (DV_OK);
}


/* ----
 * dv_get_color()
 * ----
 */

int
dv_get_color(char *data, int addr, int len)
{
	unsigned char buf[8];

	buf[0] = 8;
	buf[1] = (addr & 0xFF00) >> 8;
	buf[2] = (addr & 0x00FF);
	buf[3] = (len & 0xFF00) >> 8;
	buf[4] = (len & 0x00FF);
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;

	if (dv_send_cmd(buf) != DV_OK)
		return (DV_ERR);
	if (dv_recv_block(data, len) != DV_OK)
		return (DV_ERR);

	return (DV_OK);
}


/* ----
 * dv_set_color()
 * ----
 */

int
dv_set_color(int addr, char *data, int len)
{
	unsigned char buf[8];

	buf[0] = 9;
	buf[1] = (addr & 0xFF00) >> 8;
	buf[2] = (addr & 0x00FF);
	buf[3] = (len & 0xFF00) >> 8;
	buf[4] = (len & 0x00FF);
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;

	if (dv_send_cmd(buf) != DV_OK)
		return (DV_ERR);
	if (dv_send_block(data, len) != DV_OK)
		return (DV_ERR);
	if (dv_wait_ack() != DV_OK)
		return (DV_ERR);

	return (DV_OK);
}


/* ----
 * dv_exec()
 * ----
 * execute the program starting at <addr> on the PC Engine
 */

int
dv_exec(int addr, int slave)
{
	unsigned char buf[8];

	buf[0] = 10;
	buf[1] = (addr & 0xFF00) >> 8;
	buf[2] = (addr & 0x00FF);
	buf[3] = 0;
	buf[4] = 0;
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;

	if (dv_send_cmd(buf) != DV_OK)
		return (DV_ERR);

	return (DV_OK);
}


/* ----
 * dv_read_cd()
 * ----
 */

int
dv_read_cd(char *data, int sect)
{
	unsigned char buf[8];

	buf[0] = 12;
	buf[1] = (sect & 0xFF0000) >> 16;
	buf[2] = (sect & 0x00FF00) >> 8;
	buf[3] = (sect & 0x0000FF);
	buf[4] = 0;
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;

	if (dv_send_cmd(buf) != DV_OK)
		return (DV_ERR);
	if (dv_get_ram(data, 0x2800, 2048) != DV_OK)
		return (DV_ERR);

	return (DV_OK);
}

