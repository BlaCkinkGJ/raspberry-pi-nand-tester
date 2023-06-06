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
#define NAND_PAGE_BYTE (2048 + 64)
#define NAND_PAGES_PER_BLOCK (64)

// #define NAND_ENABLE_RESET

int nand_init(void);
int nand_read(char *data, int block, int page);
int nand_write(char *data, int block, int page);
int nand_erase(int block);
void nand_free(void);

#endif
