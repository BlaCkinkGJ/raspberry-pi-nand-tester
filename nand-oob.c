/*
 * Copyright (C) 2023 Gijun Oh
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include "nand-hamming256.h"
#include <stdio.h>
#include <stdint.h>

#include "nand.h"
#include "nand-utils.h"

// 1st or 2nd page of every initial invalid block has non-FFh.
// Therefore, if a page(or a block) is valid, it must have a FFh value.
static void nand_oob_write_block_status(char *oob)
{
	oob[0] = 0xff;
}

static void nand_oob_write_ecc_code(char *oob, char *data)
{
	nand_hamming_compute256x((uint8_t *)data, NAND_PAGE_BYTE,
				 (uint8_t *)(oob + 1));
}

int nand_oob_setup(char *oob, char *data)
{
	nand_oob_write_block_status(oob);
	nand_oob_write_ecc_code(oob, data);
	(void)data;
	return 0;
}

#ifdef DEBUG
static void nand_oob_print(char *oob)
{
	for (int i = 0; i < NAND_PAGE_OOB_BYTE; i++) {
		if (!(i % (1 << 8))) {
			printf("\n");
		}
		if (!(i % ((1 << 10)))) {
			printf("\n");
		}
		printf("%02x ", oob[i]);
	}
	printf("\n");
}
#endif

static int nand_oob_is_valid_block(char *oob)
{
	return oob[0] == 0xff;
}

static int nand_oob_is_valid_ecc(char *oob, char *data)
{
	return nand_hamming_verify256x((uint8_t *)data, NAND_PAGE_BYTE,
				       (uint8_t *)(oob + 1));
}

int nand_oob_verify(char *oob, char *data)
{
	int status_code;
#ifdef DEBUG
	nand_oob_print(oob);
#endif
	if (!nand_oob_is_valid_block(oob)) {
		return -NAND_OOB_INVALID_BLOCK;
	}
	status_code = nand_oob_is_valid_ecc(oob, data);
	// single bit error will be corrected by parity
	if (status_code == NAND_OOB_ERROR_SINGLEBIT) {
		status_code = 0;
	}
	return -status_code;
}
