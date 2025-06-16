#ifndef GF_RAID6_H
#define GF_RAID6_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern unsigned char gf_table[256][256];
extern unsigned char inv2, inv3;
extern unsigned char precomputed_inv_coeff[8];

void init_gf_table();
void init_inverses();
unsigned char gf_mult(unsigned char a, unsigned char b);

void four_drive_recovery(unsigned char** block1, unsigned char** block2,
                         const unsigned char* p_block,
                         const unsigned char* q_block, size_t block_size);

int missing_drives(unsigned char* blocks[], int count);

void eight_drive_recovery(unsigned char* blocks[6],
                          const unsigned char* p_block,
                          const unsigned char* q_block, size_t block_size);

#endif
