#include <stdio.h>

#include "nand.h"

void nand_delay_ns(int ns)
{
	struct timespec req, rem;
	req.tv_sec = 0;
	req.tv_nsec = ns;
	nanosleep(&req, &rem);
}

void nand_write_pin_mode(void)
{
	int i;
	for (i = 0; i < 8; i++) {
		pinMode(i, OUTPUT);
	}
}

void nand_digital_write(int pin, int value)
{
	digitalWrite(pin, value);
	nand_delay_ns(NAND_DEFAULT_DELAY_NS);
}

void nand_digital_write_byte(volatile int command)
{
	digitalWriteByte(command);
}

void nand_write_1_cycle(volatile int data)
{
	nand_write_pin_mode();
	nand_digital_write_byte(data);
	nand_digital_write(NAND_WE, 0);
	nand_digital_write(NAND_WE, 1);
}

void nand_read_pin_mode(void)
{
	int i;
	for (i = 0; i < 8; i++) {
		pinMode(i, INPUT);
	}
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

unsigned int nand_read_byte(void)
{
	return nand_byte_reverse(digitalReadByte());
}

unsigned int nand_read_1_cycle(void)
{
	unsigned int result = 0x00;
	nand_read_pin_mode();
	nand_digital_write(NAND_RE, 0);
	result = nand_read_byte();
	nand_digital_write(NAND_RE, 1);
	return result;
}

void nand_send_command(volatile int command)
{
	nand_write_pin_mode();
	nand_digital_write(NAND_CLE, 1);
	nand_digital_write_byte(command);
	nand_digital_write(NAND_WE, 0);
	nand_digital_write(NAND_WE, 1);
	nand_digital_write(NAND_CLE, 0);
}

void nand_send_address_1_cycle(volatile int address)
{
	nand_write_pin_mode();
	nand_digital_write(NAME_ALE, 1);
	digitalWriteByte(address);
	nand_digital_write(NAND_WE, 0);
	nand_digital_write(NAND_WE, 1);
	nand_digital_write(NAME_ALE, 0);
}

void nand_send_address(volatile int row, volatile int col)
{
	int i;
	int addrs[4] = { col & 0xff, (col >> 8) & 0xff, row & 0xff,
			 (row >> 8) & 0xff };
	nand_write_pin_mode();
	nand_digital_write(NAME_ALE, 1);
	for (i = 0; i < 4; i++) {
		nand_digital_write_byte(addrs[i]);
		nand_digital_write(NAND_WE, 0);
		nand_digital_write(NAND_WE, 1);
	}
	nand_digital_write(NAME_ALE, 0);
}

void nand_send_address_row(volatile int row)
{
	int i;
	int addrs[2] = { row & 0xff, (row >> 8) & 0xff };
	nand_write_pin_mode();
	nand_digital_write(NAME_ALE, 1);
	for (i = 0; i < 2; i++) {
		nand_digital_write_byte(addrs[i]);
		nand_digital_write(NAND_WE, 0);
		nand_digital_write(NAND_WE, 1);
	}
	nand_digital_write(NAME_ALE, 0);
}

void nand_enable_chip(void)
{
	nand_digital_write(NAND_CE, 0);
}

void nand_disable_chip(void)
{
	nand_digital_write(NAND_CE, 0);
}

void nand_wait_busy(void)
{
#ifdef NAND_READ_STATUS_WAIT_ENABLE
	unsigned int status;
	do {
		nand_send_command(0x70);
		nand_delay_ns(65);
		status = nand_read_1_cycle();
	} while (0 == ((status << 6) & 0x1));
#endif
	while (0 == digitalRead(NAND_R_B))
		;
}

void nand_read_id(unsigned int cycles[5])
{
	int i;
	nand_send_command(0x90);
	nand_send_address_1_cycle(0x00);
	for (i = 0; i < 5; i++) {
		cycles[i] = nand_read_1_cycle();
	}
}

void nand_pins_print(void)
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

void nand_page_print(char *data)
{
	for (int i = 0; i < NAND_PAGE_BYTE; i++) {
		if (!(i % (1 << 8))) {
			printf("\n");
		}
		if (!(i % ((1 << 10)))) {
			printf("\n");
		}
		printf("%02x ", data[i]);
	}
	printf("\n");
}

int nand_info_print(void)
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
		return -1;
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
	return 0;
}

void nand_status(void)
{
	unsigned int status;
	nand_send_command(0x70);
	nand_delay_ns(65);
	status = nand_read_1_cycle();
	printf("==================== I/O STATUS ===================\n");
	printf("%-45s: %s\n", "Pass/Fail", (status & 0x1) ? "Fail" : "Success");
	printf("%-45s: %s(R_B:%d)\n", "Ready/Busy",
	       ((status << 6) & 0x1) ? "Ready" : "Busy", digitalRead(NAND_R_B));
	printf("%-45s: %s\n", "WriteProtected",
	       ((status << 7) & 0x1) ? "Protected" : "Not Protected");
	printf("===================================================\n");
}

int nand_pass_fail(void)
{
	unsigned int status;
	nand_send_command(0x70);
	nand_delay_ns(65);
	status = nand_read_1_cycle();
	return (status & 0x1);
}
