/*
 * Copyright (C) 2015 Lucas Jenß
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

// Last Modified: 2023-06-18, Gijun Oh

#ifndef NAND_ECC_HAMMING256_H
#define NAND_ECC_HAMMING256_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void nand_hamming_compute256x(const uint8_t *data, uint32_t size,
			      uint8_t *code);
uint8_t nand_hamming_verify256x(uint8_t *data, uint32_t size,
				const uint8_t *code);

#ifdef __cplusplus
}
#endif

#endif
