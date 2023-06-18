/*
 * Copyright (C) 2023 Gijun Oh
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef NAND_UTILS_H
#define NAND_UTILS_H

void nand_delay_ns(int ns);

void nand_write_pin_mode(void);
void nand_digital_write(int pin, int value);
void nand_digital_write_byte(volatile int command);
void nand_write_1_cycle(volatile int data);

void nand_read_pin_mode(void);
unsigned int nand_read_byte(void);
unsigned int nand_read_1_cycle(void);

void nand_send_command(volatile int command);
void nand_send_address_1_cycle(volatile int address);
void nand_send_address(volatile int row, volatile int col);
void nand_send_address_row(volatile int row);

void nand_enable_chip(void);
void nand_disable_chip(void);

void nand_wait_busy(void);

void nand_read_id(unsigned int cycles[5]);

void nand_pins_print(void);
void nand_page_print(char *data);
int nand_info_print(void);

void nand_status(void);
int nand_pass_fail(void);

// nand-oob.c
int nand_oob_setup(char *oob, char *data);
int nand_oob_verify(char *oob, char *data);

#endif
