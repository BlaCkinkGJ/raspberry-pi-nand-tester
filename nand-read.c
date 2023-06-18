/*
 * Copyright (C) 2023 Gijun Oh
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include "nand.h"
#include "nand-utils.h"

int nand_read(char *data, int block, int page)
{
	int i = 0;
	int row, col;
	char oob[64] = { 0xff };
	if (data == NULL) {
		return -1;
	}
	nand_send_command(0x00);
	row = (block * NAND_PAGES_PER_BLOCK) + page;
	col = 0x00;
	nand_send_address(row, col);
	nand_send_command(0x30);
	nand_wait_busy();
	for (i = 0; i < NAND_PAGE_BYTE; i++) {
		data[i] = (char)nand_read_1_cycle();
	}
	for (i = 0; i < NAND_PAGE_OOB_BYTE; i++) {
		oob[i] = (char)nand_read_1_cycle();
	}
	return nand_oob_verify(oob, data);
}
