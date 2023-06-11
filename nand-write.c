#include "nand.h"
#include "nand-utils.h"
#include "rs.h"

int nand_write(char *data, int block, int page)
{
	int i = 0;
	int row, col;
	if (data == NULL) {
		return -1;
	}
	nand_send_command(0x80);
	row = (block * NAND_PAGES_PER_BLOCK) + page;
	col = 0x00;
	nand_send_address(row, col);
	nand_delay_ns(105);
	for (i = 0; i < NAND_PAGE_BYTE; i++) {
		nand_write_1_cycle((int)data[i]);
	}
	nand_send_command(0x10);
	nand_wait_busy();
	return nand_pass_fail();
}
