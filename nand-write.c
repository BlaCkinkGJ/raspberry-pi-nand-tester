/*
 * Copyright (C) 2023 Gijun Oh
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include "nand.h"
#include "nand-utils.h"

int nand_write(char *data, int block, int page)
{
	int i = 0, err;
	int row, col;
	char oob[64] = { 0 };
	if (data == NULL) {
		return -1;
	}
	err = nand_oob_setup(oob, data);
	if (err) {
		return err;
	}
	nand_send_command(0x80);
	row = (block * NAND_PAGES_PER_BLOCK) + page;
	col = 0x00;
	nand_send_address(row, col);
	nand_delay_ns(105);
	for (i = 0; i < NAND_PAGE_BYTE; i++) {
		nand_write_1_cycle((int)data[i]);
	}
	for (i = 0; i < NAND_PAGE_OOB_BYTE; i++) {
		nand_write_1_cycle((int)oob[i]);
	}
	nand_send_command(0x10);
	nand_wait_busy();
	return nand_pass_fail();
}
