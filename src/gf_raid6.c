#include "gf_raid6.h"

unsigned char gf_table[256][256];
unsigned char inv2, inv3;
unsigned char precomputed_inv_coeff[8] = {0};

void init_gf_table() {
	for (int a = 0; a < 256; a++) {
		for (int b = 0; b < 256; b++) {
			unsigned char p = 0;
			unsigned char a_val = a;
			unsigned char b_val = b;

			for (int i = 0; i < 8; i++) {
				if (b_val & 1) {
					p ^= a_val;
				}
				unsigned char hi_bit = a_val & 0x80;
				a_val <<= 1;

				if (hi_bit) {
					a_val ^= 0x1D;
				}
				b_val >>= 1;
			}
			gf_table[a][b] = p;
		}
	}
}

unsigned char gf_mult(unsigned char a, unsigned char b) {
	return gf_table[a][b];
}

void init_inverses() {
	for (int i = 0; i < 256; i++) {
		if (gf_mult(2, i) == 1) {
			inv2 = i;
		}

		if (gf_mult(3, i) == 1) {
			inv3 = i;
		}
	}

	for (int coeff = 1; coeff < 8; coeff++) {
		for (int i = 0; i < 256; i++) {
			if (gf_mult(coeff, i) == 1) {
				precomputed_inv_coeff[coeff] = i;
				break;
			}
		}
	}
}

void four_drive_recovery(unsigned char** block1, unsigned char** block2,
                         const unsigned char* p_block,
                         const unsigned char* q_block, size_t block_size) {
	if (*block1 && *block2) {
		return;
	}

	if (!*block1 && *block2 && p_block) {
		*block1 = (unsigned char*)malloc(block_size);

		for (size_t i = 0; i < block_size; i++) {
			(*block1)[i] = p_block[i] ^ (*block2)[i];
		}

		return;
	}

	if (*block1 && !*block2 && p_block) {
		*block2 = (unsigned char*)malloc(block_size);

		for (size_t i = 0; i < block_size; i++) {
			(*block2)[i] = p_block[i] ^ (*block1)[i];
		}

		return;
	}

	if (!*block1 && !*block2 && p_block && q_block) {
		*block1 = (unsigned char*)malloc(block_size);
		*block2 = (unsigned char*)malloc(block_size);

		for (size_t i = 0; i < block_size; i++) {
			unsigned char p = p_block[i];
			unsigned char q = q_block[i];
			unsigned char d2 = gf_mult(inv3, p ^ q);
			unsigned char d1 = p ^ d2;
			(*block1)[i] = d1;
			(*block2)[i] = d2;
		}

		return;
	}

	if (!*block1 && *block2 && q_block) {
		*block1 = (unsigned char*)malloc(block_size);

		for (size_t i = 0; i < block_size; i++) {
			(*block1)[i] = q_block[i] ^ gf_mult(2, (*block2)[i]);
		}

		return;
	}

	if (*block1 && !*block2 && q_block) {
		*block2 = (unsigned char*)malloc(block_size);

		for (size_t i = 0; i < block_size; i++) {
			(*block2)[i] = gf_mult(inv2, q_block[i] ^ (*block1)[i]);
		}

		return;
	}
}

int missing_drives(unsigned char* blocks[], int count) {
	int missing = 0;

	for (int i = 0; i < count; i++) {
		if (!blocks[i]) {
			missing++;
		}
	}

	return missing;
}

void eight_drive_recovery(unsigned char** blocks, const unsigned char* p_block,
                          const unsigned char* q_block, size_t block_size) {

	int missing_count = missing_drives(blocks, 6);
	if (missing_count == 0){
		return;
	}

	if (missing_count == 1 && p_block) {
		for (int idx = 0; idx < 6; idx++) {
			if (!blocks[idx]) {
				blocks[idx] = (unsigned char*)malloc(block_size);
				for (size_t i = 0; i < block_size; i++) {
					unsigned char val = p_block[i];
					for (int j = 0; j < 6; j++) {
						if (j != idx && blocks[j]) {
							val ^= blocks[j][i];
						}
					}
					blocks[idx][i] = val;
				}

				return;
			}
		}
	}

	if (missing_count == 1 && q_block) {
		for (int idx = 0; idx < 6; idx++) {
			if (!blocks[idx]) {
				int coeff = idx + 1;
				unsigned char inv_coeff = precomputed_inv_coeff[coeff];
				blocks[idx] = (unsigned char*)malloc(block_size);

				for (size_t i = 0; i < block_size; i++) {
					unsigned char val = q_block[i];
					for (int j = 0; j < 6; j++) {
						if (j != idx && blocks[j]) {
							val ^= gf_mult(j + 1, blocks[j][i]);
						}
					}

					blocks[idx][i] = gf_mult(inv_coeff, val);
				}

				return;
			}
		}
	}

	if (missing_count == 2 && p_block && q_block) {
		int idx1 = -1, idx2 = -1;

		for (int i = 0; i < 6; i++) {
			if (!blocks[i]) {
				if (idx1 == -1)
					idx1 = i;
				else
					idx2 = i;
			}
		}

		int c1 = idx1 + 1;
		int c2 = idx2 + 1;
		unsigned char inv_coeff = precomputed_inv_coeff[c1 ^ c2];

		unsigned char* temp_blocks[6];
		memcpy(temp_blocks, blocks, sizeof(temp_blocks));

		for (int i = 0; i < 6; i++) {
			if (!temp_blocks[i]) {
				temp_blocks[i] = (unsigned char*)calloc(block_size, 1);
			}
		}

		unsigned char* not_P = (unsigned char*)calloc(block_size, 1);
		unsigned char* not_Q = (unsigned char*)calloc(block_size, 1);

		for (size_t i = 0; i < block_size; i++) {
			for (int j = 0; j < 6; j++) {
				not_P[i] ^= temp_blocks[j][i];
				not_Q[i] ^= gf_mult(j + 1, temp_blocks[j][i]);
			}
		}

		blocks[idx1] = (unsigned char*)malloc(block_size);
		blocks[idx2] = (unsigned char*)malloc(block_size);

		for (size_t i = 0; i < block_size; i++) {
			unsigned char A = p_block[i] ^ not_P[i];
			unsigned char B = q_block[i] ^ not_Q[i];
			unsigned char term = B ^ gf_mult(c2, A);
			unsigned char n_byte = gf_mult(inv_coeff, term);
			unsigned char m_byte = A ^ n_byte;
			blocks[idx1][i] = n_byte;
			blocks[idx2][i] = m_byte;
		}

		for (int i = 0; i < 6; i++) {
			if (temp_blocks[i] && temp_blocks[i] != blocks[i]) {
				free(temp_blocks[i]);
			}
		}

		free(not_P);
		free(not_Q);
	}
}
