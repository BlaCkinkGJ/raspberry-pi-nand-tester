/*
 * Copyright (C) 2023 Gijun Oh
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef NAND_H
#define NAND_H

#include <wiringPi.h>
#include <time.h>

#define NAND_CLE (21)
#define NAME_ALE (22)
#define NAND_R_B (23)
#define NAND_WE (24)
#define NAND_RE (25)
#define NAND_CE (26)
#define NAND_WP (27)

#define NAND_DEFAULT_DELAY_NS (35)
#define NAND_PAGE_BYTE (2048)
#define NAND_PAGE_OOB_BYTE (64)
#define NAND_PAGES_PER_BLOCK (64)

#define NAND_HAMMING_DATA_BYTE (256)
#define NAND_HAMMING_PARITY_BYTE (3)

#define NAND_OOB_ERROR_SINGLEBIT (1)
#define NAND_OOB_ERROR_ECC (2)
#define NAND_OOB_ERROR_MULTIPLEBITS (3)
#define NAND_OOB_INVALID_BLOCK (4)

// #define NAND_ENABLE_RESET

int nand_init(void);
int nand_read(char *data, int block, int page);
int nand_write(char *data, int block, int page);
int nand_erase(int block);
void nand_free(void);

static const char *nand_read_errmsg[] = {
	"No Error",
	"A single bit is corrupted (automatically fixed by ecc)",
	"The ECC is corrupted",
	"Multiple bits are corrupted",
	"Invalid block is detected",
};

static inline const char *nand_get_read_error_msg(int status_code)
{
	return nand_read_errmsg[-status_code];
}

#endif
