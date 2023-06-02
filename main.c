#include "nand.h"

int main(void)
{
	int ret = 0;

	ret = nand_init();
	if (ret) {
		goto out;
	}
	nand_info_print();

out:
	nand_free();
	return 0;
}
