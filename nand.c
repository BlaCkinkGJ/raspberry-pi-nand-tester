#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <wiringPi.h>
#include <time.h>
#include "nand.h"

void nand_pins_print()
{
	int i;
	for (i = 0; i < 8; i++) {
		printf("D%d=%d ", i, digitalRead(i));
	}
	printf("NAND_CLE=%d ", digitalRead(NAND_CLE));
	printf("NAME_ALE=%d ", digitalRead(NAME_ALE));
	printf("NAND_R_B=%d ", digitalRead(NAND_R_B));
	printf("NAND_WE=%d ", digitalRead(NAND_WE));
	printf("NAND_RE=%d ", digitalRead(NAND_RE));
	printf("NAND_CE=%d ", digitalRead(NAND_CE));
	printf("NAND_WP=%d \n", digitalRead(NAND_WP));
}

static void nand_delay_ns(int ns)
{
	struct timespec req, rem;
	req.tv_sec = 0;
	req.tv_nsec = ns;
	nanosleep(&req, &rem);
}

static void nand_write_pin_mode(void)
{
	int i;
	for (i = 0; i < 8; i++) {
		pinMode(i, OUTPUT);
	}
}

static void nand_read_pin_mode(void)
{
	int i;
	for (i = 0; i < 8; i++) {
		pinMode(i, INPUT);
	}
}

static void nand_digital_write(int pin, int value)
{
	digitalWrite(pin, value);
	nand_delay_ns(NAND_DEFAULT_DELAY_NS);
}

static void nand_digital_write_byte(volatile int command)
{
	digitalWriteByte(command);
}

static void nand_send_command(volatile int command)
{
	nand_write_pin_mode();
	nand_digital_write(NAND_CLE, 1);
	nand_digital_write_byte(command);
	nand_digital_write(NAND_WE, 0);
	nand_digital_write(NAND_WE, 1);
	nand_digital_write(NAND_CLE, 0);
}

static void nand_send_address_1_cycle(volatile int address)
{
	nand_write_pin_mode();
	nand_digital_write(NAME_ALE, 1);
	digitalWriteByte(address);
	nand_digital_write(NAND_WE, 0);
	nand_digital_write(NAND_WE, 1);
	nand_digital_write(NAME_ALE, 0);
}

static unsigned int nand_lookup[16] = {
	0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
	0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf,
};

static unsigned int nand_byte_reverse(unsigned int n)
{
	// Reverse the top and bottom nibble then swap them.
	return (nand_lookup[n & 0b1111] << 4) | nand_lookup[n >> 4];
}

static unsigned int nand_read_byte()
{
	return nand_byte_reverse(digitalReadByte());
}

static unsigned int nand_read()
{
	unsigned int result = 0x00;
	nand_read_pin_mode();
	nand_digital_write(NAND_RE, 0);
	result = nand_read_byte();
	nand_digital_write(NAND_RE, 1);
	return result;
}

static void nand_enable_chip()
{
	nand_digital_write(NAND_CE, 0);
}

static void nand_wait_busy()
{
	while (0 == digitalRead(NAND_R_B))
		;
}

int nand_init()
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

	// reset the chip
	nand_enable_chip();
	nand_send_command(0xff);
	nand_wait_busy();
	return 0;
}

static void nand_read_id(unsigned int cycles[5])
{
	int i;
	nand_send_command(0x90);
	nand_send_address_1_cycle(0x00);
	for (i = 0; i < 5; i++) {
		cycles[i] = nand_read();
	}
}

void nand_info_print(void)
{
	const char *serial_access_minimums[] = { "50ns/30ns", "25ns",
						 "reserved", "reserved" };
	const char *processes[] = { "reserved", "21nm", "reserved",
				    "reserved" };
	unsigned int cycles[5];
	nand_read_id(cycles);
	// 1st cycle
	if (cycles[0] != 0xec) {
		printf("fail to read the marker (expect: 0xec, actual:0x%x)\n",
		       cycles[0]);
		return;
	}
	printf("==================== NAND INFO ====================\n");
	// 2st cycle
	printf("%-45s: 0x%x\n", "device code", cycles[1]);

	// 3nd cycle
	printf("%-45s: %d\n", "internal chip number", 1 << (cycles[2] & 0x3));
	printf("%-45s: %d\n", "cell type (level)",
	       1 << (1 + ((cycles[2] >> 2) & 0x3)));
	printf("%-45s: %d\n", "number of simultaneously programmed pages",
	       1 << ((cycles[2] >> 4) & 0x3));
	printf("%-45s: %d\n", "interleave program between multiple chips",
	       (cycles[2] >> 6) & 0x1);
	printf("%-45s: %d\n", "cache program", (cycles[2] >> 7) & 0x1);

	// 4th cycle
	printf("%-45s: %d\n", "page size (KB)", 1 << (cycles[3] & 0x3));
	printf("%-45s: %d\n", "block size (KB)",
	       1 << (6 + ((cycles[3] >> 4) & 0x3)));
	printf("%-45s: %d\n", "redundant area size",
	       1 << (3 + ((cycles[3] >> 2) & 0x1)));
	printf("%-45s: x%d\n", "organization",
	       1 << (3 + ((cycles[3] >> 6) & 0x1)));
	printf("%-45s: %s\n", "serial access minimum",
	       serial_access_minimums[(((cycles[3] >> 7) & 0x1) << 1) &
				      ((cycles[3] >> 3) & 0x1)]);

	// 5th cycle
	printf("%-45s: %d\n", "plane number", 1 << ((cycles[4] >> 2) & 0x3));
	printf("%-45s: %d\n", "plane size(Mb)",
	       1 << (6 + ((cycles[4] >> 4) & 0x7)));
	printf("%-45s: %s\n", "process", processes[cycles[4] & 0x3]);
	printf("===================================================\n");
}

static void nand_disable_chip()
{
	nand_digital_write(NAND_CE, 0);
}

void nand_free()
{
	nand_disable_chip();
}
