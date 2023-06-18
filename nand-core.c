/*
 * Copyright (C) 2023 Gijun Oh
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */
#include <stdio.h>

#include "nand.h"
#include "nand-utils.h"

int nand_init(void)
{
	int i;

	if (wiringPiSetup() == -1) {
		return 1;
	}
	// configure the command pins
	for (i = 21; i < 28; i++) {
		if (i == NAND_R_B) {
			pinMode(i, INPUT);
		} else {
			pinMode(i, OUTPUT);
			if (i == NAND_CE || i == NAND_RE || i == NAND_WE ||
			    i == NAND_WP) {
				nand_digital_write(i, 1);
			} else {
				nand_digital_write(i, 0);
			}
		}
	}

	// configure the data pins
	for (i = 0; i < 8; i++) {
		pinMode(i, OUTPUT);
		nand_digital_write(i, 0);
	}

	nand_enable_chip();
#ifdef NAND_ENABLE_RESET
	// reset the chip
	nand_send_command(0xff);
	nand_wait_busy();
#endif
	return 0;
}

void nand_free(void)
{
	nand_disable_chip();
}
