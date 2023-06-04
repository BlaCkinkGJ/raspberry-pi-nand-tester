#ifndef NAND_H
#define NAND_H

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

int nand_init();
int nand_read();
int nand_write();
int nand_erase(int block);
void nand_free();

void nand_status();

void nand_pins_print();
void nand_info_print();
void nand_page_print(char *data);

#endif
