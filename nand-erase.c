#include "nand.h"
#include "nand-utils.h"

int nand_erase(int block)
{
	nand_send_command(0x60);
	nand_send_address_row(block * NAND_PAGES_PER_BLOCK);
	nand_send_command(0xD0);
	nand_wait_busy();
	return nand_pass_fail();
}
