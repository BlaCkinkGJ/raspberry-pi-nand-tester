/*
 * Copyright (C) 2023 Gijun Oh
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include "nand.h"
#include "nand-utils.h"
#include <stdio.h>
#include <stdlib.h>

int erase_write_test(int block, int page)
{
	int ret = 0;
	int i = 0;
	char data[NAND_PAGE_BYTE] = {
		0,
	};

	// read
	ret = nand_read(data, block, page);
	if (ret) {
		printf("read fail (err: %s)\n", nand_get_read_error_msg(ret));
		return ret;
	}
	nand_page_print(data);
	nand_status();

	// erase
	ret = nand_erase(block);
	if (ret) {
		printf("erase fail\n");
		return ret;
	}
	ret = nand_read(data, block, page);
	if (ret) {
		printf("read fail (err: %s)\n", nand_get_read_error_msg(ret));
		return ret;
	}
	nand_page_print(data);
	nand_status();

	// write
	srand((unsigned int)time(NULL));
	for (i = 0; i < NAND_PAGE_BYTE; i++) {
		data[i] = (char)rand();
	}
	ret = nand_write(data, block, page);
	if (ret) {
		printf("write fail\n");
		return ret;
	}
	ret = nand_read(data, block, page);
	if (ret) {
		printf("read fail\n");
		return ret;
	}
	nand_page_print(data);
	nand_status();
	return 0;
}

int main(void)
{
	int ret;
	ret = nand_init();
	if (ret) {
		goto out;
	}
	ret = nand_info_print();
	if (ret) {
		goto out;
	}
	ret = erase_write_test(0, 0);
	if (ret) {
		goto out;
	}
out:
	nand_free();
	return 0;
}
