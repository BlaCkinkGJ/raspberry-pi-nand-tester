/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

// Last Modified: 2023-06-18, Gijun Oh

#include <stdint.h>
#include <stdio.h>

#include "nand-hamming256.h"
#include "nand.h"

static uint8_t nand_count_bits_in_code256(uint8_t *code)
{
	int ret = 0;
	ret += __builtin_popcount((int)code[0]);
	ret += __builtin_popcount((int)code[1]);
	ret += __builtin_popcount((int)code[2]);
	return (uint8_t)ret;
}

static void nand_compute256(const uint8_t *data, uint8_t *code, uint8_t padding)
{
	uint32_t i;
	uint8_t columnSum = 0;
	uint8_t evenLineCode = 0;
	uint8_t oddLineCode = 0;
	uint8_t evenColumnCode = 0;
	uint8_t oddColumnCode = 0;

	/*
     * Xor all bytes together to get the column sum;
     * At the same time, calculate the even and odd line codes
     */
	for (i = 0; i < 256; i++) {
		/* Allow non-multiples of 256 to be calculated by padding the data with zeroes */
		uint8_t current = 0;
		if (i < ((uint16_t)(256 - padding))) {
			current = data[i];
		}

		columnSum ^= current;

		/*
         * If the xor sum of the byte is 0, then this byte has no incidence on
         * the computed code; so check if the sum is 1.
         */
		if ((__builtin_popcount(current) & 1) == 1) {
			/*
             * Parity groups are formed by forcing a particular index bit to 0
             * (even) or 1 (odd).
             * Example on one byte:
             *
             * bits (dec)  7   6   5   4   3   2   1   0
             *      (bin) 111 110 101 100 011 010 001 000
             *                            '---'---'---'----------.
             *                                                   |
             * groups P4' ooooooooooooooo eeeeeeeeeeeeeee P4     |
             *        P2' ooooooo eeeeeee ooooooo eeeeeee P2     |
             *        P1' ooo eee ooo eee ooo eee ooo eee P1     |
             *                                                   |
             * We can see that:                                  |
             *  - P4  -> bit 2 of index is 0 --------------------'
             *  - P4' -> bit 2 of index is 1.
             *  - P2  -> bit 1 of index if 0.
             *  - etc...
             * We deduce that a bit position has an impact on all even Px if
             * the log2(x)nth bit of its index is 0
             *     ex: log2(4) = 2, bit2 of the index must be 0 (-> 0 1 2 3)
             * and on all odd Px' if the log2(x)nth bit of its index is 1
             *     ex: log2(2) = 1, bit1 of the index must be 1 (-> 0 1 4 5)
             *
             * As such, we calculate all the possible Px and Px' values at the
             * same time in two variables, evenLineCode and oddLineCode, such as
             *     evenLineCode bits: P128  P64  P32  P16  P8  P4  P2  P1
             *     oddLineCode  bits: P128' P64' P32' P16' P8' P4' P2' P1'
             */
			evenLineCode ^= (uint8_t)(255 - i);
			oddLineCode ^= (uint8_t)i;
		}
	}

	/*
     * At this point, we have the line parities, and the column sum. First, We
     * must calculate the parity group values on the column sum.
     */
	for (i = 0; i < 8; i++) {
		if (columnSum & 1) {
			evenColumnCode ^= (uint8_t)(7 - i);
			oddColumnCode ^= (uint8_t)i;
		}
		columnSum >>= 1;
	}

	/*
     * Now, we must interleave the parity values, to obtain the following layout:
     * Code[0] = Line1
     * Code[1] = Line2
     * Code[2] = Column
     * Line = Px' Px P(x-1)- P(x-1) ...
     * Column = P4' P4 P2' P2 P1' P1 PadBit PadBit
     */
	code[0] = 0;
	code[1] = 0;
	code[2] = 0;

	for (i = 0; i < 4; i++) {
		code[0] <<= 2;
		code[1] <<= 2;
		code[2] <<= 2;

		/* Line 1 */
		if ((oddLineCode & 0x80) != 0) {
			code[0] |= 2;
		}

		if ((evenLineCode & 0x80) != 0) {
			code[0] |= 1;
		}

		/* Line 2 */
		if ((oddLineCode & 0x08) != 0) {
			code[1] |= 2;
		}

		if ((evenLineCode & 0x08) != 0) {
			code[1] |= 1;
		}

		/* Column */
		if ((oddColumnCode & 0x04) != 0) {
			code[2] |= 2;
		}

		if ((evenColumnCode & 0x04) != 0) {
			code[2] |= 1;
		}

		oddLineCode <<= 1;
		evenLineCode <<= 1;
		oddColumnCode <<= 1;
		evenColumnCode <<= 1;
	}

	/* Invert codes (linux compatibility) */
	code[0] = (~(uint32_t)code[0]);
	code[1] = (~(uint32_t)code[1]);
	code[2] = (~(uint32_t)code[2]);
}

uint8_t nand_verify256(uint8_t *pucData, const uint8_t *pucOriginalCode,
		       uint8_t padding)
{
	/* Calculate new code */
	uint8_t computedCode[3];
	uint8_t correctionCode[3];

	nand_compute256(pucData, computedCode, padding);

	/* Xor both codes together */
	correctionCode[0] = computedCode[0] ^ pucOriginalCode[0];
	correctionCode[1] = computedCode[1] ^ pucOriginalCode[1];
	correctionCode[2] = computedCode[2] ^ pucOriginalCode[2];

	/* If all bytes are 0, there is no error */
	if ((correctionCode[0] == 0) && (correctionCode[1] == 0) &&
	    (correctionCode[2] == 0)) {
		return 0;
	}

	/* If there is a single bit error, there are 11 bits set to 1 */
	if (nand_count_bits_in_code256(correctionCode) == 11) {
		/* Get byte and bit indexes */
		int byte;
		int bit;

		byte = correctionCode[0] & 0x80;
		byte |= (correctionCode[0] << 1) & 0x40;
		byte |= (correctionCode[0] << 2) & 0x20;
		byte |= (correctionCode[0] << 3) & 0x10;

		byte |= (correctionCode[1] >> 4) & 0x08;
		byte |= (correctionCode[1] >> 3) & 0x04;
		byte |= (correctionCode[1] >> 2) & 0x02;
		byte |= (correctionCode[1] >> 1) & 0x01;

		bit = (correctionCode[2] >> 5) & 0x04;
		bit |= (correctionCode[2] >> 4) & 0x02;
		bit |= (correctionCode[2] >> 3) & 0x01;

		/* Correct bit */
		pucData[byte] ^= (uint8_t)(1 << bit);

		return NAND_OOB_ERROR_SINGLEBIT;
	}

	/* Check if ECC has been corrupted */
	if (nand_count_bits_in_code256(correctionCode) == 1) {
		return NAND_OOB_ERROR_ECC;
	}
	/* Otherwise, this is a multi-bit error */
	else {
		return NAND_OOB_ERROR_MULTIPLEBITS;
	}
}

void nand_hamming_compute256x(const uint8_t *pucData, uint32_t dwSize,
			      uint8_t *puCode)
{
	while (dwSize > 0) {
		uint8_t padding = 0;
		if (dwSize < 256) {
			padding = (uint8_t)(256 - dwSize);
		}

		nand_compute256(pucData, puCode, padding);

		pucData += 256;
		puCode += 3;
		dwSize -= (uint32_t)(256 - padding);
	}
}

uint8_t nand_hamming_verify256x(uint8_t *pucData, uint32_t dwSize,
				const uint8_t *pucCode)
{
	uint8_t result = 0;

	while (dwSize > 0) {
		uint8_t error, padding = 0;
		if (dwSize < 256) {
			padding = (uint8_t)(256 - dwSize);
		}
		error = nand_verify256(pucData, pucCode, padding);

		if (error == NAND_OOB_ERROR_SINGLEBIT) {
			result = NAND_OOB_ERROR_SINGLEBIT;
		} else {
			if (error) {
				return error;
			}
		}

		pucData += 256;
		pucCode += 3;
		dwSize -= (256 - padding);
	}

	return result;
}
